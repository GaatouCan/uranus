#pragma once

#include "Package.h"

#include <base/network/Connection.h>

namespace uranus {

    using network::MessageCodec;
    using network::ConnectionBase;
    using asio::awaitable;
    using std::tuple;
    using std::make_tuple;
    using std::error_code;

    class PackageCodec final : public MessageCodec<Package> {
    public:
        explicit PackageCodec(ConnectionBase &conn);
        ~PackageCodec() override;

        awaitable<error_code> encode(HandleType &&msg) override;
        awaitable<tuple<error_code, HandleType>> decode() override;
    };
}
