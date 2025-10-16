#pragma once

#include <memory>

#include "Common.h"

namespace uranus::network {

    class Connection;

    class ConnectionHandler {

    public:
        ConnectionHandler() = delete;

        explicit ConnectionHandler(Connection *conn) : conn_(conn) {}
        virtual ~ConnectionHandler() = default;

        DISABLE_COPY_MOVE(ConnectionHandler)

        [[nodiscard]] Connection *GetConnection() const {
            return conn_;
        }

    protected:
        Connection *conn_;
    };
}
