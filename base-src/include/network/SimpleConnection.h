#pragma once

#include "ConnectionAdapter.h"

#include <functional>

namespace uranus::network {

    using std::function;

    template<kCodecType Codec>
    class SimpleConnection final : public ConnectionAdapter<Codec> {

    public:
        explicit SimpleConnection(TcpSocket &&socket);
        ~SimpleConnection() override;

        using ConnectCallback       = std::function<void()>;
        using DisconnectCallback    = std::function<void()>;
        using TimeoutCallback       = std::function<void()>;
        using ErrorCodeCallback     = std::function<void(error_code)>;
        using ExceptionCallback     = std::function<void(std::exception &)>;
        using ReadMessageCallback   = std::function<void(typename ConnectionAdapter<Codec>::MessageHandleType &&)>;
        using BeforeWriteCallback   = std::function<void(typename ConnectionAdapter<Codec>::MessageHandleType *)>;
        using AfterWriteCallback    = std::function<void(typename ConnectionAdapter<Codec>::MessageHandleType &&)>;

        void onConnect      (const ConnectCallback &cb);
        void onDisconnect   (const DisconnectCallback &cb);

        void onTimeout      (const TimeoutCallback &cb);

        void onErrorCode    (const ErrorCodeCallback &cb);
        void onException    (const ExceptionCallback &cb);

        void onReadMessage  (const ReadMessageCallback &cb);

        void beforeWrite    (const BeforeWriteCallback &cb);
        void afterWrite     (const AfterWriteCallback &cb);

    protected:
        void onConnect() override;
        void onDisconnect() override;

        void onTimeout() override;

        void onErrorCode(error_code ec) override;
        void onException(std::exception &e) override;

        void onReadMessage(ConnectionAdapter<Codec>::MessageHandleType &&msg) override;

        void beforeWrite(ConnectionAdapter<Codec>::MessageType *msg) override;
        void afterWrite(ConnectionAdapter<Codec>::MessageHandleType &&msg) override;

    private:
        ConnectCallback     onConnect_;
        DisconnectCallback  onDisconnect_;
        TimeoutCallback     onTimeout_;
        ErrorCodeCallback   onErrorCode_;
        ExceptionCallback   onException_;
        ReadMessageCallback onReadMessage_;
        BeforeWriteCallback beforeWrite_;
        AfterWriteCallback  afterWrite_;
    };

    template<kCodecType Codec>
    SimpleConnection<Codec>::SimpleConnection(TcpSocket &&socket)
        : ConnectionAdapter<Codec>(std::move(socket)){
    }

    template<kCodecType Codec>
    SimpleConnection<Codec>::~SimpleConnection() = default;

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onConnect(const ConnectCallback &cb) {
        onConnect_ = cb;
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onDisconnect(const DisconnectCallback &cb) {
        onDisconnect_ = cb;
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onTimeout(const TimeoutCallback &cb) {
        onTimeout_ = cb;
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onErrorCode(const ErrorCodeCallback &cb) {
        onErrorCode_ = cb;
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onException(const ExceptionCallback &cb) {
        onException_ = cb;
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onReadMessage(const ReadMessageCallback &cb) {
        onReadMessage_ = cb;
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::beforeWrite(const BeforeWriteCallback &cb) {
        beforeWrite_ = cb;
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::afterWrite(const AfterWriteCallback &cb) {
        afterWrite_ = cb;
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onConnect() {
        if (onConnect_) {
            std::invoke(onConnect_);
        }
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onDisconnect() {
        if (onDisconnect_) {
            std::invoke(onDisconnect_);
        }
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onTimeout() {
        if (onTimeout_) {
            std::invoke(onTimeout_);
        }
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onErrorCode(error_code ec) {
        if (onErrorCode_) {
            std::invoke(onErrorCode_, ec);
        }
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onException(std::exception &e) {
        if (onException_) {
            std::invoke(onException_, e);
        }
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::onReadMessage(typename ConnectionAdapter<Codec>::MessageHandleType &&msg) {
        if (onReadMessage_) {
            std::invoke(onReadMessage_, std::move(msg));
        }
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::beforeWrite(typename ConnectionAdapter<Codec>::MessageType *msg) {
        if (beforeWrite_) {
            std::invoke(beforeWrite_, msg);
        }
    }

    template<kCodecType Codec>
    void SimpleConnection<Codec>::afterWrite(typename ConnectionAdapter<Codec>::MessageHandleType &&msg) {
        if (afterWrite_) {
            std::invoke(afterWrite_, std::move(msg));
        }
    }
}
