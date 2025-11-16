#ifndef ECS_MOVEMENT_INPUT_SYSTEM_H
#define ECS_MOVEMENT_INPUT_SYSTEM_H

#include "../world.h"
#include "../config/config_loader.h"
#include "animation_system.h"

namespace ecs {

/**
 * MovementInputSystem - Applies Input component to Transform and Animation
 * Separates input sampling from movement application
 * Uses acceleration-based physics for smooth "floaty" movement
 */
class MovementInputSystem {
public:
    // Apply input to transform velocity and select animations
    static void Update(World& world, const GameConstants& constants, float dt) {
        auto view = world.View<Input, Transform>();

        for (auto entity : view) {
            const auto& input = view.get<Input>(entity);
            auto& transform = view.get<Transform>(entity);

            // Check if entity has physics component (acceleration-based movement)
            if (world.HasComponent<Physics>(entity)) {
                auto& physics = world.GetComponent<Physics>(entity);
                ApplyPhysics(transform, physics, input, constants, dt);
            } else {
                // Fallback: direct velocity (for non-physics entities)
                transform.velocity = input.move_direction * constants.player_movement_speed;
            }

            // Select animation based on velocity (banking animation)
            if (world.HasComponent<Animation>(entity)) {
                SelectAnimation(world, entity, transform);
            }
        }
    }

private:
    // Apply physics-based movement (creates smooth "floaty" feel)
    static void ApplyPhysics(Transform& transform, Physics& physics,
                            const Input& input, const GameConstants& constants, float dt) {
        // 1. Calculate damping force (friction/drag)
        sf::Vector2f damping = -transform.velocity * physics.friction;

        // 2. Calculate input force
        sf::Vector2f inputForce = input.move_direction * physics.movement_force;

        // 3. Total force
        sf::Vector2f totalForce = inputForce + damping;

        // 4. Newton's Second Law: F = ma → a = F/m
        physics.acceleration = totalForce / physics.mass;

        // 5. Integrate acceleration into velocity
        transform.velocity += physics.acceleration * dt;

        // 6. Clamp to max speed
        float speed = std::sqrt(transform.velocity.x * transform.velocity.x +
                               transform.velocity.y * transform.velocity.y);
        if (speed > constants.player_max_speed) {
            transform.velocity *= (constants.player_max_speed / speed);
        }
    }

private:
    // Select appropriate animation based on velocity (banking)
    static void SelectAnimation(World& world, entt::entity entity, const Transform& transform) {
        int anim_id = AnimationSystem::IDLE;

        // Vertical banking based on velocity (for horizontal shooter)
        // Idle when moving left/right, bank when moving up/down
        if (transform.velocity.y < -50.0f) {
            anim_id = AnimationSystem::MOVING_UP;    // Bank up (right frame)
        } else if (transform.velocity.y > 50.0f) {
            anim_id = AnimationSystem::MOVING_DOWN;  // Bank down (left frame)
        }
        // Otherwise stay IDLE (moving left/right horizontally)

        AnimationSystem::PlayAnimation(world, entity, anim_id, false, false);
    }
};

} // namespace ecs

#endif // ECS_MOVEMENT_INPUT_SYSTEM_H
