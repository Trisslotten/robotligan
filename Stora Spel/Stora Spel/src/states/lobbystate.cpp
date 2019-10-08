#include "state.hpp"

#include "..//ecs/components.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"
#include <glob/window.hpp>

void LobbyState::Startup() {
  auto font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  ButtonComponent* button_c = GenerateButtonEntity(
      registry_lobby_, "READY", glob::window::Relative720(glm::vec2(1120,40)), font_test_);
  button_c->button_func = [&]() {
    auto& packet = engine_->GetPacket();
    packet << PacketBlockType::CLIENT_READY;
  };
}

void LobbyState::Init() {
  //
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_lobby_);

  engine_->GetClient().Connect("localhost", 1337);

  engine_->SetEnableChat(true);

  CreateBackgroundEntities();
}

void LobbyState::Update() {
  //
}

void LobbyState::UpdateNetwork() {}

void LobbyState::Cleanup() {
  //
  registry_lobby_.reset();
}

void LobbyState::CreateBackgroundEntities() {
  // add the lights to scene
  auto light_test = registry_lobby_.create();
  registry_lobby_.assign<LightComponent>(
      light_test, glm::vec3(0.1f, 0.1f, 1.0f), 30.f, 0.2f);
  registry_lobby_.assign<TransformComponent>(
      light_test, glm::vec3(12.f, -4.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  // ladda in och skapa entity för bana
  auto arena = registry_lobby_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  glob::ModelHandle model_arena =
      glob::GetModel("assets/Map_rectangular/map_rextangular.fbx");
  registry_lobby_.assign<ModelComponent>(arena, model_arena);
  registry_lobby_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                             arena_scale);
  // ladda in och skapa entity för boll
  auto ball = registry_lobby_.create();
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_lobby_.assign<ModelComponent>(ball, model_ball);
  registry_lobby_.assign<TransformComponent>(ball, glm::vec3(
	  0,-4,0), zero_vec,
                                                glm::vec3(1.0f));
  registry_lobby_.assign<BallComponent>(ball);

  // ladda in och skapa entity för robotar
  auto robot = registry_lobby_.create();
  auto& trans = registry_lobby_.assign<TransformComponent>(robot, zero_vec, zero_vec,
                                             glm::vec3(0.15f));
  glob::ModelHandle model_robot =
      glob::GetModel("assets/Mech/Mech_humanoid_posed_unified_AO.fbx");
  registry_lobby_.assign<ModelComponent>(robot, model_robot);
  trans.position = glm::vec3(10.f, -4.f, 0.f);

  // lägga ut en kamera i scenen
  auto camera = registry_lobby_.create();
  auto& cam_c = registry_lobby_.assign<CameraComponent>(camera);
  auto& cam_trans = registry_lobby_.assign<TransformComponent>(camera);
  cam_trans.position = glm::vec3(-12.f, 0.f, -3.f);
  glm::vec3 dir = glm::vec3(0) - trans.position;
  cam_c.orientation = glm::quat(glm::vec3(0.f,-0.3f,0.f));
}
