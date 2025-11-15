#ifndef ECS_BACKGROUND_SYSTEM_H
#define ECS_BACKGROUND_SYSTEM_H

#include "../world.h"
#include "../components/components.h"
#include <random>

namespace ecs {

/**
 * BackgroundSystem - Handles scrolling starfield background (Gradius-style)
 */
class BackgroundSystem {
public:
    // Initialize background stars
    static void Initialize(World& world, sf::Vector2f screen_size, int star_count = 200) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist_x(0.0f, screen_size.x);
        std::uniform_real_distribution<float> dist_y(0.0f, screen_size.y);

        for (int i = 0; i < star_count; ++i) {
            auto entity = world.CreateEntity();

            // Determine layer based on index (matches legacy SpaceLevel)
            int layer = 0;
            float parallax_factor = 1.0f;
            float radius = 0.75f;
            sf::Color color = sf::Color::White;

            float percent = (float)i / (float)star_count;

            if (percent < 0.80f) {
                // Layer 0: Slow background stars (gray)
                layer = 0;
                parallax_factor = 0.5f;
                radius = 0.75f;
                color = sf::Color(128, 128, 128);
            } else if (percent < 0.85f) {
                // Layer 1: Medium stars (gold)
                layer = 1;
                parallax_factor = 0.7f;
                radius = 1.0f;
                color = sf::Color(255, 215, 0);
            } else if (percent < 0.90f) {
                // Layer 2: Fast foreground stars (cyan, with glow)
                layer = 2;
                parallax_factor = 1.1f;
                radius = 1.5f;
                color = sf::Color(0, 255, 255);
            } else if (percent < 0.95f) {
                // Layer 3: Slow large stars (red)
                layer = 3;
                parallax_factor = 0.5f;
                radius = 2.0f;
                color = sf::Color(255, 0, 0);
            }

            // Random starting position
            sf::Vector2f position(dist_x(gen), dist_y(gen));

            // Add components
            world.AddComponent<Transform>(entity, Transform{
                .position = position,
                .last_position = position,
                .velocity = {0.0f, 0.0f}
            });

            world.AddComponent<Sprite>(entity, Sprite{
                .texture = nullptr,
                .color = color,
                .size = {radius * 2.0f, radius * 2.0f},
                .origin = {radius, radius},
                .layer = -10,  // Behind everything else
                .visible = true
            });

            world.AddComponent<Background>(entity, Background{
                .parallax_factor = parallax_factor,
                .layer = layer
            });

            // Add glow tag for large cyan stars
            if (layer == 2) {
                // These stars will glow
                world.AddComponent<ParticleTag>(entity);  // Reuse particle tag for glowing objects
            }
        }
    }

    // Update scrolling background
    static void Update(World& world, float world_speed, float dt, sf::Vector2f screen_size) {
        auto view = world.View<Transform, Background>();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist_y(0.0f, screen_size.y);

        for (auto entity : view) {
            auto& transform = view.get<Transform>(entity);
            auto& background = view.get<Background>(entity);

            // Store last position for interpolation
            transform.last_position = transform.position;

            // Scroll left based on world speed and parallax factor
            transform.position.x -= world_speed * dt * background.parallax_factor;

            // Wrap stars that go off the left edge
            if (transform.position.x < 0) {
                transform.position.x = screen_size.x;
                transform.position.y = dist_y(gen);  // Random Y position on wrap
            }
        }
    }

    // Clear all background entities (for cleanup)
    static void Clear(World& world) {
        std::vector<entt::entity> to_destroy;
        auto view = world.View<Background>();

        for (auto entity : view) {
            to_destroy.push_back(entity);
        }

        for (auto entity : to_destroy) {
            world.DestroyEntity(entity);
        }
    }
};

} // namespace ecs

#endif // ECS_BACKGROUND_SYSTEM_H
