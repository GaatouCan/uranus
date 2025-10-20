#pragma once

#include "Common.h"

#include <cstddef>


namespace uranus {
    class AbstractPlayer;
}
class PlayerFactory;

using uranus::AbstractPlayer;


class PlayerHandle final {

public:
    PlayerHandle();

    PlayerHandle(AbstractPlayer *plr, PlayerFactory *factory);
    ~PlayerHandle();

    DISABLE_COPY(PlayerHandle)

    PlayerHandle(PlayerHandle &&rhs) noexcept;
    PlayerHandle &operator=(PlayerHandle &&rhs) noexcept;

    [[nodiscard]] bool IsValid() const;

    AbstractPlayer *operator->() const noexcept;
    AbstractPlayer &operator*() const noexcept;

    [[nodiscard]] AbstractPlayer *Get() const noexcept;

    bool operator==(const PlayerHandle &rhs) const noexcept;
    bool operator==(nullptr_t) const noexcept;

    explicit operator bool() const noexcept;

    void Release() noexcept;

private:
    AbstractPlayer *plr_;
    PlayerFactory *factory_;
};
