#pragma once

#include <actor/DataAsset.h>
#include <nlohmann/json.hpp>

namespace uranus::database {

    using actor::DataAsset;

    class DA_PlayerResult final : public DataAsset {

    public:
        nlohmann::json data;
    };
}