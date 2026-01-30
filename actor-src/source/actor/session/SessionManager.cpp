#include "session/SessionManager.h"
#include "session/RequestSession.h"
#include "BaseActorContext.h"

#include <asio/bind_allocator.hpp>
#include <ranges>


namespace uranus::actor {

    SessionManager::SessionManager(BaseActorContext &ctx)
        : ctx_(ctx) {
    }

    SessionManager::~SessionManager() {
    }

    int64_t SessionManager::pushSession(SessionHandler &&handler) {
        if (ctx_.isRunning()) {
            if (const auto id = alloc_.allocate(); !sessions_.contains(id)) {
                auto sess = make_shared<RequestSession>(ctx_.shared_from_this(), std::move(handler), id);
                sessions_.emplace(id, sess);
                return id;
            }
        }

        const auto work = asio::make_work_guard(handler);
        const auto alloc = asio::get_associated_allocator(handler, asio::recycling_allocator<void>());
        asio::dispatch(
            work.get_executor(),
            asio::bind_allocator(alloc, [handler = std::move(handler)]() mutable {
                std::move(handler)(nullptr);
            })
        );

        return -1;
    }

    void SessionManager::dispatch(const int64_t id, PackageHandle &&res) {
        const auto iter = sessions_.find(id);
        if (iter == sessions_.end())
            return;

        iter->second->dispatch(std::move(res));
    }

    void SessionManager::cancel(const int64_t id) {
        if (const auto iter = sessions_.find(id); iter != sessions_.end()) {
            iter->second->cancel();
            return;
        }

        sessions_.erase(id);
    }

    void SessionManager::cancelAll() {
        for (const auto &sess: sessions_ | std::views::values) {
            sess->cancel();
        }
        sessions_.clear();
    }

    void SessionManager::removeOnComplete(const int64_t id) {
        if (!ctx_.isRunning())
            return;

        sessions_.erase(id);
    }
}
