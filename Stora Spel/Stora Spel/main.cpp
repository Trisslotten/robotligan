#include <NetAPI/networkTest.hpp>
#include <NetAPI/packet.hpp>
#include <NetAPI/socket/server.hpp>
#include <NetAPI/socket/tcpclient.hpp>
// do not touch
#undef min
#undef max
#include <entity/registry.hpp>
#include <entt.hpp>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <iostream>

#include <GLFW/glfw3.h>
#include "ability_controller_system.hpp"
#include "ball_component.hpp"
#include "collision.hpp"
#include "collision_system.hpp"
#include "physics_system.hpp"
#include "player_controller_system.hpp"
#include "print_position_system.hpp"
#include "util/input.hpp"
#include "util/meminfo.hpp"
#include "util/timer.hpp"

#include <chrono>
#include <thread>

void init() {
  glob::window::Create();
  glob::Init();
  Input::Initialize();
}

void updateSystems(entt::registry *reg, float dt) {
  player_controller::Update(*reg, dt);
  ability_controller::Update(*reg);
  UpdatePhysics(*reg, dt);
  UpdateCollisions(*reg);
}

int main(unsigned argc, char **argv) {
  Timer timer;

  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";
  entt::registry registry;

  auto entity = registry.create();
  registry.assign<BallComponent>(entity, true, false);
  registry.assign<VelocityComponent>(entity, glm::vec3(1.0f, 0.0f, 0.0f));
  registry.assign<physics::Sphere>(entity, glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);

  entity = registry.create();
  registry.assign<physics::Arena>(entity, -10.f, 10.f, 0.f, 5.f, -10.f, 10.f);
  registry.assign<VelocityComponent>(entity, glm::vec3(.0f, .0f, .0f));

  glob::window::Create();
  glob::Init();
  init();  // Initialize everything

  glob::ModelHandle ball_h = glob::GetModel("assets/Ball/Ball.fbx");

  auto avatar = registry.create();  // this is the player avatar
  registry.assign<CameraComponent>(
      avatar, (Camera *)glob::GetCamera(),
      glm::vec3(0, 1, 0));  // get the camera pointer from glob renderer
  registry.assign<PlayerComponent>(avatar);
  registry.assign<TransformComponent>(avatar, glm::vec3(-9.f, 0.f, 0.f),
                                      glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
  registry.assign<VelocityComponent>(avatar, glm::vec3(.0f, .0f, .0f));
  registry.assign<physics::OBB>(
      avatar, glm::vec3(5.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.f, 0.f),
      glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), 1.f, 1.f, 1.f);

  std::vector<glm::vec3> positions;
  double net_numframes = 0;
  double render_numframes = 0;
  Timer debugTimer;
  double net_tickrate = 5;
  double net_ticktime = 1.0 / net_tickrate;
  double net_loop_accum = 0.0;
  NetAPI::Socket::TcpClient tcp_client;
  tcp_client.Connect("127.0.0.1", 1337);
  if (tcp_client.IsConnected()) {
    std::cout << "DEBUG: Client Connected!\n";
  } else {
    std::cout << "ERROR: Client could not connect!\n";
    return EXIT_FAILURE;
  }

  timer.Restart();
  float dt = 0.0f;
  while (!glob::window::ShouldClose()) {
    dt = timer.Restart();

    Input::Reset();

    net_loop_accum += dt;
    while (net_loop_accum > net_ticktime) {
      net_loop_accum -= net_ticktime;
      auto packet = tcp_client.Recieve();
      if (!packet.IsEmpty()) {
        size_t arrsize = 0;
        packet >> arrsize;
        positions.resize(arrsize);
        packet.Remove(positions.data(), arrsize);
      }

      int actionflag = 0;
      if (Input::IsKeyDown(GLFW_KEY_UP)) actionflag |= 1;
      if (Input::IsKeyDown(GLFW_KEY_DOWN)) actionflag |= 2;
      if (Input::IsKeyDown(GLFW_KEY_LEFT)) actionflag |= 4;
      if (Input::IsKeyDown(GLFW_KEY_RIGHT)) actionflag |= 8;

      tcp_client.Send((char *)&actionflag, sizeof(int));

      net_numframes++;
    }
    render_numframes++;
    if (debugTimer.Elapsed() > 1.0) {
      double elapsed = debugTimer.Restart();
      // std::cout << "Net rate:    " << net_numframes / elapsed << "\n";
      // std::cout << "Render rate: " << render_numframes / elapsed << "\n\n";
      for (auto &p : positions) {
        std::cout << p.x << "\n";
      }
      net_numframes = 0;
      render_numframes = 0;
    }

    updateSystems(&registry, dt);

    for (const auto &p : positions) {
      glob::Submit(ball_h, p);
    }

    glob::Render();
    glob::window::Update();
  }

  glob::window::Cleanup();
  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM()
            << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM()
            << " MB\n";

  std::cout << "WSA is initialized? " << std::boolalpha
            << NetAPI::Initialization::WinsockInitialized() << std::endl;

  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM()
            << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM()
            << " MB\n";
  std::cin.ignore();
  return EXIT_SUCCESS;
}