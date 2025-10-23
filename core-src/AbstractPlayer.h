#pragma once

#include "AbstractActor.h"
#include "ActorContext.h"

#include <cstdint>
#include <string>

namespace uranus {

    class CORE_API AbstractPlayer : public AbstractActor {

    public:
        AbstractPlayer();
        ~AbstractPlayer() override;

        void SetPlayerID(int64_t id);
        [[nodiscard]] int64_t GetPlayerID() const;

    protected:
        void Send(int64_t target, const Message &msg) const;

        void SendToService(int64_t target, Message msg) const;
        void SendToService(const std::string &name, Message msg) const;

        void SendToClient(Message msg) const;

        template<asio::completion_token_for<optional<Message>> CompletionToken>
        auto AsyncCallService(int64_t target, Message req, CompletionToken &&token);

    private:
        int64_t id_;
    };

    template<asio::completion_token_for<optional<Message>> CompletionToken>
    auto AbstractPlayer::AsyncCallService(int64_t target, Message req, CompletionToken &&token) {
        req.type |= (Message::kToService | Message::kRequest);
        return GetActorContext()->AsyncCall(target, req, std::forward<CompletionToken>(token));
    }
}
