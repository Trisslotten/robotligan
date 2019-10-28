#ifndef PHYSICS_INCLUDE_BOUNDINGBOXES_HPP_
#define PHYSICS_INCLUDE_BOUNDINGBOXES_HPP_

#include <vector>
#include <glm/glm.hpp>

namespace physics {

struct Sphere {
  glm::vec3 center;
  float radius;
};

struct OBB {
  glm::vec3 center = glm::vec3(0.f);
  glm::vec3 normals[3] = {glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)};  // normalized plane normals
  float extents[3] = {1.f, 1.f, 1.f};  // lenght to each plane
};

struct Arena {
  float xmin, xmax;
  float ymin, ymax;
  float zmin, zmax;
};


struct MeshHitbox {
  std::vector<glm::vec3> pos;
  std::vector<unsigned int> indices;
};
}  // namespace physics
#endif  // PHYSICS_INCLUDE_BOUNDINGBOXES_HPP_