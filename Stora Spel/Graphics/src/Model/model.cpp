#include "model.hpp"

#include <iostream>
#include <sstream>

#include <lodepng.hpp>

namespace glob {

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
  std::vector<Vertex> vertex;
  std::vector<GLuint> indices;
  std::vector<Texture> textures;
  std::vector<Joint> bones;

  // Process the mesh
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
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

  // Process materials
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial* temp_material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture> diffuse_maps =
        LoadMaterielTextures(temp_material, aiTextureType_DIFFUSE,
                             "texture_diffuse"  // Make sure this in glsl
        );
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    std::vector<Texture> specular_maps =
        LoadMaterielTextures(temp_material, aiTextureType_SPECULAR,
                             "texture_specular"  // Make sure this in glsl
        );
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
  }

  return Mesh(vertex, indices, textures);
}

GLint Model::TextureFromFile(const char* path, std::string directory) {
  std::string filename = std::string(path);
  filename = directory + '/' + filename;

  // Generate texture id
  GLuint texture_id;
  glGenTextures(1, &texture_id);

  // Load texture
  std::vector<unsigned char> image;
  unsigned width, height;

  unsigned error = lodepng::decode(image, width, height, filename);
  if (error != 0) {
    std::cout << "ERROR: Could not load texture: " << filename << "\n";
    return false;
  }

  // Generate texture data
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image.data());
  // glGenerateMipmap(GL_TEXTURE_2D);

  // Set some parameters for the texture

  glBindTexture(GL_TEXTURE_2D, 0);  // Unbind the texture

  return texture_id;
}

void Model::LoadModel(std::string path) {
  Assimp::Importer import;

  unsigned int flags = 0;
  flags |= aiProcess_Triangulate;
  flags |= aiProcess_FlipUVs;

  const aiScene* scene =
      import.ReadFile(path, flags);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR model.cpp: During Assimp load model: "
              << import.GetErrorString() << "\n";
    return;
  }

  directory_ = path.substr(0, path.find_last_of('/'));
  ProcessNode(scene->mRootNode, scene);
  Joint* r = makeArmature(scene->mRootNode, false);
  if (r) {
	  root_joint_ = r;
	  std::cout << "Armature loaded, root bone set.\n";
	  has_armature_ = true;
	  //std::cout << (printArmature(r)) << "\n";
  }
  else {
	  std::cout << "Mesh has no valid armature.\n";
  }

  if (scene->HasAnimations()) {
	  std::cout << "Animations detected.\n";
	  int numAnimations = scene->mNumAnimations;
	  std::cout << numAnimations << " animations detected.\n";
	  for (int i = 0; i < numAnimations; i++) {
		  std::cout  << "Name: " << scene->mAnimations[i]->mName.data << "\n";
		  std::cout << "	Channels: " << scene->mAnimations[i]->mNumChannels << "\n";
		  std::cout << "	Duration: " << scene->mAnimations[i]->mDuration << "\n";
		  std::cout << "	TPS: " << scene->mAnimations[i]->mTicksPerSecond << "\n";
		  for (int j = 0; j < scene->mAnimations[i]->mNumChannels; j++) {
			  std::cout << "		Name: " << scene->mAnimations[i]->mChannels[j]->mNodeName.data << "\n";
			  //bind to armature IDs
			  //Lol how?
		  }
	  }

  }

  is_loaded_ = true;
}

std::string Model::printArmature(Joint* joint, int depth) {
	std::stringstream ss;
	ss << joint->name << "\n";
	depth++;
	for (auto j : joint->Children) {
		for (int i = 0; i < depth; i++) {
			ss << "-";
		}
		ss << printArmature(j, depth);
	}
	return ss.str();
}

Joint* Model::makeArmature(aiNode* node, bool inside_armature) {
	bool rootNode = (node->mName.data == std::string("Root"));

	if (inside_armature || rootNode) {
		Joint* j = new Joint;
		bones.push_back(j);
		j->name = node->mName.data;
		j->id = num_bones_;
		num_bones_++;

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			std::string disallowed_ending = "_end";
			bool endNode = std::equal(disallowed_ending.rbegin(), disallowed_ending.rend(), std::string(node->mChildren[i]->mName.data).rbegin());
			if (!endNode) {
				j->Children.push_back(makeArmature(node->mChildren[i], inside_armature || rootNode));
			}
		}

		return j;
	}
	else {
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			Joint* ret = makeArmature(node->mChildren[i], false);
			if (ret) {
				return ret;
			}
		}
	}
	return nullptr;
}

// TODO: check if node transform fixes up-vector from blender export
void Model::ProcessNode(aiNode* node, const aiScene* scene) {
  // Process all the nodes meshes
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		mesh_.push_back(ProcessMesh(mesh, scene));
	}
	// Then process nodes children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    ProcessNode(node->mChildren[i], scene);
  }
}

std::vector<Texture> Model::LoadMaterielTextures(aiMaterial* material,
                                                 aiTextureType type,
                                                 std::string type_name) {
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
      temp_texture.id_texture = TextureFromFile(ai_string.C_Str(), directory_);
      temp_texture.type = type_name;
      temp_texture.path = ai_string;
      std::cout << ai_string.C_Str() << "\n";
      texture.push_back(temp_texture);
      texture_loaded_.push_back(temp_texture);
    }
  }

  return texture;
}

Model::Model(const std::string& path) { LoadModel(path); }
Model::Model() {}

Model::~Model() {
	for (auto b : bones) {
		delete b;
	}
}

void Model::LoadFromFile(const std::string& path) { LoadModel(path); }

void Model::Draw(ShaderProgram& shader) {
  for (unsigned int i = 0; i < mesh_.size(); i++) {
    mesh_[i].Draw(shader);
  }
}

}  // namespace glob