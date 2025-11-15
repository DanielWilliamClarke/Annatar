#ifndef ECS_CONFIG_LOADER_H
#define ECS_CONFIG_LOADER_H

#include <toml++/toml.h>
#include <string>
#include <optional>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "../components/components.h"

namespace ecs {

/**
 * AnimationClipConfig - Configuration for a single animation clip
 */
struct AnimationClipConfig {
    std::string name;
    int id{0};
    int row{0};
    int start_col{0};
    int frame_count{1};
    float duration{0.1f};
    bool loop{true};
};

/**
 * AnimationConfig - Configuration for sprite sheet animations
 */
struct AnimationConfig {
    std::string sprite_sheet_name;
    int cols{1};
    int rows{1};
    std::vector<AnimationClipConfig> clips;
};

/**
 * WeaponConfig - Configuration data for a weapon type
 */
struct WeaponConfig {
    std::string name;
    Weapon::Type type;
    float cooldown;
    float damage;
    float bullet_speed;
    int bullets_per_shot;
    float spread_angle;
    sf::Vector2f bullet_size;
    sf::Color bullet_color;
};

/**
 * PlayerPartConfig - Configuration for one player sub-entity (ship/exhaust/turret/glowie)
 */
struct PlayerPartConfig {
    std::string sprite_sheet;
    AnimationConfig animation;
    sf::Vector2f offset{0.0f, 0.0f};  // For sub-entities (turret, glowie)
    std::string weapon;
    int weapon_slot{1};
    float orbital_radius{0.0f};  // For glowie
    float orbital_speed{0.0f};   // For glowie
};

/**
 * PlayerConfig - Configuration for complete player (all parts)
 */
struct PlayerConfig {
    PlayerPartConfig ship;
    PlayerPartConfig exhaust;
    PlayerPartConfig turret;
    PlayerPartConfig glowie;
};

/**
 * EnemyConfig - Configuration data for an enemy type
 */
struct EnemyConfig {
    std::string name;
    float health;
    Movement::Pattern movement_pattern;
    float movement_speed;
    sf::Vector2f direction;
    float sine_amplitude{0.0f};
    float sine_frequency{0.0f};
    float orbit_radius{0.0f};
    float orbit_speed{0.0f};
    std::string weapon;
    float score_value;
    sf::Vector2f size;
    float collision_radius;
    sf::Color color;
    // Animation
    AnimationConfig animation;
};

/**
 * GameConstants - Centralized game configuration
 */
struct GameConstants {
    // Player
    float player_max_health{100.0f};
    float player_max_shield{50.0f};
    float player_shield_regen_rate{10.0f};
    float player_shield_regen_delay{2.0f};
    float player_movement_speed{300.0f};
    float player_max_speed{400.0f};
    float player_collision_radius{16.0f};
    sf::Vector2f player_size{32.0f, 32.0f};
    sf::Vector2f player_starting_position{400.0f, 500.0f};
    std::string player_starting_weapon{"plasma_rifle"};
    // Physics
    float player_mass{1.0f};
    float player_friction{0.5f};
    float player_movement_force{10.0f};

    // Game
    int window_width{800};
    int window_height{600};
    int target_fps{60};
    float fixed_timestep{0.016666f};
    int max_bullets{1000};
    int max_enemies{100};
    int max_particles{500};

    // Bounds
    float bounds_min_x{0.0f};
    float bounds_max_x{800.0f};
    float bounds_min_y{0.0f};
    float bounds_max_y{600.0f};
    float despawn_margin{100.0f};

    // Collision layers
    uint32_t layer_player{0x01};
    uint32_t layer_enemy{0x02};
    uint32_t layer_enemy_bullet{0x04};
    uint32_t layer_player_bullet{0x08};
    uint32_t layer_powerup{0x10};

    // Debug
    bool debug_show_collision_shapes{false};
    bool debug_show_fps{true};
    bool debug_show_entity_count{true};
    bool debug_god_mode{false};

    // Performance
    bool use_spatial_partitioning{true};
    int quadtree_max_depth{6};
    int quadtree_max_objects{10};
};

/**
 * ConfigLoader - Loads and manages game configuration from TOML files
 */
class ConfigLoader {
public:
    ConfigLoader() = default;
    ~ConfigLoader() = default;

    // Load configuration files
    bool LoadWeapons(const std::string& filepath);
    bool LoadEnemies(const std::string& filepath);
    bool LoadConstants(const std::string& filepath);
    bool LoadPlayer(const std::string& filepath);

    // Load all configs from directory
    bool LoadAll(const std::string& config_dir = "config");

    // Get weapon config by name
    std::optional<WeaponConfig> GetWeapon(const std::string& name) const;

    // Get enemy config by name
    std::optional<EnemyConfig> GetEnemy(const std::string& name) const;

    // Get game constants
    const GameConstants& GetConstants() const { return constants; }

    // Get player configuration
    const PlayerConfig& GetPlayerConfig() const { return player_config; }

    // List available weapons/enemies
    std::vector<std::string> ListWeapons() const;
    std::vector<std::string> ListEnemies() const;

private:
    std::unordered_map<std::string, WeaponConfig> weapons;
    std::unordered_map<std::string, EnemyConfig> enemies;
    PlayerConfig player_config;
    GameConstants constants;

    // Helper methods
    static Weapon::Type ParseWeaponType(const std::string& type_str);
    static Movement::Pattern ParseMovementPattern(const std::string& pattern_str);
    static sf::Color ParseColor(const toml::array& color_array);
    static sf::Vector2f ParseVector2f(const toml::array& vec_array);
    static AnimationConfig ParseAnimation(const toml::table& entity_table);
    static PlayerPartConfig ParsePlayerPart(const toml::table& part_table);
};

} // namespace ecs

#endif // ECS_CONFIG_LOADER_H
