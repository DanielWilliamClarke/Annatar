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

    // Calculate scale factor from desired size vs actual sprite size
    float scale_x = constants.player_size.x / static_cast<float>(player_cfg.animation.sprite_width);
    float scale_y = constants.player_size.y / static_cast<float>(player_cfg.animation.sprite_height);
    float scale = std::max(scale_x, scale_y);  // Use uniform scale (3.0 for 24/8)

    // Transform
    world.AddComponent<Transform>(entity, Transform{
        .position = position,
        .last_position = position,
        .velocity = {0.0f, 0.0f},
        .rotation = 0.0f,
        .scale = scale  // 3x scale (8x8 → 24x24)
    });

    // Use direct pixel coordinates from config
    sf::Vector2i frame_size(player_cfg.animation.sprite_width, player_cfg.animation.sprite_height);
    sf::IntRect texture_rect(
        player_cfg.animation.sprite_x,
        player_cfg.animation.sprite_y,
        player_cfg.animation.sprite_width,
        player_cfg.animation.sprite_height
    );

    // Sprite
    world.AddComponent<Sprite>(entity, Sprite{
        .texture = texture,
        .texture_rect = texture_rect,  // Use direct coordinates (idle frame)
        .color = sf::Color::White,
        .size = sf::Vector2f(frame_size.x, frame_size.y),  // Actual sprite size (8x8)
        .origin = sf::Vector2f(frame_size.x / 2.0f, frame_size.y / 2.0f),  // Center (4,4)
        .layer = 10,
        .visible = true
    });

    // Animation - loaded from player.toml!
    // Always add Animation for player (has banking frames)
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

    // Add multi-weapon system (4 slots loaded from player.toml)
    const auto& weapon_slots = config.GetPlayerConfig().weapon_slots;
    Weapons weapons_component;

    for (int i = 0; i < 4; ++i) {
        const std::string& weapon_name = weapon_slots[i];
        if (!weapon_name.empty()) {
            if (auto weapon_cfg = config.GetWeapon(weapon_name)) {
                Weapon weapon = CreateWeaponFromConfig(*weapon_cfg);
                weapon.slot = i;  // Set slot number (0-3, displayed as 1-4 to user)
                weapon.active = (i == 0);  // Slot 1 active by default, others inactive
                weapons_component.slots[i] = weapon;
            } else {
                std::cerr << "Warning: Weapon '" << weapon_name << "' not found in weapons.toml for slot " << (i+1) << std::endl;
            }
        }
    }

    world.AddComponent<Weapons>(entity, std::move(weapons_component));

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

    // Get texture from config if textureAtlas is available and no texture provided
    if (!texture && textureAtlas && !ec.animation.sprite_sheet_name.empty()) {
        texture = textureAtlas->GetTexture(ec.animation.sprite_sheet_name).get();
    }

    // Calculate scale factor from desired size vs actual sprite size
    float scale_x = ec.size.x / static_cast<float>(ec.animation.sprite_width);
    float scale_y = ec.size.y / static_cast<float>(ec.animation.sprite_height);
    float scale = std::max(scale_x, scale_y);  // Use uniform scale (larger of the two)

    // Transform
    world.AddComponent<Transform>(entity, Transform{
        .position = position,
        .last_position = position,
        .velocity = {0.0f, 0.0f},
        .rotation = 0.0f,
        .scale = scale  // Scale sprite from 8x8→24x24 or 16x16→48x48
    });

    // Use direct pixel coordinates from config (supports non-uniform sprite sheets)
    sf::Vector2i frame_size(ec.animation.sprite_width, ec.animation.sprite_height);
    sf::IntRect texture_rect(
        ec.animation.sprite_x,
        ec.animation.sprite_y,
        ec.animation.sprite_width,
        ec.animation.sprite_height
    );

    // Sprite (use White for textured sprites so texture colors show properly!)
    // Note: origin is in sprite-space (not scaled), size is the actual sprite size
    world.AddComponent<Sprite>(entity, Sprite{
        .texture = texture,
        .texture_rect = texture_rect,  // Use calculated rect based on sprite_x/sprite_y
        .color = texture ? sf::Color::White : ec.color,  // White = use sprite colors, fallback to config color
        .size = sf::Vector2f(frame_size.x, frame_size.y),  // Actual sprite size (8x8 or 16x16)
        .origin = sf::Vector2f(frame_size.x / 2.0f, frame_size.y / 2.0f),  // Center origin (4,4 or 8,8)
        .layer = 5,
        .visible = true
    });

    // Animation (if animation config exists) - fully data-driven from TOML!
    // Skip animation for static sprites that use direct pixel coordinates
    bool uses_direct_coords = (ec.animation.sprite_x != 0 || ec.animation.sprite_y != 0);

    if (!uses_direct_coords && !ec.animation.clips.empty()) {
        world.AddComponent<Animation>(entity, CreateAnimationFromConfig(ec.animation, texture));
    }
    // Note: Static sprites (with sprite_x/sprite_y set) won't get Animation component
    // Their texture_rect is set once and never changes

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

    // Glow effect
    world.AddComponent<Glow>(entity, Glow{
        .color = wc.bullet_color,
        .attenuation = 300.0f,  // Higher value = tighter glow (legacy uses 500.0f)
        .enabled = true
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

    // Glow effect
    world.AddComponent<Glow>(entity, Glow{
        .color = request.color,
        .attenuation = 300.0f,  // Higher value = tighter glow (legacy uses 500.0f)
        .enabled = true
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

    // Glow effect for explosion particles
    world.AddComponent<Glow>(entity, Glow{
        .color = color,
        .attenuation = 100.0f,  // Lower value = larger glow for explosions
        .enabled = true
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

    // Use direct sprite dimensions from config (supports non-uniform sprite sheets)
    sf::Vector2i frame_size(anim_cfg.sprite_width, anim_cfg.sprite_height);

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
