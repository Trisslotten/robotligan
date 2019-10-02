#include "state.hpp"
#include <camera_component.hpp>
#include <transform_component.hpp>
#include "ecs/components/player_component.hpp"
#include "gameserver.hpp"

void LobbyState::Init(GameServer& game_server) {
  //
}

void LobbyState::Update(GameServer& game_server) {
  bool can_start = clients_ready_.size() >= 2;
  for (auto ready : clients_ready_) {
    can_start = can_start && ready.second;
  }

  if (can_start && !starting) {
    std::cout << "DEBUG: Start game countdown\n";
    start_game_timer.Restart();
    starting = true;
  }
  if (!can_start) {
    starting = false;
  }

  if (starting && start_game_timer.Elapsed() > 10.f) {
    std::cout << "DEBUG: Start game countdown is zero\n";
    game_server.ChangeState(StateType::PLAY);
  }
}

void LobbyState::Cleanup(GameServer& game_server) {
  //
}

void PlayState::Init(GameServer& game_server) {
  auto& server = game_server.getServer();
  auto& registry = game_server.getRegistry();

  for (auto& [id, client_data] : server.GetClients()) {
    NetAPI::Common::Packet to_send;
    auto header = to_send.GetHeader();
    header->receiver = id;

    to_send << PacketBlockType::GAME_START;

    server.Send(to_send);
  }
}

void PlayState::Update(GameServer& game_server) {
  auto& server = game_server.getServer();
  auto& registry = game_server.getRegistry();

  registry.view<PlayerComponent>().each(
      [&](auto entity, PlayerComponent& player_c) {
        auto inputs = players_inputs_[player_c.id];
        player_c.actions = inputs.first;
        player_c.pitch += inputs.second.x;
        player_c.yaw += inputs.second.y;
        /*
        std::cout << "Pitch: " << player_c.pitch << "\n";
        std::cout << "Yaw:   " << player_c.yaw << "\n\n";
                */
      });
  players_inputs_.clear();

  /*
  for (auto& [id, client_data] : server.GetClients()) {
    NetAPI::Common::Packet to_send;
    auto header = to_send.GetHeader();
    header->receiver = id;

    auto view_cam = registry.view<CameraComponent, PlayerComponent>();
    for (auto cam : view_cam) {
      auto& cam_c = view_cam.get<CameraComponent>(cam);
      auto& player_c = view_cam.get<PlayerComponent>(cam);
      if (id == player_c.id) {
        to_send << cam_c.orientation;
        break;
      }
    }
    to_send << PacketBlockType::CAMERA_TRANSFORM;

    auto view_ball = registry.view<BallComponent, TransformComponent>();
    for (auto ball : view_ball) {
      // auto& ball_c = view_cam.get<BallComponent>(ball);
      auto& trans_c = view_ball.get<TransformComponent>(ball);

      to_send << trans_c.rotation;
      to_send << trans_c.position;
      break;
    }
    to_send << PacketBlockType::BALL_TRANSFORM;

    auto view_players = registry.view<TransformComponent, PlayerComponent>();
    int num_players = view_players.size();
    for (auto player : view_players) {
      auto& trans_c = view_players.get<TransformComponent>(player);
      auto& player_c = view_players.get<PlayerComponent>(player);
      to_send << trans_c.rotation;
      to_send << trans_c.position;
      to_send << player_c.id;
    }
    to_send << num_players;
    to_send << PacketBlockType::PLAYERS_TRANSFORMS;

    server.Send(to_send);
  }
  */
}

void PlayState::Cleanup(GameServer& game_server) {
  //
}