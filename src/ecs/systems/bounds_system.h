#ifndef ECS_BOUNDS_SYSTEM_H
#define ECS_BOUNDS_SYSTEM_H

#include "../world.h"
#include <SFML/Graphics.hpp>

namespace ecs {

/**
 * BoundsSystem - Handles screen boundary clamping and off-screen despawning
 */
class BoundsSystem {
public:
    // Clamp player to screen bounds (Gradius-style: left 40% only)
    static void ClampPlayer(World& world, sf::FloatRect bounds) {
        auto view = world.View<PlayerTag, Transform>();

        for (auto entity : view) {
            auto& transform = view.get<Transform>(entity);

            // Full-screen movement with small margins on all sides
            float min_x = bounds.left + 20.0f;                     // 20px margin from left
            float max_x = bounds.left + bounds.width - 20.0f;      // 20px margin from right (FULL WIDTH!)
            float min_y = bounds.top + 20.0f;                      // 20px margin from top
            float max_y = bounds.top + bounds.height - 20.0f;      // 20px margin from bottom (FULL HEIGHT!)

            // Clamp X
            if (transform.position.x < min_x) {
                transform.position.x = min_x;
                transform.velocity.x = 0.0f;
            }
            if (transform.position.x > max_x) {
                transform.position.x = max_x;
                transform.velocity.x = 0.0f;
            }

            // Clamp Y
            if (transform.position.y < min_y) {
                transform.position.y = min_y;
                transform.velocity.y = 0.0f;
            }
            if (transform.position.y > max_y) {
                transform.position.y = max_y;
                transform.velocity.y = 0.0f;
            }
        }
    }

    // Despawn entities that went off-screen (enemies, bullets)
    static std::vector<entt::entity> CollectOffscreenEntities(World& world, sf::FloatRect bounds, float margin = 100.0f) {
        std::vector<entt::entity> to_despawn;

        // Check enemies and bullets
        auto view = world.View<Transform>();

        for (auto entity : view) {
            // Skip player
            if (world.HasComponent<PlayerTag>(entity)) {
                continue;
            }

            // Skip background stars
            if (world.HasComponent<Background>(entity)) {
                continue;
            }

            const auto& transform = view.get<Transform>(entity);

            // Check if entity is off-screen with margin
            bool off_left = transform.position.x < bounds.left - margin;
            bool off_right = transform.position.x > bounds.left + bounds.width + margin;
            bool off_top = transform.position.y < bounds.top - margin;
            bool off_bottom = transform.position.y > bounds.top + bounds.height + margin;

            if (off_left || off_right || off_top || off_bottom) {
                to_despawn.push_back(entity);
            }
        }

        return to_despawn;
    }
};

} // namespace ecs

#endif // ECS_BOUNDS_SYSTEM_H
