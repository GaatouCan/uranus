#include "ProtocolRouter.h"
#include "ControllerType.h"
#include "controllers/ChatController.h"

#define REGISTER_CONTROLLER(ty, ctrl) \
    case ControllerType::ty: return make_unique<ctrl>();

namespace protocol {
    unique_ptr<AbstractController> ProtocolRouter::CreateController(const ControllerType type) {
        switch (type) {
            REGISTER_CONTROLLER(kChat, ChatController)
            default: return nullptr;
        }
    }

    ProtocolRouter::ProtocolRouter(GamePlayer *plr)
        : player_(plr) {

        // Begin register protocol here
        Register(1101, &ChatController::OnRequest);
    }

    ProtocolRouter::~ProtocolRouter() {
    }

    void ProtocolRouter::Dispatch(const int id, const Message &msg) {
        constexpr int min = static_cast<int>(ControllerType::kMinimum) * 100;
        constexpr int max = static_cast<int>(ControllerType::kMaximum) * 100;

        if (id <= min || id >= max) {
            //throw std::out_of_range("ProtocolRouter::Register(): id is out of range");
            return;
        }

        const auto type = id / 100;
        const auto iter = controllers_.find(type);
        if (iter == controllers_.end())
            return;

        auto *ctrl = iter->second.get();

        const auto func_iter = functors_.find(id);
        if (func_iter == functors_.end())
            return;

        std::invoke(func_iter->second, ctrl, player_, msg);
    }
}


#undef REGISTER_CONTROLLER