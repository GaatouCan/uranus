#pragma once

#include <actor/DataAsset.h>
#include <nlohmann/json.hpp>

namespace uranus {

    using actor::DataAsset;

    class DA_PlayerResult final : public DataAsset {

    public:
        nlohmann::json data;

    public:
        DataAsset *clone() override {
            auto *res = new DA_PlayerResult();
            res->data = data;
            return res;
        }
    };
}