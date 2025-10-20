#include "PlayerHandle.h"
#include "PlayerFactory.h"
#include "AbstractPlayer.h"


PlayerHandle::PlayerHandle()
    : plr_(nullptr),
      factory_(nullptr) {
}

PlayerHandle::PlayerHandle(AbstractPlayer *plr, PlayerFactory *factory)
    : plr_(plr),
      factory_(factory) {
}

PlayerHandle::~PlayerHandle() {
    Release();
}

PlayerHandle::PlayerHandle(PlayerHandle &&rhs) noexcept {
    plr_ = rhs.plr_;
    factory_ = rhs.factory_;
    rhs.plr_ = nullptr;
    rhs.factory_ = nullptr;
}

PlayerHandle &PlayerHandle::operator=(PlayerHandle &&rhs) noexcept {
    if (this != &rhs) {
        Release();

        plr_ = rhs.plr_;
        factory_ = rhs.factory_;
        rhs.plr_ = nullptr;
        rhs.factory_ = nullptr;
    }
    return *this;
}

bool PlayerHandle::IsValid() const {
    return plr_ != nullptr;
}

AbstractPlayer *PlayerHandle::operator->() const noexcept {
    return plr_;
}

AbstractPlayer &PlayerHandle::operator*() const noexcept {
    return *plr_;
}

AbstractPlayer *PlayerHandle::Get() const noexcept {
    return plr_;
}

bool PlayerHandle::operator==(const PlayerHandle &rhs) const noexcept {
    return plr_ == rhs.plr_ && factory_ == rhs.factory_;
}

bool PlayerHandle::operator==(nullptr_t) const noexcept {
    return plr_ == nullptr;
}

PlayerHandle::operator bool() const noexcept {
    return IsValid();
}

void PlayerHandle::Release() noexcept {
    if (plr_ == nullptr)
        return;

    if (factory_ != nullptr) {
        factory_->DestroyPlayer(plr_);
    } else {
        delete plr_;
    }

    plr_ = nullptr;
    factory_ = nullptr;
}
