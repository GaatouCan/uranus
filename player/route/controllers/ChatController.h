#pragma once

#include "../AbstractController.h"
#include "Message.h"

class GamePlayer;

namespace protocol {

    using uranus::Message;

    class ChatController final : public AbstractController {

    public:
        ChatController();
        ~ChatController() override;

        void OnRequest(GamePlayer *plr, const Message &msg);
    };
}
