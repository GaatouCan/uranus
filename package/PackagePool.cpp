#include "PackagePool.h"

namespace uranus::network {
    Package *PackagePool::Create(const Handle &handle) const {
        return new Package(handle);
    }
}
