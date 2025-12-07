#pragma once

#include "base.export.h"

namespace uranus::network {

    class BASE_API ConnectionHandler {

    public:
        enum class Type {
            kInbound,
            kOutbound,
            kDeluxe
        };

        ConnectionHandler();
        virtual ~ConnectionHandler();

        [[nodiscard]] virtual Type type() const = 0;
    };
}