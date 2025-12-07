#pragma once

#include "Package.h"

#include <../../../base-src/include/network/Connection.h>

namespace uranus::actor {

    using asio::awaitable;
    using std::error_code;
    using std::tuple;
    using std::make_tuple;

    class ACTOR_API PackageCodec final : public MessageCodec<Package> {

    public:
        explicit PackageCodec(Connection &conn);
        ~PackageCodec() override;

        awaitable<error_code> encode(PackageHandle &&pkg) override;
        awaitable<tuple<error_code, PackageHandle>> decode() override;
    };
}
