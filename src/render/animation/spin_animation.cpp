#include "render/animation/spin_animation.h"

#include "render/animation/animation_manager.h"
#include "render/scene/node.h"

#include <cmath>

std::uint64_t animateSpin(AnimationManager& animations, Node& node) {
  constexpr float kTurn = 6.28318530718f;
  constexpr float kTurnMs = 900.0f;
  // One long linear animation: restarting from onComplete would modify the animation list while it is ticking.
  constexpr float kTurns = 10000.0f;
  return animations.animate(
      0.0f, kTurn * kTurns, kTurnMs * kTurns, Easing::Linear,
      [&node](float angle) { node.setRotation(std::fmod(angle, kTurn)); }, nullptr, &node
  );
}
