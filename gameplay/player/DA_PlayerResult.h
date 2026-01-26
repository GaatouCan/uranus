#pragma once

#include <actor/DataAsset.h>
#include <nlohmann/json.hpp>

namespace gameplay {

    using uranus::actor::DataAsset;

    class DA_PlayerResult final : public DataAsset {

    public:
        nlohmann::json data;
    };
}