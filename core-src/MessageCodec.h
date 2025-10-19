#pragma once

#include "Common.h"

#include <asio/awaitable.hpp>

namespace uranus {

    struct Message;
    using asio::awaitable;

    class CORE_API MessageCodec {

    public:
        MessageCodec() =  default;
        virtual ~MessageCodec() = default;

        virtual awaitable<void> Encode(Message *msg) = 0;
        virtual awaitable<void> Decode(Message *msg) = 0;
    };
}