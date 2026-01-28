#pragma once

#include "BaseActor.h"

namespace uranus::actor {

    class ACTOR_API BaseService : public BaseActor {

    public:
        BaseService();
        ~BaseService() override;

        [[nodiscard]] virtual std::string getName() const = 0;

        void sendToClient(PackageHandle &&pkg) const;
        void sendToPlayer(int64_t pid, PackageHandle &&pkg) const;
        void sendToService(const std::string &name, PackageHandle &&pkg) const;
    };
}

#define EXPORT_SERVICE(ser)                                         \
EXPORT_ACTOR_VERSION                                                \
ACTOR_EXPORT uranus::actor::BaseService *CreateInstance() {         \
    return new ser();                                               \
}                                                                   \
ACTOR_EXPORT void DeleteInstance(uranus::actor::BaseService *ptr) { \
    delete ptr;                                                     \
}
