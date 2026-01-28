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

#define EXPORT_CREATE_SERVICE(ser)                          \
ACTOR_EXPORT uranus::actor::BaseService *CreateInstance() { \
    return new ser();                                       \
}

#define EXPORT_DELETE_SERVICE                                       \
ACTOR_EXPORT void DeleteInstance(uranus::actor::BaseService *ser) { \
    delete ser;                                                     \
}