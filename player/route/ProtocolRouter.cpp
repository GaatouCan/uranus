#include "ProtocolRouter.h"
#include "ControllerType.h"
#include "controllers/ChatController.h"

namespace protocol {

    const unordered_map<ControllerType, function<unique_ptr<AbstractController>()>> gControllerCreator = {

        { ControllerType::kChat, []{ return make_unique<ChatController>(); }},

    };

    ProtocolRouter::ProtocolRouter(GamePlayer *plr)
        : player_(plr) {
    }

    ProtocolRouter::~ProtocolRouter() {
    }
}
