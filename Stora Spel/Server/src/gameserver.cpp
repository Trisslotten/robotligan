#include "gameserver.hpp"

#include <iostream>
#include <transform_component.hpp>
#include <velocity_component.hpp>

namespace {

auto CreateTestEntity(entt::registry& reg) {
  auto entity = reg.create();
  reg.assign<TransformComponent>(entity, glm::vec3(-9.f, 0.f, 0.f),
                                 glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
  reg.assign<VelocityComponent>(entity, glm::vec3(.0f, .0f, .0f));
}

}  // namespace

GameServer::~GameServer() { server_.Cleanup(); }

void GameServer::Init() {
  // asd
  server_.Setup(1337);
}

void GameServer::Update(float dt) {
  // asd
  server_.Update();
  std::cout << server_.GetConnectedPlayers() << "\n";

  glm::vec3 vel{0};
  for (short i = 0; i < server_.GetConnectedPlayers(); i++) {
    auto data = server_[i];
    if (server_.HasData(data)) {
      std::cout << "DEBUG: Packet has data!\n";
      memcpy(&vel, data.buffer, sizeof(testpos_));
      std::cout << "\t vel: " << vel.x << " " << vel.y << " " << vel.z << "\n";
    }
  }

  testpos_ += vel * dt;
  // asd_ = 2*glm::sin(global_timer.Elapsed());

  std::cout << "\t pos: " << testpos_.x << " " << testpos_.y << " "
            << testpos_.z << "\n";

  NetAPI::Socket::Data data;
  data.ID = 0;
  data.buffer = (char*)(&testpos_);
  data.len = sizeof(testpos_);
  server_.Send(data);
}
