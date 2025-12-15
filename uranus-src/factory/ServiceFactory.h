#pragma once

#include <base/noncopy.h>
#include <base/SharedLibrary.h>


namespace uranus {
    class ServiceFactory final {

        ServiceFactory();

    public:
        ~ServiceFactory();

        DISABLE_COPY_MOVE(ServiceFactory)

        static ServiceFactory& instance();
    };
} // uranus
