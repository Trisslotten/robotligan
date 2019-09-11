#ifndef INPUT_H_
#define INPUT_H_

#include "../../glm/glm/common.hpp"
#include "../../glm/glm/vec2.hpp"

class Input {
 public:
  Input() = delete;
  static bool initialize();
  static bool isKeyDown(int key);
  static bool isMouseButtonDown(int button);
  static bool isButtonPressed(int button);
  static bool isKeyPressed(int key);
  static void reset();
  static glm::vec2 mouseMov();
  static glm::vec2 mousePos();

  static bool gamepad_present();
  static float gamepad_axis(int axis);
  static bool gamepad_button_pressed(int button);
};

#endif  // INPUT_H_
