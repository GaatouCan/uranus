#include "MessageCodec.h"
#include "Connection.h"

namespace uranus::network {
    MessageCodec::MessageCodec(Connection &conn)
        : conn_(conn) {
    }

    MessageCodec::~MessageCodec() = default;

    Connection &MessageCodec::getConnection() const {
        return conn_;
    }

    TcpSocket &MessageCodec::getSocket() const {
        return conn_.getSocket();
    }
}
