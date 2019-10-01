#ifndef MODEL_HPP_
#define MODEL_HPP_

#include "mesh.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "../shader.hpp"
#include "glob/Animation.hpp"
#include "glob/joint.hpp"

namespace glob {

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


  bool is_loaded_ = false;

 public:
  Model();
  Model(const std::string& path);
  ~Model();

  void LoadFromFile(const std::string& path);
  bool IsLoaded() { return is_loaded_; };

  void Draw(ShaderProgram& shader);

  std::vector<glm::vec4> weights_;
  std::vector<glm::ivec4> bone_index_;
  std::vector<Joint*> bones_;
  std::vector<Animation*> animations_;
};

}  // namespace glob

#endif // MODEL_HPP_
