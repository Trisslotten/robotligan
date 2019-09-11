#ifndef MODEL_H_
#define MODEL_H_

#include "mesh.h"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

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
   
 public:
  Model(char* path);
  Model();
  ~Model();

  void LoadModelFromFile(const GLchar* path);
  void Draw(GLuint shader);
};

#endif
