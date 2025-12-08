#pragma once

#include "Message.h"
#include "AttributeMap.h"

#include <tuple>
#include <asio/awaitable.hpp>

namespace uranus::network {

    // using asio::awaitable;
    // using std::tuple;
    //
    // class Connection;
    // class ConnectionPipeline;
    // class ConnectionInboundHandler;
    // class ConnectionOutboundHandler;
    //
    // class BASE_API ConnectionPipelineContext final {
    //
    // public:
    //     ConnectionPipelineContext() = delete;
    //
    //     ConnectionPipelineContext(ConnectionPipeline *pipeline, size_t idx);
    //     ~ConnectionPipelineContext();
    //
    //     ConnectionPipelineContext(const ConnectionPipelineContext &rhs);
    //     ConnectionPipelineContext &operator=(const ConnectionPipelineContext &rhs);
    //
    //     ConnectionPipelineContext(ConnectionPipelineContext &&rhs) noexcept;
    //     ConnectionPipelineContext &operator=(ConnectionPipelineContext &&rhs) noexcept;
    //
    //     [[nodiscard]] Connection &getConnection() const;
    //     [[nodiscard]] AttributeMap &attr() const;
    //
    //     void fireConnect() const;
    //     void fireDisconnect() const;
    //
    //     awaitable<void> fireReceive(MessageHandle &&msg) const;
    //
    //     awaitable<MessageHandle> fireBeforeSend(MessageHandle &&msg) const;
    //     awaitable<void> fireAfterSend(MessageHandle &&msg) const;
    //
    //     void fireError(std::error_code ec) const;
    //     void fireException(const std::exception &e) const;
    //
    // private:
    //     [[nodiscard]] tuple<size_t, ConnectionInboundHandler *> getNextInboundHandler() const;
    //     [[nodiscard]] tuple<size_t, ConnectionOutboundHandler *> getPreviousOutboundHandler() const;
    //
    // private:
    //     ConnectionPipeline *pipeline_;
    //     size_t index_;
    // };
}