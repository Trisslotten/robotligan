#ifndef PRINTPOSITIONSYSTEM_H
#define PRINTPOSITIONSYSTEM_H

#include "position.h"
#include "velocity.h"

void print(entt::registry &registry) {
  registry.view<Position, Velocity>().each([](auto &pos, auto &vel) {
    std::cout << "Pos X: " << pos.x << " Pos Y: " << pos.y << " Pos Z: " << pos.z << "\n";
  });
}

#endif  // PRINTPOSITIONSYSTEM_H
