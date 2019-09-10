#include "glob/window.h"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>

namespace {

GLFWwindow* glfw_window = nullptr;

}  // namespace

namespace glob {
namespace window {

void Create() {
  if (glfw_window) {
    std::cout << "WARNING: Calling WindowCreate() when window already created"
              << std::endl;
    return;
  }

  if (!glfwInit()) {
    std::cout << "ERROR: Could not init glfw\n";
    assert(0);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

  int width = 1920;
  int height = 1080;
  const char* title_str = "Hello World";

  glfw_window =
      glfwCreateWindow(width, height, title_str, glfwGetPrimaryMonitor(), NULL);

  if (!glfw_window) {
    std::cout << "ERROR: Could not create glfw window\n";
    glfwTerminate();
    assert(0);
  }

  glfwMakeContextCurrent(glfw_window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "ERROR: Failed to initialize OpenGL context" << std::endl;
    assert(0);
  }

  printf("DEBUG: Using OpenGL %s\n", glGetString(GL_VERSION));

  // vsync 1, off 0
  glfwSwapInterval(0);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // glClear(GL_COLOR_BUFFER_BIT);
}

bool ShouldClose() { return glfwWindowShouldClose(glfw_window); }

void Update() {
  glfwSwapBuffers(glfw_window);

  glfwPollEvents();

  int state = glfwGetKey(glfw_window, GLFW_KEY_ESCAPE);
  if (state == GLFW_PRESS) {
    glfwSetWindowShouldClose(glfw_window, GLFW_TRUE);
  }
}

void Cleanup() { glfwTerminate(); }

}  // namespace window
}  // namespace glob