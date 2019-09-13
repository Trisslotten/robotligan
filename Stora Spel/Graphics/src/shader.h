#ifndef GLOB_SHADER_H_
#define GLOB_SHADER_H_

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

#define uniformDec(glmtype)                                                    \
  void uniformv(const std::string& name, GLuint count, const glmtype* values); \
  void uniform(const std::string& name, const glmtype& value);

namespace glob {

class ShaderProgram {
  const std::string SHADERS_PATH = "shaders/";

  std::unordered_map<GLenum, std::string> paths;
  std::unordered_map<GLenum, GLuint> ids;

  GLuint id;

  GLuint findUniformLocation(const std::string& name);

  std::unordered_map<std::string, GLuint> uniform_locations;

  bool compiled = false;

 public:
  ShaderProgram();
  ~ShaderProgram();

  void add(GLenum type, const std::string& path);
  void add(const std::string& path);
  void compile();

  void reload();

  void use();

  bool isCompiled() { return compiled; }

  uniformDec(GLfloat);
  uniformDec(glm::vec2);
  uniformDec(glm::vec3);
  uniformDec(glm::vec4);

  uniformDec(GLint);
  uniformDec(glm::ivec2);
  uniformDec(glm::ivec3);
  uniformDec(glm::ivec4);

  uniformDec(GLuint);
  uniformDec(glm::uvec2);
  uniformDec(glm::uvec3);
  uniformDec(glm::uvec4);

  uniformDec(glm::mat2);
  uniformDec(glm::mat3);
  uniformDec(glm::mat4);

  uniformDec(glm::mat2x3);
  uniformDec(glm::mat3x2);

  uniformDec(glm::mat2x4);
  uniformDec(glm::mat4x2);

  uniformDec(glm::mat3x4);
  uniformDec(glm::mat4x3);

  GLuint getId() { return this->id; }
};

#undef uniformDec

}  // namespace glob

#endif GLOB_SHADER_H_