#include "data_asset/DA_PlayerResult.h"

namespace uranus::login {
    DataAsset *DA_PlayerResult::clone() {
        auto *res = new DA_PlayerResult();
        res->data = data;
        return res;
    }
}
