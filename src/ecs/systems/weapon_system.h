#ifndef ECS_WEAPON_SYSTEM_H
#define ECS_WEAPON_SYSTEM_H

#include "../world.h"
#include <functional>

namespace ecs {

/**
 * BulletSpawnRequest - Data for spawning a bullet
 */
struct BulletSpawnRequest {
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
    float damage;
    sf::Color color;
    sf::Vector2f size;
    entt::entity owner;  // Who fired this bullet
};

/**
 * WeaponSystem - Handles weapon firing and cooldowns
 */
class WeaponSystem {
public:
    using BulletSpawnCallback = std::function<void(const BulletSpawnRequest&)>;

    // Update weapon cooldowns
    static void Update(World& world, float dt) {
        auto view = world.View<Weapon>();

        for (auto entity : view) {
            auto& weapon = view.get<Weapon>(entity);

            if (weapon.current_cooldown > 0.0f) {
                weapon.current_cooldown -= dt;
                if (weapon.current_cooldown < 0.0f) {
                    weapon.current_cooldown = 0.0f;
                }
            }
        }
    }

    // Try to fire weapon, returns true if fired
    static bool TryFire(World& world, entt::entity entity,
                       BulletSpawnCallback spawn_callback) {
        if (!world.HasComponent<Weapon>(entity) ||
            !world.HasComponent<Transform>(entity)) {
            return false;
        }

        auto& weapon = world.GetComponent<Weapon>(entity);
        const auto& transform = world.GetComponent<Transform>(entity);

        // Check if weapon can fire
        if (!weapon.active || weapon.current_cooldown > 0.0f) {
            return false;
        }

        // Reset cooldown
        weapon.current_cooldown = weapon.cooldown;

        // Fire based on weapon type
        switch (weapon.type) {
            case Weapon::Type::SINGLE_SHOT:
                FireSingleShot(entity, transform, weapon, spawn_callback);
                break;

            case Weapon::Type::BURST:
                FireBurst(entity, transform, weapon, spawn_callback);
                break;

            case Weapon::Type::RANDOM_SPREAD:
                FireRandomSpread(entity, transform, weapon, spawn_callback);
                break;

            case Weapon::Type::BEAM:
                // TODO: Implement beam weapons
                break;

            case Weapon::Type::HOMING:
                // TODO: Implement homing missiles
                break;
        }

        return true;
    }

    // Fire all active weapons on an entity
    static void FireAllWeapons(World& world, entt::entity entity,
                              BulletSpawnCallback spawn_callback) {
        // Check if entity has weapon component
        if (!world.HasComponent<Weapon>(entity)) {
            return;
        }

        // For now, just fire the single weapon
        // Later we'll handle multiple weapon slots
        TryFire(world, entity, spawn_callback);
    }

    // Toggle weapon active state
    static void ToggleWeapon(World& world, entt::entity entity, int slot) {
        if (!world.HasComponent<Weapon>(entity)) {
            return;
        }

        auto& weapon = world.GetComponent<Weapon>(entity);
        if (weapon.slot == slot) {
            weapon.active = !weapon.active;
        }
    }

private:
    static void FireSingleShot(entt::entity owner, const Transform& transform,
                              const Weapon& weapon, BulletSpawnCallback spawn_callback) {
        // Fire straight RIGHT (Gradius horizontal shooter!)
        sf::Vector2f direction(1.0f, 0.0f);  // Right direction

        BulletSpawnRequest request;
        request.owner = owner;
        request.position = transform.position;
        request.direction = direction;
        request.speed = weapon.bullet_speed;
        request.damage = weapon.damage;
        request.color = weapon.bullet_color;
        request.size = weapon.bullet_size;

        spawn_callback(request);
    }

    static void FireBurst(entt::entity owner, const Transform& transform,
                         const Weapon& weapon, BulletSpawnCallback spawn_callback) {
        // Fire multiple bullets in a spread pattern around RIGHT direction (Gradius!)
        float base_angle = 0.0f;  // 0° = right

        int bullets = weapon.bullets_per_shot;
        float spread = weapon.spread_angle * 3.14159f / 180.0f;

        for (int i = 0; i < bullets; ++i) {
            // Calculate angle offset for this bullet
            float offset = spread * ((float)i / (float)(bullets - 1) - 0.5f);
            if (bullets == 1) offset = 0.0f;

            float bullet_angle = base_angle + offset;
            sf::Vector2f direction(std::cos(bullet_angle), std::sin(bullet_angle));  // RIGHT-based angles

            BulletSpawnRequest request;
            request.owner = owner;
            request.position = transform.position;
            request.direction = direction;
            request.speed = weapon.bullet_speed;
            request.damage = weapon.damage;
            request.color = weapon.bullet_color;
            request.size = weapon.bullet_size;

            spawn_callback(request);
        }
    }

    static void FireRandomSpread(entt::entity owner, const Transform& transform,
                                const Weapon& weapon, BulletSpawnCallback spawn_callback) {
        // Fire bullets in random directions within spread angle around RIGHT (Gradius!)
        float base_angle = 0.0f;  // 0° = right
        float spread = weapon.spread_angle * 3.14159f / 180.0f;

        for (int i = 0; i < weapon.bullets_per_shot; ++i) {
            // Random offset within spread
            float random_offset = ((float)rand() / (float)RAND_MAX - 0.5f) * spread;
            float bullet_angle = base_angle + random_offset;

            sf::Vector2f direction(std::cos(bullet_angle), std::sin(bullet_angle));  // RIGHT-based angles

            BulletSpawnRequest request;
            request.owner = owner;
            request.position = transform.position;
            request.direction = direction;
            request.speed = weapon.bullet_speed;
            request.damage = weapon.damage;
            request.color = weapon.bullet_color;
            request.size = weapon.bullet_size;

            spawn_callback(request);
        }
    }
};

} // namespace ecs

#endif // ECS_WEAPON_SYSTEM_H
