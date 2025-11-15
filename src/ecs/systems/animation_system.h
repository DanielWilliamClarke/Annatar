#ifndef ECS_ANIMATION_SYSTEM_H
#define ECS_ANIMATION_SYSTEM_H

#include "../world.h"
#include <SFML/Graphics.hpp>

namespace ecs {

/**
 * AnimationSystem - Handles sprite sheet frame-by-frame animation
 *
 * Updates Animation components to advance frames based on timing,
 * and updates Sprite texture_rect to show the current frame.
 */
class AnimationSystem {
public:
    // Animation state IDs (matches legacy system)
    enum AnimationState {
        IDLE = 0,
        MOVING_UP = 1,
        MOVING_DOWN = 2,
        MOVING_LEFT = 3,
        MOVING_RIGHT = 4,
        ATTACKING = 5
    };

    // Update all animations
    static void Update(World& world, float dt) {
        auto view = world.View<Animation, Sprite>();

        for (auto entity : view) {
            auto& anim = view.get<Animation>(entity);
            auto& sprite = view.get<Sprite>(entity);

            // Check if current animation exists
            auto clip_it = anim.clips.find(anim.current_animation);
            if (clip_it == anim.clips.end()) {
                // No clip for current animation, skip
                continue;
            }

            auto& clip = clip_it->second;

            // Update timer
            anim.frame_timer += dt;

            // Advance frame if enough time has elapsed
            if (anim.frame_timer >= clip.frame_duration) {
                anim.frame_timer -= clip.frame_duration;  // Keep remainder
                anim.current_frame++;

                // Handle end of animation
                if (anim.current_frame >= clip.frame_count) {
                    if (clip.loop) {
                        anim.current_frame = 0;  // Loop back to start
                    } else {
                        anim.current_frame = clip.frame_count - 1;  // Stay on last frame
                        anim.finished = true;

                        // Clear priority if animation finished
                        if (anim.priority_active && anim.priority_id == anim.current_animation) {
                            anim.priority_active = false;
                        }
                    }
                }
            }

            // Calculate texture rect for current frame
            // Frame is at: (clip.start_col + current_frame, clip.row)
            int col = clip.start_col + anim.current_frame;
            int row = clip.row;

            sprite.texture_rect = sf::IntRect(
                col * anim.frame_size.x,
                row * anim.frame_size.y,
                anim.frame_size.x,
                anim.frame_size.y
            );

            // Also update sprite size to match frame size
            sprite.size = sf::Vector2f(anim.frame_size.x, anim.frame_size.y);
            sprite.origin = sf::Vector2f(anim.frame_size.x / 2.0f, anim.frame_size.y / 2.0f);
        }
    }

    // Play a specific animation
    static void PlayAnimation(World& world, entt::entity entity,
                             int animation_id, bool loop = true, bool priority = false) {
        if (!world.HasComponent<Animation>(entity)) {
            return;
        }

        auto& anim = world.GetComponent<Animation>(entity);

        // Check if blocked by priority animation
        if (anim.priority_active && anim.priority_id != animation_id) {
            return;  // Can't switch, priority animation is playing
        }

        // Check if animation exists
        if (anim.clips.find(animation_id) == anim.clips.end()) {
            return;  // Animation doesn't exist
        }

        // Switch animation if different
        if (anim.current_animation != animation_id) {
            anim.current_animation = animation_id;
            anim.current_frame = 0;
            anim.frame_timer = 0.0f;
            anim.finished = false;
        }

        // Update loop setting
        anim.clips[animation_id].loop = loop;

        // Set/clear priority
        if (priority) {
            anim.priority_active = true;
            anim.priority_id = animation_id;
        }
    }

    // Check if animation is finished (for non-looping animations)
    static bool IsFinished(const Animation& anim) {
        return anim.finished;
    }

    // Reset animation to first frame
    static void ResetAnimation(Animation& anim) {
        anim.current_frame = 0;
        anim.frame_timer = 0.0f;
        anim.finished = false;
    }
};

} // namespace ecs

#endif // ECS_ANIMATION_SYSTEM_H
