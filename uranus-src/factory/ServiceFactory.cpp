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


    static bool VerifyLibraryVersion(const SharedLibrary &lib, const std::string &path) {
        using actor::ActorVersion;
        using actor::kUranusActorABIVersion;
        using actor::kUranusActorAPIVersion;
        using actor::kUranusActorHeaderVersion;
        using VersionGetter = const ActorVersion* (*)();

        auto *getter = lib.getSymbol<VersionGetter>("GetActorVersion");
        if (getter == nullptr) {
            SPDLOG_ERROR("Failed to get service[{}] library version", path);
            return false;
        }

        const auto *ver = getter();
        if (ver == nullptr) {
            SPDLOG_ERROR("Failed to get service[{}] library version", path);
            return false;
        }

        if (ver->abi_version != kUranusActorABIVersion ||
            ver->api_version != kUranusActorAPIVersion ||
            ver->header_version != kUranusActorHeaderVersion
        ) {
            SPDLOG_ERROR("Service[{}] library version is not match!!!", path);
            return false;
        }

        return true;
    }

    ServiceFactory::ServiceFactory() {
    }

    ServiceFactory::~ServiceFactory() {
        coreMap_.clear();
        extendMap_.clear();
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
                    SPDLOG_ERROR("Load core library[{}] failed", entry.path().string());
                    continue;
                }

                if (!VerifyLibraryVersion(library, entry.path().string())) {
                    exit(-3);
                }

                const auto creator = library.getSymbol<ServiceCreator>("CreateInstance");
                const auto deleter = library.getSymbol<ServiceDeleter>("DeleteInstance");

                if (creator == nullptr || deleter == nullptr) {
                    SPDLOG_ERROR("Load core library[{}] symbol failed", entry.path().string());
                    continue;
                }

                ServiceNode node;

                node.library = library;
                node.creator = creator;
                node.deleter = deleter;

                coreMap_.insert_or_assign(filename, std::move(node));
                SPDLOG_INFO("Loaded core service[{}]", filename);
            }
        }

        for (const auto &entry: std::filesystem::directory_iterator(kExtendServiceDirectory)) {
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
                    SPDLOG_ERROR("Load extend library[{}] failed", entry.path().string());
                    continue;
                }

                if (!VerifyLibraryVersion(library, entry.path().string())) {
                    exit(-4);
                }

                const auto creator = library.getSymbol<ServiceCreator>("CreateInstance");
                const auto deleter = library.getSymbol<ServiceDeleter>("DeleteInstance");

                if (creator == nullptr || deleter == nullptr) {
                    SPDLOG_ERROR("Load extend library[{}] symbol failed", entry.path().string());
                    continue;
                }

                ServiceNode node;

                node.library = library;
                node.creator = creator;
                node.deleter = deleter;

                extendMap_.insert_or_assign(filename, std::move(node));

                SPDLOG_INFO("Loaded extend service[{}]", filename);
            }
        }
    }

    ServiceFactory::InstanceResult ServiceFactory::create(const std::string &path) {
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

        {
            shared_lock lock(mutex_);
            if (isCore) {
                if (const auto iter = coreMap_.find(filename); iter != coreMap_.end()) {
                    iter->second.count.fetch_add(1, std::memory_order_release);
                    auto *inst = std::invoke(iter->second.creator);
                    return make_tuple(inst, std::filesystem::path{});
                }
            } else {
                if (const auto iter = extendMap_.find(filename); iter != extendMap_.end()) {
                    iter->second.count.fetch_add(1, std::memory_order_release);
                    auto *inst = std::invoke(iter->second.creator);
                    return make_tuple(inst, std::filesystem::path{});
                }
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

        {
            shared_lock lock(mutex_);
            if (isCore) {
                if (const auto iter = coreMap_.find(filename); iter != coreMap_.end()) {
                    std::invoke(iter->second.deleter, ptr);
                    iter->second.count.fetch_sub(1, std::memory_order_release);
                    return;
                }
            } else {
                if (const auto iter = extendMap_.find(filename); iter != extendMap_.end()) {
                    std::invoke(iter->second.deleter, ptr);
                    iter->second.count.fetch_sub(1, std::memory_order_release);
                    return;
                }
            }
        }

        delete ptr;
    }

    void ServiceFactory::release(const std::string &path) {
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

        if (filename.empty())
            return;

        ServiceNode node;

        {
            unique_lock lock(mutex_);
            if (isCore) {
                if (const auto iter = coreMap_.find(filename); iter != coreMap_.end()) {
                    if (iter->second.count.load(std::memory_order_acquire) > 0) {
                        SPDLOG_WARN("Service[{}] still in use", path);
                        return;
                    }

                    node = std::move(iter->second);
                    coreMap_.erase(iter);
                }
            } else {
                if (const auto iter = extendMap_.find(filename); iter != extendMap_.end()) {
                    if (iter->second.count.load(std::memory_order_acquire) > 0) {
                        SPDLOG_WARN("Service[{}] still in use", path);
                        return;
                    }

                    node = std::move(iter->second);
                    extendMap_.erase(iter);
                }
            }
        }

        if (!(node.library.available() && node.creator && node.deleter && node.count.load(std::memory_order_acquire) == 0)) {
            SPDLOG_WARN("Service[{}] not found", path);
            return;
        }

        if (node.library.tryRelease()) {
            SPDLOG_INFO("Release service[{}] success", path);
        }
    }

    void ServiceFactory::reload(const std::string &path) {
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

        if (filename.empty())
            return;

        {
            shared_lock lock(mutex_);
            if (isCore) {
                if (coreMap_.contains(filename)) {
                    SPDLOG_WARN("Service[{}] already loaded", path);
                    return;
                }
            } else {
                if (extendMap_.contains(filename)) {
                    SPDLOG_WARN("Service[{}] already loaded", path);
                    return;
                }
            }
        }

        const auto realpath = std::string(kCoreServiceDirectory) + "/" + filename + ".dylib";
        SharedLibrary library(realpath);

        if (!library.available()) {
            SPDLOG_ERROR("Load core library[{}] failed", path);
            return;
        }

        if (!VerifyLibraryVersion(library, realpath))
            return;

        const auto creator = library.getSymbol<ServiceCreator>("CreateInstance");
        const auto deleter = library.getSymbol<ServiceDeleter>("DeleteInstance");

        if (creator == nullptr || deleter == nullptr) {
            SPDLOG_ERROR("Load core library[{}] symbol failed", realpath);
            return;
        }

        ServiceNode node;

        node.library = library;
        node.creator = creator;
        node.deleter = deleter;

        {
            unique_lock lock(mutex_);
            if (isCore) {
                coreMap_.insert_or_assign(filename, std::move(node));
            } else {
                extendMap_.insert_or_assign(filename, std::move(node));
            }
        }

        SPDLOG_INFO("Reload service[{}] success", path);
    }

    ServiceFactory::ServiceNode::ServiceNode() {

    }

    ServiceFactory::ServiceNode::~ServiceNode() {

    }

    ServiceFactory::ServiceNode::ServiceNode(const ServiceNode &rhs) {
        library = rhs.library;
        creator = rhs.creator;
        deleter = rhs.deleter;
        count.store(rhs.count.load(), std::memory_order_relaxed);
    }

    ServiceFactory::ServiceNode &ServiceFactory::ServiceNode::operator=(const ServiceNode &rhs) {
        if (this != &rhs) {
            library = rhs.library;
            creator = rhs.creator;
            deleter = rhs.deleter;
        }

        return *this;
    }

    ServiceFactory::ServiceNode::ServiceNode(ServiceNode &&rhs) noexcept {
        library = std::move(rhs.library);
        creator = rhs.creator;
        deleter = rhs.deleter;

        count.store(rhs.count.load(), std::memory_order_relaxed);
        rhs.count.store(0, std::memory_order_relaxed);
    }

    ServiceFactory::ServiceNode &ServiceFactory::ServiceNode::operator=(ServiceNode &&rhs) noexcept {
        if (this != &rhs) {
            library = std::move(rhs.library);
            creator = rhs.creator;
            deleter = rhs.deleter;

            count.store(rhs.count.load(), std::memory_order_relaxed);
            rhs.count.store(0, std::memory_order_relaxed);
        }

        return *this;
    }
} // uranus
