#include "PackageCodec.h"
#include "Message.h"
#include "Package.h"
#include "PackageErrc.h"

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <endian.h>
#endif

namespace uranus::network {
    PackageCodec::PackageCodec(SslStream &stream)
        : stream_(stream){
    }

    PackageCodec::~PackageCodec() {
    }

    awaitable<error_code> PackageCodec::Encode(Message *msg) {
        if (msg == nullptr)
            co_return MakeErrorCode(PackageErrc::kNullMessage);

        auto *pkg = static_cast<Package *>(msg->data);

        Package::PackageHeader header{};
        memset(&header, 0, Package::kPackageHeaderSize);

        header.id = static_cast<int32_t>(htonl(pkg->header_.id));
        header.length = static_cast<int32_t>(htonl(pkg->header_.length));

        if (pkg->header_.length <= 0) {
            const auto [ec, len] = co_await async_write(stream_, asio::buffer(&header, Package::kPackageHeaderSize));

            if (ec) {
                co_return ec;
            }

            if (len != Package::kPackageHeaderSize) {
                co_return MakeErrorCode(PackageErrc::kEncodeLengthError);
            }

            co_return std::error_code{};
        }

        if (pkg->header_.length > 1024 * 4096) {
            co_return MakeErrorCode(PackageErrc::kPayloadTooLarge);
        }

        if (pkg->payload_.size() != pkg->header_.length) {
            co_return MakeErrorCode(PackageErrc::kPayloadLengthError);
        }

        const auto buffers = {
            asio::buffer(&header, Package::kPackageHeaderSize),
            asio::buffer(pkg->payload_),
        };

        const auto [data_ec, data_len] = co_await async_write(stream_, buffers);

        if (data_ec) {
            co_return data_ec;
        }

        if (data_len != Package::kPackageHeaderSize + pkg->header_.length) {
            co_return MakeErrorCode(PackageErrc::kEncodeLengthError);
        }

        co_return MakeErrorCode(PackageErrc::kOk);
    }

    awaitable<error_code> PackageCodec::Decode(Message *msg) {
        if (msg == nullptr)
            co_return MakeErrorCode(PackageErrc::kNullMessage);

        auto *pkg = static_cast<Package *>(msg->data);

        const auto [header_ec, header_len] = co_await async_read(stream_, asio::buffer(&pkg->header_, Package::kPackageHeaderSize));

        if (header_ec) {
            co_return header_ec;
        }

        if (header_len != Package::kPackageHeaderSize) {
            co_return MakeErrorCode(PackageErrc::kDecodeLengthError);
        }

        pkg->header_.id = static_cast<int32_t>(ntohl(pkg->header_.id));
        pkg->header_.length = static_cast<int32_t>(ntohl(pkg->header_.length));

        if (pkg->header_.length > 0) {
            if (pkg->header_.length > 4096 * 1024) {
                co_return MakeErrorCode(PackageErrc::kPayloadTooLarge);
            }

            pkg->payload_.resize(pkg->header_.length);
            const auto [payload_ec, payload_len] = co_await async_read(stream_, asio::buffer(pkg->payload_));

            if (payload_ec) {
                co_return payload_ec;
            }

            if (payload_len != pkg->header_.length) {
                co_return MakeErrorCode(PackageErrc::kDecodeLengthError);
            }
        }

        co_return MakeErrorCode(PackageErrc::kOk);
    }
}
