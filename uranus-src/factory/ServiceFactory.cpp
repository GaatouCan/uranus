#include "ServiceFactory.h"

#include <actor/BaseService.h>
#include <base/utils.h>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace uranus {

    static constexpr auto kCoreServiceDirectory = "core";
    static constexpr auto kExtendServiceDirectory = "extend";

#if defined(__APPLE__) || defined(__linux__)
    static constexpr auto kLibraryPrefix = "lib";
#endif


    ServiceFactory::ServiceFactory() {
    }

    ServiceFactory::~ServiceFactory() {
        coreServices_.clear();
        extendServices_.clear();
    }

    void ServiceFactory::initial() {
        if (const auto dir = std::filesystem::path(kCoreServiceDirectory); !std::filesystem::exists(dir)) {
            try {
                std::filesystem::create_directories(dir);
            } catch (const std::filesystem::filesystem_error& e) {
                SPDLOG_ERROR(e.what());
                exit(-1);
            }
        }

        if (const auto dir = std::filesystem::path(kExtendServiceDirectory); !std::filesystem::exists(dir)) {
            try {
                std::filesystem::create_directories(dir);
            } catch (const std::filesystem::filesystem_error& e) {
                SPDLOG_ERROR(e.what());
                exit(-2);
            }
        }

        for (const auto &entry: std::filesystem::directory_iterator(kCoreServiceDirectory)) {
#if defined(_WIN32) || defined(_WIN64)
            if (entry.is_regular_file() && entry.path().extension() == ".dll") {
                const auto filename = entry.path().stem().string();
#elif defined(__APPLE__)
            if (entry.is_regular_file() && entry.path().extension() == ".dylib") {
                auto filename = entry.path().stem().string();
                if (filename.compare(0, strlen(kLibraryPrefix), kLibraryPrefix) == 0) {
                    filename.erase(0, strlen(kLibraryPrefix));
                }
#endif
                SharedLibrary library(entry.path());

                if (!library.available()) {
                    SPDLOG_ERROR("{} - Load core library[{}] failed",
                        __FUNCTION__, entry.path().string());
                    continue;
                }

                {
                    using actor::ActorVersion;
                    using actor::kUranusActorABIVersion;
                    using actor::kUranusActorAPIVersion;
                    using actor::kUranusActorHeaderVersion;
                    using VersionGetter = const ActorVersion* (*)();

                    auto *getter = library.getSymbol<VersionGetter>("GetActorVersion");
                    if (getter == nullptr) {
                        SPDLOG_ERROR("Failed to get service[{}] library version", entry.path().string());
                        exit(-2);
                    }

                    const auto *ver = getter();
                    if (ver == nullptr) {
                        SPDLOG_ERROR("Failed to get service[{}] library version", entry.path().string());
                        exit(-2);
                    }

                    if (ver->abi_version != kUranusActorABIVersion ||
                        ver->api_version != kUranusActorAPIVersion ||
                        ver->header_version != kUranusActorHeaderVersion
                    ) {
                        SPDLOG_ERROR("Service[{}] library version is not match!!!", entry.path().string());
                        exit(-2);
                    }
                }

                const auto creator = library.getSymbol<ServiceCreator>("CreateInstance");
                const auto deleter = library.getSymbol<ServiceDeleter>("DeleteInstance");

                if (creator == nullptr || deleter == nullptr) {
                    SPDLOG_ERROR("{} - Load core library[{}] symbol failed",
                        __FUNCTION__, entry.path().string());
                    continue;
                }

                ServiceNode node;

                node.lib = library;
                node.ctor = creator;
                node.del = deleter;

                coreServices_.insert_or_assign(filename, node);

                SPDLOG_INFO("{} - Loaded core service[{}]",
                    __FUNCTION__, filename);
            }
        }

        for (const auto &entry: std::filesystem::directory_iterator(kExtendServiceDirectory)) {
#if defined(_WIN32) || defined(_WIN64)
            if (entry.is_regular_file() && entry.path().extension() == ".dll") {
                const auto filename = entry.path().stem().string();
#elif defined(__APPLE__)
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                auto filename = entry.path().stem().string();
                if (filename.compare(0, strlen(kLibraryPrefix), kLibraryPrefix) == 0) {
                    filename.erase(0, strlen(kLibraryPrefix));
                }
#endif
                SharedLibrary library(entry.path());

                if (!library.available()) {
                    SPDLOG_ERROR("{} - Load extend library[{}] failed",
                        __FUNCTION__, entry.path().string());
                    continue;
                }

                const auto creator = library.getSymbol<ServiceCreator>("CreateInstance");
                const auto deleter = library.getSymbol<ServiceDeleter>("DeleteInstance");

                if (creator == nullptr || deleter == nullptr) {
                    SPDLOG_ERROR("{} - Load extend library[{}] symbol failed",
                        __FUNCTION__, entry.path().string());
                    continue;
                }

                ServiceNode node;

                node.lib = library;
                node.ctor = creator;
                node.del = deleter;

                extendServices_.insert_or_assign(filename, node);

                SPDLOG_INFO("{} - Loaded extend service[{}]",
                    __FUNCTION__, filename);
            }
        }
    }

    ServiceFactory::InstanceResult ServiceFactory::create(const std::string &path) const {
        bool isCore = false;
        std::string filename;

        utils::DivideString(path, '.', [&](const std::string_view lhs, const std::string_view rhs) {
            if (rhs.empty()) {
                SPDLOG_ERROR("filename incorrect");
                return;
            }

            isCore = lhs == kCoreServiceDirectory;
            filename = rhs;
        });

        if (filename.empty()) {
            return make_tuple(nullptr, std::filesystem::path{});
        }

        if (isCore) {
            if (const auto iter = coreServices_.find(filename); iter != coreServices_.end()) {
                auto *inst = std::invoke(iter->second.ctor);
                return make_tuple(inst, iter->second.lib.path());
            }

        } else {
            if (const auto iter = extendServices_.find(filename); iter != extendServices_.end()) {
                auto *inst = std::invoke(iter->second.ctor);
                return make_tuple(inst, iter->second.lib.path());
            }
        }

        return make_tuple(nullptr, std::filesystem::path{});
    }

    void ServiceFactory::destroy(BaseService *ptr, const std::string &path) {
        if (!ptr)
            return;

        bool isCore = false;
        std::string filename;

        utils::DivideString(path, '.', [&](const std::string_view lhs, const std::string_view rhs) {
            if (rhs.empty()) {
                SPDLOG_ERROR("filename incorrect");
                return;
            }

            isCore = lhs == kCoreServiceDirectory;
            filename = rhs;
        });

        if (filename.empty()) {
            delete ptr;
            return;
        }

        if (isCore) {
            if (const auto iter = coreServices_.find(filename); iter != coreServices_.end()) {
                std::invoke(iter->second.del, ptr);
                return;
            }
        } else {
            if (const auto iter = extendServices_.find(filename); iter != extendServices_.end()) {
                std::invoke(iter->second.del, ptr);
                return;
            }
        }

        delete ptr;
    }
} // uranus
