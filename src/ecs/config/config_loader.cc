#include "config_loader.h"
#include <iostream>
#include <filesystem>

namespace ecs {

bool ConfigLoader::LoadWeapons(const std::string& filepath) {
    try {
        auto config = toml::parse_file(filepath);
        auto weapons_table = config["weapons"];

        if (!weapons_table) {
            std::cerr << "No 'weapons' table found in " << filepath << std::endl;
            return false;
        }

        for (auto&& [key, value] : *weapons_table.as_table()) {
            std::string weapon_name = std::string(key.str());
            auto weapon_table = value.as_table();

            WeaponConfig wc;

            // Safe get with nullptr check
            if (auto node = weapon_table->get("name")) {
                wc.name = node->value_or(weapon_name);
            } else {
                wc.name = weapon_name;
            }

            if (auto node = weapon_table->get("type")) {
                wc.type = ParseWeaponType(node->value_or("single_shot"));
            } else {
                wc.type = Weapon::Type::SINGLE_SHOT;
            }

            if (auto node = weapon_table->get("cooldown")) {
                wc.cooldown = node->value_or(0.5);
            } else {
                wc.cooldown = 0.5;
            }

            if (auto node = weapon_table->get("damage")) {
                wc.damage = node->value_or(10.0);
            } else {
                wc.damage = 10.0;
            }

            if (auto node = weapon_table->get("bullet_speed")) {
                wc.bullet_speed = node->value_or(400.0);
            } else {
                wc.bullet_speed = 400.0;
            }

            if (auto node = weapon_table->get("bullets_per_shot")) {
                wc.bullets_per_shot = node->value_or(1);
            } else {
                wc.bullets_per_shot = 1;
            }

            if (auto node = weapon_table->get("spread_angle")) {
                wc.spread_angle = node->value_or(0.0);
            } else {
                wc.spread_angle = 0.0;
            }

            if (auto size_node = weapon_table->get("bullet_size")) {
                if (auto size = size_node->as_array()) {
                    wc.bullet_size = ParseVector2f(*size);
                } else {
                    wc.bullet_size = {8.0f, 16.0f};
                }
            } else {
                wc.bullet_size = {8.0f, 16.0f};
            }

            if (auto color_node = weapon_table->get("bullet_color")) {
                if (auto color = color_node->as_array()) {
                    wc.bullet_color = ParseColor(*color);
                } else {
                    wc.bullet_color = sf::Color::White;
                }
            } else {
                wc.bullet_color = sf::Color::White;
            }

            weapons[weapon_name] = wc;
        }

        std::cout << "Loaded " << weapons.size() << " weapons from " << filepath << std::endl;
        return true;

    } catch (const toml::parse_error& err) {
        std::cerr << "Failed to parse " << filepath << ": " << err.description() << std::endl;
        return false;
    }
}

bool ConfigLoader::LoadEnemies(const std::string& filepath) {
    try {
        std::cout << "[ConfigLoader] Parsing " << filepath << "..." << std::endl;
        std::cout.flush();

        auto config = toml::parse_file(filepath);

        std::cout << "[ConfigLoader] File parsed, looking for enemies table..." << std::endl;
        std::cout.flush();

        auto enemies_table = config["enemies"];

        if (!enemies_table) {
            std::cerr << "No 'enemies' table found in " << filepath << std::endl;
            return false;
        }

        std::cout << "[ConfigLoader] Processing enemy definitions..." << std::endl;
        std::cout.flush();

        for (auto&& [key, value] : *enemies_table.as_table()) {
            std::string enemy_name = std::string(key.str());
            std::cout << "[ConfigLoader]   Loading enemy: " << enemy_name << std::endl;
            std::cout.flush();

            auto enemy_table = value.as_table();

            EnemyConfig ec;

            // Safe get with nullptr check
            if (auto name_node = enemy_table->get("name")) {
                ec.name = name_node->value_or(enemy_name);
            } else {
                ec.name = enemy_name;
            }

            if (auto health_node = enemy_table->get("health")) {
                ec.health = health_node->value_or(30.0);
            } else {
                ec.health = 30.0;
            }

            if (auto pattern_node = enemy_table->get("movement_pattern")) {
                ec.movement_pattern = ParseMovementPattern(pattern_node->value_or("linear"));
            } else {
                ec.movement_pattern = Movement::Pattern::LINEAR;
            }

            if (auto speed_node = enemy_table->get("movement_speed")) {
                ec.movement_speed = speed_node->value_or(100.0);
            } else {
                ec.movement_speed = 100.0;
            }

            if (auto dir_node = enemy_table->get("direction")) {
                if (auto dir = dir_node->as_array()) {
                    ec.direction = ParseVector2f(*dir);
                } else {
                    ec.direction = {0.0f, 1.0f};
                }
            } else {
                ec.direction = {0.0f, 1.0f};
            }

            if (auto sine_amp_node = enemy_table->get("sine_amplitude")) {
                ec.sine_amplitude = sine_amp_node->value_or(0.0);
            } else {
                ec.sine_amplitude = 0.0;
            }

            if (auto sine_freq_node = enemy_table->get("sine_frequency")) {
                ec.sine_frequency = sine_freq_node->value_or(0.0);
            } else {
                ec.sine_frequency = 0.0;
            }

            if (auto orbit_rad_node = enemy_table->get("orbit_radius")) {
                ec.orbit_radius = orbit_rad_node->value_or(0.0);
            } else {
                ec.orbit_radius = 0.0;
            }

            if (auto orbit_spd_node = enemy_table->get("orbit_speed")) {
                ec.orbit_speed = orbit_spd_node->value_or(0.0);
            } else {
                ec.orbit_speed = 0.0;
            }

            if (auto weapon_node = enemy_table->get("weapon")) {
                ec.weapon = weapon_node->value_or("");
            } else {
                ec.weapon = "";
            }

            if (auto score_node = enemy_table->get("score_value")) {
                ec.score_value = score_node->value_or(100.0);
            } else {
                ec.score_value = 100.0;
            }

            if (auto size_node = enemy_table->get("size")) {
                if (auto size = size_node->as_array()) {
                    ec.size = ParseVector2f(*size);
                } else {
                    ec.size = {32.0f, 32.0f};
                }
            } else {
                ec.size = {32.0f, 32.0f};
            }

            if (auto radius_node = enemy_table->get("collision_radius")) {
                ec.collision_radius = radius_node->value_or(16.0);
            } else {
                ec.collision_radius = 16.0;
            }

            if (auto color_node = enemy_table->get("color")) {
                if (auto color = color_node->as_array()) {
                    ec.color = ParseColor(*color);
                } else {
                    ec.color = sf::Color::Red;
                }
            } else {
                ec.color = sf::Color::Red;
            }

            // Parse animation configuration
            ec.animation = ParseAnimation(*enemy_table);

            enemies[enemy_name] = ec;
            std::cout << "[ConfigLoader]   ✓ " << enemy_name << " loaded" << std::endl;
        }

        std::cout << "Loaded " << enemies.size() << " enemies from " << filepath << std::endl;
        return true;

    } catch (const toml::parse_error& err) {
        std::cerr << "Failed to parse " << filepath << ": " << err.description() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Exception loading enemies: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception loading enemies from " << filepath << std::endl;
        return false;
    }
}

bool ConfigLoader::LoadConstants(const std::string& filepath) {
    try {
        auto config = toml::parse_file(filepath);

        // Player constants
        if (auto player = config["player"].as_table()) {
            if (auto node = player->get("max_health")) constants.player_max_health = node->value_or(100.0);
            if (auto node = player->get("max_shield")) constants.player_max_shield = node->value_or(50.0);
            if (auto node = player->get("shield_regen_rate")) constants.player_shield_regen_rate = node->value_or(10.0);
            if (auto node = player->get("shield_regen_delay")) constants.player_shield_regen_delay = node->value_or(2.0);
            if (auto node = player->get("movement_speed")) constants.player_movement_speed = node->value_or(300.0);
            if (auto node = player->get("max_speed")) constants.player_max_speed = node->value_or(400.0);
            if (auto node = player->get("collision_radius")) constants.player_collision_radius = node->value_or(16.0);

            if (auto size_node = player->get("size")) {
                if (auto size = size_node->as_array()) {
                    constants.player_size = ParseVector2f(*size);
                }
            }

            if (auto pos_node = player->get("starting_position")) {
                if (auto pos = pos_node->as_array()) {
                    constants.player_starting_position = ParseVector2f(*pos);
                }
            }

            if (auto weapon_node = player->get("starting_weapon")) {
                constants.player_starting_weapon = weapon_node->value_or("plasma_rifle");
            }

            // Physics parameters
            if (auto node = player->get("mass")) constants.player_mass = node->value_or(1.0);
            if (auto node = player->get("friction")) constants.player_friction = node->value_or(0.5);
            if (auto node = player->get("movement_force")) constants.player_movement_force = node->value_or(1200.0);
        }

        // Game constants
        if (auto game = config["game"].as_table()) {
            if (auto node = game->get("window_width")) constants.window_width = node->value_or(800);
            if (auto node = game->get("window_height")) constants.window_height = node->value_or(600);
            if (auto node = game->get("target_fps")) constants.target_fps = node->value_or(60);
            if (auto node = game->get("fixed_timestep")) constants.fixed_timestep = node->value_or(0.016666);
            if (auto node = game->get("max_bullets")) constants.max_bullets = node->value_or(1000);
            if (auto node = game->get("max_enemies")) constants.max_enemies = node->value_or(100);
            if (auto node = game->get("max_particles")) constants.max_particles = node->value_or(500);
            if (auto node = game->get("world_speed")) constants.world_speed = node->value_or(100.0f);
        }

        // Bounds
        if (auto bounds = config["bounds"].as_table()) {
            if (auto node = bounds->get("min_x")) constants.bounds_min_x = node->value_or(0.0);
            if (auto node = bounds->get("max_x")) constants.bounds_max_x = node->value_or(800.0);
            if (auto node = bounds->get("min_y")) constants.bounds_min_y = node->value_or(0.0);
            if (auto node = bounds->get("max_y")) constants.bounds_max_y = node->value_or(600.0);
            if (auto node = bounds->get("despawn_margin")) constants.despawn_margin = node->value_or(100.0);
        }

        // Collision layers
        if (auto layers = config["collision_layers"].as_table()) {
            if (auto node = layers->get("player")) constants.layer_player = node->value_or(0x01);
            if (auto node = layers->get("enemy")) constants.layer_enemy = node->value_or(0x02);
            if (auto node = layers->get("enemy_bullet")) constants.layer_enemy_bullet = node->value_or(0x04);
            if (auto node = layers->get("player_bullet")) constants.layer_player_bullet = node->value_or(0x08);
            if (auto node = layers->get("powerup")) constants.layer_powerup = node->value_or(0x10);
        }

        // Debug
        if (auto debug = config["debug"].as_table()) {
            if (auto node = debug->get("show_collision_shapes")) constants.debug_show_collision_shapes = node->value_or(false);
            if (auto node = debug->get("show_fps")) constants.debug_show_fps = node->value_or(true);
            if (auto node = debug->get("show_entity_count")) constants.debug_show_entity_count = node->value_or(true);
            if (auto node = debug->get("god_mode")) constants.debug_god_mode = node->value_or(false);
        }

        // Performance
        if (auto perf = config["performance"].as_table()) {
            if (auto node = perf->get("use_spatial_partitioning")) constants.use_spatial_partitioning = node->value_or(true);
            if (auto node = perf->get("quadtree_max_depth")) constants.quadtree_max_depth = node->value_or(6);
            if (auto node = perf->get("quadtree_max_objects")) constants.quadtree_max_objects = node->value_or(10);
        }

        std::cout << "Loaded constants from " << filepath << std::endl;
        return true;

    } catch (const toml::parse_error& err) {
        std::cerr << "Failed to parse " << filepath << ": " << err.description() << std::endl;
        return false;
    }
}

bool ConfigLoader::LoadAll(const std::string& config_dir) {
    bool success = true;

    std::cout << "[ConfigLoader] Config directory: " << config_dir << std::endl;
    std::cout << "[ConfigLoader] Current working directory: " << std::filesystem::current_path() << std::endl;
    std::cout.flush();

    std::string weapons_path = config_dir + "/weapons.toml";
    std::string enemies_path = config_dir + "/enemies.toml";
    std::string constants_path = config_dir + "/constants.toml";

    std::cout << "[ConfigLoader] Checking if files exist..." << std::endl;
    std::cout << "  weapons.toml: " << (std::filesystem::exists(weapons_path) ? "✓" : "✗") << std::endl;
    std::cout << "  enemies.toml: " << (std::filesystem::exists(enemies_path) ? "✓" : "✗") << std::endl;
    std::cout << "  constants.toml: " << (std::filesystem::exists(constants_path) ? "✓" : "✗") << std::endl;
    std::cout.flush();

    std::cout << "[ConfigLoader] Loading weapons..." << std::endl;
    std::cout.flush();
    success &= LoadWeapons(weapons_path);

    std::cout << "[ConfigLoader] Loading enemies..." << std::endl;
    std::cout.flush();
    success &= LoadEnemies(enemies_path);

    std::cout << "[ConfigLoader] Loading constants..." << std::endl;
    std::cout.flush();
    success &= LoadConstants(constants_path);

    std::cout << "[ConfigLoader] Loading player..." << std::endl;
    std::cout.flush();
    std::string player_path = config_dir + "/player.toml";
    success &= LoadPlayer(player_path);

    std::cout << "[ConfigLoader] All configs loaded" << std::endl;
    return success;
}

std::optional<WeaponConfig> ConfigLoader::GetWeapon(const std::string& name) const {
    auto it = weapons.find(name);
    if (it != weapons.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<EnemyConfig> ConfigLoader::GetEnemy(const std::string& name) const {
    auto it = enemies.find(name);
    if (it != enemies.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> ConfigLoader::ListWeapons() const {
    std::vector<std::string> names;
    names.reserve(weapons.size());
    for (const auto& [name, _] : weapons) {
        names.push_back(name);
    }
    return names;
}

std::vector<std::string> ConfigLoader::ListEnemies() const {
    std::vector<std::string> names;
    names.reserve(enemies.size());
    for (const auto& [name, _] : enemies) {
        names.push_back(name);
    }
    return names;
}

Weapon::Type ConfigLoader::ParseWeaponType(const std::string& type_str) {
    if (type_str == "single_shot") return Weapon::Type::SINGLE_SHOT;
    if (type_str == "burst") return Weapon::Type::BURST;
    if (type_str == "beam") return Weapon::Type::BEAM;
    if (type_str == "homing") return Weapon::Type::HOMING;
    if (type_str == "random_spread") return Weapon::Type::RANDOM_SPREAD;
    return Weapon::Type::SINGLE_SHOT;
}

Movement::Pattern ConfigLoader::ParseMovementPattern(const std::string& pattern_str) {
    if (pattern_str == "linear") return Movement::Pattern::LINEAR;
    if (pattern_str == "orbital") return Movement::Pattern::ORBITAL;
    if (pattern_str == "sine_wave") return Movement::Pattern::SINE_WAVE;
    if (pattern_str == "follow_target") return Movement::Pattern::FOLLOW_TARGET;
    if (pattern_str == "scripted") return Movement::Pattern::SCRIPTED;
    return Movement::Pattern::LINEAR;
}

sf::Color ConfigLoader::ParseColor(const toml::array& color_array) {
    if (color_array.size() >= 3) {
        auto r_node = color_array.get(0);
        auto g_node = color_array.get(1);
        auto b_node = color_array.get(2);

        if (r_node && g_node && b_node) {
            int r = r_node->value_or(255);
            int g = g_node->value_or(255);
            int b = b_node->value_or(255);
            return sf::Color(r, g, b);
        }
    }
    return sf::Color::White;
}

sf::Vector2f ConfigLoader::ParseVector2f(const toml::array& vec_array) {
    if (vec_array.size() >= 2) {
        auto x_node = vec_array.get(0);
        auto y_node = vec_array.get(1);

        if (x_node && y_node) {
            float x = x_node->value_or(0.0);
            float y = y_node->value_or(0.0);
            return {x, y};
        }
    }
    return {0.0f, 0.0f};
}

PlayerPartConfig ConfigLoader::ParsePlayerPart(const toml::table& part_table) {
    PlayerPartConfig part;

    // Parse sprite sheet name
    if (auto sprite_node = part_table.get("sprite_sheet")) {
        part.sprite_sheet = sprite_node->value_or("");
    }

    // Parse animation
    part.animation = ParseAnimation(part_table);

    // Parse offset (for sub-entities like turret, glowie)
    if (auto offset_node = part_table.get("offset")) {
        if (auto offset_array = offset_node->as_array()) {
            part.offset = ParseVector2f(*offset_array);
        }
    }

    // Parse weapon
    if (auto weapon_node = part_table.get("weapon")) {
        part.weapon = weapon_node->value_or("");
    }

    // Parse weapon slot
    if (auto slot_node = part_table.get("weapon_slot")) {
        part.weapon_slot = slot_node->value_or(1);
    }

    // Parse orbital parameters (for glowie)
    if (auto radius_node = part_table.get("orbital_radius")) {
        part.orbital_radius = radius_node->value_or(0.0);
    }

    if (auto speed_node = part_table.get("orbital_speed")) {
        part.orbital_speed = speed_node->value_or(0.0);
    }

    return part;
}

AnimationConfig ConfigLoader::ParseAnimation(const toml::table& entity_table) {
    AnimationConfig anim_config;

    // Parse sprite sheet info
    if (auto sprite_node = entity_table.get("animation_sprite_sheet")) {
        anim_config.sprite_sheet_name = sprite_node->value_or("");
    }

    if (auto cols_node = entity_table.get("animation_cols")) {
        anim_config.cols = cols_node->value_or(1);
    }

    if (auto rows_node = entity_table.get("animation_rows")) {
        anim_config.rows = rows_node->value_or(1);
    }

    // Parse direct pixel coordinates (for non-uniform sprite sheets)
    if (auto sprite_x_node = entity_table.get("sprite_x")) {
        anim_config.sprite_x = sprite_x_node->value_or(0);
    }

    if (auto sprite_y_node = entity_table.get("sprite_y")) {
        anim_config.sprite_y = sprite_y_node->value_or(0);
    }

    if (auto sprite_width_node = entity_table.get("sprite_width")) {
        anim_config.sprite_width = sprite_width_node->value_or(8);
    }

    if (auto sprite_height_node = entity_table.get("sprite_height")) {
        anim_config.sprite_height = sprite_height_node->value_or(8);
    }

    // Parse grid position (legacy, for uniform sprite sheets)
    if (auto sprite_col_node = entity_table.get("sprite_col")) {
        anim_config.sprite_col = sprite_col_node->value_or(0);
    }

    if (auto sprite_row_node = entity_table.get("sprite_row")) {
        anim_config.sprite_row = sprite_row_node->value_or(0);
    }

    // Check for animation array (complex multi-animation entities)
    if (auto animations_node = entity_table.get("animations")) {
        if (auto animations_array = animations_node->as_array()) {
            for (size_t i = 0; i < animations_array->size(); ++i) {
                auto anim_node = animations_array->get(i);
                if (!anim_node) continue;

                auto anim_table = anim_node->as_table();
                if (!anim_table) continue;

                AnimationClipConfig clip;

                if (auto name_node = anim_table->get("name")) {
                    clip.name = name_node->value_or("idle");
                }

                if (auto id_node = anim_table->get("id")) {
                    clip.id = id_node->value_or(0);
                }

                if (auto row_node = anim_table->get("row")) {
                    clip.row = row_node->value_or(0);
                }

                if (auto start_col_node = anim_table->get("start_col")) {
                    clip.start_col = start_col_node->value_or(0);
                }

                if (auto frame_count_node = anim_table->get("frame_count")) {
                    clip.frame_count = frame_count_node->value_or(1);
                }

                if (auto duration_node = anim_table->get("duration")) {
                    clip.duration = duration_node->value_or(0.1);
                }

                if (auto loop_node = anim_table->get("loop")) {
                    clip.loop = loop_node->value_or(true);
                }

                anim_config.clips.push_back(clip);
            }
        }
    } else {
        // Simple inline animation (single IDLE animation)
        AnimationClipConfig clip;
        clip.id = 0;  // IDLE
        clip.name = "idle";
        clip.row = 0;
        clip.start_col = 0;

        if (auto frame_count_node = entity_table.get("animation_cols")) {
            clip.frame_count = frame_count_node->value_or(1);
        }

        if (auto duration_node = entity_table.get("animation_frame_duration")) {
            clip.duration = duration_node->value_or(0.1);
        }

        if (auto loop_node = entity_table.get("animation_loop")) {
            clip.loop = loop_node->value_or(true);
        }

        anim_config.clips.push_back(clip);
    }

    return anim_config;
}

bool ConfigLoader::LoadPlayer(const std::string& filepath) {
    try {
        std::cout << "[ConfigLoader] Loading player config from " << filepath << "..." << std::endl;
        std::cout.flush();

        auto config = toml::parse_file(filepath);

        // Load player.ship
        if (auto ship_node = config["player"]["ship"]) {
            if (auto ship_table = ship_node.as_table()) {
                player_config.ship = ParsePlayerPart(*ship_table);
                std::cout << "[ConfigLoader]   ✓ Player ship config loaded" << std::endl;
            }
        }

        // Load player.exhaust
        if (auto exhaust_node = config["player"]["exhaust"]) {
            if (auto exhaust_table = exhaust_node.as_table()) {
                player_config.exhaust = ParsePlayerPart(*exhaust_table);
                std::cout << "[ConfigLoader]   ✓ Player exhaust config loaded" << std::endl;
            }
        }

        // Load player.turret
        if (auto turret_node = config["player"]["turret"]) {
            if (auto turret_table = turret_node.as_table()) {
                player_config.turret = ParsePlayerPart(*turret_table);
                std::cout << "[ConfigLoader]   ✓ Player turret config loaded" << std::endl;
            }
        }

        // Load player.glowie
        if (auto glowie_node = config["player"]["glowie"]) {
            if (auto glowie_table = glowie_node.as_table()) {
                player_config.glowie = ParsePlayerPart(*glowie_table);
                std::cout << "[ConfigLoader]   ✓ Player glowie config loaded" << std::endl;
            }
        }

        std::cout << "Loaded player configuration from " << filepath << std::endl;
        return true;

    } catch (const toml::parse_error& err) {
        std::cerr << "Failed to parse " << filepath << ": " << err.description() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Exception loading player config: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception loading player config from " << filepath << std::endl;
        return false;
    }
}

} // namespace ecs
