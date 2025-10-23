#pragma once

#include "Common.h"

#include <asio/awaitable.hpp>


namespace uranus {

    struct Message;
    using asio::awaitable;
    using std::error_code;

    class CORE_API MessageCodec {

    public:
        MessageCodec() = default;
        virtual ~MessageCodec() = default;

        virtual awaitable<error_code> Initial() = 0;

        virtual awaitable<error_code> Encode(const Message &msg) = 0;
        virtual awaitable<error_code> Decode(const Message &msg) = 0;
    };
}