#include "Connection.h"

namespace uranus {
    Connection::Connection(TcpSocket &&socket)
        : socket_(std::move(socket)) {
    }

    Connection::~Connection() {
    }

    TcpSocket &Connection::getSocket() {
        return socket_;
    }

    AttributeMap &Connection::attr() {
        return attr_;
    }
}
