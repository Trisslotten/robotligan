#ifndef MODEL_HPP_
#define MODEL_HPP_

#include "mesh.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "../shader.hpp"

namespace glob {


struct Joint {
	std::vector<Joint*> Children;
	glm::vec3 position = glm::vec3(0.f);
	glm::mat4 transform = glm::mat4(0.f);
	char id;
	std::string name;
};

class Model {
 private:
  Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

  GLint TextureFromFile(const char* path, std::string directory);

  void LoadModel(std::string path);
  void ProcessNode(aiNode* node, const aiScene* scene);
  std::string printArmature(Joint* joint, int depth = 0);
  Joint* makeArmature(aiNode* node, bool inside_armature);

  std::vector<Texture> texture_loaded_;
  std::vector<Mesh> mesh_;
  std::vector<Texture> LoadMaterielTextures(aiMaterial* material,
                                            aiTextureType type,
                                            std::string type_name);

  std::string directory_;

  Joint* root_joint_;
  std::vector<glm::vec3> weights_;
  std::vector<Joint*> bones;
  unsigned int num_bones_ = 0;
  bool has_armature_ = false;

  bool is_loaded_ = false;

 public:
  Model();
  Model(const std::string& path);
  ~Model();

  void LoadFromFile(const std::string& path);
  bool IsLoaded() { return is_loaded_; };

  void Draw(ShaderProgram& shader);
};

}  // namespace glob

#endif // MODEL_HPP_
