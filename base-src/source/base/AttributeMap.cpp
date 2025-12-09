#include "AttributeMap.h"

namespace uranus {
    AttributeMap::AttributeMap() = default;
    AttributeMap::~AttributeMap() = default;

    bool AttributeMap::has(const std::string &key) const {
        return attr_.contains(key);
    }

    void AttributeMap::clear() {
        attr_.clear();
    }

    void AttributeMap::erase(const std::string &key) {
        attr_.erase(key);
    }
}
