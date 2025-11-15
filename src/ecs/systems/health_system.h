#ifndef ECS_HEALTH_SYSTEM_H
#define ECS_HEALTH_SYSTEM_H

#include "../world.h"
#include <vector>

namespace ecs {

/**
 * HealthSystem - Manages health, shields, and damage
 */
class HealthSystem {
public:
    // Update shield regeneration
    static void Update(World& world, float dt) {
        auto view = world.View<Health>();

        for (auto entity : view) {
            auto& health = view.get<Health>(entity);

            if (health.dead || health.invulnerable) {
                continue;
            }

            // Update shield regeneration timer
            if (health.shield < health.shield_maximum) {
                health.time_since_damage += dt;

                // Start regenerating shield after delay
                if (health.time_since_damage >= health.shield_regen_delay) {
                    health.shield += health.shield_regen_rate * dt;
                    if (health.shield > health.shield_maximum) {
                        health.shield = health.shield_maximum;
                    }
                }
            }
        }
    }

    // Apply damage to an entity
    static void ApplyDamage(World& world, entt::entity entity, float damage) {
        if (!world.HasComponent<Health>(entity)) {
            return;
        }

        auto& health = world.GetComponent<Health>(entity);

        if (health.dead || health.invulnerable) {
            return;
        }

        // Reset shield regen timer
        health.time_since_damage = 0.0f;

        // Damage shields first
        if (health.shield > 0.0f) {
            float shield_damage = std::min(damage, health.shield);
            health.shield -= shield_damage;
            damage -= shield_damage;
        }

        // Then damage health
        if (damage > 0.0f) {
            health.current -= damage;
            if (health.current <= 0.0f) {
                health.current = 0.0f;
                health.dead = true;
            }
        }
    }

    // Heal an entity
    static void Heal(World& world, entt::entity entity, float amount) {
        if (!world.HasComponent<Health>(entity)) {
            return;
        }

        auto& health = world.GetComponent<Health>(entity);

        if (!health.dead) {
            health.current += amount;
            if (health.current > health.maximum) {
                health.current = health.maximum;
            }
        }
    }

    // Restore shield
    static void RestoreShield(World& world, entt::entity entity, float amount) {
        if (!world.HasComponent<Health>(entity)) {
            return;
        }

        auto& health = world.GetComponent<Health>(entity);

        if (!health.dead) {
            health.shield += amount;
            if (health.shield > health.shield_maximum) {
                health.shield = health.shield_maximum;
            }
        }
    }

    // Check if entity is dead and collect dead entities
    static std::vector<entt::entity> CollectDeadEntities(World& world) {
        std::vector<entt::entity> dead_entities;
        auto view = world.View<Health>();

        for (auto entity : view) {
            const auto& health = view.get<Health>(entity);
            if (health.dead) {
                dead_entities.push_back(entity);
            }
        }

        return dead_entities;
    }
};

} // namespace ecs

#endif // ECS_HEALTH_SYSTEM_H
