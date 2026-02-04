#pragma once

#include "base/noncopy.h"
#include "base/Message.h"


namespace uranus {

    class AttributeMap;

    namespace network {

        class BASE_API Connection {

        public:
            Connection();
            virtual ~Connection();

            DISABLE_COPY_MOVE(Connection)

            virtual void connect() = 0;
            virtual void disconnect() = 0;
            [[nodiscard]] virtual bool isConnected() const = 0;

            virtual AttributeMap &attr() = 0;
            [[nodiscard]] virtual const AttributeMap &attr() const = 0;

            virtual void sendMessage(MessageHandle &&msg) = 0;
            virtual void sendMessage(Message *msg) = 0;
        };
    }
}