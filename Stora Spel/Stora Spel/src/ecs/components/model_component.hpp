#ifndef MODEL_COMPONENT_HPP_
#define MODEL_COMPONENT_HPP_

#include "glob/graphics.hpp"


struct ModelComponent {
  std::vector<glob::ModelHandle> handles;


  bool invisible = false;
  
  // TODO: have a material index for each handle in handles-vector
  int diffuse_index = 0;
  bool cast_shadow = true;
  
  glm::vec3 offset = glm::vec3(0.f);
  glm::quat rot_offset = glm::quat();

  float emission_strength = 1.0f;
};

#endif  // MODEL_COMPONENT_HPP_
