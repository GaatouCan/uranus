#include "ConnectionInboundHandler.h"

namespace uranus::network {
    ConnectionInboundHandler::ConnectionInboundHandler() {
    }

    ConnectionInboundHandler::~ConnectionInboundHandler() {
    }

    ConnectionHandler::Type ConnectionInboundHandler::type() const {
        return Type::kInbound;
    }
}
