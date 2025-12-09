#pragma once

#include "base/Message.h"
#include "base/types.h"
#include "base/noncopy.h"

#include <tuple>
#include <asio/awaitable.hpp>

namespace uranus::network {
    using std::error_code;
    using std::tuple;
    using std::make_tuple;
    using asio::awaitable;

    class Connection;

    class BASE_API MessageCodec {

    public:
        MessageCodec() = delete;

        explicit MessageCodec(Connection &conn);
        virtual ~MessageCodec();

        DISABLE_COPY_MOVE(MessageCodec)

        [[nodiscard]] Connection &getConnection() const;
        [[nodiscard]] TcpSocket &getSocket() const;

        virtual awaitable<tuple<error_code, MessageHandle> > decode() = 0;
        virtual awaitable<error_code> encode(Message *msg) = 0;

    private:
        Connection &conn_;
    };
}
