#include "../util/input.hpp"

#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <unordered_map>

#include <glob/window.hpp>
#include <glm/glm.hpp>

namespace {
static bool inititalized = false;
static std::unordered_map<int, int> keys;
static std::unordered_map<int, int> pre_keys;

static std::unordered_map<int, int> buttons;
static std::unordered_map<int, int> pre_buttons;

static std::string characters;

float axis_threshold = 0.2;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods) {
  if (action == GLFW_PRESS) {
    pre_keys[key]++;
  }
}
void MouseCallback(GLFWwindow* window, int button, int action, int mods) {
  if (action == GLFW_PRESS) {
    pre_buttons[button]++;
  }
}
void CharacterCallback(GLFWwindow* window, unsigned int codepoint) {
  //std::cout << codepoint << "\n";
  characters += codepoint;
}
}  // namespace
bool Input::Initialize() {
  if (!glob::window::IsInitialized() || inititalized) {
    return false;
  }
  glob::window::SetKeyCallback((GLFWkeyfun)KeyCallback);
  glob::window::SetMouseCallback((GLFWmousebuttonfun)MouseCallback);
  glob::window::SetCharacterCallback((GLFWcharfun)CharacterCallback);
  inititalized = true;
  return true;
}

bool Input::IsKeyDown(int key) { return glob::window::KeyDown(key); }

bool Input::IsMouseButtonDown(int button) {
  return glob::window::MouseButtonDown(button);
}

bool Input::IsButtonPressed(int button) {
  if (buttons[button] > 0) {
    return true;
  } else {
    return false;
  }
}

bool Input::IsKeyPressed(int key) {
  if (keys[key] > 0) {
    return true;
  } else {
    return false;
  }
}

std::string Input::GetCharacters() { return characters; }
void Input::Reset() {
  keys = pre_keys;
  pre_keys.clear();
  buttons = pre_buttons;
  pre_buttons.clear();
  characters = "";
}

glm::vec2 Input::MouseMov() { return glob::window::MouseMovement(); }

glm::vec2 Input::MousePos() { return glob::window::MousePosition(); }

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