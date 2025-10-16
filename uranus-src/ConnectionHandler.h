#pragma once

#include "Common.h"

namespace uranus {

    class Message;

    namespace network {

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

            virtual void OnConnected() {}

            virtual void OnReadMessage(Message *msg) {}
            virtual void OnWriteMessage(Message *msg) {}

            virtual void OnTimeout() {}
            virtual void OnDisconnect() {}

        protected:
            Connection *conn_;
        };
    }
}
