#include "AbstractPlayer.h"
#include "ActorContext.h"
#include "Message.h"


namespace uranus {

    AbstractPlayer::AbstractPlayer()
        : id_(kInvalidPlayerID){
    }

    AbstractPlayer::~AbstractPlayer() {
    }

    void AbstractPlayer::SetPlayerID(const int64_t id) {
        id_ = id;
    }

    int64_t AbstractPlayer::GetPlayerID() const {
        if (id_ < 0)
            return kInvalidPlayerID;
        return id_;
    }

    void AbstractPlayer::Send(const int64_t target, const Message &msg) const {
        GetActorContext()->Send(target, msg);
    }

    void AbstractPlayer::SendToService(const int64_t target, Message msg) const {
        msg.type |= Message::kToService;
        GetActorContext()->Send(target, msg);
    }

    void AbstractPlayer::SendToService(const std::string &name, Message msg) const {
        msg.type |= Message::kToService;
        GetActorContext()->SendToService(name, msg);
    }

    void AbstractPlayer::SendToClient(Message msg) const {
        msg.type |= Message::kToClient;
        GetActorContext()->Send(id_, msg);
    }
}
