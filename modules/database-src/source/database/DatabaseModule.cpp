#include "DatabaseModule.h"


namespace uranus::database {
    DatabaseModule::DatabaseModule() {
    }

    DatabaseModule::~DatabaseModule() {
    }

    void DatabaseModule::start() {

    }

    void DatabaseModule::stop() {
    }

    void DatabaseModule::onQueryResult(const ResultCallback &cb) {
        onResult_ = cb;
    }
}
