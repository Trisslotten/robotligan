#ifndef GLOB_HANDLE_TYPES_HPP_
#define GLOB_HANDLE_TYPES_HPP_

#include <functional>

namespace glob {

// https://softwareengineering.stackexchange.com/questions/243154/c-strongly-typed-typedef
template <typename T, typename Meaning>
struct Explicit {
  Explicit() {}
  Explicit(T value) : value(value) {}
  inline operator T() const { return value; }
  Explicit& operator++() {
    value++;
    return *this;
  }
  Explicit operator++(int) {
    Explicit<T, Meaning> result;
    ++(*this);
    return result;
  }
  T value;
};

typedef unsigned long HandleType;
typedef Explicit<HandleType, struct ModelHandleTag> ModelHandle;
typedef Explicit<HandleType, struct ParticleSystemHandleTag> ParticleSystemHandle;
typedef Explicit<HandleType, struct Font2DHandleTag> Font2DHandle;
typedef Explicit<HandleType, struct GUIHandleTag> GUIHandle;
typedef Explicit<HandleType, struct E2DModelHandleTag> E2DHandle;

}  // namespace glob

namespace std {
template <typename T, typename Meaning>
struct hash<glob::Explicit<T, Meaning>> {
  size_t operator()(const glob::Explicit<T, Meaning>& x) const {
    return x.value;
  }
};
}  // namespace std

#endif  // GLOB_HANDLE_TYPES_HPP_