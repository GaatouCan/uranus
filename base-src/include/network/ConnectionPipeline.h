#pragma once

#include "Message.h"
#include "noncopy.h"

#include <memory>
#include <vector>
#include <tuple>
#include <asio/awaitable.hpp>

namespace uranus::network {

    using std::vector;
    using std::tuple;
    using std::unique_ptr;
    using std::make_tuple;
    using std::make_unique;
    using asio::awaitable;
    using std::exception;
    using std::error_code;

    class Connection;
    class ConnectionHandler;
    class ConnectionInboundHandler;
    class ConnectionOutboundHandler;

    class BASE_API ConnectionPipeline final {

        friend class ConnectionPipelineContext;

    public:
        ConnectionPipeline() = delete;

        explicit ConnectionPipeline(Connection &conn);
        ~ConnectionPipeline();

        DISABLE_COPY_MOVE(ConnectionPipeline)

        [[nodiscard]] Connection &getConnection() const;

        void onConnect();
        void onDisconnect();

        awaitable<void> onReceive(MessageHandle &&msg);

        awaitable<MessageHandle> beforeSend(MessageHandle &&msg);
        awaitable<void> afterSend(MessageHandle &&msg);

        void onError(std::error_code ec);
        void onException(const std::exception &e);

    private:
        [[nodiscard]] tuple<size_t, ConnectionInboundHandler *> getNextInboundHandler() const;
        [[nodiscard]] tuple<size_t, ConnectionOutboundHandler *> getPreviousOutboundHandler() const;

    private:
        Connection &conn_;
        vector<unique_ptr<ConnectionHandler>> handlers_;
    };
}
