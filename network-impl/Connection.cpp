#include "Connection.h"
#include "PackagePool.h"

#include <asio/experimental/awaitable_operators.hpp>

#include <spdlog/spdlog.h>

#include "Message.h"
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <endian.h>
#endif

using namespace asio::experimental::awaitable_operators;

namespace uranus::network {
    Connection::Connection(SslStream &&stream)
        : stream_(std::move(stream)),
          server_(nullptr) {

        pool_ = make_shared<PackagePool>();
        pool_->Initial(64);
    }

    Connection::~Connection() {
    }

    void Connection::ConnectToClient() {
        co_spawn(stream_.get_executor(), [self = shared_from_this()]() mutable -> awaitable<void> {
            const auto [ec] = co_await self->stream_.async_handshake(asio::ssl::stream_base::server);
            if (ec) {
                // TODO
                self->Disconnect();
            }

            co_await (
                self->ReadPackage() &&
                self->WritePackage() &&
                self->Watchdog()
            );
        }, detached);
    }

    void Connection::Disconnect() {
        if (!stream_.next_layer().is_open()) {
            return;
        }

        stream_.next_layer().close();

        // TODO
    }

    awaitable<void> Connection::ReadPackage() {
        try {
            auto *pkg = pool_->Acquire();

            const auto [header_ec, header_len] = co_await async_read(stream_, asio::buffer(&pkg->header_, Package::kPackageHeaderSize));

            if (header_ec) {
                // TODO
                this->Disconnect();
                co_return;
            }

            if (header_len != Package::kPackageHeaderSize) {
                this->Disconnect();
                co_return;
            }

            pkg->header_.id = static_cast<int32_t>(ntohl(pkg->header_.id));
            pkg->header_.length = static_cast<int32_t>(ntohl(pkg->header_.length));

            if (pkg->header_.length > 0) {
                if (pkg->header_.length > 4096 * 1024) {
                    // TODO: Too long
                    this->Disconnect();
                    co_return;
                }

                pkg->payload_.resize(pkg->header_.length);
                const auto [payload_ec, payload_len] = co_await async_read(stream_, asio::buffer(pkg->payload_));

                if (payload_ec) {
                    // TODO:
                    this->Disconnect();
                    co_return;
                }

                if (payload_len != pkg->header_.length) {
                    // TODO:
                    this->Disconnect();
                    co_return;
                }
            }

            auto *msg = new Message();

            msg->type != Message::kFromClient;
            msg->session = 0;
            msg->data = reinterpret_cast<void *>(pkg);
            msg->length = sizeof(Package);

            // TODO: Handle Pacakge
        } catch (const std::exception &e) {
            // TODO
        }
    }

    awaitable<void> Connection::WritePackage() {
        try {

        } catch (const std::exception &e) {
            // TODO
        }
    }

    awaitable<void> Connection::Watchdog() {
    }
}
