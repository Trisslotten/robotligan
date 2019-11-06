#ifndef POST_PROCESS_HPP_
#define POST_PROCESS_HPP_

// no move plz
#include <glad/glad.h>
// no move plz
#include <GLFW/glfw3.h>

#include "shader.hpp"
#include "blur.hpp"

namespace glob {

class PostProcess {
public:
  void Init(Blur& blur);

  void BeforeDraw();

  void AfterDraw(Blur& blur);
  
  void BindColorTex(GLuint slot);
  void BindEmissionTex(GLuint slot);
  void BindDepthTex(GLuint slot);
  void BindNormalTex(GLuint slot);
  void BindPositionTex(GLuint slot);

private:
GLuint framebuffer_ = 0;
GLuint renderbuffer_ = 0;
GLuint draw_color_texture_ = 0;
GLuint draw_emission_texture_ = 0;
GLuint draw_normal_texture_ = 0;
GLuint draw_depth_texture_ = 0;
GLuint draw_position_texture_ = 0;

GLuint blurred_emission_texture = 0;

uint64_t blur_id_;

};

}  // namespace glob

#endif  // POST_PROCESS_HPP_