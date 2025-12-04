#include "AttributeMap.h"

namespace uranus {
    AttributeMap::AttributeMap() {
    }

    AttributeMap::~AttributeMap() {
    }

    bool AttributeMap::hasAttr(const std::string &key) const {
        return attr_.contains(key);
    }
}
