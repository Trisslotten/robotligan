#include "state.hpp"
#include <camera_component.hpp>
#include <transform_component.hpp>

void LobbyState::Init(entt::registry& registry, NetAPI::Socket::Server& server)
{

}

void LobbyState::Update(entt::registry& registry, NetAPI::Socket::Server& server)
{

}

void PlayState::Init(entt::registry& registry, NetAPI::Socket::Server& server)
{
}

void PlayState::Update(entt::registry& registry, NetAPI::Socket::Server& server)
{
	for (auto& [id, client_data] : server.GetClients()) {
		NetAPI::Common::Packet to_send;
		auto header = to_send.GetHeader();
		header->receiver = id;

		/*
		glm::vec3 ball_pos{0};
		registry_.view<TransformComponent, BallComponent>().each(
			[&](auto entity, auto& trans_c, auto& ball) {
			  ball_pos = trans_c.position;
			});
		to_send << ball_pos;
		to_send << PacketBlockType::TEST_BALL_P;
		*/
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
			//auto& ball_c = view_cam.get<BallComponent>(ball);
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
}
