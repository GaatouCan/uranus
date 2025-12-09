#pragma once

#include "base/base.export.h"

namespace uranus::network {
    class BASE_API ConnectionHandler {

    public:
        enum class HandlerType {
            kInbound,
            kOutbound,
            kDeluxe
        };

        ConnectionHandler();
        virtual ~ConnectionHandler();

        [[nodiscard]] virtual HandlerType type() const = 0;
    };
}