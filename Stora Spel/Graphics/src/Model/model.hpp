#ifndef MODEL_H_
#define MODEL_H_

#include "mesh.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "../shader.hpp"

namespace glob {

class Model {
 private:
  Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

  GLint TextureFromFile(const char* path, std::string directory);

  void LoadModel(std::string path);
  void ProcessNode(aiNode* node, const aiScene* scene);

  std::vector<Texture> texture_loaded_;
  std::vector<Mesh> mesh_;
  std::vector<Texture> LoadMaterielTextures(aiMaterial* material,
                                            aiTextureType type,
                                            std::string type_name);

  std::string directory_;

  bool is_loaded = false;

 public:
  Model();
  Model(const std::string& path);
  ~Model();

  void LoadFromFile(const std::string& path);
  bool IsLoaded() { return is_loaded; };

  void Draw(ShaderProgram& shader);
};

}  // namespace glob

#endif
