#include "entity/Entity_Appearance.h"

namespace uranus::database::entity {
    Entity_Appearance::Entity_Appearance() {
    }

    Entity_Appearance::~Entity_Appearance() {
    }

    void Entity_Appearance::read(mysqlx::Row &row) {
        if (row.isNull())
            return;

        current_avatar = row[0].get<int>();
        current_frame = row[1].get<int>();
        current_avatar = row[2].get<int>();
    }
}
