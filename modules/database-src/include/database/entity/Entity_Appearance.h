#pragma once

#include "database/database.export.h"
#include "database/Entity.h"

namespace uranus::database::entity {

    class DATABASE_API Entity_Appearance final : public Entity {

    public:
        Entity_Appearance();
        ~Entity_Appearance() override;

    protected:
        void read(mysqlx::Row &row) override;

    public:
        int current_avatar = 0;
        int current_frame = 0;
        int current_background = 0;
    };
}
