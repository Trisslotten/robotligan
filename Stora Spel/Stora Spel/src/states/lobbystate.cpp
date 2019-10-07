#include "state.hpp"

#include "..//ecs/components.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"

void LobbyState::Startup() {
  auto font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  ButtonComponent* button_c = GenerateButtonEntity(
      registry_lobby_, "READY", glm::vec2(100, 200), font_test_);
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
      light_test, glm::vec3(0.1f, 0.1f, 1.0f), 15.f, 0.4f);
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
  // ladda in och skapa entity för böll
  auto ball = registry_lobby_.create();
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_lobby_.assign<ModelComponent>(ball, model_ball);
  registry_lobby_.assign<TransformComponent>(ball, zero_vec, zero_vec,
                                                glm::vec3(1.0f));
  registry_lobby_.assign<BallComponent>(ball);

  // ladda in och skapa entity för robotar

  // lägga ut en kamera i scenen
  auto camera = registry_lobby_.create();
  auto& cam_c = registry_lobby_.assign<CameraComponent>(camera);
  auto& trans = registry_lobby_.assign<TransformComponent>(camera);
  trans.position = glm::vec3(-10, 0, 0);
  glm::vec3 dir = glm::vec3(0) - trans.position;
  cam_c.orientation = glm::quat(glm::vec3(0,-0.67f,0));
}
