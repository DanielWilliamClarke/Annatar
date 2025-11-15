#include "enemy_spawn_system.h"
#include <toml++/toml.h>
#include <iostream>
#include <algorithm>

namespace ecs {

// Static member initialization
std::vector<SpawnWaveConfig> EnemySpawnSystem::waves;
std::mt19937 EnemySpawnSystem::rng;
std::uniform_real_distribution<float> EnemySpawnSystem::dist01(0.0f, 1.0f);
bool EnemySpawnSystem::enabled = true;
int EnemySpawnSystem::total_spawned = 0;

void EnemySpawnSystem::Initialize(unsigned int seed) {
    if (seed == 0) {
        std::random_device rd;
        seed = rd();
    }
    rng.seed(seed);
    total_spawned = 0;
    enabled = true;
}

void EnemySpawnSystem::AddWave(const SpawnWaveConfig& wave) {
    waves.push_back(wave);
}

void EnemySpawnSystem::AddContinuousWave(const std::string& enemy_type,
                                        float interval,
                                        int max_concurrent,
                                        float interval_variance) {
    SpawnWaveConfig wave;
    wave.enemy_pool = {enemy_type};
    wave.interval = interval;
    wave.interval_variance = interval_variance;
    wave.max_concurrent = max_concurrent;
    wave.continuous = true;
    wave.position_variance = 50.0f;  // Default variance
    AddWave(wave);
}

void EnemySpawnSystem::AddDiscreteWave(const std::vector<std::string>& enemy_pool,
                                      int count,
                                      float delay,
                                      float spawn_interval) {
    SpawnWaveConfig wave;
    wave.enemy_pool = enemy_pool;
    wave.interval = spawn_interval;
    wave.interval_variance = 0.0f;
    wave.max_concurrent = count;
    wave.continuous = false;
    wave.position_variance = 50.0f;  // Default variance
    wave.timer = -delay;  // Negative timer = delay before first spawn
    AddWave(wave);
}

bool EnemySpawnSystem::LoadSpawnWaves(const std::string& filepath) {
    try {
        auto config = toml::parse_file(filepath);

        // Check if spawn_waves section exists
        if (!config.contains("spawn_waves")) {
            std::cerr << "Warning: No [spawn_waves] section found in " << filepath << std::endl;
            return false;
        }

        auto spawn_waves = config["spawn_waves"];

        // Parse wave configurations
        // The TOML format is: wave_N_delay, wave_N_count, wave_N_enemy
        // We'll iterate through all keys and extract wave numbers

        std::map<int, SpawnWaveConfig> wave_map;

        // Iterate through all spawn_wave keys
        for (const auto& [key, value] : *spawn_waves.as_table()) {
            std::string key_str = std::string(key.str());

            // Parse key format: wave_N_property
            if (key_str.find("wave_") == 0) {
                // Extract wave number
                size_t first_underscore = key_str.find('_');
                size_t second_underscore = key_str.find('_', first_underscore + 1);

                if (first_underscore == std::string::npos || second_underscore == std::string::npos) {
                    continue;
                }

                int wave_num = std::stoi(key_str.substr(first_underscore + 1, second_underscore - first_underscore - 1));
                std::string property = key_str.substr(second_underscore + 1);

                // Create wave config if doesn't exist
                if (wave_map.find(wave_num) == wave_map.end()) {
                    wave_map[wave_num] = SpawnWaveConfig();
                    wave_map[wave_num].continuous = false;  // Discrete waves by default
                }

                // Set property
                if (property == "delay") {
                    wave_map[wave_num].timer = -value.value<float>().value();  // Negative = delay
                } else if (property == "count") {
                    wave_map[wave_num].max_concurrent = value.value<int>().value();
                } else if (property == "enemy") {
                    wave_map[wave_num].enemy_pool.push_back(value.value<std::string>().value());
                } else if (property == "enemies") {
                    // Support array of enemies
                    if (value.is_array()) {
                        for (const auto& enemy : *value.as_array()) {
                            wave_map[wave_num].enemy_pool.push_back(enemy.value<std::string>().value());
                        }
                    }
                } else if (property == "interval") {
                    wave_map[wave_num].interval = value.value<float>().value();
                } else if (property == "interval_variance") {
                    wave_map[wave_num].interval_variance = value.value<float>().value();
                } else if (property == "position_variance") {
                    wave_map[wave_num].position_variance = value.value<float>().value();
                } else if (property == "continuous") {
                    wave_map[wave_num].continuous = value.value<bool>().value();
                }
            }
        }

        // Add waves to system in order
        for (auto& [wave_num, wave_config] : wave_map) {
            // Set default interval for discrete waves if not specified
            if (!wave_config.continuous && wave_config.interval == 0.0f) {
                wave_config.interval = 0.5f;  // Default 0.5s between spawns in a wave
            }

            // Set default position variance if not specified
            if (wave_config.position_variance == 0.0f) {
                wave_config.position_variance = 50.0f;
            }

            AddWave(wave_config);
            std::cout << "Loaded spawn wave " << wave_num << ": "
                     << wave_config.enemy_pool.size() << " enemy types, "
                     << "interval=" << wave_config.interval << "s, "
                     << (wave_config.continuous ? "continuous" : "discrete")
                     << std::endl;
        }

        return !wave_map.empty();

    } catch (const toml::parse_error& err) {
        std::cerr << "Error parsing spawn waves from " << filepath << ": "
                  << err.description() << std::endl;
        return false;
    }
}

void EnemySpawnSystem::Update(World& world,
                             float dt,
                             EntityFactory& factory,
                             const sf::FloatRect& bounds,
                             ITextureAtlas& textureAtlas) {
    if (!enabled) {
        return;
    }

    // Update alive counts for all waves
    UpdateWaveAliveCounts(world);

    // Update each wave
    for (auto& wave : waves) {
        // Skip completed discrete waves
        if (!wave.continuous && wave.completed) {
            continue;
        }

        // Update timer
        wave.timer += dt;

        // Calculate actual spawn interval with variance
        float spawn_interval = wave.interval;
        if (wave.interval_variance > 0.0f) {
            float variance = GetRandomVariance() * wave.interval_variance;
            spawn_interval += variance;
        }

        // Check if it's time to spawn
        if (wave.timer >= spawn_interval) {
            // Try to spawn enemy
            if (TrySpawnEnemy(world, wave, factory, bounds, textureAtlas)) {
                // Reset timer
                wave.timer = 0.0f;

                // Increment spawn count
                wave.spawned_count++;
                wave.alive_count++;
                total_spawned++;

                // Check if discrete wave is complete
                if (!wave.continuous && wave.spawned_count >= wave.max_concurrent) {
                    wave.completed = true;
                }
            }
        }
    }
}

void EnemySpawnSystem::Clear() {
    waves.clear();
    total_spawned = 0;
}

size_t EnemySpawnSystem::GetWaveCount() {
    return waves.size();
}

int EnemySpawnSystem::GetTotalSpawned() {
    return total_spawned;
}

void EnemySpawnSystem::SetEnabled(bool is_enabled) {
    enabled = is_enabled;
}

bool EnemySpawnSystem::IsEnabled() {
    return enabled;
}

bool EnemySpawnSystem::TrySpawnEnemy(World& world,
                                    SpawnWaveConfig& wave,
                                    EntityFactory& factory,
                                    const sf::FloatRect& bounds,
                                    ITextureAtlas& textureAtlas) {
    // Check if we've reached the concurrent limit
    if (wave.max_concurrent > 0 && wave.alive_count >= wave.max_concurrent) {
        return false;
    }

    // Check if enemy pool is empty
    if (wave.enemy_pool.empty()) {
        std::cerr << "Warning: Empty enemy pool in spawn wave" << std::endl;
        return false;
    }

    // Select random enemy type from pool
    std::string enemy_type = SelectRandomEnemyType(wave.enemy_pool);

    // Get random spawn position
    sf::Vector2f spawn_pos = GetRandomSpawnPosition(bounds, wave.position_variance);

    // Get texture for enemy (try to load from atlas)
    sf::Texture* texture = nullptr;
    // Note: EntityFactory will handle texture loading internally via ConfigLoader

    // Create enemy entity
    try {
        entt::entity enemy = factory.CreateEnemy(enemy_type, spawn_pos, texture);

        // Verify entity was created
        if (enemy != entt::null && world.IsValid(enemy)) {
            return true;
        } else {
            std::cerr << "Warning: Failed to create enemy of type '" << enemy_type << "'" << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating enemy '" << enemy_type << "': " << e.what() << std::endl;
        return false;
    }
}

sf::Vector2f EnemySpawnSystem::GetRandomSpawnPosition(const sf::FloatRect& bounds,
                                                     float variance) {
    // Spawn at right edge of bounds
    float x = bounds.left + bounds.width;

    // Random Y position within bounds, with variance
    float center_y = bounds.top + bounds.height / 2.0f;
    float y_offset = GetRandomVariance() * variance;
    float y = center_y + y_offset;

    // Clamp to bounds (with some margin)
    float margin = 50.0f;
    y = std::max(bounds.top + margin, std::min(bounds.top + bounds.height - margin, y));

    return sf::Vector2f(x, y);
}

std::string EnemySpawnSystem::SelectRandomEnemyType(const std::vector<std::string>& enemy_pool) {
    if (enemy_pool.empty()) {
        return "";
    }

    if (enemy_pool.size() == 1) {
        return enemy_pool[0];
    }

    // Select random index
    std::uniform_int_distribution<size_t> dist(0, enemy_pool.size() - 1);
    size_t index = dist(rng);

    return enemy_pool[index];
}

float EnemySpawnSystem::GetRandomVariance() {
    return (dist01(rng) * 2.0f) - 1.0f;  // [-1.0, 1.0]
}

void EnemySpawnSystem::UpdateWaveAliveCounts(World& world) {
    // Count total enemies with EnemyTag
    auto enemy_view = world.View<EnemyTag>();
    int total_enemies = std::distance(enemy_view.begin(), enemy_view.end());

    // For simplicity, we'll assume enemies are evenly distributed across active waves
    // A more sophisticated approach would track which wave spawned which enemy
    // For now, we'll just update alive counts proportionally

    // Count active (non-completed) waves
    int active_waves = 0;
    for (const auto& wave : waves) {
        if (wave.continuous || !wave.completed) {
            active_waves++;
        }
    }

    if (active_waves == 0) {
        return;
    }

    // Simple approach: distribute enemy count across waves
    // This is approximate but sufficient for concurrent limiting
    for (auto& wave : waves) {
        if (wave.continuous || !wave.completed) {
            // Estimate alive count for this wave
            // In a real implementation, you'd track entity -> wave mapping
            wave.alive_count = total_enemies / active_waves;
        } else {
            wave.alive_count = 0;
        }
    }
}

} // namespace ecs
