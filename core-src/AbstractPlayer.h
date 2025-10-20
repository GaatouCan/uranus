#pragma once

#include "AbstractActor.h"

#include <cstdint>

namespace uranus {
    class CORE_API AbstractPlayer : public AbstractActor {

    public:
        AbstractPlayer();
        ~AbstractPlayer() override;

        [[nodiscard]] virtual int64_t GetPlayerID() const = 0;
    };
}