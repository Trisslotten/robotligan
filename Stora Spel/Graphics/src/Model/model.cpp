#include "model.hpp"

#include <iostream>
#include <sstream>

#include <glm/ext.hpp>
#include <lodepng.hpp>
#include <textureslots.hpp>
#include "Model/modelconfig.hpp"
#include "material/material.hpp"
#include "usegl.hpp"

#include <glob/AssimpToGLMConverter.hpp>

namespace glob {

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, glm::mat4 transform) {
  std::vector<Vertex> vertex;
  std::vector<GLuint> indices;
  std::vector<Texture> textures;
  std::vector<Joint> bones;
  std::vector<glm::vec4> weights;
  std::vector<glm::ivec4> boneIndex;

  // Process the mesh
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    // create weight objects

    weights.push_back(glm::vec4(0.f));
    boneIndex.push_back(glm::ivec4(-1));

    Vertex temp_vertex;
    glm::vec3 vector_vertices;
    // Process vertices
    vector_vertices.x = mesh->mVertices[i].x;
    vector_vertices.y = mesh->mVertices[i].y;
    vector_vertices.z = mesh->mVertices[i].z;
    temp_vertex.position = vector_vertices;
    // Process texture

    int tex_coord_index = 0;
    // TODO: not use this hack
    if (mesh->mTextureCoords[1]) {
      tex_coord_index = 1;
    }
    if (mesh->mTextureCoords[0]) {
      glm::vec2 vector_texture;
      vector_texture.x = mesh->mTextureCoords[tex_coord_index][i].x;
      vector_texture.y = mesh->mTextureCoords[tex_coord_index][i].y;
      temp_vertex.texture = vector_texture;
    } else {
      temp_vertex.texture = glm::vec2(0.0f, 0.0f);
    }
    // Process normals
    if (mesh->mNormals) {
      glm::vec3 vector_normals;
      vector_normals.x = mesh->mNormals[i].x;
      vector_normals.y = mesh->mNormals[i].y;
      vector_normals.z = mesh->mNormals[i].z;
      temp_vertex.normals = vector_normals;
    }
    vertex.push_back(temp_vertex);
  }

  // Process faces / indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace temp_faces = mesh->mFaces[i];
    for (GLuint y = 0; y < temp_faces.mNumIndices; y++) {
      indices.push_back(temp_faces.mIndices[y]);
    }
  }

  std::string config_path = filepath_.substr(0, filepath_.find_last_of('.'));
  config_path += ".mcfg";
  ModelConfig config{config_path};
  if (config.isLoaded()) {
    auto num_diff = config.GetInt("num_diffuse_textures");
    if (num_diff) {
      num_diffuse_textures_ = *num_diff;
    }

    std::unordered_map<materials::Type, std::string> wanted_textures;
    auto normal_map = config.GetWord("normal_map");
    if (normal_map) {
      wanted_textures[materials::NORMAL] = *normal_map;
    }
    auto normal_map_scale = config.GetFloat("normal_map_scale");
    if (normal_map_scale) {
      normal_map_scale_ = *normal_map_scale;
    }

    auto metallic_map = config.GetWord("metallic_map");
    if (metallic_map) {
      wanted_textures[materials::METALLIC] = *metallic_map;
    }
    auto metallic_map_scale = config.GetFloat("metallic_map_scale");
    if (metallic_map_scale) {
      metallic_map_scale_ = *metallic_map_scale;
    }

    auto roughness_map = config.GetWord("roughness_map");
    if (roughness_map) {
      wanted_textures[materials::ROUGHNESS] = *roughness_map;
    }
    auto roughness_map_scale = config.GetFloat("roughness_map_scale");
    if (roughness_map_scale) {
      roughness_map_scale_ = *roughness_map_scale;
    }

    auto is_glass = config.GetBool("is_glass");
    if (is_glass) {
      is_glass_ = *is_glass;
    }

    material_ = materials::Get(wanted_textures);
  }

  // std::cout << "glob::kModelUseGL: " << glob::kModelUseGL << "\n";
  if (glob::kModelUseGL) {
    // Process materials
    if (mesh->mMaterialIndex >= 0) {
      aiMaterial* temp_material = scene->mMaterials[mesh->mMaterialIndex];
      std::vector<Texture> diffuse_maps =
          LoadMaterielTextures(temp_material, aiTextureType_DIFFUSE,
                               "texture_diffuse", TEXTURE_SLOT_DIFFUSE

          );
      textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

      std::vector<Texture> specular_maps =
          LoadMaterielTextures(temp_material, aiTextureType_SPECULAR,
                               "texture_specular", TEXTURE_SLOT_SPECULAR);
      textures.insert(textures.end(), specular_maps.begin(),
                      specular_maps.end());

      std::vector<Texture> emissive_maps =
          LoadMaterielTextures(temp_material, aiTextureType_EMISSIVE,
                               "texture_emissive", TEXTURE_SLOT_EMISSIVE);
      textures.insert(textures.end(), emissive_maps.begin(),
                      emissive_maps.end());
    }
  }

  if (mesh->HasBones()) {
    // std::cout << "Mesh bones: " << mesh->mNumBones << "\n";
    // find and store bones (no children assigned)
    for (int i = 0; i < mesh->mNumBones; i++) {
      Joint* j = new Joint();
      j->id = i;
      j->name = mesh->mBones[i]->mName.data;
      j->offset = AssToGLM::ConvertToGLM4x4(mesh->mBones[i]->mOffsetMatrix);
      bones_.push_back(j);

      // set vec4 arrays for weight and bone index (influencing bone)
      for (int w = 0; w < mesh->mBones[i]->mNumWeights; w++) {
        if (mesh->mBones[i]->mWeights[w].mWeight >
            0.01f) {  // cull bones with very small weights (optimization)
          int boneIndexLength = 0;
          for (int bil = 0; bil < 4;
               bil++) {  // find suitable position for the bone id within the
                         // vec4, the vec is initialized with -1 so anything
                         // above that is already claimed
            if (boneIndex.at(mesh->mBones[i]->mWeights[w].mVertexId)[bil] < 0) {
              boneIndexLength = bil;
              bil = 4;
            }
          }
          boneIndex.at(
              mesh->mBones[i]->mWeights[w].mVertexId)[boneIndexLength] = j->id;
          weights.at(mesh->mBones[i]->mWeights[w].mVertexId)[boneIndexLength] =
              mesh->mBones[i]->mWeights[w].mWeight;
        }
      }
    }
    return Mesh(vertex, indices, textures, weights, boneIndex, transform);
  }

  return Mesh(vertex, indices, textures, transform);
}

GLint Model::TextureFromFile(const char* path, std::string directory,
                             aiTextureType type) {
  std::string filename = std::string(path);
  filename = directory + '/' + filename;

  GLint internal_format = GL_RGBA;
  LodePNGColorType color_type = LodePNGColorType::LCT_RGBA;
  
  if (type == aiTextureType_EMISSIVE) {
    internal_format = GL_RED;
    color_type = LodePNGColorType::LCT_GREY;
    is_emissive_ = true;
  }

  // Load texture
  std::vector<unsigned char> image;
  unsigned width, height;
  unsigned error = lodepng::decode(image, width, height, filename, color_type);
  if (error != 0) {
    std::cout << "ERROR: Could not load texture: " << filename << "\n";
    return false;
  }

  // Generate texture id
  GLuint texture_id;
  glGenTextures(1, &texture_id);

  // Set some parameters for the texture
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0,
               internal_format, GL_UNSIGNED_BYTE, image.data());
  // glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);  // Unbind the texture

  return texture_id;
}

void Model::LoadModel(std::string path) {
  Assimp::Importer import;

  unsigned int flags = 0;
  flags |= aiProcess_Triangulate;
  flags |= aiProcess_FlipUVs;

  filepath_ = path;

  const aiScene* scene = import.ReadFile(path, flags);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR model.cpp: During Assimp load model: "
              << import.GetErrorString() << "\n";
    return;
  }

  globalInverseTransform_ =
      AssToGLM::ConvertToGLM4x4(scene->mRootNode->mTransformation);
  globalInverseTransform_ = glm::inverse(globalInverseTransform_);

  directory_ = path.substr(0, path.find_last_of('/'));

  ProcessNode(scene->mRootNode, scene);

  MakeArmature(scene->mRootNode);

  // std::cout << "Bone size: " << bones_.size() << "\n";

  if (bones_.size() > 0) {
    int rootBone = 0;
    for (int i = 0; i < bones_.size(); i++) {
      if (bones_.at(i)->name == "Armature") {
        rootBone = i;
        break;
      }
    }
    // std::cout << PrintArmature(*bones_.at(rootBone), 0) << "\n";
  }

  if (scene->HasAnimations()) {
    // load animations
    int numAnimations = scene->mNumAnimations;
    // std::cout << numAnimations << " animations detected.\n";
    for (int i = 0; i < numAnimations; i++) {
      Animation* anim = new Animation();
      anim->name_ = scene->mAnimations[i]->mName.data;
      anim->duration_ = scene->mAnimations[i]->mDuration;
      anim->tick_per_second_ = scene->mAnimations[i]->mTicksPerSecond;

      std::cout << "Animation " << i << " : " << anim->name_ << "\n";

      // load channels
      for (int j = 0; j < scene->mAnimations[i]->mNumChannels; j++) {
        std::string jointName =
            scene->mAnimations[i]->mChannels[j]->mNodeName.data;
        Channel channel;
        for (int k = 0;
             k < scene->mAnimations[i]->mChannels[j]->mNumPositionKeys; k++) {
          channel.position_keys.push_back(
              scene->mAnimations[i]->mChannels[j]->mPositionKeys[k]);
        }
        for (int k = 0;
             k < scene->mAnimations[i]->mChannels[j]->mNumRotationKeys; k++) {
          channel.rotation_keys.push_back(
              scene->mAnimations[i]->mChannels[j]->mRotationKeys[k]);
        }
        for (int k = 0;
             k < scene->mAnimations[i]->mChannels[j]->mNumScalingKeys; k++) {
          channel.scaling_keys.push_back(
              scene->mAnimations[i]->mChannels[j]->mScalingKeys[k]);
        }
        // find relevant bone for channel
        char id = 0;
        for (auto j : bones_) {
          if (j->name == jointName) {
            id = j->id;
          }
        }
        channel.boneID = id;
        if (channel.position_keys.size() == 0 &&
            channel.rotation_keys.size() == 0 &&
            channel.scaling_keys.size() == 0) {
          // std::cout << "Empty channel!\n";
        }
        anim->channels_.push_back(channel);
      }
      animations_.push_back(anim);
    }
  }

  calcSortSphere();

  is_loaded_ = true;
}

std::string Model::PrintArmature(Joint parent, int depth) {
  std::stringstream ss;
  for (int i = 0; i < depth; i++) {
    ss << "-";
  }
  ss << parent.name << "\n";
  for (int i = 0; i < parent.children.size(); i++) {
    ss << PrintArmature(*bones_.at(parent.children.at(i)), depth + 1);
  }
  return ss.str();
}

Joint* Model::MakeArmature(aiNode* node) {
  // Takes all known bones from the bones_ vector and structures it (assigns
  // children)
  bool rootNode = (node->mName.data == std::string("Root"));

  bool knownBone = false;
  for (auto& b : bones_) {
    if (node->mName.data == b->name) {  // node is known bone
      b->transform = AssToGLM::ConvertToGLM4x4(node->mTransformation);
      knownBone = true;
      for (int n = 0; n < node->mNumChildren; n++) {
        for (auto PCB : bones_) {
          if (node->mChildren[n]->mName.data == PCB->name) {  // found child
            b->children.push_back(
                PCB->id);  // may god forgive me for stacking so many for-loops
          }
        }
      }
    }
  }

  // node isn't known bone, but might be the progenitor for the entire armature

  if (!knownBone) {
    Joint* j = new Joint;
    j->name = "Armature";
    j->offset = glm::mat4(0.f);
    j->transform = glm::rotate(AssToGLM::ConvertToGLM4x4(node->mTransformation),
                               3.1416f / 2.f, glm::vec3(1.f, 0.f, 0.f));
    bool knownChildren = false;
    for (int n = 0; n < node->mNumChildren; n++) {
      for (auto PCB : bones_) {
        if (node->mChildren[n]->mName.data == PCB->name) {  // found child
          j->children.push_back(PCB->id);
          knownChildren = true;
        }
      }
    }
    if (knownChildren) {
      // std::cout << "Armature created from " << node->mName.data << "\n";
      j->id = bones_.size();
      bones_.push_back(j);
    } else {
      delete j;
    }
  }

  for (int i = 0; i < node->mNumChildren; i++) {
    MakeArmature(node->mChildren[i]);
  }
  return nullptr;
}

// TODO: check if node transform fixes up-vector from blender export
void Model::ProcessNode(aiNode* node, const aiScene* scene, glm::mat4 parent_transform) {
  glm::mat4 transform = parent_transform * AssToGLM::ConvertToGLM4x4(node->mTransformation);
  // Process all the nodes meshes
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    mesh_.push_back(ProcessMesh(mesh, scene, transform));
  }
  // Then process nodes children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene, transform);
  }
}

std::vector<Texture> Model::LoadMaterielTextures(aiMaterial* material,
                                                 aiTextureType type,
                                                 std::string type_name,
                                                 int tex_slot) {
  std::vector<Texture> texture;

  for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
    aiString ai_string;
    material->GetTexture(type, i, &ai_string);

    bool skip_loop = false;

    // Check if texture already has been loaded
    for (unsigned int y = 0; y < texture_loaded_.size() && !skip_loop; y++) {
      if (strcmp(texture_loaded_[y].path.C_Str(), ai_string.C_Str()) == 0) {
        texture.push_back(texture_loaded_[y]);
        skip_loop = true;
      }
    }
    if (!skip_loop) {
      Texture temp_texture;
      temp_texture.id_texture =
          TextureFromFile(ai_string.C_Str(), directory_, type);
      temp_texture.type = type_name;
      temp_texture.path = ai_string;
      temp_texture.slot = tex_slot;
      texture.push_back(temp_texture);
      texture_loaded_.push_back(temp_texture);
    }
  }

  return texture;
}

void Model::calcSortSphere()
{
  float radius = 0;
  glm::vec3 center;

  for (int i = 0; i < mesh_.size(); i++) {
    auto vertices = mesh_[i].GetVertexData();
    glm::mat4 mesh_trans = mesh_[i].GetTransform();

    glm::vec3 mesh_center{0};
    for (const auto& vertex : *vertices) {
      glm::vec3 transformed = mesh_trans * glm::vec4(vertex.position, 1);
      mesh_center += transformed;
    }
    center += mesh_center / float(vertices->size());
  }
  center /= float(mesh_.size());
  
  for (int i = 0; i < mesh_.size(); i++) {
    auto vertices = mesh_[i].GetVertexData();
    glm::mat4 mesh_trans = mesh_[i].GetTransform();
    float mesh_radius = 0;
    for (const auto& vertex : *vertices) {
      glm::vec3 transformed = mesh_trans * glm::vec4(vertex.position, 1);

      float dist = length(transformed - center);
      if (dist > radius) {
        radius = dist;
      } 
    } 
  }

  sort_sphere_center = center;
  sort_sphere_radius = radius;
}

Model::Model(const std::string& path) { LoadModel(path); }
Model::Model() {}

Model::~Model() {
  for (auto b : bones_) {
    delete b;
  }
  for (auto a : animations_) {
    delete a;
  }
}

void Model::LoadFromFile(const std::string& path) { LoadModel(path); }

void Model::Draw(ShaderProgram& shader) {
  material_.Bind(shader);
  shader.uniform("normal_map_scale", normal_map_scale_);
  shader.uniform("metallic_map_scale", metallic_map_scale_);
  shader.uniform("roughness_map_scale", roughness_map_scale_);
  shader.uniform("num_diffuse_textures", num_diffuse_textures_);
  shader.uniform("is_glass", is_glass_ ? 1 : 0);
  for (unsigned int i = 0; i < mesh_.size(); i++) {
    mesh_[i].Draw(shader);
  }
}

float Model::MaxDistance(glm::mat4 transform, glm::vec3 point) {
  float result = 0.f;
  for (int i = 0; i < mesh_.size(); i++) {
    auto vertices = mesh_[i].GetVertexData();
    glm::mat4 mesh_trans = mesh_[i].GetTransform();
    glm::mat4 curr_transform = transform * mesh_trans;
    for (const auto& vertex : *vertices) {
      glm::vec3 transformed = curr_transform * glm::vec4(vertex.position, 1);
      int len = length(point - transformed);
      if (len > result) {
        result = len;
      }
    }
  }
  return result;
}

MeshData Model::GetMeshData() {
  MeshData mesh_data;
  for (unsigned int i = 0; i < mesh_.size(); i++) {
    MeshData temp = mesh_[i].GetMeshData();

    mesh_data.indices.reserve(mesh_data.indices.size() + temp.indices.size());
    mesh_data.pos.reserve(mesh_data.pos.size() + temp.pos.size());
    const unsigned int offset = mesh_data.pos.size();
    for (auto& i : temp.indices) mesh_data.indices.push_back(i + offset);
    for (auto& p : temp.pos) mesh_data.pos.push_back(p);
  }

  return mesh_data;
}

float Model::TraceSortSphere(glm::vec3 cam_dir, glm::vec3 cam_pos, glm::mat4 transform)
{
  glm::vec3 center = sort_sphere_center;
  glm::vec3 offset = sort_sphere_radius * normalize(glm::vec3(1,1,1));
  center = transform * glm::vec4(center, 1);
  offset = transform * glm::vec4(offset, 1);
  float radius = length(center - offset);
  float radius2 = radius * radius;

  float t0, t1;

  glm::vec3 l = center - cam_pos;
  float tca = dot(l, cam_dir);
  float d2 = dot(l, l) - tca*tca;
  if (d2 <= radius2) {
    float thc = sqrt(radius2 - d2);
    t0 = tca - thc;
    t1 = tca + thc;

    if(t0 > t1) std::swap(t0, t1);
    if (t0 < 0) {
      return t1;
    }
    else {
      return t0;
    }
  }
  else {
    return length(cam_pos - center) - radius;
  }
}

}  // namespace glob