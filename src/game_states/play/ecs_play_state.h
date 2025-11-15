#ifndef ECS_PLAY_STATE_H
#define ECS_PLAY_STATE_H

#include "game_states/game_states.h"
#include "state/state.h"
#include "ecs/ecs.h"
#include "ecs/config/config_loader.h"
#include "ecs/factories/entity_factory.h"
#include "ecs/systems/enemy_spawn_system.h"
#include <memory>

class ITextureAtlas;
class IRenderer;
class PlayerInput;

/**
 * ECSPlayState - Main gameplay state using ECS architecture
 * Parallel implementation to PlayState for gradual migration
 */
class ECSPlayState : public State<GameStates> {
public:
    ECSPlayState(
        std::shared_ptr<ITextureAtlas> textureAtlas,
        sf::FloatRect bounds
    );
    ~ECSPlayState() override = default;

    void Update(float dt) override;
    void Draw(const std::shared_ptr<IRenderer>& renderer, float interp) const override;

protected:
    void Setup() override;
    void TearDown() override;

private:
    // ECS core
    ecs::World world;
    ecs::ConfigLoader config;
    std::unique_ptr<ecs::EntityFactory> factory;

    // Game resources
    std::shared_ptr<ITextureAtlas> textureAtlas;
    sf::FloatRect bounds;
    float worldSpeed;

    // Entities
    entt::entity player;

    // Helper methods
    void HandleCollision(entt::entity a, entt::entity b, const sf::Vector2f& collision_point);
    void CleanupDeadEntities();
    void CleanupExpiredEntities(float dt);
    void SpawnEnemy(const std::string& type, sf::Vector2f position);
    bool PlayerDied() const;
};

#endif // ECS_PLAY_STATE_H
