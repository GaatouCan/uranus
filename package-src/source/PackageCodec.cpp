#include "PackageCodec.h"

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <endian.h>
#endif

namespace uranus {

    struct PackageHeader {
        int64_t id = 0;
        size_t length = 0;
    };

    PackageCodec::PackageCodec(Connection &conn)
        : MessageCodec(conn) {
    }

    PackageCodec::~PackageCodec() {
    }

    awaitable<error_code> PackageCodec::encode(PackageHandle &&msg) {
        if (msg == nullptr)
            co_return error_code{};

        PackageHeader header;

#if defined(_WIN32) || defined(_WIN64)
        header.id = static_cast<int64_t>(htonll(msg->id_));
        header.length = static_cast<int64_t>(htonll(msg->payload_.size()));
#else
        header.id = static_cast<int64_t>(htobe64(msg->id_));
        header.length = static_cast<int64_t>(htobe64(msg->payload_.size()));
#endif

        if (msg->payload_.empty()) {
            const auto [ec, len] = co_await asio::async_write(getSocket(), asio::buffer(&header, sizeof(PackageHeader)));

            if (ec)
                co_return ec;

            if (len != sizeof(PackageHeader)) {
                // TODO
                co_return error_code{};
            }

            co_return error_code{};
        }

        const auto buffers = {
            asio::buffer(&header, sizeof(PackageHeader)),
            asio::buffer(msg->payload_),
        };

        const auto payloadLength = msg->payload_.size();
        const auto [ec, len] = co_await asio::async_write(getSocket(), buffers);

        if (ec) {
            co_return ec;
        }

        if (len != sizeof(PackageHeader) + payloadLength) {
            // TODO
            co_return error_code{};
        }

        co_return error_code{};
    }

    awaitable<tuple<error_code, PackageHandle>> PackageCodec::decode() {

    }
}
