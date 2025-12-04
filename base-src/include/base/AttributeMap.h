#pragma once

#include "base.export.h"

#include <any>
#include <optional>
#include <unordered_map>
#include <string>

namespace uranus {
    class BASE_API AttributeMap final {

    public:
        AttributeMap();
        ~AttributeMap();

        [[nodiscard]] bool hasAttr(const std::string& key) const;

    private:
        std::unordered_map<std::string, std::any> attr_;
    };
}