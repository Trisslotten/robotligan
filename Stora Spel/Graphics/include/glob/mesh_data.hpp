#ifndef MESH_DATA_HPP_
#define MESH_DATA_HPP_

#include <vector>

#include <glm/vec3.hpp>

namespace glob {
  struct MeshData {
  std::vector<glm::vec3> pos;
  std::vector<unsigned int> indices;
};

}  // namespace glob

#endif  // MESH_DATA_HPP_