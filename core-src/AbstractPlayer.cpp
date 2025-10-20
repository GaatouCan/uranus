#include "AbstractPlayer.h"
#include "ActorContext.h"


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

    void AbstractPlayer::SendToService(int64_t target, Message *msg) const {
        GetActorContext()->SendToService(target, msg);
    }

    void AbstractPlayer::SendToService(const std::string &name, Message *msg) const {
        GetActorContext()->SendToService(name, msg);
    }

    void AbstractPlayer::SendToClient(Message *msg) const {
        GetActorContext()->SendToClient(id_, msg);
    }
}
