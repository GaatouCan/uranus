#pragma once

#include <mysqlx/xdevapi.h>

namespace uranus::database {

    class DatabaseModule;

    class Entity {

        friend class DatabaseModule;

    public:
        Entity() = default;
        virtual ~Entity() = default;

    protected:
        virtual void read(mysqlx::Row &row) = 0;
    };
}