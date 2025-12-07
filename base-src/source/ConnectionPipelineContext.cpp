#include "ConnectionPipelineContext.h"
#include "ConnectionPipeline.h"
#include "ConnectionInboundHandler.h"
#include "ConnectionOutboundHandler.h"

namespace uranus::network {
    ConnectionPipelineContext::ConnectionPipelineContext(ConnectionPipeline *pipeline, const size_t idx)
        : pipeline_(pipeline),
          index_(idx) {
    }

    ConnectionPipelineContext::~ConnectionPipelineContext() = default;

    ConnectionPipelineContext::ConnectionPipelineContext(const ConnectionPipelineContext &rhs) {
        if (this != &rhs) {
            pipeline_ = rhs.pipeline_;
            index_ = rhs.index_;
        }
    }

    ConnectionPipelineContext &ConnectionPipelineContext::operator=(const ConnectionPipelineContext &rhs) {
        if (this != &rhs) {
            pipeline_ = rhs.pipeline_;
            index_ = rhs.index_;
        }
        return *this;
    }

    ConnectionPipelineContext::ConnectionPipelineContext(ConnectionPipelineContext &&rhs) noexcept {
        if (this != &rhs) {
            pipeline_ = rhs.pipeline_;
            index_ = rhs.index_;
            rhs.pipeline_ = nullptr;
            rhs.index_ = 0;
        }
    }

    ConnectionPipelineContext &ConnectionPipelineContext::operator=(ConnectionPipelineContext &&rhs) noexcept {
        if (this != &rhs) {
            pipeline_ = rhs.pipeline_;
            index_ = rhs.index_;
            rhs.pipeline_ = nullptr;
            rhs.index_ = 0;
        }
        return *this;
    }

    void ConnectionPipelineContext::fireConnect() const {
        auto [idx, handler] = getNextInboundHandler();
        if (handler == nullptr)
            return;

        handler->onConnect(ConnectionPipelineContext(pipeline_, idx));
    }

    void ConnectionPipelineContext::fireDisconnect() const {
        auto [idx, handler] = getNextInboundHandler();
        if (handler == nullptr)
            return;

        handler->onDisconnect(ConnectionPipelineContext(pipeline_, idx));
    }

    awaitable<void> ConnectionPipelineContext::fireReceive(MessageHandle &&msg) const {
        auto [idx, handler] = getNextInboundHandler();
        if (handler == nullptr)
            co_return;

        co_await handler->onReceive(ConnectionPipelineContext(pipeline_, idx), std::move(msg));
    }

    awaitable<MessageHandle> ConnectionPipelineContext::fireBeforeSend(MessageHandle &&msg) const {
        auto [idx, handler] = getPreviousOutboundHandler();
        if (handler == nullptr)
            co_return std::move(msg);

        auto output = co_await handler->beforeSend(ConnectionPipelineContext(pipeline_, idx), std::move(msg));
        co_return std::move(output);
    }

    awaitable<void> ConnectionPipelineContext::fireAfterSend(MessageHandle &&msg) const {
        auto [idx, handler] = getPreviousOutboundHandler();
        if (handler == nullptr)
            co_return;

        co_await handler->afterSend(ConnectionPipelineContext(pipeline_, idx), std::move(msg));
    }

    void ConnectionPipelineContext::fireError(std::error_code ec) const {
        auto [idx, handler] = getNextInboundHandler();
        if (handler == nullptr)
            return;

        handler->onError(ConnectionPipelineContext(pipeline_, idx), ec);
    }

    void ConnectionPipelineContext::fireException(const std::exception &e) const {
        auto [idx, handler] = getNextInboundHandler();
        if (handler == nullptr)
            return;

        handler->onException(ConnectionPipelineContext(pipeline_, idx), e);
    }

    tuple<size_t, ConnectionInboundHandler *> ConnectionPipelineContext::getNextInboundHandler() const {
        // Current is the last one
        if (index_ + 1 >= pipeline_->handlers_.size())
            return make_tuple(0, nullptr);

        for (size_t idx = index_ + 1; idx < pipeline_->handlers_.size(); ++idx) {
            if (auto *temp = pipeline_->handlers_[idx].get(); temp->type() == ConnectionHandler::Type::kInbound) {
                return make_tuple(idx, dynamic_cast<ConnectionInboundHandler *>(temp));
            }
        }

        return make_tuple(0, nullptr);
    }

    tuple<size_t, ConnectionOutboundHandler *> ConnectionPipelineContext::getPreviousOutboundHandler() const {
        // Current is the head one
        if (index_ == 0)
            return make_tuple(0, nullptr);

        for (size_t idx = index_ - 1; idx > 0; --idx) {
            if (auto *temp = pipeline_->handlers_[idx].get(); temp->type() == ConnectionHandler::Type::kOutbound) {
                return make_tuple(idx, dynamic_cast<ConnectionOutboundHandler *>(temp));
            }
        }

        return make_tuple(0, nullptr);
    }
}