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

            // Check if using physics-based movement
            if (movement.use_physics && world.HasComponent<Physics>(entity)) {
                UpdatePhysicsMovement(world, entity, transform, movement, dt);
            } else {
                // Update pattern-specific movement
                UpdateMovementPattern(world, entity, transform, movement, dt);

                // Apply velocity
                transform.position += transform.velocity * dt;

                // Clamp to max speed
                float speed = std::sqrt(transform.velocity.x * transform.velocity.x +
                                       transform.velocity.y * transform.velocity.y);
                if (speed > movement.max_speed) {
                    transform.velocity *= (movement.max_speed / speed);
                }
            }

            // Add world scrolling (Gradius-style background drift)
            if (movement.world_speed > 0.0f) {
                sf::Vector2f world_velocity(-movement.world_speed, 0.0f);  // Scroll LEFT
                transform.position += world_velocity * dt;

                // Also apply to orbit center if orbiting
                if (movement.pattern == Movement::Pattern::ORBITAL) {
                    movement.orbit_center += world_velocity * dt;
                }
            }

            // AUTO-ROTATE: Point sprite in direction of velocity
            if (transform.velocity.x != 0.0f || transform.velocity.y != 0.0f) {
                float angle = std::atan2(transform.velocity.y, transform.velocity.x);
                transform.rotation = angle * (180.0f / 3.14159f) + 90.0f;
                // +90° adjusts for vertical shooter sprite orientation (bottom points down → right)
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

            // AUTO-ROTATE: Point sprite in direction of velocity (for bullets, particles, etc.)
            if (transform.velocity.x != 0.0f || transform.velocity.y != 0.0f) {
                float angle = std::atan2(transform.velocity.y, transform.velocity.x);
                transform.rotation = angle * (180.0f / 3.14159f) + 90.0f;
                // +90° adjusts for vertical shooter sprite orientation
            }
        }
    }

private:
    static void UpdateMovementPattern(World& world, entt::entity entity, Transform& transform, Movement& movement, float dt) {
        movement.pattern_time += dt;

        switch (movement.pattern) {
            case Movement::Pattern::LINEAR:
                // Linear movement - constant velocity in direction
                transform.velocity = movement.direction * movement.speed;
                break;

            case Movement::Pattern::ORBITAL:
                {
                    // Initialize orbit center on first update (store spawn position)
                    if (!movement.orbit_initialized) {
                        movement.orbit_center = transform.position;
                        movement.orbit_initialized = true;
                    }

                    // Circular orbit around spawn position
                    float angle = movement.pattern_time * movement.orbit_speed;
                    float x = movement.orbit_center.x + std::cos(angle) * movement.orbit_radius;
                    float y = movement.orbit_center.y + std::sin(angle) * movement.orbit_radius;

                    // Set position directly (not velocity) for smooth circular orbit
                    transform.position = sf::Vector2f(x, y);

                    // Calculate velocity for interpolation (tangent to circle)
                    float tangent_angle = angle + 1.5708f;  // +90 degrees in radians
                    transform.velocity = sf::Vector2f(
                        std::cos(tangent_angle) * movement.orbit_speed * movement.orbit_radius,
                        std::sin(tangent_angle) * movement.orbit_speed * movement.orbit_radius
                    );
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
                {
                    // Find player entity to chase
                    auto players = world.View<PlayerTag, Transform>();
                    bool found_player = false;

                    for (auto player : players) {
                        // Get first player
                        const auto& player_transform = world.GetComponent<Transform>(player);

                        // Calculate direction to player
                        sf::Vector2f to_player = player_transform.position - transform.position;
                        float distance = std::sqrt(to_player.x * to_player.x + to_player.y * to_player.y);

                        if (distance > 0.001f) {
                            // Normalize direction and apply speed
                            sf::Vector2f direction = to_player / distance;
                            transform.velocity = direction * movement.speed;
                        } else {
                            // At player position, stop
                            transform.velocity = sf::Vector2f(0.0f, 0.0f);
                        }

                        found_player = true;
                        break;  // Only chase first player
                    }

                    if (!found_player) {
                        // No player found, move in default direction
                        transform.velocity = movement.direction * movement.speed;
                    }
                }
                break;

            case Movement::Pattern::SCRIPTED:
                // TODO: Implement Lua scripted movement
                break;
        }
    }

    static void UpdatePhysicsMovement(World& world, entt::entity entity, Transform& transform, Movement& movement, float dt) {
        auto& physics = world.GetComponent<Physics>(entity);

        // Calculate total force (gravity + thrust + any pattern forces)
        sf::Vector2f total_force = physics.gravity + physics.thrust;

        // Apply pattern-based forces if needed
        // (For now, physics enemies use linear movement with forces)
        sf::Vector2f pattern_force = movement.direction * movement.speed * physics.mass;
        total_force += pattern_force;

        // F = ma → a = F/m
        physics.acceleration = total_force / physics.mass;

        // Update velocity: v = v + a*dt
        transform.velocity += physics.acceleration * dt;

        // Apply friction/damping
        transform.velocity *= (1.0f - physics.friction * dt);

        // Apply velocity to position
        transform.position += transform.velocity * dt;

        // Clamp to max speed
        float speed = std::sqrt(transform.velocity.x * transform.velocity.x +
                               transform.velocity.y * transform.velocity.y);
        if (speed > movement.max_speed) {
            transform.velocity *= (movement.max_speed / speed);
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
