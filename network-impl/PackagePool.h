#pragma once

#include "base/Recycler.h"
#include "Package.h"

namespace uranus::network {
    class PackagePool final : public Recycler<Package> {
    protected:
        Package *Create(const Handle &) const override;
    };
}