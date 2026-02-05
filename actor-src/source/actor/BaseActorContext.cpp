#include "BaseActorContext.h"
#include "BaseActor.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <asio/detached.hpp>
#include <asio/co_spawn.hpp>
#include <ranges>


namespace uranus::actor {

    using namespace asio::experimental::awaitable_operators;
    using asio::co_spawn;
    using asio::detached;

    BaseActorContext::BaseActorContext(asio::any_io_executor ctx, ActorHandle &&actor)
        : exec_(std::move(ctx)),
          handle_(std::move(actor)),
          mailbox_(exec_, 1024),
          ticker_(exec_),
          sessionManager_(*this),
          timerManager_(*this) {
// #ifndef NDEBUG
//         assert(handle_ != nullptr);
// #endif
//         handle_->onInitial(this);
    }

    BaseActorContext::~BaseActorContext() {

    }

    AttributeMap &BaseActorContext::attr() {
        return attr_;
    }

    const AttributeMap &BaseActorContext::attr() const {
        return attr_;
    }

    void BaseActorContext::run(DataAssetHandle &&data) {
        if (terminated_.test(std::memory_order_acquire))
            return;

        if (running_.test_and_set(std::memory_order_acq_rel))
            return;

        handle_->onStart(data.get());

        co_spawn(exec_, [self = shared_from_this()]() mutable -> awaitable<void> {
            if (self->handle_->enableTick_) {
                co_await(
                  self->process() &&
                  self->tick()
                );
            } else {
                co_await self->process();
            }
        }, detached);
    }

    void BaseActorContext::terminate() {
        if (terminated_.test_and_set(std::memory_order_acq_rel))
            return;

        asio::dispatch(exec_, [self = shared_from_this()]() mutable {
            if (self->terminated_.test(std::memory_order_acquire))
                return;

            self->ticker_.cancel();
            self->mailbox_.cancel();
            self->mailbox_.close();

            self->sessionManager_.cancelAll();
            self->timerManager_.cancelAll();
        });
    }

    bool BaseActorContext::isInitial() const {
        return !running_.test()
            && !terminated_.test();
    }

    bool BaseActorContext::isTerminated() const {
        return terminated_.test();
    }

    asio::any_io_executor &BaseActorContext::executor() {
        return exec_;
    }

    bool BaseActorContext::isRunning() const {
        return mailbox_.is_open()
            && running_.test()
            && !terminated_.test();
    }

    BaseActor *BaseActorContext::getActor() const {
        if (handle_)
            return handle_.get();

        return nullptr;
    }

    void BaseActorContext::pushEnvelope(Envelope &&envelope) {
        if (!isRunning())
            return;

        mailbox_.try_send_via_dispatch(std::error_code{}, std::move(envelope));
    }

    RepeatedTimerHandle BaseActorContext::createTimer(
        const RepeatedTask &task,
        const SteadyDuration delay,
        const SteadyDuration rate
    ) {

        if (!isRunning())
            return {-1, nullptr};

        return timerManager_.createTimer(task, delay, rate);
    }

    RepeatedTimerHandle BaseActorContext::createTimer(
        const RepeatedTask &task,
        const SteadyTimePoint point,
        const SteadyDuration rate
    ) {
        if (!isRunning())
            return {-1, nullptr};

        return timerManager_.createTimerWithTimepoint(task, point, rate);
    }

    void BaseActorContext::cancelTimer(const RepeatedTimerHandle &handle) {
        TimerManager::cancelTimer(handle);
    }

    void BaseActorContext::createSession(
        const int ty,
        const int64_t target,
        PackageHandle &&req,
        SessionHandler &&handle
    ) {
        if (const auto sess = sessionManager_.pushSession(std::move(handle)); sess > 0) {
            this->sendRequest(ty, sess, target, std::move(req));
        }
    }

    void BaseActorContext::onErrorCode(std::error_code ec) {
    }

    void BaseActorContext::onException(std::exception &e) {
    }

    bool BaseActorContext::cleanUp() {
        if (!terminated_.test(std::memory_order_acquire))
            return false;

        return true;
    }

    awaitable<void> BaseActorContext::process() {
        try {
            while (isRunning()) {
                // 从邮箱中读取一条信息
                auto [ec, evl] = co_await mailbox_.async_receive();

                if (ec == asio::error::operation_aborted ||
                    ec == asio::experimental::error::channel_closed) {
                    break;
                }

                if (ec) {
                    this->onErrorCode(ec);
                    break;
                }

                if (!isRunning())
                    break;

                switch (evl.type) {
                    case Envelope::kPackage: {
                        if (auto *pkg = std::get_if<PackageHandle>(&evl.variant)) {
                            handle_->onRequest(evl.source, std::move(*pkg));
                        }
                    }
                    break;
                    case Envelope::kRequest: {
                        if (auto *pkg = std::get_if<PackageHandle>(&evl.variant)) {
                            const auto sess = evl.session;
                            const auto from = evl.source;

                            int type = 0;
                            if ((evl.type & Package::kFromPlayer) != 0) {
                                type = Package::kToPlayer;
                            }
                            if ((evl.type & Package::kFromService) != 0) {
                                type = Package::kToService;
                            }

                            auto res = handle_->onRequest(evl.source, std::move(*pkg));
                            this->sendResponse(type, sess, from, std::move(res));
                        }
                    }
                    break;
                    case Envelope::kResponse: {
                        if (auto *res = std::get_if<PackageHandle>(&evl.variant)) {
                            sessionManager_.dispatch(evl.session, std::move(*res));
                        }
                    }
                    break;
                    case Envelope::kDataAsset: {
                        if (const auto *da = std::get_if<DataAssetHandle>(&evl.variant)) {
                            handle_->onEvent(evl.event, da->get());
                        }
                    }
                    break;
                    case Envelope::kTickInfo: {
                        if (const auto *info = std::get_if<ActorTickInfo>(&evl.variant)) {
                            handle_->onTick(info->now, info->delta);
                        }
                    }
                    break;
                    case Envelope::kCallback: {
                        if (auto *task = std::get_if<ActorCallback>(&evl.variant)) {
                            std::invoke(*task, handle_.get());
                        }
                    }
                    break;
                    default: break;
                }
            }

            this->cleanUp();

            // Call actor terminate
            handle_->onTerminate();
        } catch (std::exception &e) {
            this->onException(e);
        }
    }

    awaitable<void> BaseActorContext::tick() {
        try {
            auto point = std::chrono::steady_clock::now();
            constexpr SteadyDuration kTickDelta = std::chrono::milliseconds(500);
            while (isRunning()) {
                point += kTickDelta;
                ticker_.expires_at(point);

                const auto [ec] = co_await ticker_.async_wait();
                if (ec) {
                    this->onErrorCode(ec);
                    break;
                }

                auto evl = Envelope::makeTickInfo(point, kTickDelta);
                this->pushEnvelope(std::move(evl));
            }
        } catch (std::exception &e) {
            this->onException(e);
        }
    }
}
