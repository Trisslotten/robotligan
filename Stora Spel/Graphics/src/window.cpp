#include "glob/window.hpp"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cassert>
#include <iostream>

namespace {
GLFWwindow* glfw_window = nullptr;
bool initd = false;
glm::dvec2 mouse_pos(0, 0);
glm::vec2 last_mouse_pos(0, 0);

}  // namespace

namespace glob {

namespace window {

unsigned int window_width = 1280;
unsigned int window_height = 720;

void Create() {
  if (glfw_window) {
    std::cout << "WARNING window.cpp: Calling WindowCreate() when window "
                 "already created"
              << std::endl;
    return;
  }

  if (!glfwInit()) {
    std::cout << "ERROR: Could not init glfw\n";
    assert(0);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

  const char* title_str = "Robotligan";

  glfw_window =
      glfwCreateWindow(window_width, window_height, title_str, NULL, NULL);

  if (!glfw_window) {
    std::cout << "ERROR window.cpp: Could not create glfw window\n";
    glfwTerminate();
    assert(0);
  }

  glfwMakeContextCurrent(glfw_window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "ERROR window.cpp: Failed to initialize OpenGL context"
              << std::endl;
    assert(0);
  }

  printf("DEBUG window.cpp: Using OpenGL %s\n", glGetString(GL_VERSION));

  glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // vsync 1, off 0
  glfwSwapInterval(1);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDisable(GL_CULL_FACE);

  initd = true;
  // glClear(GL_COLOR_BUFFER_BIT);
}

bool ShouldClose() { return glfwWindowShouldClose(glfw_window); }

void UpdateMousePos() {
  last_mouse_pos = mouse_pos;
  glfwGetCursorPos(glfw_window, &mouse_pos.x, &mouse_pos.y);
}

void Update() {
  glfwSwapBuffers(glfw_window);

  glfwPollEvents();
  UpdateMousePos();

  int state = glfwGetKey(glfw_window, GLFW_KEY_0);
  if (state == GLFW_PRESS) {
    glfwSetWindowShouldClose(glfw_window, GLFW_TRUE);
  }
}

void Cleanup() { glfwTerminate(); }

bool IsInitialized() { return initd; }

GLFWwindow* GetGlfwWindow() { return glfw_window; }

bool MouseButtonDown(int button) {
  return glfwGetMouseButton(glfw_window, button) == GLFW_PRESS;
}

void SetKeyCallback(void (*key_callback)(GLFWwindow*, int, int, int, int)) {
  glfwSetKeyCallback(glfw_window, (GLFWkeyfun)key_callback);
}

void SetMouseCallback(void (*key_callback)(GLFWwindow*, int, int, int)) {
  glfwSetMouseButtonCallback(glob::window::GetGlfwWindow(),
                             (GLFWmousebuttonfun)key_callback);
}

void SetMouseLocked(bool val) {
  if (val) {
    glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
  else {
    glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

glm::vec2 GetWindowDimensions() { return glm::vec2(window_width, window_height); }

bool KeyDown(int key) { return GLFW_PRESS == glfwGetKey(glfw_window, key); }

glm::vec2 MouseMovement() {
  glm::vec2 pos = MousePosition();
  glm::vec2 result = pos - last_mouse_pos;
  return result;
}

glm::vec2 MousePosition() { return glm::vec2(mouse_pos); }

}  // namespace window
}  // namespace glob