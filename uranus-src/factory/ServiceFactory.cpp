#include "ServiceFactory.h"
#include "AbstractService.h"
#include "base/Utils.h"

#include <set>
#include <filesystem>
#include <spdlog/spdlog.h>


inline constexpr auto kLinuxLibraryPrefix = "lib";


#define CREATE_LOCAL_SERVICE(name, clazz) \
if (res[1] == #name) { \
    auto *pService = new clazz(); \
    return { pService, this, path }; \
}

void ServiceFactory::LoadService() {
    for (const auto &entry: std::filesystem::directory_iterator(kCoreServiceDirectory)) {
#if defined(_WIN32) || defined(_WIN64)
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            const auto filename = entry.path().stem().string();
#elif
        if (entry.is_regular_file() && entry.path().extension() == ".so") {
            auto filename = entry.path().stem().string();
            if (filename.compare(0, strlen(kLinuxLibraryPrefix), kLinuxLibraryPrefix) == 0) {
                filename.erase(0, strlen(kLinuxLibraryPrefix));
            }
#endif
            SharedLibrary library(entry.path());

            if (!library.IsValid()) {
                SPDLOG_ERROR("{} - Load core library[{}] failed",
                    __FUNCTION__, entry.path().string());
                continue;
            }

            const auto creator = library.GetSymbol<ServiceCreator>("CreateInstance");
            const auto destroyer = library.GetSymbol<ServiceDestroyer>("DestroyInstance");

            if (creator == nullptr || destroyer == nullptr) {
                SPDLOG_ERROR("{} - Load core library[{}] symbol failed",
                    __FUNCTION__, entry.path().string());
                continue;
            }

            LibraryNode node;

            node.library = library;
            node.creator = creator;
            node.destroyer = destroyer;

            core_map_.insert_or_assign(filename, node);

            SPDLOG_INFO("{} - Loaded core service[{}]",
                __FUNCTION__, filename);
        }
    }

    for (const auto &entry: std::filesystem::directory_iterator(kExtendServiceDirectory)) {
#if defined(_WIN32) || defined(_WIN64)
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            const auto filename = entry.path().stem().string();
#elif
        if (entry.is_regular_file() && entry.path().extension() == ".so") {
            auto filename = entry.path().stem().string();
            if (filename.compare(0, strlen(kLinuxLibraryPrefix), kLinuxLibraryPrefix) == 0) {
                filename.erase(0, strlen(kLinuxLibraryPrefix));
            }
#endif
            SharedLibrary library(entry.path());

            if (!library.IsValid()) {
                SPDLOG_ERROR("{} - Load extend library[{}] failed",
                    __FUNCTION__, entry.path().string());
                continue;
            }

            const auto creator = library.GetSymbol<ServiceCreator>("CreateInstance");
            const auto destroyer = library.GetSymbol<ServiceDestroyer>("DestroyInstance");

            if (creator == nullptr || destroyer == nullptr) {
                SPDLOG_ERROR("{} - Load extend library[{}] symbol failed",
                    __FUNCTION__, entry.path().string());
                continue;
            }

            LibraryNode node;
            node.library = library;
            node.creator = creator;
            node.destroyer = destroyer;

            extend_map_.insert_or_assign(filename, node);

            SPDLOG_INFO("{} - Loaded extend service[{}]",
                __FUNCTION__, filename);
        }
    }
}

ServiceHandle ServiceFactory::CreateInstance(const std::string &path) {
    const std::vector<std::string> res = utils::SplitString(path, '.');

    if (res.size() != 2)
        return {};

    if (res[0] == "core") {
        if (const auto iter = core_map_.find(res[1]); iter != core_map_.end()) {
            if (const auto creator = iter->second.creator; creator != nullptr) {
                SPDLOG_DEBUG("{} - Create core service[{}]",
                    __FUNCTION__, path);

                auto *pService = std::invoke(creator);
                return { pService, this, path };
            }
        }
    }

    if (res[0] == "extend") {
        if (const auto iter = extend_map_.find(res[1]); iter != extend_map_.end()) {
            if (const auto creator = iter->second.creator; creator != nullptr) {
                SPDLOG_DEBUG("{} - Create extend service[{}]",
                    __FUNCTION__, path);

                auto *pService = std::invoke(creator);
                return { pService, this, path };
            }
        }
    }

    return {};
}

void ServiceFactory::DestroyInstance(AbstractService *pService, const std::string &path) {
    const std::vector<std::string> res = utils::SplitString(path, '.');

    if (res.size() != 2) {
        SPDLOG_WARN("{} - Parse path[{}] failed, use default deleter",
            __FUNCTION__, path);
        delete pService;
        return;
    }

    if (res[0] == "core") {

        // const std::set<std::string> set = {
        //     "gameworld"
        // };
        //
        // if (set.contains(res[1])) {
        //     delete pService;
        //     return;
        // }

        if (const auto iter = core_map_.find(res[1]); iter != core_map_.end()) {
            if (const auto destroyer = iter->second.destroyer; destroyer != nullptr) {
                SPDLOG_DEBUG("{} - Destroy core service[{}]",
                    __FUNCTION__, path);

                std::invoke(destroyer, pService);
                return;
            }
        }
    }

    if (res[0] == "extend") {
        if (const auto iter = extend_map_.find(res[1]); iter != extend_map_.end()) {
            if (const auto destroyer = iter->second.destroyer; destroyer != nullptr) {
                SPDLOG_DEBUG("{} - Destroy extend service[{}]",
                    __FUNCTION__, path);

                std::invoke(destroyer, pService);
                return;
            }
        }
    }

    SPDLOG_WARN("{} - Failed to find path[{}], use default deleter",
        __FUNCTION__, path);

    delete pService;
}


#undef CREATE_LOCAL_SERVICE