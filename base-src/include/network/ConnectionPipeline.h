#pragma once

#include "base/Message.h"
#include "base/noncopy.h"

#include <vector>
#include <memory>
#include <functional>
#include <asio/awaitable.hpp>

namespace uranus {
    class AttributeMap;
}

namespace uranus::network {

    class Connection;
    class ConnectionPipelineContext;
    class ConnectionHandler;
    class ConnectionInboundHandler;
    class ConnectionOutboundHandler;

    using std::error_code;
    using std::exception;
    using std::unique_ptr;
    using std::make_unique;
    using std::vector;
    using std::function;
    using asio::awaitable;

    class BASE_API ConnectionPipeline final {

        friend class ConnectionPipelineContext;

        using HandlerUnique = unique_ptr<ConnectionHandler, function<void(ConnectionHandler *)>>;

        using ConnectFunctor        = function<void(ConnectionPipelineContext &)>;
        using DisconnectFunctor     = function<void(ConnectionPipelineContext &)>;
        using ErrorFunctor          = function<void(ConnectionPipelineContext &, error_code)>;
        using ExceptionFunctor      = function<void(ConnectionPipelineContext &, exception &)>;
        using TimeoutFunctor        = function<void(ConnectionPipelineContext &)>;

        using ReceiveFunctor        = function<awaitable<void>(ConnectionPipelineContext &, MessageHandle &&)>;
        using BeforeSendFunctor     = function<awaitable<void>(ConnectionPipelineContext &, Message *)>;
        using AfterSendFunctor      = function<awaitable<void>(ConnectionPipelineContext &, MessageHandle &&)>;

    public:
        ConnectionPipeline() = delete;

        explicit ConnectionPipeline(Connection &conn);
        ~ConnectionPipeline();

        DISABLE_COPY_MOVE(ConnectionPipeline)

        ConnectionPipeline &pushBack(ConnectionHandler *handler, const function<void(ConnectionHandler *)> &del = nullptr);

        [[nodiscard]] Connection &getConnection() const;
        [[nodiscard]] AttributeMap &attr() const;

        void setConnectCallback     (const ConnectFunctor       &cb);
        void setDisconnectCallback  (const DisconnectFunctor    &cb);
        void setErrorCallback       (const ErrorFunctor         &cb);
        void setExceptionCallback   (const ExceptionFunctor     &cb);
        void setTimeoutCallback     (const TimeoutFunctor       &cb);
        void setReceiveCallback     (const ReceiveFunctor       &cb);
        void setBeforeSendCallback  (const BeforeSendFunctor    &cb);
        void setAfterSendCallback   (const AfterSendFunctor     &cb);

        void onConnect      ();
        void onDisconnect   ();
        void onError        (error_code ec);
        void onException    (exception &e);
        void onTimeout      ();

        awaitable<void> onReceive   (MessageHandle  &&msg);

        awaitable<void> beforeSend  (Message        *msg);
        awaitable<void> afterSend   (MessageHandle  &&msg);

    private:
        Connection &conn_;
        vector<HandlerUnique> handlers_;

        vector<ConnectionInboundHandler *>  inbounds_;
        vector<ConnectionOutboundHandler *> outbounds_;

        ConnectFunctor      onConnect_;
        DisconnectFunctor   onDisconnect_;
        ErrorFunctor        onError_;
        ExceptionFunctor    onException_;
        TimeoutFunctor      onTimeout_;

        ReceiveFunctor      onReceive_;
        BeforeSendFunctor   beforeSend_;
        AfterSendFunctor    afterSend_;
    };

}