#include "Connection.h"

#include <chrono>
#include <format>

namespace uranus {
    Connection::Connection(TcpSocket &&socket)
        : socket_(std::move(socket)),
          watchdog_(socket_.get_executor()) {
        const auto now = std::chrono::system_clock::now();
        const auto durationSinceEpoch = now.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);

#ifdef URANUS_SSL
        key_ = std::format("{}-{}", socket_.next_layer().remote_endpoint().address().to_string(), secondsSinceEpoch.count());
#else
        key_ = std::format("{}-{}", socket_.remote_endpoint().address().to_string(), secondsSinceEpoch.count());
#endif
    }

    Connection::~Connection() {
    }

    TcpSocket &Connection::getSocket() {
        return socket_;
    }

    bool Connection::isConnected() const {
#ifdef URANUS_SSL
        return socket_.next_layer().is_open();
#else
        return socket_.is_open();
#endif
    }

    const std::string &Connection::getKey() const {
        return key_;
    }

    AttributeMap &Connection::attr() {
        return attr_;
    }

    awaitable<void> Connection::watchdog() {
        try {

        } catch (const std::exception &e) {

        }
    }
}
