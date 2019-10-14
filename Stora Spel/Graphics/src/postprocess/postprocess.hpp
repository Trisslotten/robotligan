#ifndef POST_PROCESS_HPP_
#define POST_PROCESS_HPP_

// no move plz
#include <glad/glad.h>
// no move plz
#include <GLFW/glfw3.h>

#include "shader.hpp"

namespace glob {

class PostProcess {
public:
  void Init();

  void BeforeDraw();

  void AfterDraw();
  
  void BindColorTex(GLuint slot);
  void BindEmissionTex(GLuint slot);

private:
GLuint framebuffer_ = 0;
GLuint renderbuffer_ = 0;
GLuint draw_color_texture_ = 0;
GLuint draw_emission_texture_ = 0;
GLuint emission_blur_textures_[2] = {0, 0};
int emission_blur_tex_index_ = 0;

ShaderProgram kawase_blur_compute_;

};

}  // namespace glob

#endif  // POST_PROCESS_HPP_