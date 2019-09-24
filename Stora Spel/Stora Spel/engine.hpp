#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <entt.hpp>
#include <unordered_map>
#include <glob/graphics.hpp>
#include "shared/shared.hpp"

class Engine {
 public:
  void Init();

  void Update(float dt);

  void Render();

 private:
  void UpdateSystems(float dt);

  entt::registry registry_;

  std::unordered_map<int, PlayerAction> keybinds_;

  glob::Font2DHandle font_test_;
  glob::Font2DHandle font_test2_;
};

#endif  // ENGINE_HPP_