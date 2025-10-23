#pragma once

#include "AbstractActor.h"

#include <string>


namespace uranus {

    class CORE_API AbstractService : public AbstractActor {

    public:
        AbstractService();
        ~AbstractService() override;

        void SetServiceID(int64_t id);
        [[nodiscard]] int64_t GetServiceID() const;

        [[nodiscard]] virtual std::string GetServiceName() const = 0;

        void Send(int64_t target, const Message &msg) const;

        void SendToService(int64_t target, Message msg) const;
        void SendToService(const std::string &target, Message msg) const;

        void SendToPlayer(int64_t pid, Message msg) const;
        void SendToClient(int64_t pid, Message msg) const;

    private:
        int64_t id_;
    };
}
