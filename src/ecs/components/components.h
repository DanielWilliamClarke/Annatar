#ifndef ECS_COMPONENTS_H
#define ECS_COMPONENTS_H

#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include <vector>

namespace ecs {

// Transform component - position, rotation, scale, velocity
struct Transform {
    sf::Vector2f position{0.0f, 0.0f};
    sf::Vector2f last_position{0.0f, 0.0f};  // For interpolation
    sf::Vector2f velocity{0.0f, 0.0f};
    float rotation{0.0f};  // In degrees
    float scale{1.0f};
};

// Sprite component - visual representation
struct Sprite {
    sf::Texture* texture{nullptr};  // Non-owning pointer to texture atlas
    sf::IntRect texture_rect;  // Sub-rectangle for sprite sheet
    sf::Color color{sf::Color::White};
    sf::Vector2f size{32.0f, 32.0f};
    sf::Vector2f origin{16.0f, 16.0f};  // Relative to size
    int layer{0};  // Render layer (higher = drawn later)
    bool visible{true};
};

// Health component - HP, shields, regeneration
struct Health {
    float current{100.0f};
    float maximum{100.0f};
    float shield{0.0f};
    float shield_maximum{0.0f};
    float shield_regen_rate{0.0f};  // Per second
    float shield_regen_delay{2.0f};  // Seconds after taking damage
    float time_since_damage{0.0f};
    bool invulnerable{false};
    bool dead{false};
};

// Weapon component - weapon state and config
struct Weapon {
    enum class Type {
        SINGLE_SHOT,
        BURST,
        BEAM,
        HOMING,
        RANDOM_SPREAD
    };

    Type type{Type::SINGLE_SHOT};
    int slot{0};  // Weapon slot number (1-4)
    bool active{true};  // Is this weapon slot enabled?
    float cooldown{0.5f};  // Seconds between shots
    float current_cooldown{0.0f};  // Time remaining until can fire
    float damage{10.0f};
    float bullet_speed{400.0f};
    int bullets_per_shot{1};
    float spread_angle{0.0f};  // For burst weapons
    sf::Color bullet_color{255, 255, 255};
    sf::Vector2f bullet_size{8.0f, 16.0f};
    std::string script_id;  // Lua script for custom behavior
};

// Physics component - for acceleration-based movement (creates "floaty" feel)
struct Physics {
    float mass{1.0f};                    // Mass in kg
    float friction{0.5f};                // Damping coefficient (higher = faster stopping)
    float movement_force{10.0f};         // Force magnitude from input
    sf::Vector2f acceleration{0.0f, 0.0f};  // Current acceleration (calculated each frame)

    // Enemy physics forces (for entities with Movement.use_physics = true)
    sf::Vector2f gravity{0.0f, 0.0f};    // Gravity force vector (e.g., {0, 9.81})
    sf::Vector2f thrust{0.0f, 0.0f};     // Thrust force vector (e.g., {0, -9.81})
};

// Movement component - movement behavior
struct Movement {
    enum class Pattern {
        LINEAR,
        ORBITAL,
        SINE_WAVE,
        FOLLOW_TARGET,
        SCRIPTED
    };

    Pattern pattern{Pattern::LINEAR};
    float speed{100.0f};
    float max_speed{400.0f};
    float acceleration{200.0f};

    // Pattern-specific data
    float orbit_radius{150.0f};
    float orbit_speed{2.0f};
    float sine_amplitude{50.0f};
    float sine_frequency{1.0f};
    float pattern_time{0.0f};  // Time in current pattern

    sf::Vector2f direction{0.0f, -1.0f};  // Normalized direction
    std::string script_id;  // Lua script for custom movement

    // World scrolling (Gradius-style background drift)
    float world_speed{0.0f};  // Background scroll speed (added to enemy movement)

    // Orbital movement tracking
    sf::Vector2f orbit_center{0.0f, 0.0f};  // Center point for orbital movement
    bool orbit_initialized{false};  // Has orbit center been set?

    // Physics integration flag
    bool use_physics{false};  // If true, use Physics component for force-based movement
};

// Collision component - collision detection data
struct Collision {
    enum class Shape {
        CIRCLE,
        RECTANGLE
    };

    Shape shape{Shape::CIRCLE};
    float radius{16.0f};  // For circle
    sf::Vector2f rect_size{32.0f, 32.0f};  // For rectangle
    sf::Vector2f offset{0.0f, 0.0f};  // Offset from transform position

    // Collision layers (bit flags)
    uint32_t layer{0};  // What layer am I on?
    uint32_t mask{0xFFFFFFFF};  // What layers do I collide with?

    bool enabled{true};
};

// Lifetime component - auto-destroy after time
struct Lifetime {
    float duration{5.0f};  // Total lifetime in seconds
    float elapsed{0.0f};  // Time elapsed
};

// AI component - AI state and behavior
struct AI {
    enum class State {
        IDLE,
        PATROL,
        CHASE,
        ATTACK,
        FLEE
    };

    State state{State::IDLE};
    float state_time{0.0f};  // Time in current state
    float detection_range{200.0f};
    float attack_range{100.0f};
    std::optional<entt::entity> target;  // Current target entity
    std::string script_id;  // Lua script for custom AI
};

// Score component - value when destroyed
struct Score {
    float value{100.0f};
};

// Input component - player input state
struct Input {
    sf::Vector2f move_direction{0.0f, 0.0f};
    bool fire{false};
    bool weapon_slot_1{false};
    bool weapon_slot_2{false};
    bool weapon_slot_3{false};
    bool weapon_slot_4{false};
};

// Parent component - for hierarchical entities
struct Parent {
    entt::entity parent_entity{entt::null};
    sf::Vector2f local_position{0.0f, 0.0f};
    float local_rotation{0.0f};
};

// Children component - for entities with sub-objects
struct Children {
    std::vector<entt::entity> entities;
};

// Animation clip - defines one animation sequence (one row in sprite sheet)
struct AnimationClip {
    int row{0};                  // Which row in sprite sheet
    int start_col{0};            // Starting column
    int frame_count{1};          // How many frames in this clip
    float frame_duration{0.1f};  // Seconds per frame
    bool loop{true};             // Loop or play once
};

// Animation component - handles sprite sheet frame animation
struct Animation {
    // Sprite sheet layout
    sf::Vector2i frame_size{32, 32};         // Width/height per frame (pixels)
    int total_cols{1};                        // Total columns in sprite sheet
    int total_rows{1};                        // Total rows in sprite sheet

    // Animation clips (keyed by animation ID: IDLE=0, MOVING_UP=1, etc.)
    std::unordered_map<int, AnimationClip> clips;

    // Current playback state
    int current_animation{0};     // Which animation is playing
    int current_frame{0};         // Current frame index within animation
    float frame_timer{0.0f};      // Time accumulator
    bool finished{false};         // Animation completed (for non-looping)

    // Priority system (some animations can't be interrupted)
    bool priority_active{false};  // Is a priority animation playing?
    int priority_id{-1};          // ID of priority animation
};

// Background component - for parallax scrolling stars
struct Background {
    float parallax_factor{1.0f};  // Speed multiplier for parallax effect
    int layer{0};  // Which parallax layer (0-3)
};

// Tags for entity types (empty structs, presence indicates type)
struct PlayerTag {};
struct EnemyTag {};
struct BulletTag {};
struct ParticleTag {};
struct PowerupTag {};
struct BackgroundTag {};

} // namespace ecs

#endif // ECS_COMPONENTS_H
