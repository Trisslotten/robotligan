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
  glm::vec3 center;
  glm::vec3 normals[3];  // normalized plane normals
  float extents[3];      // lenght to each plane
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