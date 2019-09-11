#include "../util/input.h"

//#include "window.h"


#include <unordered_map>
#include <iostream>
namespace
{
	static bool inititalized = false;
	static std::unordered_map<int, int> keys;
	static std::unordered_map<int, int> pre_keys;

	static std::unordered_map<int, int> buttons;
	static std::unordered_map<int, int> pre_buttons;

	float axis_threshold = 0.2;

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			pre_keys[key]++;
		}
	}
	void mouseCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS) {
			pre_buttons[button]++;
		}

	}
}
bool Input::initialize()
{
	if (!Window::getWindow().isInitialized() || inititalized)
	{
		return false;
	}

	glfwSetKeyCallback(Window::getWindow().getGLFWWindow(), key_callback);
	glfwSetMouseButtonCallback(Window::getWindow().getGLFWWindow(), mouseCallback);
	inititalized = true;
	return true;
}

bool Input::isKeyDown(int key)
{
	return Window::getWindow().keyDown(key);
}

bool Input::isMouseButtonDown(int key)
{
	return Window::getWindow().mouseButtonDown(key);
}

bool Input::isButtonPressed(int button)
{
	if (buttons[button] > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Input::isKeyPressed(int key)
{
	if (keys[key] > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Input::reset()
{
	keys = pre_keys;
	pre_keys.clear();
	buttons = pre_buttons;
	pre_buttons.clear();
}

glm::vec2 Input::mouseMov()
{
	return Window::getWindow().mouseMovement();
}

glm::vec2 Input::mousePos()
{
	return Window::getWindow().mousePosition();
}

bool Input::gamepad_present() {
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
		if ((ret > 0 && ret < axis_threshold) || (ret < 0 && ret > -axis_threshold)) {
			ret = 0.f;
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
}