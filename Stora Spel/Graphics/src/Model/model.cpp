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
	//create weight objects
	weights_.push_back(glm::vec4(0.f));
	bone_index_.push_back(glm::ivec4(-1));

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

  if (mesh->HasBones()) {
	  std::cout << "Mesh bones: " << mesh->mNumBones << "\n";
	  for (int i = 0; i < mesh->mNumBones; i++) {
		Joint* j = new Joint();
		j->id = i;
		j->name = mesh->mBones[i]->mName.data;
		//std::cout << j->name << "\n";
		bones_.push_back(j);

		for (int w = 0; w < mesh->mBones[i]->mNumWeights; w++){
			int boneIndexLength = 0;
			for (int bil = 0; bil < 4; bil++) {//find suitable position for the bone id
				if (bone_index_.at(w)[bil] == -1) {
					boneIndexLength = bil;
					break;
				}
			}
			bone_index_.at(mesh->mBones[i]->mWeights[w].mVertexId)[boneIndexLength] = j->id;
			weights_.at(mesh->mBones[i]->mWeights[w].mVertexId)[boneIndexLength] = mesh->mBones[i]->mWeights[w].mWeight;
			//std::cout << mesh->mBones[i]->mWeights[w].mVertexId << " : " << bone_index_.at(mesh->mBones[i]->mWeights[w].mVertexId)[boneIndexLength] << " : " << weights_.at(mesh->mBones[i]->mWeights[w].mVertexId)[boneIndexLength] << "\n";
		}
	  }

	  std::cout << "Node bones: " << bones_.size() << "\n";

	  for (int i = 0; i < mesh->mNumVertices; i++) {
		  for (int w = 0; w < 4; w++) {
			  std::cout << bone_index_.at(i)[w];
			  if (w != 3) {
				  std::cout << ", ";
			  }
		  }
		  std::cout << " | ";
		  for (int w = 0; w < 4; w++) {
			  std::cout << weights_.at(i)[w];
			  if (w != 3) {
				  std::cout << ", ";
			  }
		  }
		  std::cout << "\n";
	  }
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

  MakeArmature(scene->mRootNode);
  std::cout << PrintArmature() << "\n";

  if (scene->HasAnimations()) {
	  int numAnimations = scene->mNumAnimations;
	  std::cout << numAnimations << " animations detected.\n";
	  for (int i = 0; i < numAnimations; i++) {
		  Animation anim;
		  anim.name_ = scene->mAnimations[i]->mName.data;
		  anim.duration_ = scene->mAnimations[i]->mDuration;
		  anim.tick_per_second_ = scene->mAnimations[i]->mTicksPerSecond;

		  //std::cout  << "Name: " << anim.name_ << "\n";
		  //std::cout << "  Duration: " << anim.duration_ << "\n";
		  //std::cout << "  TPS: " << anim.tick_per_second_ << "\n";

		  for (int j = 0; j < scene->mAnimations[i]->mNumChannels; j++) {
			  std::string jointName = scene->mAnimations[i]->mChannels[j]->mNodeName.data;
			  //std::cout << "    Name: " << jointName << "\n";
			  Channel channel;
			  for (int k = 0; k < scene->mAnimations[i]->mChannels[j]->mNumPositionKeys; k++) {
				  channel.positionKeys.push_back(scene->mAnimations[i]->mChannels[j]->mPositionKeys[k]);
			  }
			  for (int k = 0; k < scene->mAnimations[i]->mChannels[j]->mNumRotationKeys; k++) {
				  channel.rotationKeys.push_back(scene->mAnimations[i]->mChannels[j]->mRotationKeys[k]);
			  }
			  for (int k = 0; k < scene->mAnimations[i]->mChannels[j]->mNumScalingKeys; k++) {
				  channel.scalingKeys.push_back(scene->mAnimations[i]->mChannels[j]->mScalingKeys[k]);
			  }


			  //find appropriate bone
			  char id = 0;
			  for (auto j : bones_) {
				  if (j->name == jointName) {
					  id = j->id;
					  //std::cout << jointName << "\n";
				  }
			  }

			  channel.boneID = id;

			  anim.channels_.push_back(channel);
		  }

		  animations_.push_back(anim);
	  }

  }

  is_loaded_ = true;
}

std::string Model::PrintArmature() {
	std::stringstream ss;
	for (auto b : bones_) {
		ss << b->name << "\n";
		for (auto c : b->Children) {
			ss << "-" << c->name << "\n";
		}
	}
	return ss.str();
}

Joint* Model::MakeArmature(aiNode* node) {
	bool rootNode = (node->mName.data == std::string("Root"));

	for (auto b : bones_) {
		if (node->mName.data == b->name) {//node is known bone
			for (int n = 0; n < node->mNumChildren; n++) {
				for (auto PCB : bones_) {
					if (node->mChildren[n]->mName.data == PCB->name) {//found child
						b->Children.push_back(PCB);
					}
				}
			}
		}
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		MakeArmature(node->mChildren[i]);
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
	for (auto b : bones_) {
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