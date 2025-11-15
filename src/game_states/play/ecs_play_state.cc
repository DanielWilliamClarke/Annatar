#include "ecs_play_state.h"
#include "renderer/i_renderer.h"
#include "util/texture_atlas.h"
#include <iostream>

ECSPlayState::ECSPlayState(
    std::shared_ptr<ITextureAtlas> textureAtlas,
    sf::FloatRect bounds
)
    : textureAtlas(textureAtlas)
    , bounds(bounds)
    , worldSpeed(100.0f)
    , player(entt::null)
{
    // No legacy PlayerInput needed - pure ECS!
}

void ECSPlayState::Setup() {
    std::cout << "[ECS] Setting up ECS Play State..." << std::endl;

    // Load configuration from TOML files
    std::cout << "[ECS] Loading configuration..." << std::endl;
    std::cout.flush();

    if (!config.LoadAll("config")) {
        std::cerr << "[ECS] Failed to load configuration files!" << std::endl;
        return;
    }

    std::cout << "[ECS] Getting constants..." << std::endl;
    std::cout.flush();
    const auto& constants = config.GetConstants();
    std::cout << "[ECS] Configuration loaded successfully" << std::endl;

    // Create entity factory
    std::cout << "[ECS] Creating entity factory..." << std::endl;
    std::cout.flush();
    factory = std::make_unique<ecs::EntityFactory>(world, config);
    factory->SetTextureAtlas(textureAtlas);  // Allows factory to load textures from config
    std::cout << "[ECS] Entity factory created" << std::endl;

    // Initialize scrolling background (Gradius-style!)
    std::cout << "[ECS] Initializing scrolling starfield..." << std::endl;
    std::cout.flush();
    sf::Vector2f screen_size(bounds.width, bounds.height);
    ecs::BackgroundSystem::Initialize(world, screen_size, 200);
    std::cout << "[ECS] Starfield initialized (200 stars, 4 parallax layers)" << std::endl;

    // Create player
    std::cout << "[ECS] Creating player..." << std::endl;
    std::cout.flush();
    player = factory->CreatePlayer(
        constants.player_starting_position,
        textureAtlas->GetTexture("playerShip").get()
    );
    std::cout << "[ECS] Player created at ("
              << constants.player_starting_position.x << ", "
              << constants.player_starting_position.y << ")" << std::endl;

    // Spawn initial enemies from right edge (Gradius style!)
    std::cout << "[ECS] Spawning enemies from right edge..." << std::endl;
    std::cout.flush();
    SpawnEnemy("basic", {850.0f, 150.0f});  // Right edge, top
    SpawnEnemy("basic", {900.0f, 360.0f});  // Right edge, middle
    SpawnEnemy("basic", {950.0f, 550.0f});  // Right edge, bottom

    std::cout << "[ECS] Initial enemies spawned" << std::endl;
    std::cout << "[ECS] Total entities: " << world.GetEntityCount() << std::endl;
}

void ECSPlayState::TearDown() {
    std::cout << "[ECS] Tearing down ECS Play State..." << std::endl;
    world.Clear();
}

void ECSPlayState::Update(float dt) {
    // === PURE ECS SYSTEM UPDATE ORDER ===

    // 1. Input System - Sample keyboard, update Input components
    ecs::InputSystem::Update(world);

    // 2. Movement Input System - Apply input to velocity/acceleration and select animations
    ecs::MovementInputSystem::Update(world, config.GetConstants(), dt);  // Pass dt for physics!

    // 3. Background System - Scrolling starfield (Gradius parallax!)
    sf::Vector2f screen_size(bounds.width, bounds.height);
    ecs::BackgroundSystem::Update(world, worldSpeed, dt, screen_size);

    // 4. Movement System - Update positions based on velocity
    ecs::MovementSystem::UpdateSimple(world, dt);

    // 5. Bounds System - Clamp player to screen
    ecs::BoundsSystem::ClampPlayer(world, bounds);

    // 6. Animation System - Advance sprite frames (before rendering!)
    ecs::AnimationSystem::Update(world, dt);

    // 7. Weapon System - Update cooldowns
    ecs::WeaponSystem::Update(world, dt);

    // 8. Fire weapons if player presses fire
    auto players = world.View<ecs::PlayerTag, ecs::Input>();
    for (auto entity : players) {
        const auto& player_input = world.GetComponent<ecs::Input>(entity);

        if (player_input.fire) {
            ecs::WeaponSystem::TryFire(world, entity, [&](const ecs::BulletSpawnRequest& request) {
                factory->CreateBullet(request, true, nullptr);
            });
        }
    }

    // 4. Detect collisions
    ecs::CollisionSystem::DetectCollisions(world, [&](auto a, auto b, auto pt) {
        HandleCollision(a, b, pt);
    });

    // 5. Cleanup dead entities
    CleanupDeadEntities();

    // 6. Cleanup expired entities (bullets with lifetime)
    CleanupExpiredEntities(dt);

    // 7. Check game over
    if (PlayerDied()) {
        std::cout << "[ECS] Player died! Returning to menu..." << std::endl;
        this->Back();
    }

    // 8. ESC to menu
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
        this->Forward(GameStates::MENU);
    }
}

void ECSPlayState::Draw(const std::shared_ptr<IRenderer>& renderer, float interp) const {
    // Render all ECS entities with interpolation
    // Note: RenderSystem::Render takes non-const world, but only reads
    ecs::RenderSystem::Render(const_cast<ecs::World&>(world), renderer->GetTarget(), interp);

    // Debug: Render collision shapes if enabled
    if (config.GetConstants().debug_show_collision_shapes) {
        ecs::RenderSystem::RenderDebug(const_cast<ecs::World&>(world), renderer->GetDebugTarget());
    }
}

void ECSPlayState::HandleCollision(entt::entity a, entt::entity b, const sf::Vector2f& collision_point) {
    // Determine collision type and handle appropriately
    bool a_is_bullet = world.HasComponent<ecs::BulletTag>(a);
    bool b_is_bullet = world.HasComponent<ecs::BulletTag>(b);
    bool a_is_enemy = world.HasComponent<ecs::EnemyTag>(a);
    bool b_is_enemy = world.HasComponent<ecs::EnemyTag>(b);
    bool a_is_player = world.HasComponent<ecs::PlayerTag>(a);
    bool b_is_player = world.HasComponent<ecs::PlayerTag>(b);

    // Player bullet hits enemy
    if (a_is_bullet && b_is_enemy) {
        // Get bullet damage (from weapon that fired it)
        float damage = 10.0f;  // Default damage
        if (world.HasComponent<ecs::Collision>(a)) {
            // TODO: Store damage in bullet component or collision
            damage = 25.0f;
        }

        // Apply damage to enemy
        ecs::HealthSystem::ApplyDamage(world, b, damage);

        // Create hit effect
        factory->CreateExplosion(collision_point, sf::Color(255, 200, 0), 8);

        // Destroy bullet
        world.DestroyEntity(a);
    }
    // Enemy hits player
    else if (a_is_enemy && b_is_player) {
        // Apply damage to player
        ecs::HealthSystem::ApplyDamage(world, b, 20.0f);

        // Create hit effect
        factory->CreateExplosion(collision_point, sf::Color(255, 0, 0), 8);

        // Destroy enemy (kamikaze)
        world.DestroyEntity(a);
    }
    // Mirror cases
    else if (b_is_bullet && a_is_enemy) {
        HandleCollision(b, a, collision_point);
    }
    else if (b_is_enemy && a_is_player) {
        HandleCollision(b, a, collision_point);
    }
}

void ECSPlayState::CleanupDeadEntities() {
    auto dead = ecs::HealthSystem::CollectDeadEntities(world);

    for (auto entity : dead) {
        // Award score if entity has score component
        if (world.HasComponent<ecs::Score>(entity)) {
            float score = world.GetComponent<ecs::Score>(entity).value;
            // TODO: Add score to player or HUD
            std::cout << "[ECS] Enemy destroyed! Score: " << score << std::endl;
        }

        // Create explosion effect
        if (world.HasComponent<ecs::Transform>(entity)) {
            auto& transform = world.GetComponent<ecs::Transform>(entity);
            factory->CreateExplosion(transform.position, sf::Color::Red, 16);
        }

        // Destroy entity
        world.DestroyEntity(entity);
    }
}

void ECSPlayState::CleanupExpiredEntities(float dt) {
    auto expired = ecs::LifetimeSystem::Update(world, dt);

    for (auto entity : expired) {
        world.DestroyEntity(entity);
    }
}

void ECSPlayState::SpawnEnemy(const std::string& type, sf::Vector2f position) {
    factory->CreateEnemy(type, position, textureAtlas->GetTexture("enemy1").get());
}

bool ECSPlayState::PlayerDied() const {
    auto players = world.View<ecs::PlayerTag, ecs::Health>();
    for (auto entity : players) {
        const auto& health = world.GetComponent<ecs::Health>(entity);
        if (health.dead) {
            return true;
        }
    }
    return false;
}
