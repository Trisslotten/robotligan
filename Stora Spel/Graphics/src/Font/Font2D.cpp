
#include <iostream>

#include <msdfgen/core/Bitmap.h>
#include <msdfgen/core/save-bmp.h>
#include "Font2D.hpp"

#include <direct.h>
#include <lodepng.hpp>

bool fileAlreadyExists(const std::string& path) {
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

void glob::Font2D::GenerateMsdfFont(const std::string& font_path,
                                    const std::string& output_path) {
  msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
  if (ft) {
    font = loadFont(ft, font_path.c_str());
    if (font) {
      msdfgen::Shape shape;
      unsigned int height = 32;
      unsigned int width = 32;
      unsigned int end_char = 255;
      unsigned int start_char = 0;
      unsigned int num_chars = end_char - start_char;

      unsigned int side_dim = glm::ceil(sqrt(num_chars));

      unsigned int v_size =
          (unsigned int)4 * width * height * side_dim * side_dim;
      std::vector<unsigned char> pixels;
      pixels.resize(v_size);
      // pixels.assign(v_size, 0);

      double fscale = 0;
      msdfgen::getFontScale(fscale, font);

      double kerning = 0;
      msdfgen::getKerning(kerning, font, (int)'R', (int)'I');

      std::cout << "[DEBUG] fonten scalen: " << fscale << "\n";
      std::cout << "[DEBUG] kerningen: " << kerning << "\n";

      for (int i = 0; i < num_chars; i++) {
        if (loadGlyph(shape, font, start_char + i)) {
          shapes.push_back(shape);
          double l = 0, r = 0, t = 0, b = 0;
          shape.bounds(l, b, r, t);
          //std::cout << "Char: " << (char)i << " " << l << ", " << r << "\n";

          shape.normalize();
          //                      max. angle
          edgeColoringSimple(shape, 3.0);
          //           image width, height
          msdfgen::Bitmap<float, 3> msdf(height, width);
          //                     range, scale, translation
          msdfgen::generateMSDF(msdf, shape, 4.0, 16 / fscale,
                                msdfgen::Vector2(10.0, 10.0));

          for (unsigned int yy = 0; yy < height; yy++)
            for (unsigned int xx = 0; xx < width; xx++) {
              char r = msdfgen::pixelFloatToByte(msdf(xx, yy)[0]);
              char g = msdfgen::pixelFloatToByte(msdf(xx, yy)[1]);
              char b = msdfgen::pixelFloatToByte(msdf(xx, yy)[2]);

              int jumps_down = i / side_dim;
              int jumps_right = i % side_dim;

              unsigned int pos =
                  (side_dim * width - (jumps_down * width + yy) - 1) *
                      side_dim * width * 4 +
                  (jumps_right * height + xx) * 4;

              pixels[pos] = r;
              pixels[pos + 1] = g;
              pixels[pos + 2] = b;
              pixels[pos + 3] = 255;
            }

          // msdfgen::savePng(msdf, output_name.c_str());
        }
      }
      if (lodepng::encode("Assets/fonts/test_1.png", pixels, width * side_dim,
                          height * side_dim, LCT_RGBA)) {
        std::cout << "[ERROR] Could not generate sprite-sheet for font\n";
      }
      font_texture_ = pixels;

      GLuint texture_id;
      glGenTextures(1, &texture_id);

      // Load texture

      // Generate texture data
      glBindTexture(GL_TEXTURE_2D, texture_id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width * side_dim,
                   height * side_dim, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                   pixels.data());
      // glGenerateMipmap(GL_TEXTURE_2D);

      // Set some parameters for the texture

      glBindTexture(GL_TEXTURE_2D, 0);  // Unbind the texture
      tex_id = texture_id;
      destroyFont(font);
    }
    deinitializeFreetype(ft);
  }
}

std::string glob::Font2D::GenerateFontDirectoryPath(const std::string* path) {
  std::string p = "assets/fonts/generated";

  int start_of_name = path->find_last_of("/");
  int end_of_name = path->find_last_of(".");
  int len = end_of_name - start_of_name;
  std::string only_name = path->substr(start_of_name, len);

  std::string font_folder_name = p + only_name + "_msdf/";

  std::cout << "[DEBUG] Creating folder: " << font_folder_name << "\n";

  /*if (!CreateDirectory((LPCWSTR)(font_folder_name.c_str()), NULL)) {
    std::cout << "[ERROR] Could not create font directory! \n";
  }*/

  //_mkdir(font_folder_name.c_str());

  return font_folder_name;
}

glob::Font2D::Font2D() {}

glob::Font2D::Font2D(const std::string& path) { LoadFromFile(path); }

glob::Font2D::~Font2D() {}

bool glob::Font2D::LoadFromFile(const std::string& path) {
  std::string font_folder_name = GenerateFontDirectoryPath(&path);
  GenerateMsdfFont(path, font_folder_name);
  is_loaded_ = true;
  directory_ = font_folder_name;
  return true;
}

void glob::Font2D::Draw(ShaderProgram& shader, glm::vec2 pos, unsigned int size,
                        std::string text) {
  const char* chars = text.c_str();
  unsigned int len = text.length();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  shader.uniform("pxRange", 2.f);
  shader.uniform("bgColor", glm::vec4(0, 0, 0, 0));
  shader.uniform("fgColor", glm::vec4(0, 1, 0, 1));
  shader.uniform("msdf", 0);
  shader.uniform("screen_dims", glm::vec2(1280, 720));
  shader.uniform("t_pos", pos);
  shader.uniform("size", size);

  double offset_accum = 0;
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND); 
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
  glDepthFunc(GL_ALWAYS);
  for (int i = 0; i < len; i++) {
    char cur = chars[i];

    //double kerning = 0;
    //if (i + 1 < len) msdfgen::getKerning(kerning, font, cur, chars[i + 1]);

	
	    // shader.uniform("character", (GLint)cur);
    shader.uniform("character", cur);
    shader.uniform("t_pos", pos + glm::vec2(offset_accum, 0));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    

    //std::cout << "Char: " << (char)cur << " " << l << ", " << r << "\n";

	//double kerning = 0;
    //if (i + 1 < len) msdfgen::getKerning(kerning, font, (int)cur, (int)chars[i + 1]);

    if (cur == ' ') {
      offset_accum += size/3;
    } else {
      double l = 0, r = 0, t = 0, b = 0;
      shapes[cur].bounds(l, b, r, t);
      offset_accum += double(size) * (r / 32.0);
	}
    //std::cout << offset_accum << "\n";
  }
  //std::cout << "\n";
  glDepthFunc(GL_LESS);
  glDisable(GL_BLEND); 
  glEnable(GL_CULL_FACE);

  glBindTexture(GL_TEXTURE_2D, 0);
}