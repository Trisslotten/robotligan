#ifndef PRINTPOSITIONSYSTEM_HPP
#define PRINTPOSITIONSYSTEM_HPP

#include "position.hpp"
#include "velocity.hpp"

void print(entt::registry &registry) {
  registry.view<Position, Velocity>().each([](auto &pos, auto &vel) {
    std::cout << "Pos X: " << pos.x << " Pos Y: " << pos.y << " Pos Z: " << pos.z << "\n";
  });
}

#endif  // PRINTPOSITIONSYSTEM_HPP
