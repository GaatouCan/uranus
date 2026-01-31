#pragma once

#include "BaseConnection.h"


namespace uranus::network {

    using std::tuple;
    using std::error_code;
    using std::derived_from;

    template<class T>
    requires derived_from<T, Message>
    class MessageCodec {

    public:
        using MessageType = std::decay_t<T>;
        using MessageHandleType = Message::Pointer<MessageType>;
        using ResultTuple = tuple<error_code, MessageHandleType>;

        MessageCodec() = delete;

        explicit MessageCodec(BaseConnection &conn);
        virtual ~MessageCodec();

        DISABLE_COPY_MOVE(MessageCodec)

        [[nodiscard]] BaseConnection &connection() const;
        [[nodiscard]] TcpSocket &socket() const;

        virtual awaitable<ResultTuple> decode() = 0;
        virtual awaitable<error_code> encode(MessageType *msg) = 0;

    private:
        BaseConnection &conn_;
    };

    template<class T>
    requires derived_from<T, Message>
    MessageCodec<T>::MessageCodec(BaseConnection &conn)
        : conn_(conn) {
    }

    template<class T>
    requires derived_from<T, Message>
    MessageCodec<T>::~MessageCodec() {
    }

    template<class T>
    requires derived_from<T, Message>
    BaseConnection &MessageCodec<T>::connection() const {
        return conn_;
    }

    template<class T>
    requires derived_from<T, Message>
    TcpSocket &MessageCodec<T>::socket() const {
        return conn_.socket();
    }
}
