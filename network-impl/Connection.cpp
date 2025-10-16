#include "Connection.h"
#include "PackagePool.h"
#include "Message.h"
#include "base/Utils.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <spdlog/spdlog.h>

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
          server_(nullptr),
          output_(stream_.get_executor(), 1024),
          watchdog_(stream_.get_executor()),
          expiration_(std::chrono::seconds(30)) {

        pool_ = make_shared<PackagePool>();
        pool_->Initial(64);

        key_ = fmt::format("{}-{}", stream_.next_layer().remote_endpoint().address().to_string(), utils::UnixTime());
    }

    Connection::~Connection() {
    }

    void Connection::SetGameServer(GameServer *server) {
        server_ = server;
    }

    GameServer *Connection::GetGameServer() const {
        return server_;
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
        if (!IsConnected()) {
            return;
        }

        stream_.next_layer().close();
        output_.close();
        watchdog_.cancel();

        // TODO
    }

    bool Connection::IsConnected() const {
        return stream_.next_layer().is_open();
    }

    void Connection::SendToClient(Message *msg) {
        if (msg == nullptr) return;

        if (!stream_.next_layer().is_open()) {
            delete msg;
            return;
        }

        output_.try_send_via_dispatch(error_code{}, unique_ptr<Message>(msg));
    }

    awaitable<void> Connection::ReadPackage() {
        try {
            while (IsConnected()) {
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

                msg->type |= Message::kFromClient;
                msg->session = 0;
                msg->data = reinterpret_cast<void *>(pkg);
                msg->length = sizeof(Package);

                // TODO: Handle Pacakge
            }
        } catch (const std::exception &e) {
            // TODO
        }
    }

    awaitable<void> Connection::WritePackage() {
        try {
            while (IsConnected() && output_.is_open()) {
                const auto [ec, msg] = co_await output_.async_receive();
                if (ec) {
                    // TODO
                    break;
                }

                if (msg == nullptr)
                    continue;

                const auto *pkg = static_cast<Package *>(msg->data);
                if (pkg == nullptr) {
                    // TODO
                    continue;
                }

                Package::PackageHeader header{};
                memset(&header, 0, Package::kPackageHeaderSize);

                header.id = static_cast<int32_t>(htonl(pkg->header_.id));
                header.length = static_cast<int32_t>(htonl(pkg->header_.length));

                if (pkg->header_.length <= 0) {
                    const auto [ec, len] = co_await async_write(stream_, asio::buffer(&header, Package::kPackageHeaderSize));

                    if (ec) {
                        // TODO
                        this->Disconnect();
                        co_return;
                    }

                    if (len != Package::kPackageHeaderSize) {
                        // TODO
                        this->Disconnect();
                        co_return;
                    }

                    continue;
                }

                if (pkg->header_.length > 1024 * 4096) {
                    // TODO:
                    continue;
                }

                const auto buffers = {
                    asio::const_buffer(&header, Package::kPackageHeaderSize),
                    asio::buffer(pkg->payload_),
                };

                const auto [data_ec, data_len] = co_await async_write(stream_, buffers);

                if (data_ec) {
                    // TODO
                    this->Disconnect();
                    co_return;
                }

                if (data_len != Package::kPackageHeaderSize + pkg->header_.length) {
                    // TODO
                    this->Disconnect();
                    co_return;
                }
            }

            while (true) {
                if (const auto [ec, msg] = co_await output_.async_receive();
                    ec && ec == asio::error::operation_aborted)
                    break;
            }
        } catch (const std::exception &e) {
            // TODO
        }
    }

    awaitable<void> Connection::Watchdog() {
        if (expiration_ <= SteadyDuration::zero())
            co_return;

        try {
            SteadyTimePoint now;

            do {
                watchdog_.expires_at(received_ + expiration_);

                if (auto [ec] = co_await watchdog_.async_wait(); ec) {
                    if (ec == asio::error::operation_aborted) {

                    }
                        // SPDLOG_INFO("{} - Connection[{}] canceled watchdog timer",
                        //         __FUNCTION__, key_);
                    else {

                    }
                        // SPDLOG_INFO("{} - Connection[{}] - {}", __FUNCTION__, key_,
                        //         ec.message());

                    co_return;
                }

                now = std::chrono::steady_clock::now();
            } while (received_ + expiration_ > now);

            if (IsConnected()) {
                this->Disconnect();
            }
        } catch (const std::exception &e) {
            // SPDLOG_ERROR("{} - Connection[{}], Exception: {}", __FUNCTION__, key_, e.what());
        }
    }
}
