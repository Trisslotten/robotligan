#ifndef MODEL_HPP_
#define MODEL_HPP_

#include "mesh.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "../shader.hpp"
#include "../Animation.hpp"

namespace glob {


struct Joint {
	std::vector<Joint*> Children;
	glm::vec3 position = glm::vec3(0.f);
	glm::mat4 transform = glm::mat4(0.f);
	char id;
	std::string name;
	std::vector<int> weights;
};

class Model {
 private:
  Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

  GLint TextureFromFile(const char* path, std::string directory);

  void LoadModel(std::string path);
  void ProcessNode(aiNode* node, const aiScene* scene);
  std::string PrintArmature();
  Joint* MakeArmature(aiNode* node);

  std::vector<Texture> texture_loaded_;
  std::vector<Mesh> mesh_;
  std::vector<Texture> LoadMaterielTextures(aiMaterial* material,
                                            aiTextureType type,
                                            std::string type_name);

  std::string directory_;

  std::vector<glm::vec4> weights_;
  std::vector<glm::ivec4> bone_index_;

  std::vector<Joint*> bones_;
  unsigned int num_bones_ = 0;


  std::vector<Animation> animations_;


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
