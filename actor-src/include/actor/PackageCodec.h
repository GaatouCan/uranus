#pragma once

#include "Package.h"

#include <network/Connection.h>

namespace uranus::actor {

    using network::MessageCodec;
    using network::Connection;
    using asio::awaitable;
    using std::error_code;
    using std::tuple;
    using std::make_tuple;

    class ACTOR_API PackageCodec final : public MessageCodec<Package> {

    public:
        explicit PackageCodec(Connection &conn);
        ~PackageCodec() override;

        awaitable<error_code> encode(Package *msg) override;
        awaitable<tuple<error_code, PackageHandle>> decode() override;
    };
}
