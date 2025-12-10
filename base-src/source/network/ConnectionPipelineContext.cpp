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
        if (index_ >= pipeline_.inbounds_.size())
            return;

        ConnectionPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.inbounds_[index_]->onConnect(ctx);
    }

    void ConnectionPipelineContext::fireDisconnect() const {
        if (index_ >= pipeline_.inbounds_.size())
            return;

        ConnectionPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.inbounds_[index_]->onDisconnect(ctx);
    }

    void ConnectionPipelineContext::fireError(const error_code ec) const {
        if (index_ >= pipeline_.inbounds_.size())
            return;

        ConnectionPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.inbounds_[index_]->onError(ctx, ec);
    }

    void ConnectionPipelineContext::fireException(exception &e) const {
        if (index_ >= pipeline_.inbounds_.size())
            return;

        ConnectionPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.inbounds_[index_]->onException(ctx, e);
    }

    void ConnectionPipelineContext::fireTimeout() const {
        if (index_ >= pipeline_.inbounds_.size())
            return;

        ConnectionPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.inbounds_[index_]->onTimeout(ctx);
    }

    awaitable<void> ConnectionPipelineContext::fireReceive(MessageHandle &&msg) const {
        if (index_ >= pipeline_.inbounds_.size())
            co_return;

        ConnectionPipelineContext ctx(pipeline_, index_ + 1);
        co_await pipeline_.inbounds_[index_]->onReceive(ctx, std::move(msg));
    }

    awaitable<void> ConnectionPipelineContext::fireBeforeSend(Message *msg) const {
        if (index_ >= pipeline_.outbounds_.size())
            co_return;

        ConnectionPipelineContext ctx(pipeline_, index_ + 1);
        co_await pipeline_.outbounds_[index_]->beforeSend(ctx, msg);
    }

    awaitable<void> ConnectionPipelineContext::fireAfterSend(MessageHandle &&msg) const {
        if (index_ >= pipeline_.outbounds_.size())
            co_return;

        ConnectionPipelineContext ctx(pipeline_, index_ + 1);
        co_await pipeline_.outbounds_[index_]->afterSend(ctx, std::move(msg));
    }
}
