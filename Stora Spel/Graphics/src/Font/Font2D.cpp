
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
    msdfgen::FontHandle* font = loadFont(ft, font_path.c_str());
    if (font) {
      msdfgen::Shape shape;
      unsigned int height = 32;
      unsigned int width = 32;
      unsigned int end_char = 200;
      unsigned int start_char = 32;
      unsigned int num_chars = end_char - start_char;

      unsigned int v_size = (unsigned int)4 * width * height * num_chars;
      std::vector<unsigned char> pixels;
      pixels.resize(v_size);
      //pixels.assign(v_size, 0);

      unsigned int side_dim = sqrt(num_chars + 1);

      for (int x = 0; x < side_dim - 1; x++) {
        for (int y = 0; y < side_dim - 1; y++) {
          if (loadGlyph(shape, font, start_char + x + y)) {
            shape.normalize();
            //                      max. angle
            edgeColoringSimple(shape, 3.0);
            //           image width, height
            msdfgen::Bitmap<float, 3> msdf(height, width);
            //                     range, scale, translation
            msdfgen::generateMSDF(msdf, shape, 4.0, 1.0,
                                  msdfgen::Vector2(4.0, 4.0));

            unsigned int cur_x = x * width;
            unsigned int cur_y = y * height;

            for (unsigned int yy = 0; yy < height; yy++)
              for (unsigned int xx = 0; xx < width; xx++) {
                char r = msdfgen::pixelFloatToByte(msdf(xx, yy)[0]);
                char g = msdfgen::pixelFloatToByte(msdf(xx, yy)[1]);
                char b = msdfgen::pixelFloatToByte(msdf(xx, yy)[2]);

                unsigned int pos = cur_x * height + cur_y * width + yy + xx + 0;
                pixels[pos] = r;
                pixels[pos+1] = g;
                pixels[pos+2] = b;
                pixels[pos + 3] = 255;
              }

            // msdfgen::savePng(msdf, output_name.c_str());
          }
        }
      }
      pixels[0] = 1;
      if (lodepng::encode("Assets/fonts/test_1.png", pixels, width * side_dim,
                          height * side_dim, LCT_RGB)) {
        std::cout << "[ERROR] Could not generate sprite-sheet for font\n";
      }

      /*for (int i = start_char; i < end_char; i++) {
        std::string output_name = output_path + std::to_string(i) + ".png";
        if (!fileAlreadyExists(output_name)) {
          if (loadGlyph(shape, font, i)) {
            shape.normalize();
            //                      max. angle
            edgeColoringSimple(shape, 3.0);
            //           image width, height
            msdfgen::Bitmap<float, 3> msdf(32, 32);
            //                     range, scale, translation
            msdfgen::generateMSDF(msdf, shape, 4.0, 1.0,
                                  msdfgen::Vector2(4.0, 4.0));

            msdfgen::savePng(msdf, output_name.c_str());
          }
        }
      }*/
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

void glob::Font2D::Draw(ShaderProgram& shader) {}
