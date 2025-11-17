#ifndef ECS_INPUT_SYSTEM_H
#define ECS_INPUT_SYSTEM_H

#include "../world.h"
#include "util/math_utils.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>

namespace ecs {

/**
 * InputSystem - Samples keyboard input and updates Input components
 * Pure ECS approach - no legacy PlayerInput dependency
 */
class InputSystem {
public:
    // Update all Input components from keyboard state
    static void Update(World& world, const sf::RenderWindow& window) {
       auto view = world.View<Input, Transform>();

        for (auto entity : view) {
            auto& input = view.get<Input>(entity);
            auto& transform = view.get<Transform>(entity);

            // Sample movement input
            input.move_direction = SampleMovement();

            // Sample fire button
            input.fire = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

            // aim with mouse position (convert from screen pixels to world coords)
            SampleMouseAim(input, window, transform.position);

            // Sample weapon slot toggles (1-4 keys)
            UpdateWeaponSlots(input);
        }
    }

    // Reset key state tracking (call when game state changes)
    static void Reset() {
        key_pressed.clear();
    }

private:
    // Track key press states for toggle detection
    static std::unordered_map<sf::Keyboard::Key, bool> key_pressed;

    // Sample movement from WASD or arrow keys
    static sf::Vector2f SampleMovement() {
        sf::Vector2f movement(0.0f, 0.0f);

        // Vertical movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            movement.y = -1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            movement.y = 1.0f;
        }

        // Horizontal movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            movement.x = -1.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            movement.x = 1.0f;
        }

        return movement;
    }

    // Update weapon slot toggles with press-release detection
    static void UpdateWeaponSlots(Input& input) {
        ProcessToggle(sf::Keyboard::Num1, input.weapon_slot_1, true);   // Slot 1 default ON
        ProcessToggle(sf::Keyboard::Num2, input.weapon_slot_2, false);  // Slot 2 default OFF
        ProcessToggle(sf::Keyboard::Num3, input.weapon_slot_3, false);  // Slot 3 default OFF
        ProcessToggle(sf::Keyboard::Num4, input.weapon_slot_4, false);  // Slot 4 default OFF
    }

    // Toggle slot on key release (Gradius-style weapon slot switching)
    static void ProcessToggle(sf::Keyboard::Key key, bool& slot_active, bool default_state) {
        bool currently_pressed = sf::Keyboard::isKeyPressed(key);
        bool was_pressed = key_pressed[key];

        // On first frame, initialize to default
        if (key_pressed.find(key) == key_pressed.end()) {
            slot_active = default_state;
            key_pressed[key] = false;
        }

        // Toggle on release (key was pressed, now released)
        if (was_pressed && !currently_pressed) {
            slot_active = !slot_active;
        }

        key_pressed[key] = currently_pressed;
    }

    static void SampleMouseAim(Input& input, const sf::RenderWindow& window,
                                       sf::Vector2f player_position = sf::Vector2f(0.0f, 0.0f)) {
        // Get mouse in window pixel space, convert to world space using current view
        sf::Vector2i mouse_pixels = sf::Mouse::getPosition(window);
        input.mouse_position = window.mapPixelToCoords(mouse_pixels);

        sf::Vector2f aim_dir = input.mouse_position - player_position;
        if (aim_dir.x != 0.0f || aim_dir.y != 0.0f) {
            input.aim_direction = Dimensions::Normalise(aim_dir);
        }
    }
};

// Static member - must be defined in .cc file for proper linking
// For header-only, we use inline
inline std::unordered_map<sf::Keyboard::Key, bool> InputSystem::key_pressed;

} // namespace ecs

#endif // ECS_INPUT_SYSTEM_H
