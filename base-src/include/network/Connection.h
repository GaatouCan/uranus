#pragma once

#include "base/noncopy.h"
#include "base/Message.h"
#include "base/AttributeMap.h"


namespace uranus::network {

    class Connection {

    public:
        Connection() = default;
        virtual ~Connection() = default;

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