# Annatar Game - Major Refactoring Plan

## Overview
This document outlines the comprehensive refactoring plan to transform Annatar from a component-based architecture with tight coupling into a clean, maintainable ECS (Entity Component System) architecture with Lua scripting support.

## Current Problems Identified

### Critical Issues
1. **Excessive Shared Pointer Usage** (118+ instances)
   - Everything uses `std::shared_ptr` even when ownership is clear
   - Unnecessary reference counting overhead
   - Confusing ownership semantics

2. **Tight Coupling Through Accessor Classes**
   - Components coupled via multiple inheritance of accessor classes
   - Example: `IWeaponComponent` inherits from 4 accessor classes
   - Hard to test and inflexible

3. **EntityObject God Class**
   - EntityObject knows about ALL component types
   - Changing any component type requires recompiling everything
   - Violates open/closed principle

4. **Complex Builder Dependencies**
   - PlayerEntityBuilder creates 4 sub-objects, each requiring 5+ dependencies
   - PlayStateBuilder has complex interdependent Build methods
   - Fragile initialization order

5. **Configuration Hardcoded**
   - All enemies, weapons, and game data hardcoded in builders
   - Makes tuning/balancing incredibly difficult
   - JSON schema exists but unused

### Secondary Issues
6. **No Object Pooling** - Memory fragmentation from frequent allocations
7. **Magic Numbers** - Unclear constants throughout codebase
8. **Mediator Pattern Overuse** - Heavy `std::function` callbacks create unclear control flow
9. **Header Dependency Explosion** - 236 includes, circular dependencies
10. **QuadTree Rebuilt Every Frame** - Allocating tree nodes constantly

## Refactoring Plan

### Phase 1: Foundation - Switch to Proper ECS (Week 1-2)
**Goal**: Replace current component system with lightweight ECS (EnTT library)

#### Tasks
1. ✅ **Integrate EnTT** - Modern ECS library, header-only, excellent performance
2. 🔄 **Create ECS component structs** - Transform, Sprite, Health, Weapon, Movement as pure data
3. **Build systems** - MovementSystem, RenderSystem, CollisionSystem, WeaponSystem
4. **Migrate entities** - Player, Enemy, Bullet to ECS entities
5. **Remove old architecture** - EntityObject, accessor classes

#### Benefits
- No more EntityObject god class
- No accessor coupling
- Better performance with cache-friendly data layout
- Simpler to add/remove components dynamically

### Phase 2: Lua Integration (Week 2-3)
**Goal**: Make game data-driven with Lua configs and behaviors

#### Tasks
1. **Integrate sol2** - Modern C++/Lua binding library
2. **Create Lua API** - Expose entity creation, components, bullets
3. **Migrate configs** - Move enemy types, weapon defs, bullet patterns to Lua
4. **Script behaviors** - Enemy movement patterns, custom weapons in Lua
5. **Hot-reloading** - Change configs/behaviors without recompiling

#### Benefits
- Add weapons/enemies by writing Lua files
- Rapid iteration without recompilation
- Game designers can modify gameplay without C++ knowledge
- Easy to test different configurations

### Phase 3: Performance Optimization (Week 3-4)
**Goal**: Fix memory issues and improve frame rate

#### Tasks
1. **Object pooling** - Pool bullets, enemies, particles (avoid allocations)
2. **Reduce shared_ptr** - Use ECS handles, raw pointers for non-owning refs
3. **Persistent QuadTree** - Update tree instead of rebuilding each frame
4. **Parallelize systems** - Use ThreadedWorkload for collision/updates
5. **Profile and optimize** - Identify remaining bottlenecks

#### Benefits
- 60+ FPS even with 1000s of bullets
- Reduced memory fragmentation
- Better CPU utilization with parallelism

### Phase 4: Developer Experience (Week 4-5)
**Goal**: Make development pleasant again

#### Tasks
1. **Extract magic numbers** - Create Lua config files for all constants
2. **Build tool** - Lua script to generate test scenarios
3. **Debug UI** - ImGui tools to spawn enemies, test weapons, view ECS
4. **Documentation** - How to add weapons, enemies, bosses via Lua

#### Benefits
- Add features in minutes instead of hours
- Easy testing and debugging
- Clear documentation for future development

### Phase 5: Documentation (Final Step)
**Goal**: Create comprehensive `claude.md` file

#### Tasks
1. **Architecture overview** - ECS design, system interactions, Lua integration
2. **Refactoring summary** - What was changed and why
3. **Developer guide** - How to add weapons, enemies, levels
4. **Performance guide** - Profiling results, optimization techniques
5. **Troubleshooting** - Common issues and solutions
6. **Future improvements** - Suggested next steps

## Architecture After Refactor

```
ECS World (EnTT)
├── Entities (just IDs - entt::entity)
├── Components (pure data structs)
│   ├── Transform {position, rotation, velocity}
│   ├── Sprite {texture, color, scale}
│   ├── Health {current, max, shield, shieldRegen}
│   ├── Weapon {type, cooldown, slot, damage}
│   ├── Movement {pattern_id, speed, behavior}
│   ├── Collision {radius, layer, mask}
│   └── AI {script_id, state}
└── Systems (logic, uses Lua)
    ├── MovementSystem → reads Lua movement scripts
    ├── WeaponSystem → reads Lua weapon configs
    ├── CollisionSystem → uses pooled quadtree
    ├── AISystem → executes Lua AI behaviors
    └── RenderSystem → batch rendering

Lua Scripts
├── config/
│   └── constants.lua (all magic numbers)
├── enemies/
│   ├── basic_shooter.lua
│   └── orbital_bomber.lua
├── weapons/
│   ├── plasma_rifle.lua
│   ├── homing_missiles.lua
│   └── shotgun.lua
├── levels/
│   └── level_1.lua
└── behaviors/
    ├── movement/
    │   ├── circular.lua
    │   └── sine_wave.lua
    └── ai/
        └── aggressive.lua
```

## Example: Adding New Weapon

### Before Refactor
1. Create new weapon component class (inherit from IWeaponComponent)
2. Implement 4 accessor classes
3. Create factory class
4. Update builder to wire dependencies
5. Modify EntityObject to support new component
6. Recompile entire project
7. **Time: 2+ hours, 8+ files modified**

### After Refactor
Create `weapons/shotgun.lua`:
```lua
return {
  name = "Shotgun",
  cooldown = 0.5,
  bullets_per_shot = 5,
  spread_angle = 30,
  damage = 15,
  bullet_speed = 400,
  bullet_size = {x = 8, y = 8},
  color = {255, 100, 0},
  sound = "shotgun_fire"
}
```
**Time: 5 minutes, 1 file created**

## Timeline

- **Phase 1**: 1-2 weeks (ECS migration)
- **Phase 2**: 1 week (Lua integration)
- **Phase 3**: 1 week (Performance)
- **Phase 4**: 3-5 days (Polish)
- **Phase 5**: 1 day (claude.md documentation)
- **Total**: 4-5 weeks for complete refactor

## Risks & Mitigation

### Risk: Breaking existing gameplay during migration
**Mitigation**:
- Keep old code alongside new ECS code
- Migrate system-by-system
- Test each phase before proceeding
- Git branches for each major phase

### Risk: Learning curve for EnTT + sol2
**Mitigation**:
- Both libraries have excellent documentation
- Simple, well-documented examples for each pattern
- Start with simple systems and gradually add complexity

### Risk: Performance regression during transition
**Mitigation**:
- Profile before and after each phase
- Keep benchmarks for critical systems
- Can roll back if performance degrades

## Success Metrics

- [ ] Can add new enemy type in < 10 minutes
- [ ] Can add new weapon in < 5 minutes
- [ ] Maintains 60 FPS with 1000+ bullets
- [ ] Hot-reload Lua configs without restart
- [ ] < 50 `std::shared_ptr` instances in codebase
- [ ] All magic numbers in config files
- [ ] Compile time < 30 seconds for full rebuild

## Current Progress

### Completed
- ✅ Analyzed codebase and identified issues
- ✅ Created comprehensive refactoring plan
- ✅ Integrated EnTT library into CMake build system

### In Progress
- 🔄 Creating ECS component structs

### Todo
- ⏳ Build core ECS systems
- ⏳ Migrate entities to ECS
- ⏳ Integrate Lua scripting
- ⏳ Performance optimizations
- ⏳ Developer tools and documentation
