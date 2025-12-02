#pragma once

#include <base/Message.h>

namespace uranus {
    class Package final : public Message {

    public:
        Package();
        ~Package() override;
    };
}
