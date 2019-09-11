#ifndef GLOB_GRAPHICS_H_
#define GLOB_GRAPHICS_H_

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace glob {

EXPORT void Init();

//EXPORT void Submit(Model) 

// submit things to render, for testing
EXPORT void DebugSubmitSphere(glm::vec3 pos, float radius);
EXPORT void DebugSubmitCube(glm::vec3 pos, glm::vec3 side_lengths,
                            glm::quat orientation = glm::quat());
EXPORT void DebugSubmitPlane(glm::vec3 pos, glm::vec2 side_lengths, glm::vec3 up);

// render submitted items.
EXPORT void Render();



}  // namespace glob

#endif  // GLOB_GRAPHICS_H_