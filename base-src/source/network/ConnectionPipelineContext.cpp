#include "ConnectionPipelineContext.h"
#include "ConnectionPipeline.h"
#include "ConnectionInboundHandler.h"
#include "ConnectionOutboundHandler.h"


namespace uranus::network {
    ConnectionPipelineContext::ConnectionPipelineContext(ConnectionPipeline &pipeline, const size_t idx)
        : pipeline_(pipeline),
          index_(idx) {
    }

    ConnectionPipelineContext::~ConnectionPipelineContext() {
    }

    ConnectionPipeline &ConnectionPipelineContext::pipeline() const {
        return pipeline_;
    }

    Connection &ConnectionPipelineContext::getConnection() const {
        return pipeline_.getConnection();
    }

    void ConnectionPipelineContext::fireConnect() const {
        if (auto [idx, handler] = getNextInboundHandler(); handler) {
            ConnectionPipelineContext ctx(pipeline_, idx);
            handler->onConnect(ctx);
        }
    }

    void ConnectionPipelineContext::fireDisconnect() const {
        if (auto [idx, handler] = getNextInboundHandler(); handler) {
            ConnectionPipelineContext ctx(pipeline_, idx);
            handler->onDisconnect(ctx);
        }
    }

    void ConnectionPipelineContext::fireError(const error_code ec) const {
        if (auto [idx, handler] = getNextInboundHandler(); handler) {
            ConnectionPipelineContext ctx(pipeline_, idx);
            handler->onError(ctx, ec);
        }
    }

    void ConnectionPipelineContext::fireException(exception &e) const {
        if (auto [idx, handler] = getNextInboundHandler(); handler) {
            ConnectionPipelineContext ctx(pipeline_, idx);
            handler->onException(ctx, e);
        }
    }

    void ConnectionPipelineContext::fireTimeout() const {
        if (auto [idx, handler] = getNextInboundHandler(); handler) {
            ConnectionPipelineContext ctx(pipeline_, idx);
            handler->onTimeout(ctx);
        }
    }

    awaitable<void> ConnectionPipelineContext::fireReceive(MessageHandle &&msg) const {
        if (auto [idx, handler] = getNextInboundHandler(); handler) {
            ConnectionPipelineContext ctx(pipeline_, idx);
            co_await handler->onReceive(ctx, std::move(msg));
        }
    }

    awaitable<void> ConnectionPipelineContext::fireBeforeSend(Message *msg) const {
        if (auto [idx, handler] = getPreviousOutboundHandler(); handler) {
            ConnectionPipelineContext ctx(pipeline_, idx);
            co_await handler->beforeSend(ctx, msg);
        }
    }

    awaitable<void> ConnectionPipelineContext::fireAfterSend(MessageHandle &&msg) const {
        if (auto [idx, handler] = getPreviousOutboundHandler(); handler) {
            ConnectionPipelineContext ctx(pipeline_, idx);
            co_await handler->afterSend(ctx, std::move(msg));
        }
    }

    tuple<size_t, ConnectionInboundHandler *> ConnectionPipelineContext::getNextInboundHandler() const {
        // Current is the last one
        if (index_ >= pipeline_.handlers_.size())
            return make_tuple(0, nullptr);

        for (size_t idx = index_; idx < pipeline_.handlers_.size(); ++idx) {
            if (auto *temp = pipeline_.handlers_[idx].get(); temp->type() == ConnectionHandler::HandlerType::kInbound) {
                return make_tuple(idx + 1, dynamic_cast<ConnectionInboundHandler *>(temp));
            }
        }

        return make_tuple(0, nullptr);
    }

    tuple<size_t, ConnectionOutboundHandler *> ConnectionPipelineContext::getPreviousOutboundHandler() const {
        // Current is the head one
        if (index_ == 0)
            return make_tuple(0, nullptr);

        // Deal with while idx == 0
        for (size_t idx = index_; idx > 0; --idx) {
            const auto i = idx - 1;
            if (auto *temp = pipeline_.handlers_[i].get(); temp->type() == ConnectionHandler::HandlerType::kOutbound) {
                return make_tuple(i, dynamic_cast<ConnectionOutboundHandler *>(temp));
            }
        }

        return make_tuple(0, nullptr);
    }
}
