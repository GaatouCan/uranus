#pragma once

#include "package.export.h"

#include <base/Message.h>
#include <base/Recycler.h>

namespace uranus {
    class PACKAGE_API Package final : public Message {

        DECLARE_RECYCLER_GET(Package)

        explicit Package(const PackageRecyclerHandle &handle);
    public:
        Package() = delete;
        ~Package() override;

        void recycle();

    private:
        PackageRecyclerHandle handle_;
    };

    DECLARE_RECYCLER(Package)
}