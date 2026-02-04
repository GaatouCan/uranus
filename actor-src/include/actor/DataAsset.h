#pragma once

#include "actor.export.h"

namespace uranus::actor {

    class ACTOR_API DataAsset {

    public:
        DataAsset();
        virtual ~DataAsset();

        virtual DataAsset *clone() = 0;
    };
}
