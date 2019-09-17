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
  }

  for (short i = 0; i < server_.GetConnectedPlayers(); i++) {
    auto data = server_[i];
    if (server_.HasData(data)) {
      int actionflag = 0;
      memcpy(&actionflag, data.buffer, sizeof(int));

      glm::vec3 vel;
      if ((actionflag & 1) == 1) vel += glm::vec3(1, 0, 0);
      if ((actionflag & 2) == 2) vel += glm::vec3(-1, 0, 0);
      if ((actionflag & 4) == 4) vel += glm::vec3(0, 0, -1);
      if ((actionflag & 8) == 8) vel += glm::vec3(0, 0, 1);
      if (length(vel) > 0.0001f) vel = normalize(vel);

      positions_[i] += 5.f * vel * dt;
    }
  }

  NetAPI::Common::Packet packet;
  packet << positions_.size();
  packet.Add(positions_.data(), positions_.size());

  NetAPI::Socket::Data data;
  data.ID = 0;
  data.buffer = packet.GetRaw();
  data.len = packet.GetPacketSize();

  server_.Send(data);
}
