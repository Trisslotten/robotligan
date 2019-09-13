#ifndef GLOB_WINDOW_HPP_
#define GLOB_WINDOW_HPP_

#ifdef MAKEDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

//TODO: move window outside glob?

namespace glob {
namespace window {

EXPORT void Create();
EXPORT bool ShouldClose();
EXPORT void Update();
EXPORT void Cleanup();

}  // namespace window
}  // namespace glob

#endif  // GLOB_WINDOW_HPP_