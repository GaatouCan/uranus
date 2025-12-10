#pragma once

#include "Message.h"

#include <asio/awaitable.hpp>

namespace uranus::network {

    class Connection;
    class ConnectionPipeline;
    class ConnectionInboundHandler;
    class ConnectionOutboundHandler;

    using asio::awaitable;
    using std::error_code;
    using std::exception;

    class BASE_API ConnectionPipelineContext final {

        friend class ConnectionPipeline;

        ConnectionPipelineContext(ConnectionPipeline &pipeline, size_t idx);

    public:
        ConnectionPipelineContext() = delete;
        ~ConnectionPipelineContext();

        [[nodiscard]] ConnectionPipeline &pipeline() const;
        [[nodiscard]] Connection &getConnection() const;

        void fireConnect() const;
        void fireDisconnect() const;
        void fireError(error_code ec) const;
        void fireException(exception &e) const;
        void fireTimeout() const;

        awaitable<void> fireReceive(MessageHandle &&msg) const;
        awaitable<void> fireBeforeSend(Message *msg) const;
        awaitable<void> fireAfterSend(MessageHandle &&msg) const;

    private:
        ConnectionPipeline &pipeline_;
        const size_t index_;
    };
}