#include "gameserver.hpp"

#include <iostream>
#include <transform_component.hpp>
#include <velocity_component.hpp>

namespace {}  // namespace

GameServer::~GameServer() { server_.Cleanup(); }

void GameServer::Init() { server_.Setup(1337); }

void GameServer::Update(float dt) {
  server_.Update();

  int diff = server_.GetConnectedPlayers() - positions_.size();
  for (int i = 0; i < diff; i++) {
    positions_.push_back(glm::vec3(0));
    vs_.push_back(glm::vec3(0));
  }

  for (short i = 0; i < server_.GetConnectedPlayers(); i++) {
    auto packet = server_[i];

    glm::vec3 target_vel{0};

    if (!packet.IsEmpty()) {
      int actionflag = 0;
      packet >> actionflag;

      glm::vec3 vel{0};
      if ((actionflag & 1) == 1) vel += glm::vec3(1, 0, 0);
      if ((actionflag & 2) == 2) vel += glm::vec3(-1, 0, 0);
      if ((actionflag & 4) == 4) vel += glm::vec3(0, 0, -1);
      if ((actionflag & 8) == 8) vel += glm::vec3(0, 0, 1);
      if (length(vel) > 0.0001f) vel = normalize(vel);

      target_vel = 10.f * vel;
      // std::cout << "actionflag=" << actionflag << "\n";
    }
    float t = 0.02f;
    vs_[i] = glm::mix(vs_[i], target_vel, 1.f - glm::pow(t, dt));
    positions_[i] += vs_[i] * dt;
  }
  for (auto& p : positions_) {
    std::cout << p.x << "\n";
  }
  std::cout << "\n";

  if (positions_.size() > 0) {
    NetAPI::Common::Packet packet;
    packet.Add(positions_.data(), positions_.size());
    packet << (int)positions_.size();

    server_.Send(packet);
  }
}
