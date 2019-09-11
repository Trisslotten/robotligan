#ifndef PHYSICS_INCLUDE_BOUNDINGBOXES_H_
#define PHYSICS_INCLUDE_BOUNDINGBOXES_H_

#include <vec3.hpp>

struct Sphere {
  glm::vec3 center;
  float radius;
};

struct OBB {
  glm::vec3 center;
  glm::vec3 normals[3]; // normalized plane normals
  float extents[3];     // lenght to each plane
};

struct Arena {
  float xmin, xmax;
  float ymin, ymax;
  float zmin, zmax;
};

#endif  // PHYSICS_INCLUDE_BOUNDINGBOXES_H_