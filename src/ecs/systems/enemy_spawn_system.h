#ifndef ECS_ENEMY_SPAWN_SYSTEM_H
#define ECS_ENEMY_SPAWN_SYSTEM_H

#include "../world.h"
#include "../config/config_loader.h"
#include "../factories/entity_factory.h"
#include "util/i_texture_atlas.h"
#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include <string>
#include <memory>

namespace ecs {

/**
 * SpawnWaveConfig - Configuration for a single spawn wave
 * Supports both continuous spawning and discrete wave-based spawning
 */
struct SpawnWaveConfig {
    // Core spawn configuration
    std::vector<std::string> enemy_pool;    // Pool of enemy types to spawn from
    float interval;                          // Base spawn interval (seconds)
    float interval_variance;                 // ±variance for timing randomness (0.0 = no variance)
    int max_concurrent;                      // Max enemies alive from this wave (0 = unlimited)
    bool continuous;                         // true = spawn continuously, false = spawn once then done

    // Position randomness
    float position_variance;                 // Y-position variance (pixels)

    // Wave state (internal tracking)
    float timer;                            // Current spawn timer
    int spawned_count;                      // Total enemies spawned
    int alive_count;                        // Currently alive enemies from this wave
    bool completed;                         // For non-continuous waves

    SpawnWaveConfig()
        : interval(1.0f)
        , interval_variance(0.0f)
        , max_concurrent(0)
        , continuous(true)
        , position_variance(0.0f)
        , timer(0.0f)
        , spawned_count(0)
        , alive_count(0)
        , completed(false) {}
};

/**
 * EnemySpawnSystem - Manages enemy spawning with support for:
 * - Time-based spawning (continuous intervals)
 * - Wave-based spawning (discrete groups)
 * - Randomness/variance for variety
 * - Per-wave entity limits
 * - TOML-driven configuration
 */
class EnemySpawnSystem {
public:
    /**
     * Initialize the spawn system with random seed
     */
    static void Initialize(unsigned int seed = 0);

    /**
     * Add a spawn wave configuration
     * @param wave Wave configuration to add
     */
    static void AddWave(const SpawnWaveConfig& wave);

    /**
     * Add a simple continuous spawn wave
     * @param enemy_type Single enemy type to spawn
     * @param interval Spawn interval in seconds
     * @param max_concurrent Max concurrent enemies (0 = unlimited)
     * @param interval_variance Optional timing variance (default 0.0)
     */
    static void AddContinuousWave(const std::string& enemy_type,
                                  float interval,
                                  int max_concurrent = 0,
                                  float interval_variance = 0.0f);

    /**
     * Add a discrete spawn wave (spawns once then completes)
     * @param enemy_pool Pool of enemy types to randomly select from
     * @param count Number of enemies to spawn
     * @param delay Delay before spawning this wave (seconds)
     * @param spawn_interval Interval between individual spawns in the wave
     */
    static void AddDiscreteWave(const std::vector<std::string>& enemy_pool,
                               int count,
                               float delay,
                               float spawn_interval = 0.5f);

    /**
     * Load spawn waves from TOML configuration
     * @param filepath Path to enemies.toml with [spawn_waves] section
     * @return true if loaded successfully
     */
    static bool LoadSpawnWaves(const std::string& filepath);

    /**
     * Update spawn system - spawns enemies based on waves and timing
     * @param world ECS world
     * @param dt Delta time
     * @param factory Entity factory for creating enemies
     * @param bounds Spawn bounds (enemies spawn at right edge)
     * @param textureAtlas Texture atlas for enemy sprites
     */
    static void Update(World& world,
                      float dt,
                      EntityFactory& factory,
                      const sf::FloatRect& bounds,
                      ITextureAtlas& textureAtlas);

    /**
     * Clear all spawn waves
     */
    static void Clear();

    /**
     * Get current wave count
     */
    static size_t GetWaveCount();

    /**
     * Get total enemies spawned
     */
    static int GetTotalSpawned();

    /**
     * Enable/disable spawning
     */
    static void SetEnabled(bool enabled);

    /**
     * Check if spawning is enabled
     */
    static bool IsEnabled();

private:
    // Spawn waves
    static std::vector<SpawnWaveConfig> waves;

    // Random number generation
    static std::mt19937 rng;
    static std::uniform_real_distribution<float> dist01;

    // State
    static bool enabled;
    static int total_spawned;

    /**
     * Try to spawn an enemy from a wave
     * @return true if enemy was spawned
     */
    static bool TrySpawnEnemy(World& world,
                             SpawnWaveConfig& wave,
                             EntityFactory& factory,
                             const sf::FloatRect& bounds,
                             ITextureAtlas& textureAtlas);

    /**
     * Get random spawn position within bounds
     * @param bounds Spawn bounds
     * @param variance Y-position variance
     * @return Spawn position (right edge, random Y)
     */
    static sf::Vector2f GetRandomSpawnPosition(const sf::FloatRect& bounds,
                                               float variance);

    /**
     * Select random enemy type from pool
     */
    static std::string SelectRandomEnemyType(const std::vector<std::string>& enemy_pool);

    /**
     * Get random variance value [-1.0, 1.0]
     */
    static float GetRandomVariance();

    /**
     * Update alive count for all waves (scan ECS world for enemies)
     */
    static void UpdateWaveAliveCounts(World& world);
};

} // namespace ecs

#endif // ECS_ENEMY_SPAWN_SYSTEM_H
