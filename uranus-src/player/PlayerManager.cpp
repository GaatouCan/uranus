#include "PlayerManager.h"
#include "PlayerContext.h"
#include "GameWorld.h"
#include "gateway/Gateway.h"
#include "gateway/ClientConnection.h"
#include "factory/PlayerFactory.h"

#include <actor/BasePlayer.h>
#include <login/data_asset/DA_PlayerResult.h>
#include <database/DatabaseModule.h>
#include <login/LoginAuth.h>
#include <spdlog/spdlog.h>


namespace uranus {

    using actor::BaseActor;
    using database::DatabaseModule;
    using login::DA_PlayerResult;

    PlayerManager::PlayerManager(GameWorld &world)
        : world_(world) {
        SPDLOG_DEBUG("PlayerManager created");
    }

    PlayerManager::~PlayerManager() {
        players_.clear();

        SPDLOG_DEBUG("PlayerManager destroyed");
    }

    GameWorld &PlayerManager::getWorld() const {
        return world_;
    }

    void PlayerManager::start() {
        // Initial the player manager
        PLAYER_FACTORY.initial();
    }

    void PlayerManager::stop() {
    }

    void PlayerManager::onPlayerLogin(const int64_t pid, const shared_ptr<ClientConnection> &client) {
        if (!world_.isRunning())
            return;

        if (client == nullptr || pid < 0)
            return;

        // Maybe login repeated
        if (const auto plr = find(pid); plr) {
            client->attr().erase("WAITING_DB");
            login::LoginAuth::sendLoginProcessInfo(client, pid, "Reconnect to player actor");
            SPDLOG_WARN("Player[{}] already exists!", pid);
            return;
        }

        auto [plr, path] = PLAYER_FACTORY.create();

        if (!plr)
            return;

        auto handle = ActorHandle(plr, [](BaseActor *ptr) {
            if (!ptr)
                return;

            if (auto *temp = dynamic_cast<BasePlayer *>(ptr)) {
                PLAYER_FACTORY.destroy(temp);
                return;
            }

            delete ptr;
        });

        auto strand = asio::make_strand(world_.getWorkerIOContext());
        auto ctx = std::make_shared<PlayerContext>(strand, std::move(handle));
        SPDLOG_INFO("Player[{}] context created", pid);

        // In normal, that would not get the repeated one
        shared_ptr<PlayerContext> old;

        {
            unique_lock lock(mutex_);

            // If the same player id actor has exists
            if (const auto it = players_.find(pid); it != players_.end()) {
                old = it->second;
                players_.erase(it);
            }

            players_.insert_or_assign(pid, ctx);
        }

        // Stop the old one
        if (old) {
            SPDLOG_WARN("Player[{}] actor repeated!", pid);
            old->terminate();
        }

        ctx->setPlayerId(pid);
        ctx->attr().set("LIBRARY_PATH", path.string());
        ctx->setPlayerManager(this);

        SPDLOG_INFO("Add player[{}] to PlayerManager", pid);

        if (auto *db = GET_MODULE(&world_, DatabaseModule)) {
            login::LoginAuth::sendLoginProcessInfo(client, pid, "Acquire player data from database");
            SPDLOG_INFO("Acquire player[{}] data from database", pid);

            db->queryPlayer(pid, [&world = world_, pid](const std::string &res) {
                if (res.empty())
                    return;

                // Run in the main thread
                asio::post(world.getIOContext(), [&world = world, pid, res] {
                    if (const auto *mgr = GET_MODULE(&world, PlayerManager)) {
                        mgr->onPlayerData(pid, res);
                    }
                });
            });
        }
    }

    void PlayerManager::onPlayerData(const int64_t pid, const std::string &str) const {
        if (!world_.isRunning())
            return;

        if (const auto *gateway = GET_MODULE(&world_, Gateway)) {
            if (const auto client = gateway->find(pid)) {
                asio::dispatch(client->socket().get_executor(), [&client, pid] {
                    client->attr().erase("WAITING_DB");
                    login::LoginAuth::sendLoginProcessInfo(client, pid, "Acquire player data success");
                });
            }
        }

        if (const auto plr = find(pid)) {
            auto data = make_unique<DA_PlayerResult>();
            data->data = nlohmann::json::parse(str);

            SPDLOG_INFO("Acquire player[{}] data success", pid);
            plr->run(std::move(data));
        }
    }

    void PlayerManager::onPlayerLogout(const int64_t pid) {
        if (!world_.isRunning())
            return;

        shared_ptr<PlayerContext> ctx;

        {
            unique_lock lock(mutex_);

            if (const auto it = players_.find(pid); it != players_.end()) {
                ctx = it->second;
                players_.erase(it);
            }
        }

        if (ctx) {
            SPDLOG_INFO("Remove player[{}]", pid);
            ctx->terminate();
        }
    }

    shared_ptr<PlayerContext> PlayerManager::find(const int64_t pid) const {
        if (!world_.isRunning())
            return nullptr;

        shared_lock lock(mutex_);
        const auto it = players_.find(pid);
        return it != players_.end() ? it->second : nullptr;
    }

    set<shared_ptr<PlayerContext>> PlayerManager::getPlayerSet(const set<int64_t> &pids) const {
        if (!world_.isRunning())
            return {};

        set<shared_ptr<PlayerContext>> result;
        shared_lock lock(mutex_);

        for (const auto pid : pids) {
            if (const auto it = players_.find(pid); it != players_.end()) {
                result.insert(it->second);
            }
        }

        return result;
    }
} // uranus
