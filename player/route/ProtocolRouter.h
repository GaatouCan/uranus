#pragma once

#include "AbstractController.h"
#include "Message.h"

#include <functional>
#include <memory>
#include <unordered_map>

#include "../GamePlayer.h"

class GamePlayer;

namespace protocol {

    using uranus::Message;
    using std::function;
    using std::unordered_map;
    using std::unique_ptr;
    using std::make_unique;

    using ProtocolFunctor = function<void(AbstractController *, GamePlayer *, const Message &)>;

    class ProtocolRouter final {

    public:
        explicit ProtocolRouter(GamePlayer *plr);
        ~ProtocolRouter();

        DISABLE_COPY_MOVE(ProtocolRouter)

    private:
        GamePlayer *const player_;
        unordered_map<int, unique_ptr<AbstractController>> controllers_;
        unordered_map<int, ProtocolFunctor> functors_;
    };
}
