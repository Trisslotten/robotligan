#ifndef GLOB_GRAPHICS_H_
#define GLOB_GRAPHICS_H_

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

namespace glob {

EXPORT void Init();

EXPORT void Render();

}  // namespace glob

#endif  // GLOB_GRAPHICS_H_