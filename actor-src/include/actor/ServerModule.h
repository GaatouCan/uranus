#pragma once


#include <base/noncopy.h>

namespace uranus::actor {

    class ServerModule {

    public:
        ServerModule() noexcept = default;
        virtual ~ServerModule() = default;

        DISABLE_COPY_MOVE(ServerModule)

        virtual const char *getModuleName() = 0;

        virtual void start() = 0;
        virtual void stop() = 0;
    };

#define SERVER_MODULE_NAME(s) \
[[nodiscard]] constexpr const char *getModuleName() override { \
    return #s; \
}

}