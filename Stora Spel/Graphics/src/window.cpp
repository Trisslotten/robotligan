#include "glob/window.hpp"

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
    std::cout << "WARNING window.cpp: Calling WindowCreate() when window already created"
              << std::endl;
    return;
  }

  if (!glfwInit()) {
    std::cout << "ERROR: Could not init glfw\n";
    assert(0);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

  int width = 1280;
  int height = 720;
  const char* title_str = "Hello World";

  glfw_window =
      glfwCreateWindow(width, height, title_str, NULL, NULL);

  if (!glfw_window) {
    std::cout << "ERROR window.cpp: Could not create glfw window\n";
    glfwTerminate();
    assert(0);
  }

  glfwMakeContextCurrent(glfw_window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "ERROR window.cpp: Failed to initialize OpenGL context" << std::endl;
    assert(0);
  }

  printf("DEBUG window.cpp: Using OpenGL %s\n", glGetString(GL_VERSION));

  glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // vsync 1, off 0
  glfwSwapInterval(1);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDisable(GL_CULL_FACE);

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