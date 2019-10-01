#ifndef INPUT_HPP_
#define INPUT_HPP_

#include "../../glm/glm/common.hpp"
#include "../../glm/glm/vec2.hpp"

class Input {
 public:
  Input() = delete;
  static bool Initialize();
  static bool IsKeyDown(int key);
  static bool IsMouseButtonDown(int button);
  static bool IsButtonPressed(int button);
  static bool IsKeyPressed(int key);
  static void Reset();
  static glm::vec2 MouseMov();
  static glm::vec2 MousePos();

  static bool GamepadPresent();
  static float GamepadAxis(int axis);
  static bool GamepadButtonPressed(int button);
};

#endif  // INPUT_HPP_
