#ifndef ECS_MOVEMENT_SYSTEM_H
#define ECS_MOVEMENT_SYSTEM_H

#include "../world.h"
#include <cmath>

namespace ecs {

/**
 * MovementSystem - Handles entity movement and position updates
 */
class MovementSystem {
public:
    // Update all entities with Transform and Movement components
    static void Update(World& world, float dt) {
        auto view = world.View<Transform, Movement>();

        for (auto entity : view) {
            auto& transform = view.get<Transform>(entity);
            auto& movement = view.get<Movement>(entity);

            // Store last position for interpolation
            transform.last_position = transform.position;

            // Update pattern-specific movement
            UpdateMovementPattern(transform, movement, dt);

            // Apply velocity
            transform.position += transform.velocity * dt;

            // Clamp to max speed
            float speed = std::sqrt(transform.velocity.x * transform.velocity.x +
                                   transform.velocity.y * transform.velocity.y);
            if (speed > movement.max_speed) {
                transform.velocity *= (movement.max_speed / speed);
            }
        }
    }

    // Update entities with only Transform (direct velocity control)
    static void UpdateSimple(World& world, float dt) {
        auto view = world.View<Transform>();

        for (auto entity : view) {
            // Skip entities that have Movement component (handled by main Update)
            if (world.HasComponent<Movement>(entity)) {
                continue;
            }

            auto& transform = view.get<Transform>(entity);
            transform.last_position = transform.position;
            transform.position += transform.velocity * dt;
        }
    }

private:
    static void UpdateMovementPattern(Transform& transform, Movement& movement, float dt) {
        movement.pattern_time += dt;

        switch (movement.pattern) {
            case Movement::Pattern::LINEAR:
                // Linear movement - velocity already set
                transform.velocity = movement.direction * movement.speed;
                break;

            case Movement::Pattern::ORBITAL:
                {
                    // Circular orbit around a point (using transform.position as center initially)
                    float angle = movement.pattern_time * movement.orbit_speed;
                    float x = std::cos(angle) * movement.orbit_radius;
                    float y = std::sin(angle) * movement.orbit_radius;

                    // Calculate velocity to reach target position
                    sf::Vector2f target(x, y);
                    sf::Vector2f offset = target - transform.position;
                    transform.velocity = offset * movement.orbit_speed;
                }
                break;

            case Movement::Pattern::SINE_WAVE:
                {
                    // Sine wave movement - oscillate perpendicular to direction
                    float sine_offset = std::sin(movement.pattern_time * movement.sine_frequency) * movement.sine_amplitude;

                    // Move forward
                    sf::Vector2f forward = movement.direction * movement.speed;

                    // Add perpendicular oscillation
                    sf::Vector2f perpendicular(-movement.direction.y, movement.direction.x);
                    perpendicular = Normalize(perpendicular);

                    transform.velocity = forward + perpendicular * sine_offset;
                }
                break;

            case Movement::Pattern::FOLLOW_TARGET:
                // TODO: Implement target following (requires AI component)
                break;

            case Movement::Pattern::SCRIPTED:
                // TODO: Implement Lua scripted movement
                break;
        }
    }

    static sf::Vector2f Normalize(const sf::Vector2f& v) {
        float length = std::sqrt(v.x * v.x + v.y * v.y);
        if (length > 0.0001f) {
            return sf::Vector2f(v.x / length, v.y / length);
        }
        return sf::Vector2f(0.0f, 0.0f);
    }
};

} // namespace ecs

#endif // ECS_MOVEMENT_SYSTEM_H
