#ifndef ECS_RENDER_SYSTEM_H
#define ECS_RENDER_SYSTEM_H

#include "../world.h"
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

            // Interpolate position for smooth rendering
            sf::Vector2f render_pos = Interpolate(
                transform.last_position,
                transform.position,
                interpolation
            );

            RenderSprite(target, sprite, render_pos, transform.rotation, transform.scale);
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

    static sf::Vector2f Interpolate(const sf::Vector2f& a, const sf::Vector2f& b, float t) {
        return a + (b - a) * t;
    }
};

} // namespace ecs

#endif // ECS_RENDER_SYSTEM_H
