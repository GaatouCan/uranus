#include "Connection.h"

#include <chrono>
#include <format>

namespace uranus {
    Connection::Connection(TcpSocket &&socket)
        : socket_(std::move(socket)) {

        const auto now = std::chrono::system_clock::now();
        const auto durationSinceEpoch = now.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);

        key_ = std::format("{}-{}", socket_.next_layer().remote_endpoint().address().to_string(), secondsSinceEpoch.count());
    }

    Connection::~Connection() {
    }

    TcpSocket &Connection::getSocket() {
        return socket_;
    }

    const std::string &Connection::getKey() const {
        return key_;
    }

    AttributeMap &Connection::attr() {
        return attr_;
    }
}
