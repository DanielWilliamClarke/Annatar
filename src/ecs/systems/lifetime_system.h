#ifndef ECS_LIFETIME_SYSTEM_H
#define ECS_LIFETIME_SYSTEM_H

#include "../world.h"
#include <vector>

namespace ecs {

/**
 * LifetimeSystem - Manages entity lifetimes and auto-destruction
 */
class LifetimeSystem {
public:
    // Update lifetimes and collect expired entities
    static std::vector<entt::entity> Update(World& world, float dt) {
        std::vector<entt::entity> expired_entities;
        auto view = world.View<Lifetime>();

        for (auto entity : view) {
            auto& lifetime = view.get<Lifetime>(entity);
            lifetime.elapsed += dt;

            if (lifetime.elapsed >= lifetime.duration) {
                expired_entities.push_back(entity);
            }
        }

        return expired_entities;
    }

    // Check if entity is expired
    static bool IsExpired(const Lifetime& lifetime) {
        return lifetime.elapsed >= lifetime.duration;
    }
};

} // namespace ecs

#endif // ECS_LIFETIME_SYSTEM_H
