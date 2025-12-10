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

    ConnectionPipeline &ConnectionPipeline::pushBack(ConnectionHandler *handler, const function<void(ConnectionHandler *)> &del) {
        // Default deleter
        if (del == nullptr) {
            handlers_.emplace_back(handler, [](ConnectionHandler *h){ delete h; });
        } else {
            handlers_.emplace_back(handler, del);
        }

        if (handler->type() == ConnectionHandler::HandlerType::kInbound ||
            handler->type() == ConnectionHandler::HandlerType::kDeluxe) {
            inbounds_.emplace_back(dynamic_cast<ConnectionInboundHandler *>(handler));
        }

        if (handler->type() == ConnectionHandler::HandlerType::kOutbound ||
            handler->type() == ConnectionHandler::HandlerType::kDeluxe) {
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

    void ConnectionPipeline::setConnectCallback(const ConnectFunctor &cb) {
        onConnect_ = cb;
    }

    void ConnectionPipeline::setDisconnectCallback(const DisconnectFunctor &cb) {
        onDisconnect_ = cb;
    }

    void ConnectionPipeline::setErrorCallback(const ErrorFunctor &cb) {
        onError_ = cb;
    }

    void ConnectionPipeline::setExceptionCallback(const ExceptionFunctor &cb) {
        onException_ = cb;
    }

    void ConnectionPipeline::setTimeoutCallback(const TimeoutFunctor &cb) {
        onTimeout_ = cb;
    }

    void ConnectionPipeline::setReceiveCallback(const ReceiveFunctor &cb) {
        onReceive_ = cb;
    }

    void ConnectionPipeline::setBeforeSendCallback(const BeforeSendFunctor &cb) {
        beforeSend_ = cb;
    }

    void ConnectionPipeline::setAfterSendCallback(const AfterSendFunctor &cb) {
        afterSend_ = cb;
    }

    void ConnectionPipeline::onConnect() {
        if (onConnect_) {
            ConnectionPipelineContext ctx(*this, 0);
            std::invoke(onConnect_, ctx);
        } else if (!inbounds_.empty()) {
            ConnectionPipelineContext ctx(*this, 1);
            inbounds_.front()->onConnect(ctx);
        }
    }

    void ConnectionPipeline::onDisconnect() {
        if (onDisconnect_) {
            ConnectionPipelineContext ctx(*this, 0);
            std::invoke(onDisconnect_, ctx);
        } else if (!inbounds_.empty()) {
            ConnectionPipelineContext ctx(*this, 1);
            inbounds_.front()->onDisconnect(ctx);
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
        } else if (!inbounds_.empty()) {
            ConnectionPipelineContext ctx(*this, 1);
            inbounds_.front()->onException(ctx, e);
        }
    }

    void ConnectionPipeline::onTimeout() {
        if (onTimeout_) {
            ConnectionPipelineContext ctx(*this, 0);
            std::invoke(onTimeout_, ctx);
        } else if (!inbounds_.empty()) {
            ConnectionPipelineContext ctx(*this, 1);
            inbounds_.front()->onTimeout(ctx);
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
        co_return;
    }

    awaitable<void> ConnectionPipeline::beforeSend(Message *msg) {
        if (beforeSend_) {
            ConnectionPipelineContext ctx(*this, 0);
            co_await std::invoke(beforeSend_, ctx, msg);
        } else if (!outbounds_.empty()) {
            ConnectionPipelineContext ctx(*this, 1);
            co_await outbounds_.front()->beforeSend(ctx, msg);
        }
        co_return;
    }

    awaitable<void> ConnectionPipeline::afterSend(MessageHandle &&msg) {
        if (afterSend_) {
            ConnectionPipelineContext ctx(*this, 0);
            co_await std::invoke(afterSend_, ctx, std::move(msg));
        } else if (!outbounds_.empty()) {
            ConnectionPipelineContext ctx(*this, 1);
            co_await outbounds_.front()->afterSend(ctx, std::move(msg));
        }
        co_return;
    }
}
