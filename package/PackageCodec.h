#pragma once

#include "MessageCodec.h"
#include "base/Types.h"

#include <asio/ssl/stream.hpp>

namespace uranus::network {

    using SslStream = asio::ssl::stream<TcpSocket>;

    class NETWORK_API PackageCodec final : public MessageCodec {

        SslStream &stream_;

    public:
        explicit PackageCodec(SslStream &stream);
        ~PackageCodec() override;

        awaitable<error_code> Initial() override;

        awaitable<error_code> Encode(const Message &msg) override;
        awaitable<error_code> Decode(const Message &msg) override;
    };
}
