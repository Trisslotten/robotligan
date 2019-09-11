#ifndef GLOB_WINDOW_H_
#define GLOB_WINDOW_H_

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#include <glm/glm.hpp>

// TODO: move window outside glob?

// dummies
class GLFWwindow {};

// dummies

namespace glob {
namespace window {

EXPORT void Create();
EXPORT bool ShouldClose();
EXPORT void Update();
EXPORT void Cleanup();
EXPORT bool IsInitialized();
EXPORT GLFWwindow* GetGlfwWindow();
EXPORT bool KeyDown(int key);
EXPORT glm::vec2 MouseMovement();
EXPORT glm::vec2 MousePosition();
EXPORT bool MouseButtonDown(int button);
EXPORT void SetKeyCallback(void (*GLFWkeyfun)(GLFWwindow*, int, int, int, int));
EXPORT void SetMouseCallback(void (*GLFWmousebuttonfun)(GLFWwindow*, int, int,
                                                        int));

}  // namespace window
}  // namespace glob

#endif  // GLOB_WINDOW_H_