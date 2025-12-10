#include "ConnectionPipeline.h"
#include "Connection.h"
#include "ConnectionInboundHandler.h"
#include "ConnectionOutboundHandler.h"
#include "ConnectionPipelineContext.h"

namespace uranus::network {
    ConnectionPipeline::ConnectionPipeline(Connection &conn)
        : conn_(conn) {
    }

    ConnectionPipeline::~ConnectionPipeline() {
    }

    ConnectionPipeline &ConnectionPipeline::pushBack(ConnectionHandler *handler) {
        // auto unique = unique_ptr<ConnectionHandler>(handler);
        handlers_.emplace_back(handler);
        if (handler->type() == ConnectionHandler::HandlerType::kInbound || handler->type() == ConnectionHandler::HandlerType::kDeluxe) {
            inbounds_.emplace_back(dynamic_cast<ConnectionInboundHandler *>(handler));
        }

        if (handler->type() == ConnectionHandler::HandlerType::kOutbound || handler->type() == ConnectionHandler::HandlerType::kDeluxe) {
            outbounds_.insert(outbounds_.begin(), dynamic_cast<ConnectionOutboundHandler *>(handler));
        }

        return *this;
    }

    Connection &ConnectionPipeline::getConnection() const {
        return conn_;
    }

    AttributeMap &ConnectionPipeline::attr() const {
        return conn_.attr();
    }

    void ConnectionPipeline::onConnect() {
        if (onConnect_) {
            ConnectionPipelineContext ctx(*this, 0);
            std::invoke(onConnect_, ctx);
        } else {
            if (auto [idx, handler] = getNextInboundHandler(); handler != nullptr) {
                ConnectionPipelineContext ctx(*this, idx);
                handler->onConnect(ctx);
            }
        }
    }

    void ConnectionPipeline::onDisconnect() {
        if (onDisconnect_) {
            ConnectionPipelineContext ctx(*this, 0);
            std::invoke(onDisconnect_, ctx);
        } else {
            if (auto [idx, handler] = getNextInboundHandler(); handler != nullptr) {
                ConnectionPipelineContext ctx(*this, idx);
                handler->onDisconnect(ctx);
            }
        }
    }

    void ConnectionPipeline::onError(error_code ec) {
        if (onError_) {
            ConnectionPipelineContext ctx(*this, 0);
            std::invoke(onError_, ctx, ec);
        } else if (!inbounds_.empty()) {
            ConnectionPipelineContext ctx(*this, 1);
            inbounds_.front()->onError(ctx, ec);
        }
    }

    void ConnectionPipeline::onException(exception &e) {
        if (onException_) {
            ConnectionPipelineContext ctx(*this, 0);
            std::invoke(onException_, ctx, e);
        } else {
            if (auto [idx, handler] = getNextInboundHandler(); handler != nullptr) {
                ConnectionPipelineContext ctx(*this, idx);
                handler->onException(ctx, e);
            }
        }
    }

    void ConnectionPipeline::onTimeout() {
        if (onTimeout_) {
            ConnectionPipelineContext ctx(*this, 0);
            std::invoke(onTimeout_, ctx);
        } else {
            if (auto [idx, handler] = getNextInboundHandler(); handler != nullptr) {
                ConnectionPipelineContext ctx(*this, idx);
                handler->onTimeout(ctx);
            }
        }
    }

    awaitable<void> ConnectionPipeline::onReceive(MessageHandle &&msg) {
        if (onReceive_) {
            ConnectionPipelineContext ctx(*this, 0);
            co_await std::invoke(onReceive_, ctx, std::move(msg));
        } else if (!inbounds_.empty()) {
            ConnectionPipelineContext ctx(*this, 1);
            co_await inbounds_.front()->onReceive(ctx, std::move(msg));
        }
    }

    awaitable<void> ConnectionPipeline::beforeSend(Message *msg) {
        if (beforeSend_) {
            ConnectionPipelineContext ctx(*this, handlers_.size());
            co_await std::invoke(beforeSend_, ctx, msg);
        } else {
            if (auto [idx, handler] = getPreviousOutboundHandler(); handler != nullptr) {
                ConnectionPipelineContext ctx(*this, idx);
                co_await handler->beforeSend(ctx, msg);
            }
        }
    }

    awaitable<void> ConnectionPipeline::afterSend(MessageHandle &&msg) {
        if (afterSend_) {
            ConnectionPipelineContext ctx(*this, handlers_.size());
            co_await std::invoke(afterSend_, ctx, std::move(msg));
        } else {
            if (auto [idx, handler] = getPreviousOutboundHandler(); handler != nullptr) {
                ConnectionPipelineContext ctx(*this, idx);
                co_await handler->afterSend(ctx, std::move(msg));
            }
        }
    }

    tuple<size_t, ConnectionInboundHandler *> ConnectionPipeline::getNextInboundHandler() const {
        if (handlers_.empty())
            return make_tuple(0, nullptr);

        for (size_t idx = 0; idx < handlers_.size(); ++idx) {
            if (auto *temp = handlers_[idx].get(); temp->type() == ConnectionHandler::HandlerType::kInbound) {
                return make_tuple(idx + 1, dynamic_cast<ConnectionInboundHandler *>(temp));
            }
        }

        return make_tuple(0, nullptr);
    }

    tuple<size_t, ConnectionOutboundHandler *> ConnectionPipeline::getPreviousOutboundHandler() const {
        if (handlers_.empty())
            return make_tuple(0, nullptr);

        // Deal with while idx == 0
        for (size_t idx = handlers_.size(); idx > 0; --idx) {
            const auto i = idx - 1;
            if (auto *temp = handlers_[i].get(); temp->type() == ConnectionHandler::HandlerType::kOutbound) {
                return make_tuple(i, dynamic_cast<ConnectionOutboundHandler *>(temp));
            }
        }

        return make_tuple(0, nullptr);
    }
}
