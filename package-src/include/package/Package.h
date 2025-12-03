#pragma once

#include "package.export.h"

#include <base/Message.h>
#include <base/Recycler.h>


namespace uranus {
    class PACKAGE_API Package final : public Message {

        DECLARE_MESSAGE_POOL_GET(Package)

        explicit Package(const PackageRecyclerHandle &handle);
        ~Package() override;

    public:
        Package() = delete;

        void recycle();

    private:
        PackageRecyclerHandle handle_;
    };

    DECLARE_MESSAGE_POOL(Package)
}
