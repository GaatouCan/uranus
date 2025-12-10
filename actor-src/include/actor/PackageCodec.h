#pragma once

#include "actor.export.h"

#include <network/MessageCodec.h>

namespace uranus::actor {

    using network::MessageCodec;
    using network::Connection;
    using asio::awaitable;
    using std::error_code;
    using std::tuple;
    using std::make_tuple;

    class ACTOR_API PackageCodec final : public MessageCodec {

    public:
        explicit PackageCodec(Connection &conn);
        ~PackageCodec() override;

        awaitable<error_code> encode(Message *msg) override;
        awaitable<tuple<error_code, MessageHandle>> decode() override;
    };
}
