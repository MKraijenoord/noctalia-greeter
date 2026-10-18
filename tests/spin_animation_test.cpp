#include "render/animation/animation_manager.h"
#include "render/animation/spin_animation.h"
#include "render/scene/spinner_node.h"

#include <cmath>
#include <cstdio>
#include <string_view>

namespace {

  constexpr float kPi = 3.14159265f;

  bool expectNear(std::string_view name, float actual, float expected) {
    if (std::fabs(actual - expected) < 0.01f) {
      return true;
    }
    std::fprintf(
        stderr, "%.*s: expected %.4f, got %.4f\n", static_cast<int>(name.size()), name.data(), expected, actual
    );
    return false;
  }

  bool expectActive(std::string_view name, const AnimationManager& animations, bool expected) {
    if (animations.hasActive() == expected) {
      return true;
    }
    std::fprintf(
        stderr, "%.*s: expected animation %s\n", static_cast<int>(name.size()), name.data(),
        expected ? "running" : "stopped"
    );
    return false;
  }

} // namespace

int main() {
  bool passed = true;
  AnimationManager animations;
  SpinnerNode spinner;
  animateSpin(animations, spinner);

  animations.tick(225.0f);
  passed = expectNear("quarter turn after 225 ms", spinner.rotation(), kPi * 0.5f) && passed;

  for (int frame = 0; frame < 50; ++frame) {
    animations.tick(900.0f);
  }
  animations.tick(225.0f);
  passed = expectNear("keeps turning after fifty turns", spinner.rotation(), kPi) && passed;
  passed = expectActive("still spinning after fifty turns", animations, true) && passed;

  animations.cancelForOwner(&spinner);
  passed = expectActive("cancelling the spinner's animations stops it", animations, false) && passed;
  return passed ? 0 : 1;
}
