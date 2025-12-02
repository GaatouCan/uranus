#pragma once

#include "Package.h"

#include <base/network/Connection.h>

namespace uranus {

    using network::Connection;
    using network::ConnectionHandler;

    class GameWorldHandler final : public ConnectionHandler<Package> {

    public:
        explicit GameWorldHandler(Connection& conn);
        ~GameWorldHandler() override;

        void onReceive(HandleType &&msg) override;
        void onWrite(Type *msg) override;
    };
}