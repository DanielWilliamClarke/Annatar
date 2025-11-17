#ifndef ECS_RENDER_SYSTEM_H
#define ECS_RENDER_SYSTEM_H

#include "../world.h"
#include "../../renderer/i_glow_shader_renderer.h"
#include "../../renderer/i_renderer.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>

namespace ecs {

/**
 * RenderSystem - Handles rendering of sprites
 */
class RenderSystem {
public:
    // Render all entities with Sprite and Transform components
    static void Render(World& world, sf::RenderTarget& target, float interpolation = 1.0f) {
        // Collect all renderable entities
        std::vector<entt::entity> entities;
        auto view = world.View<Transform, Sprite>();

        for (auto entity : view) {
            const auto& sprite = view.get<Sprite>(entity);
            if (sprite.visible) {
                entities.push_back(entity);
            }
        }

        // Sort by layer (lower layers drawn first)
        std::sort(entities.begin(), entities.end(), [&world](entt::entity a, entt::entity b) {
            const auto& sprite_a = world.GetComponent<Sprite>(a);
            const auto& sprite_b = world.GetComponent<Sprite>(b);
            return sprite_a.layer < sprite_b.layer;
        });

        // Render each entity
        for (auto entity : entities) {
            const auto& transform = world.GetComponent<Transform>(entity);
            const auto& sprite = world.GetComponent<Sprite>(entity);
            const auto input = world.TryGetComponent<Input>(entity);

            // Interpolate position for smooth rendering
            sf::Vector2f render_pos = Interpolate(
                transform.last_position,
                transform.position,
                interpolation
            );

            RenderSprite(target, sprite, render_pos, transform.rotation, transform.scale);

            if (input) {
                RenderAim(target, render_pos, input->mouse_position);
            }
        }
    }

    // Render glow effects using an IRenderer (uses renderer's AddGlow method)
    static void RenderGlow(World& world, class IRenderer& renderer, float interpolation = 1.0f) {
        auto view = world.View<Transform, Glow>();

        for (auto entity : view) {
            const auto& transform = view.get<Transform>(entity);
            const auto& glow = view.get<Glow>(entity);

            if (!glow.enabled) continue;

            // Interpolate position for smooth rendering
            sf::Vector2f render_pos = Interpolate(
                transform.last_position,
                transform.position,
                interpolation
            );

            // Add glow at entity position
            renderer.AddGlow(render_pos, glow.color, glow.attenuation);
        }
    }

    // Render debug collision shapes
    static void RenderDebug(World& world, sf::RenderTarget& target) {
        auto view = world.View<Transform, Collision>();

        for (auto entity : view) {
            const auto& transform = view.get<Transform>(entity);
            const auto& collision = view.get<Collision>(entity);

            if (!collision.enabled) continue;

            sf::Vector2f pos = transform.position + collision.offset;

            if (collision.shape == Collision::Shape::CIRCLE) {
                sf::CircleShape circle(collision.radius);
                circle.setPosition(pos.x - collision.radius, pos.y - collision.radius);
                circle.setFillColor(sf::Color::Transparent);
                circle.setOutlineColor(sf::Color::Green);
                circle.setOutlineThickness(1.0f);
                target.draw(circle);
            } else if (collision.shape == Collision::Shape::RECTANGLE) {
                sf::RectangleShape rect(collision.rect_size);
                rect.setPosition(pos.x - collision.rect_size.x / 2.0f,
                               pos.y - collision.rect_size.y / 2.0f);
                rect.setFillColor(sf::Color::Transparent);
                rect.setOutlineColor(sf::Color::Green);
                rect.setOutlineThickness(1.0f);
                target.draw(rect);
            }
        }
    }

private:
    static void RenderSprite(sf::RenderTarget& target, const Sprite& sprite,
                            const sf::Vector2f& position, float rotation, float scale) {
        if (!sprite.texture) {
            // No texture - render colored rectangle
            sf::RectangleShape rect(sprite.size);
            rect.setOrigin(sprite.origin);
            rect.setPosition(position);
            rect.setRotation(rotation);
            rect.setScale(scale, scale);
            rect.setFillColor(sprite.color);
            target.draw(rect);
        } else {
            // Render textured sprite
            sf::Sprite sf_sprite(*sprite.texture, sprite.texture_rect);
            sf_sprite.setOrigin(sprite.origin);
            sf_sprite.setPosition(position);
            sf_sprite.setRotation(rotation);
            sf_sprite.setScale(scale, scale);
            sf_sprite.setColor(sprite.color);
            target.draw(sf_sprite);
        }
    }

    static void RenderAim(sf::RenderTarget& target, const sf::Vector2f& position,
                        const sf::Vector2f& mouse_position) {

        auto color = sf::Color(255, 0, 0, 100);
                            
        sf::Vertex line[] =
        {
            sf::Vertex(position, color),
            sf::Vertex(mouse_position, color)
        };
        target.draw(line, 2, sf::Lines);
    }

    static sf::Vector2f Interpolate(const sf::Vector2f& a, const sf::Vector2f& b, float t) {
        return a + (b - a) * t;
    }
};

} // namespace ecs

#endif // ECS_RENDER_SYSTEM_H
