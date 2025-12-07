#include "ConnectionOutboundHandler.h"

namespace uranus::network {
    ConnectionOutboundHandler::ConnectionOutboundHandler() {
    }

    ConnectionOutboundHandler::~ConnectionOutboundHandler() {
    }

    ConnectionHandler::Type ConnectionOutboundHandler::type() const {
        return Type::kOutbound;
    }
}
