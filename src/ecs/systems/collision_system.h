#ifndef ECS_COLLISION_SYSTEM_H
#define ECS_COLLISION_SYSTEM_H

#include "../world.h"
#include <vector>
#include <functional>
#include <cmath>

namespace ecs {

/**
 * CollisionPair - Represents a collision between two entities
 */
struct CollisionPair {
    entt::entity entity_a;
    entt::entity entity_b;
    sf::Vector2f collision_point;
};

/**
 * CollisionSystem - Handles collision detection between entities
 */
class CollisionSystem {
public:
    using CollisionCallback = std::function<void(entt::entity, entt::entity, const sf::Vector2f&)>;

    // Detect all collisions and invoke callback
    static void DetectCollisions(World& world, CollisionCallback callback) {
        std::vector<CollisionPair> collisions;

        // Get all entities with collision components
        std::vector<entt::entity> entities;
        auto view = world.View<Transform, Collision>();
        for (auto entity : view) {
            entities.push_back(entity);
        }

        // Broad phase - check all pairs (O(n^2) - will optimize with QuadTree later)
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                auto entity_a = entities[i];
                auto entity_b = entities[j];

                const auto& transform_a = world.GetComponent<Transform>(entity_a);
                const auto& collision_a = world.GetComponent<Collision>(entity_a);
                const auto& transform_b = world.GetComponent<Transform>(entity_b);
                const auto& collision_b = world.GetComponent<Collision>(entity_b);

                // Skip if either collision is disabled
                if (!collision_a.enabled || !collision_b.enabled) {
                    continue;
                }

                // Check collision layers
                if ((collision_a.layer & collision_b.mask) == 0 &&
                    (collision_b.layer & collision_a.mask) == 0) {
                    continue;
                }

                // Narrow phase - actual collision test
                sf::Vector2f pos_a = transform_a.position + collision_a.offset;
                sf::Vector2f pos_b = transform_b.position + collision_b.offset;

                if (TestCollision(collision_a, pos_a, collision_b, pos_b)) {
                    // Calculate collision point (midpoint)
                    sf::Vector2f collision_point = (pos_a + pos_b) * 0.5f;

                    collisions.push_back({entity_a, entity_b, collision_point});
                }
            }
        }

        // Invoke callback for each collision
        for (const auto& collision : collisions) {
            callback(collision.entity_a, collision.entity_b, collision.collision_point);
        }
    }

    // Test collision between two collision components
    static bool TestCollision(const Collision& a, const sf::Vector2f& pos_a,
                             const Collision& b, const sf::Vector2f& pos_b) {
        if (a.shape == Collision::Shape::CIRCLE && b.shape == Collision::Shape::CIRCLE) {
            return TestCircleCircle(pos_a, a.radius, pos_b, b.radius);
        } else if (a.shape == Collision::Shape::RECTANGLE && b.shape == Collision::Shape::RECTANGLE) {
            return TestRectRect(pos_a, a.rect_size, pos_b, b.rect_size);
        } else {
            // Circle-Rect collision
            if (a.shape == Collision::Shape::CIRCLE) {
                return TestCircleRect(pos_a, a.radius, pos_b, b.rect_size);
            } else {
                return TestCircleRect(pos_b, b.radius, pos_a, a.rect_size);
            }
        }
    }

private:
    static bool TestCircleCircle(const sf::Vector2f& pos_a, float radius_a,
                                 const sf::Vector2f& pos_b, float radius_b) {
        float dx = pos_b.x - pos_a.x;
        float dy = pos_b.y - pos_a.y;
        float distance_sq = dx * dx + dy * dy;
        float radius_sum = radius_a + radius_b;
        return distance_sq <= (radius_sum * radius_sum);
    }

    static bool TestRectRect(const sf::Vector2f& pos_a, const sf::Vector2f& size_a,
                            const sf::Vector2f& pos_b, const sf::Vector2f& size_b) {
        float left_a = pos_a.x - size_a.x / 2.0f;
        float right_a = pos_a.x + size_a.x / 2.0f;
        float top_a = pos_a.y - size_a.y / 2.0f;
        float bottom_a = pos_a.y + size_a.y / 2.0f;

        float left_b = pos_b.x - size_b.x / 2.0f;
        float right_b = pos_b.x + size_b.x / 2.0f;
        float top_b = pos_b.y - size_b.y / 2.0f;
        float bottom_b = pos_b.y + size_b.y / 2.0f;

        return !(right_a < left_b || right_b < left_a ||
                bottom_a < top_b || bottom_b < top_a);
    }

    static bool TestCircleRect(const sf::Vector2f& circle_pos, float radius,
                               const sf::Vector2f& rect_pos, const sf::Vector2f& rect_size) {
        // Find closest point on rectangle to circle center
        float left = rect_pos.x - rect_size.x / 2.0f;
        float right = rect_pos.x + rect_size.x / 2.0f;
        float top = rect_pos.y - rect_size.y / 2.0f;
        float bottom = rect_pos.y + rect_size.y / 2.0f;

        float closest_x = std::max(left, std::min(circle_pos.x, right));
        float closest_y = std::max(top, std::min(circle_pos.y, bottom));

        float dx = circle_pos.x - closest_x;
        float dy = circle_pos.y - closest_y;
        float distance_sq = dx * dx + dy * dy;

        return distance_sq <= (radius * radius);
    }
};

} // namespace ecs

#endif // ECS_COLLISION_SYSTEM_H
