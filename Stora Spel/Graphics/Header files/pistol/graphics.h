#ifndef PISTOL_GRAPHICS_H_
#define PISTOL_GRAPHICS_H_

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

EXPORT int GraphicsTestPistol();


#endif // PISTOL_GRAPHICS_H_