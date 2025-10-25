#pragma once

#include "AbstractController.h"
#include "Message.h"
#include "ControllerType.h"


#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>


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

        template<class T>
        requires std::is_base_of_v<AbstractController, T>
        void Register(int id, void(T::*fn)(GamePlayer *, const Message &));

        void Dispatch(int id, const Message &msg);

    private:
        static unique_ptr<AbstractController> CreateController(ControllerType type);

    private:
        GamePlayer *const player_;
        unordered_map<int, unique_ptr<AbstractController>> controllers_;
        unordered_map<int, ProtocolFunctor> functors_;
    };

    template<class T>
    requires std::is_base_of_v<AbstractController, T>
    void ProtocolRouter::Register(const int id, void(T::*fn)(GamePlayer *, const Message &)) {
        constexpr int min = static_cast<int>(ControllerType::kMinimum) * 100;
        constexpr int max = static_cast<int>(ControllerType::kMaximum) * 100;

        if (id <= min || id >= max) {
            throw std::out_of_range("ProtocolRouter::Register(): id is out of range");
        }

        if (const auto type = (id / 100); !controllers_.contains(type)) {
            auto ctrl = CreateController(static_cast<ControllerType>(type));

            if (ctrl == nullptr) {
                throw std::out_of_range("ProtocolRouter::Register(): Could not create controller");
            }

            controllers_.insert_or_assign(type, std::move(ctrl));
        }

        functors_[id] = [fn](AbstractController *ctrl, GamePlayer *plr, const Message &msg) {
            if (auto *temp = dynamic_cast<T *>(ctrl)) {
                (temp->*fn)(plr, msg);
            }
        };
    }
}
