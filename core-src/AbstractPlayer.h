#pragma once

#include "AbstractActor.h"

#include <cstdint>
#include <string>

namespace uranus {
    class CORE_API AbstractPlayer : public AbstractActor {

    public:
        AbstractPlayer();
        ~AbstractPlayer() override;

        void SetPlayerID(int64_t id);
        [[nodiscard]] int64_t GetPlayerID() const;

        void Send(int64_t target, Message *msg) const;

        void SendToService(int64_t target, Message *msg) const;
        void SendToService(const std::string &name, Message *msg) const;

        void SendToClient(Message *msg) const;

    private:
        int64_t id_;
    };
}
