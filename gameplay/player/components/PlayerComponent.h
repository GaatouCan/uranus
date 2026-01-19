#pragma once

#include <base/noncopy.h>
#include <cstdint>

namespace gameplay {

    class ComponentModule;
    class GamePlayer;

    class PlayerComponent {

    public:
        PlayerComponent() = delete;

        explicit PlayerComponent(ComponentModule &module);
        virtual ~PlayerComponent();

        DISABLE_COPY_MOVE(PlayerComponent)

        [[nodiscard]] virtual const char *getComponentName() const = 0;

        [[nodiscard]] ComponentModule &getComponentModule() const;
        [[nodiscard]] GamePlayer &getPlayer() const;
        [[nodiscard]] int64_t getPlayerId() const;

        virtual void onLogin();
        virtual void onLogout();

    private:
        ComponentModule &module_;
    };

#define SEND_TO_CLIENT(plr, id, msg)                                    \
{                                                                       \
    auto pkg = uranus::actor::Package::getHandle();                     \
    pkg->setId(static_cast<int64_t>(protocol::ProtocolID::id));         \
                                                                        \
    pkg->payload_.resize((msg).ByteSizeLong());                         \
    (msg).SerializeToArray(pkg->payload_.data(), pkg->payload_.size()); \
                                                                        \
    (plr).sendToClient(std::move(pkg));                                 \
}

}