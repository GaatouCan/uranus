#pragma once

#include "AbstractActor.h"
#include "ActorContext.h"

#include <string>

namespace uranus {

    class CORE_API AbstractService : public AbstractActor {

    public:
        AbstractService();
        ~AbstractService() override;

        void SetServiceID(int64_t id);
        [[nodiscard]] int64_t GetServiceID() const;

        [[nodiscard]] virtual std::string GetServiceName() const = 0;

    protected:
        void Send(int64_t target, const Message &msg) const;

        void SendToService(int64_t target, Message msg) const;
        void SendToService(const std::string &target, Message msg) const;

        void SendToPlayer(int64_t pid, Message msg) const;
        void SendToClient(int64_t pid, Message msg) const;

        template<asio::completion_token_for<optional<Message>> CompletionToken>
        auto AsyncCallService(int64_t target, Message req, CompletionToken &&token);

        template<asio::completion_token_for<optional<Message>> CompletionToken>
        auto AsyncCallPlayer(int64_t target, Message req, CompletionToken &&token);

    private:
        int64_t id_;
    };

    template<asio::completion_token_for<optional<Message>> CompletionToken>
    auto AbstractService::AsyncCallService(int64_t target, Message req, CompletionToken &&token) {
        req.type |= (Message::kToService | Message::kRequest);
        return GetActorContext()->AsyncCall(target, req, std::forward<CompletionToken>(token));
    }

    template<asio::completion_token_for<optional<Message>> CompletionToken>
    auto AbstractService::AsyncCallPlayer(int64_t target, Message req, CompletionToken &&token) {
        req.type |= (Message::kToPlayer | Message::kRequest);
        return GetActorContext()->AsyncCall(target, req, std::forward<CompletionToken>(token));
    }
}
