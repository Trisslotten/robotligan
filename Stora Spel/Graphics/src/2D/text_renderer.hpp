#ifndef TEXT_RENDERER_HPP_
#define TEXT_RENDERER_HPP_

#include <msdfgen/core/render-sdf.h>
#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdfgen.h>
#include <msdfgen/core/Bitmap.h>
#include<msdfgen/core/save-bmp.h>
//#include <lodepng.hpp>
//#include <tinyxml2.hpp>

using namespace msdfgen;

class TextRenderer {
 private:
 public:
  void test() {
    FreetypeHandle *ft = initializeFreetype();
    if (ft) {
      FontHandle *font = loadFont(ft, "C:\\Windows\\Fonts\\arialbd.ttf");
      if (font) {
        Shape shape;
        if (loadGlyph(shape, font, 'A')) {
          shape.normalize();
          //                      max. angle
          edgeColoringSimple(shape, 3.0);
          //           image width, height
          Bitmap<float,3> msdf(32.f, 32.f);
          //                     range, scale, translation
          generateMSDF(msdf, shape, 4.0, 1.0, Vector2(4.0, 4.0));
          savePng(msdf, "output.png");
        }
        destroyFont(font);
      }
      deinitializeFreetype(ft);
    }
  }
};

#endif