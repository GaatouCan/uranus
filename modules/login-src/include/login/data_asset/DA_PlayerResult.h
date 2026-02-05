#pragma once

#include "login/login.export.h"

#include <actor/DataAsset.h>
#include <nlohmann/json.hpp>


namespace uranus::login {

    using actor::DataAsset;

    class LOGIN_API DA_PlayerResult final : public DataAsset {

    public:
        nlohmann::json data;

    public:
        DataAsset *clone() override;
    };
}