#pragma once

#include "BaseActor.h"

namespace uranus::actor {

    class ACTOR_API BaseService : public BaseActor {

    public:
        BaseService();
        ~BaseService() override;

        [[nodiscard]] virtual std::string getName() const = 0;
    };
}