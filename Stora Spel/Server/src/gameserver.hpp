#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#define NOMINMAX
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <shared.hpp>
#include <vector>

#include "replay machine/replay_machine.hpp"
#include "util/global_settings.hpp"

#include "../src/message.hpp"
#include "util/timer.hpp"

class GameServer {
 public:
  ~GameServer();
  void Init(double in_update_rate);
  void Update(float dt);

 private:
  void UpdateSystems(float dt);

  void HandlePacketBlock(NetAPI::Common::Packet& packet, unsigned short id);

  void HandleNewTeam();

  void CreatePlayer(PlayerID id);
  void CreateEntities();
  void ResetEntities();
  void AddBallComponents(entt::entity& entity, glm::vec3 in_pos,
                         glm::vec3 in_vel);
  void AddArenaComponents(entt::entity& entity);
  void CreatePickUpComponents();
  void CreateGoals();

  // Replay stuff---
  bool StartRecording(unsigned int in_replay_length_seconds);
  void Record(std::bitset<10>& in_bitset, float& in_x_value, float& in_y_value,
              const float& in_dt);
  void Replay(std::bitset<10>& in_bitset, float& in_x_value, float& in_y_value);
  //---

  NetAPI::Socket::Server server_;
  int last_num_players_ = 0;
  entt::registry registry_;

  std::vector<PlayerID> created_players_;
  std::vector<std::pair<PlayerID, unsigned int>> new_teams_;
  std::unordered_map<int, std::pair<uint16_t, glm::vec2>> players_inputs_;
  std::vector<entt::entity> pick_ups_;
  std::vector<Message> messages;

  int test_player_guid_ = 0;

  int last_spawned_team_ = 1;

  int red_players_ = 0;
  int blue_players_ = 0;
  std::vector<unsigned int> scores;

  // Replay stuff ---
  ReplayMachine* replay_machine_ = nullptr;
  double update_rate_ = 0.0f;
  bool record_ = false;
  bool replay_ = false;
  //---
};

#endif  // GAME_SERVER_HPP_