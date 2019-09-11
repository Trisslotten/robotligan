#include "../util/input.h"
#include <GLFW/glfw3.h>
#include <glob/window.h>
#include <glm/glm.hpp>
//#include "../../Graphics/external/include/GLFW/glfw3.h"

#include <iostream>
#include <unordered_map>

namespace {
static bool inititalized = false;
static std::unordered_map<int, int> keys;
static std::unordered_map<int, int> pre_keys;

static std::unordered_map<int, int> buttons;
static std::unordered_map<int, int> pre_buttons;

float axis_threshold = 0.2;

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mods) {
  if (action == GLFW_PRESS) {
    pre_keys[key]++;
    std::cout << key << "\n";
  }
}
void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
  if (action == GLFW_PRESS) {
    pre_buttons[button]++;
    std::cout << button << "\n";
  }
}
}  // namespace
bool Input::initialize() {
  if (!glob::window::IsInitialized() || inititalized) {
    return false;
  }
  glob::window::SetKeyCallback((GLFWkeyfun)key_callback);
  glob::window::SetMouseCallback((GLFWmousebuttonfun)mouseCallback);
  inititalized = true;
  return true;
}

bool Input::isKeyDown(int key) { return glob::window::KeyDown(key); }

bool Input::isMouseButtonDown(int button) {
  return glob::window::MouseButtonDown(button);
}

bool Input::isButtonPressed(int button) {
  if (buttons[button] > 0) {
    return true;
  } else {
    return false;
  }
}

bool Input::isKeyPressed(int key) {
  if (keys[key] > 0) {
    return true;
  } else {
    return false;
  }
}

void Input::reset() {
  keys = pre_keys;
  pre_keys.clear();
  buttons = pre_buttons;
  pre_buttons.clear();
}

glm::vec2 Input::mouseMov() { return glob::window::MouseMovement(); }

glm::vec2 Input::mousePos() { return glob::window::MousePosition(); }

/*bool Input::gamepad_present() {
        int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
        if (present) {
                if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
                {
                        return true;
                }
        }
        return false;
}

float Input::gamepad_axis(int axis) {
        if (!gamepad_present())
                return 0;
        GLFWgamepadstate gamepad_state;
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state)) {
                float ret = gamepad_state.axes[axis];
                if ((ret > 0 && ret < axis_threshold) || (ret < 0 && ret >
-axis_threshold)) { ret = 0.f;
                }
                return glm::pow(ret, 3);
        }
        return 0;
}

bool Input::gamepad_button_pressed(int button) {
        if (!gamepad_present())
                return 0;
        GLFWgamepadstate gamepad_state;
        if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state)) {
                return gamepad_state.buttons[button];
        }
        return 0;
}*/