#include "PackageCodec.h"
#include "Package.h"

#include <asio/read.hpp>
#include <asio/write.hpp>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#elifdef __APPLE__

#elif
#include <arpa/inet.h>
#endif

namespace uranus::actor {

    struct PackageHeader {
        int64_t id = 0;
        size_t length = 0;
    };

    PackageCodec::PackageCodec(Connection &conn)
        : MessageCodec(conn) {
    }

    PackageCodec::~PackageCodec() {
    }

    awaitable<error_code> PackageCodec::encode(Package *pkg) {
        if (pkg == nullptr)
            co_return error_code{};

        PackageHeader header;

#if defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
        header.id = static_cast<int64_t>(htonll(pkg->id_));
        header.length = static_cast<int64_t>(htonll(pkg->payload_.size()));
#else
        header.id = static_cast<int64_t>(htobe64(pkg->id_));
        header.length = static_cast<int64_t>(htobe64(pkg->payload_.size()));
#endif

        if (pkg->payload_.empty()) {
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
            asio::buffer(pkg->payload_),
        };

        const auto payloadLength = pkg->payload_.size();
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
        PackageHeader header;

        // Read header
        {
            const auto [ec, len] = co_await asio::async_read(getSocket(), asio::buffer(&header, sizeof(PackageHeader)));
            if (ec) {
                co_return make_tuple(ec, nullptr);
            }

#if defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            header.id = static_cast<int64_t>(ntohll(header.id));
            header.length = static_cast<int64_t>(ntohll(header.length));
#else
            header.id = static_cast<int64_t>(be64toh(header.id));
            header.length = static_cast<int64_t>(be64toh(header.length));
#endif
        }

        auto pkg = Package::getHandle();

        pkg->id_ = header.id;

        if (header.length > 0) {
            pkg->payload_.resize(header.length);
            const auto [ec, len] = co_await asio::async_read(getSocket(), asio::buffer(pkg->payload_));

            if (ec) {
                co_return make_tuple(ec, nullptr);
            }

            if (len != header.length) {
                // TODO
                co_return make_tuple(error_code{}, nullptr);
            }
        }

        co_return make_tuple(error_code{}, std::move(pkg));
    }
}
