#pragma once

#include "BaseConnection.h"

namespace uranus::network {

    using std::tuple;
    using std::error_code;

    template<class T>
    requires std::is_base_of_v<Message, T>
    class MessageCodec {

    public:
        using MessageType = std::decay_t<T>;
        using MessageHandleType = Message::Pointer<MessageType>;

        MessageCodec() = delete;

        explicit MessageCodec(BaseConnection &conn);
        virtual ~MessageCodec();

        DISABLE_COPY_MOVE(MessageCodec)

        [[nodiscard]] BaseConnection &connection() const;
        [[nodiscard]] TcpSocket &socket() const;

        virtual awaitable<tuple<error_code, MessageHandleType> > decode() = 0;
        virtual awaitable<error_code> encode(MessageType *msg) = 0;

    private:
        BaseConnection &conn_;
    };

    template<class T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::MessageCodec(BaseConnection &conn)
        : conn_(conn) {
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::~MessageCodec() {
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    BaseConnection &MessageCodec<T>::connection() const {
        return conn_;
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    TcpSocket &MessageCodec<T>::socket() const {
        return conn_.socket();
    }
}
