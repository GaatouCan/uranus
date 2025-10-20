#pragma once

#include "../other/SharedLibrary.h"
#include "ServiceHandle.h"

#include <unordered_map>


class ServiceFactory final {

    using ServiceCreator = AbstractService *(*)();
    using ServiceDestroyer = void (*)(AbstractService *);

    struct LibraryNode {
        SharedLibrary       library;
        ServiceCreator      creator;
        ServiceDestroyer    destroyer;
    };

public:
    void LoadService() override;

    [[nodiscard]] ServiceHandle CreateInstance(const std::string &path) override;
    void DestroyInstance(AbstractService *pService, const std::string &path) override;

private:
    std::unordered_map<std::string, LibraryNode> core_map_;
    std::unordered_map<std::string, LibraryNode> extend_map_;
};
