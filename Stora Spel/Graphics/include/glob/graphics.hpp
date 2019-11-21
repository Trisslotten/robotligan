#ifndef GLOB_GRAPHICS_HPP_
#define GLOB_GRAPHICS_HPP_

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "animation.hpp"
#include "camera.hpp"
#include "glob/mesh_data.hpp"
#include "handletypes.hpp"
#include "joint.hpp"

namespace glob {

struct animData {
  std::vector<Joint> bones;
  std::vector<Animation*> animations;
  glm::mat4 globalInverseTransform;
  // groups
  bool humanoid = false;
  int hip = -1;
  std::vector<int> upperBody;
  std::vector<int> spine;
  std::vector<int> leftLeg;
  std::vector<int> rightLeg;
  std::vector<int> leftArm;
  std::vector<int> rightArm;
  std::vector<int> arms;
  std::vector<int> legs;

  void makeGroup(int root, std::vector<int>* group) {
    group->push_back(root);
    for (int i = 0; i < bones.at(root).children.size(); i++) {
      makeGroup(bones.at(root).children.at(i), group);
    }
  }

  int armatureRoot;
};

/*
 * Initialize renderer.
 * Must be called before other functions.
 */
EXPORT void Init();

/*
 * Returns a model handle for the specified model file.
 * Skips loading if model is loaded.
 */
EXPORT ModelHandle GetModel(const std::string& filepath);

EXPORT ModelHandle GetTransparentModel(const std::string& filepath);

EXPORT ParticleSystemHandle CreateParticleSystem();

EXPORT void DestroyParticleSystem(ParticleSystemHandle handle);

EXPORT void ResetParticles(ParticleSystemHandle handle);

EXPORT void SetEmitPosition(ParticleSystemHandle handle, glm::vec3 pos);

EXPORT void SetParticleDirection(ParticleSystemHandle handle, glm::vec3 dir);

EXPORT void SetParticleSettings(
    ParticleSystemHandle handle,
    std::unordered_map<std::string, std::string> map);

EXPORT void SetParticleSettings(ParticleSystemHandle handle,
                                std::string filename);

EXPORT Font2DHandle GetFont(const std::string& filepath);
EXPORT GUIHandle GetGUIItem(const std::string& filepath);

/*
 *Get model information relevant to the animation component
 *
 */

EXPORT animData GetAnimationData(ModelHandle handle);

EXPORT E2DHandle GetE2DItem(const std::string& filepath);

EXPORT glob::MeshData GetMeshData(ModelHandle model_h);

EXPORT void UpdateParticles(ParticleSystemHandle handle, float dt);

EXPORT void SubmitLightSource(glm::vec3 pos, glm::vec3 color,
                              glm::float32 radius, glm::float32 ambient);
// Submit Bone Animated Mesh
EXPORT void SubmitBAM(ModelHandle model_h, glm::mat4 transform,
                      std::vector<glm::mat4> bone_transforms,
                      int material_index = 0,
                      bool cast_shadow = true, float emissive_strength = 1.0f);
EXPORT void SubmitBAM(const std::vector<ModelHandle>& handles,
                      glm::mat4 transform,
                      std::vector<glm::mat4> bone_transforms,
                      int material_index = 0,
                      bool cast_shadow = true, float emissive_strength = 1.0f);
EXPORT void Submit(ModelHandle model_h, glm::vec3 pos, int material_index = 0,
                   bool cast_shadow = true,
                   float emissive_strength = 1.0f);
EXPORT void Submit(ModelHandle model_h, glm::mat4 transform,
                   int material_index = 0,
                   bool cast_shadow = true, float emissive_strength = 1.0f);
EXPORT void Submit(const std::vector<ModelHandle>& handles, glm::mat4 transform,
                   int material_index = 0,
                   bool cast_shadow = true, float emissive_strength = 1.0f);
EXPORT void SubmitParticles(ParticleSystemHandle handle);
EXPORT void SubmitCube(glm::mat4 t);
EXPORT void SubmitWireframeMesh(ModelHandle model_h);
EXPORT void LoadWireframeMesh(ModelHandle model_h,
                              const std::vector<glm::vec3>& vertices,
                              const std::vector<unsigned int>& indices);

EXPORT double GetWidthOfText(Font2DHandle font_handle, std::string text,
                             float size);
EXPORT double GetWidthOfChatText(Font2DHandle font_handle, std::string text,
                                 float size);
EXPORT void Submit(Font2DHandle font_h, glm::vec2 pos, unsigned int size,
                   std::string text, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                   bool visible = true, bool equal_spacing = false,
                   float spacing = 13.0f);

EXPORT void Submit(Font2DHandle font_h, glm::vec3 pos, float size,
                   std::string text, glm::vec4 color = glm::vec4(1, 1, 1, 1),
                   glm::mat4 rot = glm::mat4(1.0f));

EXPORT void Submit(GUIHandle gui_h, glm::vec2 pos, float scale,
                   float scale_x = 100.0f, float opacity = 1.0f, float rot = 0.0f);
EXPORT void Submit(E2DHandle e2D_h, glm::vec3 pos, float scale,
                   float rotDegrees, glm::vec3 rotAxis);

EXPORT void Submit(E2DHandle e2D_h, glm::vec3 pos, glm::mat4 matrix);

EXPORT void SubmitTrail(const std::vector<glm::vec3>& pos_history, float width,
                        glm::vec4 color);

EXPORT Camera& GetCamera();
EXPORT void SetCamera(Camera camera);

EXPORT void SetModelUseGL(bool use_gl);

EXPORT void SetSSAO(bool val);

EXPORT void ReloadShaders();

EXPORT void SetInvisibleEffect(bool in_bool);

EXPORT void SetBlackout(bool blackout);

EXPORT void SetSky(const std::string& texture);

EXPORT void AddSpotlight(glm::vec3 position, glm::mat4 transform);

/*
 * Render all items submitted this frame
 */
EXPORT void Render();

}  // namespace glob

#endif  // GLOB_GRAPHICS_HPP_