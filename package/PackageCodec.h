#pragma once

#include "MessageCodec.h"
#include "base/Types.h"

#include <asio/ssl/stream.hpp>

namespace uranus::network {

    using SslStream = asio::ssl::stream<TcpSocket>;

    class PackageCodec final : public MessageCodec {

        SslStream &stream_;

    public:
        explicit PackageCodec(SslStream &stream);
        ~PackageCodec() override;

        awaitable<void> Encode(Message *msg) override;
        awaitable<void> Decode(Message *msg) override;
    };
}
