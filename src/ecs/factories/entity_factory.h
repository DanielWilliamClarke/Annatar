#ifndef ECS_ENTITY_FACTORY_H
#define ECS_ENTITY_FACTORY_H

#include "../world.h"
#include "../config/config_loader.h"
#include "../systems/weapon_system.h"
#include "util/i_texture_atlas.h"
#include "util/i_random_number_source.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <memory>

namespace ecs {

/**
 * EntityFactory - Creates entities with components based on configuration
 */
class EntityFactory {
public:
    EntityFactory(World& world, const ConfigLoader& config, std::unique_ptr<IRandomNumberSource<int>> randomSource)
        : world(world), config(config), randomSource(std::move(randomSource)),textureAtlas(nullptr) {}

    // Set texture atlas for loading textures by name
    void SetTextureAtlas(std::shared_ptr<ITextureAtlas> atlas) { textureAtlas = atlas; }

    // Create player entity (loads texture from config if textureAtlas set)
    entt::entity CreatePlayer(sf::Vector2f position, sf::Texture* texture = nullptr);

    // Create enemy entity from config
    entt::entity CreateEnemy(const std::string& enemy_type, sf::Vector2f position,
                            sf::Texture* texture = nullptr);

    // Create bullet entity from weapon config
    entt::entity CreateBullet(const std::string& weapon_name, sf::Vector2f position,
                             sf::Vector2f direction, entt::entity owner,
                             bool is_player_bullet = true, sf::Texture* texture = nullptr);

    // Create bullet from spawn request (from WeaponSystem)
    entt::entity CreateBullet(const BulletSpawnRequest& request,
                             bool is_player_bullet = true, sf::Texture* texture = nullptr);

    // Create particle effect
    entt::entity CreateParticle(sf::Vector2f position, sf::Vector2f velocity,
                               sf::Color color, float lifetime, float size = 4.0f);

    // Create explosion effect
    void CreateExplosion(sf::Vector2f position, sf::Color color = sf::Color::Red,
                        int particle_count = 16);

private:
    World& world;
    const ConfigLoader& config;
    std::shared_ptr<ITextureAtlas> textureAtlas;
    const std::unique_ptr<IRandomNumberSource<int>> randomSource;

    // Helper to create weapon component from config
    Weapon CreateWeaponFromConfig(const WeaponConfig& wc);

    // Helper to create animation component from config
    Animation CreateAnimationFromConfig(const AnimationConfig& anim_cfg, sf::Texture* texture);
};

} // namespace ecs

#endif // ECS_ENTITY_FACTORY_H
