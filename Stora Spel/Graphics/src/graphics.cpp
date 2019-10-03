#include "glob/graphics.hpp"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <unordered_map>

#include "Model/model.hpp"
#include "glob/camera.hpp"
#include "shader.hpp"

#include "Font/Font2D.hpp"

#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdfgen.h>

namespace glob {
namespace {

struct RenderItem {
  Model *model;
  glm::mat4 transform;
};

struct BoneAnimatedRenderItem {
	Model* model;
	glm::mat4 transform;
	std::vector<glm::mat4> bone_transforms;//may be a performance bottleneck, pointer instead?
	int numBones;
};

struct TextItem {
  Font2D *font;
  glm::vec2 pos;
  unsigned int size;
  std::string text;
  glm::vec4 color;
};

struct LightItem {
	glm::vec3 pos;
	glm::vec3 color;
	glm::float32 radius;
	glm::float32 ambient;
};

ShaderProgram test_shader;
ShaderProgram model_shader;
ShaderProgram animated_model_shader;
ShaderProgram text_shader;
ShaderProgram wireframe_shader;

std::vector<ShaderProgram*> mesh_render_group;

GLuint triangle_vbo, triangle_vao;
GLuint cube_vbo, cube_vao;
GLuint quad_vbo, quad_vao;

float num_frames = 0;

Camera camera{
    glm::vec3(25, 5, 0), glm::vec3(0, 3, 0), 90, 16.f / 9.f, 0.1f, 100.f};

/*
TextureHandle current_texture_guid = 1;
std::unordered_map<std::string, TextureHandle> texture_handles;
std::unordered_map<TextureHandle, Texture> textures;
*/

ModelHandle current_model_guid = 1;
Font2DHandle current_font_guid = 1;
std::unordered_map<std::string, ModelHandle> model_handles;
std::unordered_map<ModelHandle, Model> models;
std::unordered_map<std::string, Font2DHandle> font_2D_handles;
std::unordered_map<Font2DHandle, Font2D> fonts;

std::vector<RenderItem> items_to_render;
std::vector<LightItem> lights_to_render;
std::vector<BoneAnimatedRenderItem> bone_animated_items_to_render;
std::vector<glm::mat4> cubes;
std::vector<TextItem> text_to_render;

void DrawFullscreenQuad() {
  glBindVertexArray(triangle_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
}

}  // namespace
void DrawCube(glm::mat4 t) {
  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  wireframe_shader.use();
  wireframe_shader.uniform("cam_transform", cam_transform);
  wireframe_shader.uniform("model_transform", t);
  glBindVertexArray(cube_vao);
  glDisable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(0);
}

void Init() {
//std::cout << "Max uniform size: " << MAX_VERTEX_UNIFORM_COMPONENTS_ARB << "\n";

  test_shader.add("testshader.frag");
  test_shader.add("testshader.vert");
  test_shader.compile();

  model_shader.add("modelshader.vert");
  model_shader.add("modelshader.frag");
  model_shader.compile();

  animated_model_shader.add("animatedmodelshader.vert");
  animated_model_shader.add("modelshader.frag");
  animated_model_shader.compile();

  mesh_render_group.push_back(&animated_model_shader);
  mesh_render_group.push_back(&model_shader);

  wireframe_shader.add("modelshader.vert");
  wireframe_shader.add("wireframe.frag");
  wireframe_shader.compile();

  text_shader.add("text2Dshader.vert");
  text_shader.add("text2Dshader.frag");
  text_shader.compile();

  glGenVertexArrays(1, &triangle_vao);
  glBindVertexArray(triangle_vao);
  std::vector<glm::vec3> vertices{
      {0, 1, 1},
      {0, -1, 1},
      {0, -1, -1},
  };
  glGenBuffers(1, &triangle_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                        (GLvoid *)0);


  glGenVertexArrays(1, &cube_vao);
  glBindVertexArray(cube_vao);
  std::vector<glm::vec3> vertices_cube {
      {1, 1, 1}, {1, -1, 1}, {-1, 1, 1}, // z+ face
      {-1, -1, 1}, {1, -1, 1},  {-1, 1, 1},

      {1, 1, -1}, {1, -1, -1}, {1, -1, 1}, // x+ face
      {1, 1, -1},  {1, 1, 1},  {1, -1, 1},

      {-1, 1, -1}, {1, 1, -1},  {1, 1, 1}, // y+ face
      {-1, 1, -1},  {1, 1, 1},    {-1, 1, 1},

      {-1, -1, -1}, {1, 1, -1},  {-1, 1, -1}, // z- face
      {1, 1, -1},   {-1, -1, -1}, {1, -1, -1},

      {-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, // y- face
      {-1, -1, 1},  {1, -1, 1},  {1, -1, -1},

      {-1, -1, -1}, {-1, -1, 1}, {-1, 1, -1}, // x- face
      {-1, -1, 1},  {-1, 1, -1}, {-1, 1, 1}
  };
  glGenBuffers(1, &cube_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices_cube.size(),
               vertices_cube.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                        (GLvoid *)0);

  std::vector<glm::vec3> quad_vertices{
      {-1, -1, 0}, {1, -1, 0}, {-1, 1, 0}, {1, 1, 0}};
  glGenVertexArrays(1, &quad_vao);
  glBindVertexArray(quad_vao);
  glGenBuffers(1, &quad_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * quad_vertices.size(),
               quad_vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                        (GLvoid *)0);

  glBindVertexArray(0);
}

// H=Handle, A=Asset
template <class H, class A>
H GetAsset(std::unordered_map<std::string, H> &handles,
           std::unordered_map<H, A> &assets, H &guid,
           const std::string filepath) {
  H result = 0;

  auto item = handles.find(filepath);
  if (item == handles.end()) {
    std::cout << "DEBUG graphics.cpp: Loading asset '" << filepath << "'\n";
    A &asset = assets[guid];
    asset.LoadFromFile(filepath);
    if (asset.IsLoaded()) {
      handles[filepath] = guid;
      result = guid;
      guid++;
    } else {
      // remove the asset since it could not load
      assets.erase(guid);
    }
  } else {
    // if asset is loaded
    std::cout << "DEBUG graphics.cpp: Asset '" << filepath
              << "' already loaded\n";
    result = item->second;
  }

  return result;
}

ModelHandle GetModel(const std::string &filepath) {
  return GetAsset<ModelHandle, Model>(model_handles, models, current_model_guid,
                                      filepath);
}
Font2DHandle GetFont(const std::string &filepath) {
  return GetAsset<Font2DHandle, Font2D>(font_2D_handles, fonts, current_font_guid, filepath);
}

animData GetAnimationData(ModelHandle handle) {
	auto res = models.find(handle);
	animData data;
	if (res == models.end()) {
		std::cout << "ERROR graphics.cpp: could not find submitted model\n";
		return data;
	}

	glob::Model* model = &res->second;

	//copy armature
	for (auto source : model->bones_) {
		glob::Joint j;
		j.id = source->id;
		j.name = source->name;
		j.offset = source->offset;
		j.transform = source->transform;
		j.f_transform = source->f_transform;
		for (auto c : source->children) {
			//std::cout << c << "\n";
			j.children.push_back(c);
		}
		data.bones.push_back(j);
	}

	//copy animations
	for (auto source : model->animations_) {
		glob::Animation a;
		a.name_ = source->name_;
		a.duration_ = source->duration_;
		a.current_frame_time_ = source->current_frame_time_;
		a.tick_per_second_ = source->tick_per_second_;
		a.channels_ = source->channels_;
		data.animations.push_back(a);
	}

	data.globalInverseTransform = model->globalInverseTransform;

	return data;
}
/*
TextureHandle GetTexture(const std::string &filepath) {
  return GetAsset<TextureHandle, Texture>(texture_handles, textures,
                                          current_texture_guid, filepath);
}
*/

void SubmitLightSource(glm::vec3 pos, glm::vec3 color, glm::float32 radius, glm::float32 ambient) {
	LightItem  item;
	item.pos = pos;
	item.color = color;
	item.radius = radius;
	item.ambient = ambient;
	lights_to_render.push_back(item);
}

void SubmitBAM(ModelHandle model_h, glm::mat4 transform, std::vector<glm::mat4> bone_transforms) {//Submit Bone Animated Mesh
	BoneAnimatedRenderItem BARI;

	auto find_res = models.find(model_h);
	if (find_res == models.end()) {
		std::cout << "ERROR graphics.cpp: could not find submitted model\n";
		return;
	}
	BARI.model = &find_res->second;
	BARI.bone_transforms = bone_transforms;

	const glm::mat4 pre_rotation =
		glm::rotate(glm::pi<float>() / 2.f, glm::vec3(0, 1, 0)) *
		glm::rotate(-glm::pi<float>() / 2.f, glm::vec3(1, 0, 0));

	BARI.transform = transform * pre_rotation;
	BARI.numBones = BARI.bone_transforms.size();
	bone_animated_items_to_render.push_back(BARI);
}

void Submit(ModelHandle model_h, glm::vec3 pos) {
  glm::mat4 transform = glm::translate(pos);
  Submit(model_h, transform);
}

void Submit(ModelHandle model_h, glm::mat4 transform) {
  auto find_res = models.find(model_h);
  if (find_res == models.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted model\n";
    return;
  }

  const glm::mat4 pre_rotation =
      glm::rotate(glm::pi<float>() / 2.f, glm::vec3(0, 1, 0)) *
      glm::rotate(-glm::pi<float>() / 2.f, glm::vec3(1, 0, 0));

  RenderItem to_render;
  to_render.model = &find_res->second;
  to_render.transform = transform * pre_rotation;
  items_to_render.push_back(to_render);
}

void Submit(Font2DHandle font_h, glm::vec2 pos, unsigned int size,
            std::string text, glm::vec4 color) {
  auto find_res = fonts.find(font_h);
  if (find_res == fonts.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted font! \n";
    return;
  }

  TextItem to_render;
  to_render.font = &find_res->second;
  to_render.pos = pos;
  to_render.size = size;
  to_render.text = text;
  to_render.color = color;
  text_to_render.push_back(to_render);
}

void SubmitCube(glm::mat4 t) { cubes.push_back(t); }

void Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  int lightNR;
  for (auto& shader : mesh_render_group) {
	lightNR = 0;
	shader->use();
	for (auto& light_item : lights_to_render) {
		  shader->uniform("light_pos[" + std::to_string(lightNR) + "]", light_item.pos);
		  shader->uniform("light_col[" + std::to_string(lightNR) + "]", light_item.color);
		  shader->uniform("light_radius[" + std::to_string(lightNR) + "]", light_item.radius);
		  shader->uniform("light_amb[" + std::to_string(lightNR) + "]", light_item.ambient);
		  lightNR++;
	  }
	shader->uniform("NR_OF_LIGHTS", (int)lights_to_render.size());

	shader->uniform("cam_transform", cam_transform);
  }

  model_shader.use();
  //model_shader.uniform("num_frames", num_frames);
  for (auto &render_item : items_to_render) {
    model_shader.uniform("model_transform", render_item.transform);
    render_item.model->Draw(model_shader);
  }

  animated_model_shader.use();
  //render bone animated items
  for (auto& BARI : bone_animated_items_to_render) {
	  animated_model_shader.uniform("model_transform", BARI.transform);
	  int numBones = 0;
	  for (auto& bone : BARI.bone_transforms) {
		  animated_model_shader.uniform("bone_transform[" + std::to_string(numBones) + "]", bone);
		  numBones++;
	  }
	  //animated_model_shader.uniform("NR_OF_BONES", (int)BARI.bone_transforms.size());
	  BARI.model->Draw(animated_model_shader);
  }

  // render wireframe cubes
  for (auto &m : cubes) DrawCube(m);

  glBindVertexArray(quad_vao);
  text_shader.use();
  for (auto &text_item : text_to_render) {
    text_item.font->Draw(text_shader, text_item.pos, text_item.size,
                        text_item.text, text_item.color);
  }
  lights_to_render.clear();
  items_to_render.clear();
  bone_animated_items_to_render.clear();
  text_to_render.clear();
  cubes.clear();

  num_frames++;
}

void* GetCamera() { return (void*)&camera; }

}  // namespace glob