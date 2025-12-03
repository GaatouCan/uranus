#include "package/Package.h"

namespace uranus {
    Package::Package(const PackageRecyclerHandle &handle)
        : handle_(handle) {
    }

    Package::~Package() {
    }

    void Package::recycle() {
        handle_.recycle(this);
    }

    IMPLEMENT_RECYCLER_GET(Package)

    IMPLEMENT_RECYCLER(Package)
}
