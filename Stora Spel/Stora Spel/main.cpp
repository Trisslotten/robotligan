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

  glob::Font2DHandle font_test =
      glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  float time = 0.f;
  while (!glob::window::ShouldClose()) {
    // tick
    time += 1.f / 60.f;
    // render

   
	glob::Submit(font_test, glm::vec2(100, 200), 64, "SKRIVER. !COOL!");

    glob::Render();
    glob::window::Update();
  }

  glob::window::Cleanup();

  return EXIT_SUCCESS;
}