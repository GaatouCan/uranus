#pragma once

#include <actor/DataAsset.h>
#include <memory>
#include <nlohmann/json.hpp>

namespace gameplay {

    using uranus::actor::DataAsset;

    class DA_PlayerResult final : public DataAsset {

    public:
        std::shared_ptr<nlohmann::json> data;
    };
}