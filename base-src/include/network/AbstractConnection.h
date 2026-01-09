#pragma once

#include "base/Message.h"
#include "base/noncopy.h"

#include <string>

namespace uranus::network {

    class AbstractConnection {

    public:
        AbstractConnection() = default;
        virtual ~AbstractConnection() = default;

        DISABLE_COPY_MOVE(AbstractConnection)

        virtual void connect() = 0;
        virtual void disconnect() = 0;
        [[nodiscard]] virtual bool isConnected() const = 0;

        [[nodiscard]] virtual const std::string &getKey() const = 0;

        virtual void sendMessage(MessageHandle &&msg) = 0;
        virtual void sendMessage(Message *msg) = 0;
    };
}