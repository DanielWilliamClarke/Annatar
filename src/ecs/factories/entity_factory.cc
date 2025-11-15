#include "entity_factory.h"
#include "util/i_texture_atlas.h"
#include <cmath>
#include <iostream>

namespace ecs {

entt::entity EntityFactory::CreatePlayer(sf::Vector2f position, sf::Texture* texture) {
    const auto& constants = config.GetConstants();
    const auto& player_cfg = config.GetPlayerConfig().ship;  // Load from player.toml!

    auto entity = world.CreateEntity();

    // Get texture from config if textureAtlas is available
    if (!texture && textureAtlas) {
        texture = textureAtlas->GetTexture(player_cfg.sprite_sheet).get();
    }

    // Transform
    world.AddComponent<Transform>(entity, Transform{
        .position = position,
        .last_position = position,
        .velocity = {0.0f, 0.0f},
        .rotation = 0.0f,
        .scale = 2.0f  // 2x scale for player (Gradius-style larger ship!)
    });

    // Calculate frame size from config (not hardcoded!)
    sf::Vector2i frame_size(constants.player_size.x, constants.player_size.y);
    if (texture && player_cfg.animation.cols > 0 && player_cfg.animation.rows > 0) {
        auto texture_size = texture->getSize();
        frame_size.x = texture_size.x / player_cfg.animation.cols;
        frame_size.y = texture_size.y / player_cfg.animation.rows;
    }

    // Sprite
    world.AddComponent<Sprite>(entity, Sprite{
        .texture = texture,
        .texture_rect = sf::IntRect(0, 0, frame_size.x, frame_size.y),  // First frame
        .color = sf::Color::White,
        .size = sf::Vector2f(frame_size.x, frame_size.y),
        .origin = sf::Vector2f(frame_size.x / 2.0f, frame_size.y / 2.0f),
        .layer = 10,  // TODO: Move to config
        .visible = true
    });

    // Animation - loaded from player.toml!
    if (!player_cfg.animation.clips.empty()) {
        world.AddComponent<Animation>(entity, CreateAnimationFromConfig(player_cfg.animation, texture));
    }

    // Health
    world.AddComponent<Health>(entity, Health{
        .current = constants.player_max_health,
        .maximum = constants.player_max_health,
        .shield = constants.player_max_shield,
        .shield_maximum = constants.player_max_shield,
        .shield_regen_rate = constants.player_shield_regen_rate,
        .shield_regen_delay = constants.player_shield_regen_delay
    });

    // Collision
    world.AddComponent<Collision>(entity, Collision{
        .shape = Collision::Shape::CIRCLE,
        .radius = constants.player_collision_radius,
        .layer = constants.layer_player,
        .mask = constants.layer_enemy | constants.layer_enemy_bullet
    });

    // Input
    world.AddComponent<Input>(entity);

    // Physics (for smooth acceleration-based movement)
    world.AddComponent<Physics>(entity, Physics{
        .mass = constants.player_mass,
        .friction = constants.player_friction,
        .movement_force = constants.player_movement_force,
        .acceleration = {0.0f, 0.0f}
    });

    // Player tag
    world.AddComponent<PlayerTag>(entity);

    // Add starting weapon if configured
    if (auto weapon_cfg = config.GetWeapon(constants.player_starting_weapon)) {
        world.AddComponent<Weapon>(entity, CreateWeaponFromConfig(*weapon_cfg));
    }

    return entity;
}

entt::entity EntityFactory::CreateEnemy(const std::string& enemy_type,
                                       sf::Vector2f position, sf::Texture* texture) {
    auto enemy_cfg = config.GetEnemy(enemy_type);
    if (!enemy_cfg) {
        std::cerr << "Unknown enemy type: " << enemy_type << std::endl;
        return entt::null;
    }

    const auto& ec = *enemy_cfg;
    const auto& constants = config.GetConstants();

    auto entity = world.CreateEntity();

    // Transform
    world.AddComponent<Transform>(entity, Transform{
        .position = position,
        .last_position = position,
        .velocity = {0.0f, 0.0f},
        .rotation = 0.0f,
        .scale = 1.0f
    });

    // Calculate frame size from animation config
    sf::Vector2i frame_size(ec.size.x, ec.size.y);
    if (texture && ec.animation.cols > 0 && ec.animation.rows > 0) {
        auto texture_size = texture->getSize();
        frame_size.x = texture_size.x / ec.animation.cols;
        frame_size.y = texture_size.y / ec.animation.rows;
    }

    // Sprite (use White for textured sprites so texture colors show properly!)
    world.AddComponent<Sprite>(entity, Sprite{
        .texture = texture,
        .texture_rect = sf::IntRect(0, 0, frame_size.x, frame_size.y),  // Start with first frame
        .color = texture ? sf::Color::White : ec.color,  // White = use texture colors, fallback to config color
        .size = sf::Vector2f(frame_size.x, frame_size.y),
        .origin = sf::Vector2f(frame_size.x / 2.0f, frame_size.y / 2.0f),
        .layer = 5,
        .visible = true
    });

    // Animation (if animation config exists) - fully data-driven from TOML!
    if (!ec.animation.clips.empty()) {
        world.AddComponent<Animation>(entity, CreateAnimationFromConfig(ec.animation, texture));
    }

    // Health
    world.AddComponent<Health>(entity, Health{
        .current = ec.health,
        .maximum = ec.health,
        .shield = 0.0f,
        .shield_maximum = 0.0f
    });

    // Movement
    world.AddComponent<Movement>(entity, Movement{
        .pattern = ec.movement_pattern,
        .speed = ec.movement_speed,
        .max_speed = ec.movement_speed * 1.5f,
        .orbit_radius = ec.orbit_radius,
        .orbit_speed = ec.orbit_speed,
        .sine_amplitude = ec.sine_amplitude,
        .sine_frequency = ec.sine_frequency,
        .direction = ec.direction,
        .world_speed = constants.world_speed  // Gradius-style background scrolling
    });

    // Collision
    world.AddComponent<Collision>(entity, Collision{
        .shape = Collision::Shape::CIRCLE,
        .radius = ec.collision_radius,
        .layer = constants.layer_enemy,
        .mask = constants.layer_player | constants.layer_player_bullet
    });

    // Score
    world.AddComponent<Score>(entity, Score{
        .value = ec.score_value
    });

    // Add weapon if configured
    if (!ec.weapon.empty()) {
        if (auto weapon_cfg = config.GetWeapon(ec.weapon)) {
            world.AddComponent<Weapon>(entity, CreateWeaponFromConfig(*weapon_cfg));
        }
    }

    // Enemy tag
    world.AddComponent<EnemyTag>(entity);

    return entity;
}

entt::entity EntityFactory::CreateBullet(const std::string& weapon_name,
                                        sf::Vector2f position, sf::Vector2f direction,
                                        entt::entity owner, bool is_player_bullet,
                                        sf::Texture* texture) {
    auto weapon_cfg = config.GetWeapon(weapon_name);
    if (!weapon_cfg) {
        std::cerr << "Unknown weapon: " << weapon_name << std::endl;
        return entt::null;
    }

    const auto& wc = *weapon_cfg;
    const auto& constants = config.GetConstants();

    auto entity = world.CreateEntity();

    // Normalize direction
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.0001f) {
        direction /= length;
    }

    // Transform with velocity
    world.AddComponent<Transform>(entity, Transform{
        .position = position,
        .last_position = position,
        .velocity = direction * wc.bullet_speed,
        .rotation = std::atan2(direction.x, -direction.y) * 180.0f / 3.14159f,
        .scale = 1.0f
    });

    // Sprite
    world.AddComponent<Sprite>(entity, Sprite{
        .texture = texture,
        .color = wc.bullet_color,
        .size = wc.bullet_size,
        .origin = wc.bullet_size * 0.5f,
        .layer = is_player_bullet ? 8 : 3,
        .visible = true
    });

    // Collision
    world.AddComponent<Collision>(entity, Collision{
        .shape = Collision::Shape::CIRCLE,
        .radius = std::min(wc.bullet_size.x, wc.bullet_size.y) * 0.5f,
        .layer = is_player_bullet ? constants.layer_player_bullet : constants.layer_enemy_bullet,
        .mask = is_player_bullet ? constants.layer_enemy : constants.layer_player
    });

    // Lifetime (bullets despawn after 5 seconds)
    world.AddComponent<Lifetime>(entity, Lifetime{
        .duration = 5.0f,
        .elapsed = 0.0f
    });

    // Bullet tag
    world.AddComponent<BulletTag>(entity);

    return entity;
}

entt::entity EntityFactory::CreateBullet(const BulletSpawnRequest& request,
                                        bool is_player_bullet, sf::Texture* texture) {
    const auto& constants = config.GetConstants();

    auto entity = world.CreateEntity();

    // Normalize direction
    sf::Vector2f direction = request.direction;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.0001f) {
        direction /= length;
    }

    // Transform with velocity
    world.AddComponent<Transform>(entity, Transform{
        .position = request.position,
        .last_position = request.position,
        .velocity = direction * request.speed,
        .rotation = std::atan2(direction.x, -direction.y) * 180.0f / 3.14159f,
        .scale = 1.0f
    });

    // Sprite
    world.AddComponent<Sprite>(entity, Sprite{
        .texture = texture,
        .color = request.color,
        .size = request.size,
        .origin = request.size * 0.5f,
        .layer = is_player_bullet ? 8 : 3,
        .visible = true
    });

    // Collision
    world.AddComponent<Collision>(entity, Collision{
        .shape = Collision::Shape::CIRCLE,
        .radius = std::min(request.size.x, request.size.y) * 0.5f,
        .layer = is_player_bullet ? constants.layer_player_bullet : constants.layer_enemy_bullet,
        .mask = is_player_bullet ? constants.layer_enemy : constants.layer_player
    });

    // Lifetime
    world.AddComponent<Lifetime>(entity, Lifetime{
        .duration = 5.0f,
        .elapsed = 0.0f
    });

    // Bullet tag
    world.AddComponent<BulletTag>(entity);

    return entity;
}

entt::entity EntityFactory::CreateParticle(sf::Vector2f position, sf::Vector2f velocity,
                                          sf::Color color, float lifetime, float size) {
    auto entity = world.CreateEntity();

    // Transform
    world.AddComponent<Transform>(entity, Transform{
        .position = position,
        .last_position = position,
        .velocity = velocity,
        .rotation = 0.0f,
        .scale = 1.0f
    });

    // Sprite
    world.AddComponent<Sprite>(entity, Sprite{
        .texture = nullptr,
        .color = color,
        .size = {size, size},
        .origin = {size * 0.5f, size * 0.5f},
        .layer = 1,
        .visible = true
    });

    // Lifetime
    world.AddComponent<Lifetime>(entity, Lifetime{
        .duration = lifetime,
        .elapsed = 0.0f
    });

    // Particle tag
    world.AddComponent<ParticleTag>(entity);

    return entity;
}

void EntityFactory::CreateExplosion(sf::Vector2f position, sf::Color color, int particle_count) {
    const float TWO_PI = 6.28318f;
    const float speed = 150.0f;
    const float lifetime = 0.5f;

    for (int i = 0; i < particle_count; ++i) {
        float angle = (float)i / (float)particle_count * TWO_PI;
        sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);
        CreateParticle(position, velocity, color, lifetime, 4.0f);
    }
}

Weapon EntityFactory::CreateWeaponFromConfig(const WeaponConfig& wc) {
    return Weapon{
        .type = wc.type,
        .slot = 1,
        .active = true,
        .cooldown = wc.cooldown,
        .current_cooldown = 0.0f,
        .damage = wc.damage,
        .bullet_speed = wc.bullet_speed,
        .bullets_per_shot = wc.bullets_per_shot,
        .spread_angle = wc.spread_angle,
        .bullet_color = wc.bullet_color,
        .bullet_size = wc.bullet_size,
        .script_id = ""
    };
}

Animation EntityFactory::CreateAnimationFromConfig(const AnimationConfig& anim_cfg, sf::Texture* texture) {
    Animation anim;

    // Calculate frame size from texture and config
    sf::Vector2i frame_size(32, 32);
    if (texture && anim_cfg.cols > 0 && anim_cfg.rows > 0) {
        auto texture_size = texture->getSize();
        frame_size.x = texture_size.x / anim_cfg.cols;
        frame_size.y = texture_size.y / anim_cfg.rows;
    }

    anim.frame_size = frame_size;
    anim.total_cols = anim_cfg.cols;
    anim.total_rows = anim_cfg.rows;
    anim.current_animation = 0;  // Start with first animation
    anim.current_frame = 0;
    anim.frame_timer = 0.0f;
    anim.finished = false;

    // Add all animation clips from config
    for (const auto& clip_cfg : anim_cfg.clips) {
        anim.clips[clip_cfg.id] = AnimationClip{
            .row = clip_cfg.row,
            .start_col = clip_cfg.start_col,
            .frame_count = clip_cfg.frame_count,
            .frame_duration = clip_cfg.duration,
            .loop = clip_cfg.loop
        };
    }

    return anim;
}

} // namespace ecs
