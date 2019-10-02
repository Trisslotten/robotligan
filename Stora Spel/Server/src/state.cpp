#include "state.hpp"
#include <camera_component.hpp>
#include <transform_component.hpp>
#include "ecs/components/player_component.hpp"
#include "gameserver.hpp"

void LobbyState::Init(GameServer& game_server) {
  // a
  start_game_timer.Restart();
}

void LobbyState::Update(GameServer& game_server) {
  if (start_game_timer.Elapsed() > 5.f) {
    game_server.ChangeState(StateType::PLAY);
  }
}

void LobbyState::Cleanup(GameServer& game_server) {}

void PlayState::Init(GameServer& game_server) {
  auto& server = game_server.getServer();
  auto& registry = game_server.getRegistry();

  for (auto& [id, client_data] : server.GetClients()) {
    NetAPI::Common::Packet to_send;
    auto header = to_send.GetHeader();
    header->receiver = id;

    std::string message = "Game Started!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

    to_send.Add(message.data(), message.size());
    to_send << message.size();
	to_send << PacketBlockType::TEST_STRING;

    server.Send(to_send);
  }
}

void PlayState::Update(GameServer& game_server) {
  auto& server = game_server.getServer();
  auto& registry = game_server.getRegistry();

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

void PlayState::Cleanup(GameServer& game_server) {}
