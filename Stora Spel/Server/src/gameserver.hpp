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
#include "util/timer.hpp"

class GameServer {
 public:
  ~GameServer();
  void Init(double in_update_rate);
  void Update(float dt);
  void AddScore(unsigned int team);

 private:
  void UpdateSystems(float dt);

  void HandlePacketBlock(NetAPI::Common::Packet& packet, unsigned short id);

  void CreatePlayer(PlayerID id);
  void CreateEntities(glm::vec3* in_pos_arr, unsigned int in_num_pos);
  void ResetEntities(glm::vec3* in_pos_arr, unsigned int in_num_pos);
  void AddBallComponents(entt::entity& entity, glm::vec3 in_pos,
                         glm::vec3 in_vel);
  void AddArenaComponents(entt::entity& entity);
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
  std::unordered_map<int, std::pair<uint16_t, glm::vec2>> players_inputs_;

  int test_player_guid_ = 0;

  std::vector<unsigned int> scores;

  // Replay stuff ---
  ReplayMachine* replay_machine_ = nullptr;
  double update_rate_ = 0.0f;
  bool record_ = false;
  bool replay_ = false;
  //---
};

#endif  // GAME_SERVER_HPP_