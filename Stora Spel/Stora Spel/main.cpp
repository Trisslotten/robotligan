#include <NetAPI/networkTest.hpp>
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


  double net_numframes = 0;
  double render_numframes = 0;
  Timer debugTimer;
  double net_tickrate = 20.0;
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

  glm::vec3 testpos{};

  timer.Restart();
  float dt = 0.0f;
  while (!glob::window::ShouldClose()) {
    dt = timer.Restart();

    Input::Reset();

    net_loop_accum += dt;
    while (net_loop_accum > net_ticktime) {
      net_loop_accum -= net_ticktime;

      auto buffer = tcp_client.Recive();
      size_t len = tcp_client.GetLastRecvLen();

      if (strcmp(buffer, NetAPI::Common::kNoDataAvailable) == 0 ||
          strcmp(buffer, NetAPI::Common::kFailedToRecieve) == 0) {
        // inte data

      } else {
        //std::cout << "packet length: " << len << "\n";
        //std::cout << "tcp error: " << tcp_client.QuerryError() << "\n";
        memcpy(&testpos, buffer, sizeof(testpos));
        //std::cout << "\t pos: " << testpos.x << " " << testpos.y << " " << testpos.z << "\n";
      }
      byte id = tcp_client.GetID();

      glm::vec3 vel{};
      if (Input::IsKeyDown(GLFW_KEY_UP)) vel += glm::vec3(1, 0, 0);
      if (Input::IsKeyDown(GLFW_KEY_DOWN)) vel += glm::vec3(-1, 0, 0);
      if (Input::IsKeyDown(GLFW_KEY_LEFT)) vel += glm::vec3(0, 0, -1);
      if (Input::IsKeyDown(GLFW_KEY_RIGHT)) vel += glm::vec3(0, 0, 1);
	  if(length(vel) > 0.001f)
		vel = normalize(vel);
      tcp_client.Send((char *)&vel, sizeof(vel));

	  net_numframes++;
    }
	render_numframes++;
	if (debugTimer.Elapsed() > 1.0) {
		double elapsed = debugTimer.Restart();
		//std::cout << "Net rate:    " << net_numframes / elapsed << "\n";
		//std::cout << "Render rate: " << render_numframes / elapsed << "\n\n";
		net_numframes = 0;
		render_numframes = 0;
	}
	

    updateSystems(&registry, dt);

    glob::Submit(ball_h, testpos);

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

  std::cin.ignore();
  return EXIT_SUCCESS;
}