#pragma once

#include "Package.h"

#include <../../../base-src/include/base/Connection.h>

namespace uranus {
    using network::MessageCodec;
    using network::Connection;
    using asio::awaitable;
    using std::error_code;
    using std::tuple;
    using std::make_tuple;

    class PACKAGE_API PackageCodec final : public MessageCodec<Package> {

    public:
        explicit PackageCodec(Connection &conn);
        ~PackageCodec() override;

        awaitable<error_code> encode(PackageHandle &&pkg) override;
        awaitable<tuple<error_code, PackageHandle>> decode() override;
    };
}
