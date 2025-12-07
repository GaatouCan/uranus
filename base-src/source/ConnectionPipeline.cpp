#include "ConnectionPipeline.h"
#include "ConnectionInboundHandler.h"
#include "ConnectionOutboundHandler.h"

namespace uranus::network {
    ConnectionPipeline::ConnectionPipeline(Connection &conn)
        : conn_(conn) {
    }

    ConnectionPipeline::~ConnectionPipeline() {
    }

    Connection &ConnectionPipeline::getConnection() const {
        return conn_;
    }

    void ConnectionPipeline::onReceive(MessageHandle &&msg) {
        auto [idx, handler] = getNextInboundHandler();
        if (handler == nullptr)
            return;

        handler->onReceive(ConnectionPipelineContext(this, idx), std::move(msg));
    }

    tuple<size_t, ConnectionInboundHandler *> ConnectionPipeline::getNextInboundHandler() const {
        if (handlers_.empty())
            return make_tuple(0, nullptr);

        size_t idx = 0;
        ConnectionInboundHandler *handler = nullptr;

        for (; idx < handlers_.size(); ++idx) {
            if (auto *temp = handlers_[idx].get(); temp->type() == ConnectionHandler::Type::kInbound) {
                handler = dynamic_cast<ConnectionInboundHandler *>(temp);
                break;
            }
        }

        return make_tuple(idx, handler);
    }

    tuple<size_t, ConnectionOutboundHandler *> ConnectionPipeline::getPreviousOutboundHandler() const {
        if (handlers_.empty())
            return make_tuple(0, nullptr);

        size_t idx = handlers_.size() - 1;
        ConnectionOutboundHandler *handler = nullptr;

        for (; idx > 0; --idx) {
            if (auto *temp = handlers_[idx].get(); temp->type() == ConnectionHandler::Type::kOutbound) {
                handler = dynamic_cast<ConnectionOutboundHandler *>(temp);
            }
        }

        return make_tuple(idx, handler);
    }

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
}
