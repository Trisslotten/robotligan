#include <iostream>

#include <entt.hpp>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <glob/window.hpp>

#include "PrintPositionSystem.h"

//#include <glad/glad.h>

int main(unsigned argc, char **argv) {
  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";

  entt::registry registry;

  auto entity = registry.create();
  registry.assign<Position>(entity, 1.0f, 2.0f, 3.0f);
  registry.assign<Velocity>(entity, 3.0f, 2.0f, 1.0f);

  print(registry);

  glob::window::Create();
  glob::Init();

  glob::ModelHandle model_h =
      glob::GetModel("assets/Mech/Mech_humanoid_posed_unified_AO.fbx");
  glob::ModelHandle model_h2 = glob::GetModel("assets/Mech/Ball.obj");

  float time = 0.f;
  while (!glob::window::ShouldClose()) {
    // tick
    time += 1.f / 60.f;
    // render

    glob::Submit(model_h, glm::translate(glm::vec3(0)) *
                              glm::rotate(time * 0.5f, glm::vec3(0, 1, 0)));
    glob::Submit(model_h2,
                 glm::translate(glm::vec3(0, 9, 3.f * glm::sin(time))));

    glob::Render();
    glob::window::Update();
  }

  glob::window::Cleanup();

  return EXIT_SUCCESS;
}