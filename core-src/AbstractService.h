#pragma once

#include "AbstractActor.h"

#include <string>


namespace uranus {

    class CORE_API AbstractService : public AbstractActor {

    public:
        AbstractService();
        ~AbstractService() override;

        [[nodiscard]] virtual int64_t GetServiceID() const = 0;
        [[nodiscard]] virtual std::string GetServiceName() const = 0;

        void SendToService(int64_t target, Message *msg) const;
        void SendToService(const std::string &target, Message *msg) const;

        void SendToPlayer(int64_t pid, Message *msg) const;
        void SendToClient(int64_t pid, Message *msg) const;
    };
}
