#ifndef ECS_WORLD_H
#define ECS_WORLD_H

#include <entt/entt.hpp>
#include "components/components.h"

namespace ecs {

/**
 * World - Main ECS registry wrapper
 * Manages all entities and components in the game
 */
class World {
public:
    World() = default;
    ~World() = default;

    // Non-copyable
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    // Entity creation/destruction
    entt::entity CreateEntity() {
        return registry.create();
    }

    void DestroyEntity(entt::entity entity) {
        registry.destroy(entity);
    }

    bool IsValid(entt::entity entity) const {
        return registry.valid(entity);
    }

    // Component access
    template<typename Component>
    decltype(auto) AddComponent(entt::entity entity, Component&& component = {}) {
        if constexpr (std::is_empty_v<Component>) {
            // For empty types (tags), emplace and return stored component
            registry.emplace<Component>(entity);
            return registry.get<Component>(entity);
        } else {
            return registry.emplace<Component>(entity, std::forward<Component>(component));
        }
    }

    template<typename Component>
    Component& GetComponent(entt::entity entity) {
        return registry.get<Component>(entity);
    }

    template<typename Component>
    const Component& GetComponent(entt::entity entity) const {
        return registry.get<Component>(entity);
    }

    template<typename Component>
    bool HasComponent(entt::entity entity) const {
        return registry.all_of<Component>(entity);
    }

    template<typename Component>
    void RemoveComponent(entt::entity entity) {
        registry.remove<Component>(entity);
    }

    template<typename Component>
    Component* TryGetComponent(entt::entity entity) {
        return registry.try_get<Component>(entity);
    }

    // View for iterating entities with specific components
    template<typename... Components>
    auto View() {
        return registry.view<Components...>();
    }

    template<typename... Components>
    auto View() const {
        return registry.view<Components...>();
    }

    // Direct registry access for advanced use
    entt::registry& GetRegistry() { return registry; }
    const entt::registry& GetRegistry() const { return registry; }

    // Clear all entities
    void Clear() {
        registry.clear();
    }

    // Get entity count
    size_t GetEntityCount() const {
        return registry.storage<entt::entity>()->size();
    }

private:
    entt::registry registry;
};

} // namespace ecs

#endif // ECS_WORLD_H
