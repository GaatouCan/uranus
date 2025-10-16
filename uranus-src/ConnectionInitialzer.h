#pragma once

#include "Common.h"

#include <memory>

namespace uranus::network {

    class Connection;

    class ConnectionInitializer {

    public:
        ConnectionInitializer() = default;
        virtual ~ConnectionInitializer() = default;

        DISABLE_COPY_MOVE(ConnectionInitializer)

        virtual void OnAcceptConnection(const std::shared_ptr<Connection>& conn) = 0;
    };
}