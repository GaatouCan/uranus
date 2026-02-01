#include "WorldMonitor.h"

#include <spdlog/spdlog.h>

namespace uranus {
    WorldMonitor::WorldMonitor(GameWorld &world)
        : world_(world) {
        SPDLOG_DEBUG("WorldMonitor created");
    }

    WorldMonitor::~WorldMonitor() {
        SPDLOG_DEBUG("WorldMonitor destroyed");
    }

    void WorldMonitor::start() {
    }

    void WorldMonitor::stop() {
    }
} // uranus
