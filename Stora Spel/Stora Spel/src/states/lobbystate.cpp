#include "state.hpp"

#include <glob/window.hpp>
#include "..//ecs/components.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"

struct ReadyButtonComponent {};

void LobbyState::ReadyButtonFunc() {
  me_ready_ = !me_ready_;
  auto view_ready_button =
      registry_lobby_
          .view<ButtonComponent, TransformComponent, ReadyButtonComponent>();
  if (me_ready_) {
    auto& packet = engine_->GetPacket();
    packet << PacketBlockType::CLIENT_READY;
    for (auto button : view_ready_button) {
      ButtonComponent& b_c = registry_lobby_.get<ButtonComponent>(button);
      b_c.text_normal_color = glm::vec4(0.f, 1.f, 0.f, 1.f);
      b_c.text_hover_color = glm::vec4(0.f, .6f, 0.f, 1.f);
      break;
    }
    // b_c.text_current_color = glm::vec4(0.f, 1.f, 0.f, 1.f);
    // b_c.text_hover_color = glm::vec4(0.f, 1.f, 0.f, 1.f);
  } else {
    auto& packet = engine_->GetPacket();
    packet << PacketBlockType::CLIENT_NOT_READY;

    for (auto button : view_ready_button) {
      ButtonComponent& b_c = registry_lobby_.get<ButtonComponent>(button);
      b_c.text_normal_color = glm::vec4(1.f, 0.f, 0.f, 1.f);
      b_c.text_hover_color = glm::vec4(.6f, 0.f, 0.f, 1.f);
      break;
    }
    // b_c.text_current_color = glm::vec4(1.f, 0.f, 0.f, 1.f);
    // b_c.text_normal_color = glm::vec4(1.f, 0.f, 0.f, 1.f);
    // b_c.text_hover_color = glm::vec4(1.f, 0.f, 0.f, 1.f);
  }
}

void LobbyState::SendJoinTeam(unsigned int team) {
  NetAPI::Common::Packet& packet = engine_->GetPacket();
  packet << team;
  packet << PacketBlockType::LOBBY_SELECT_TEAM;
}

void LobbyState::Startup() {
  auto font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  auto button = registry_lobby_.create();
  ButtonComponent& b_c = registry_lobby_.assign<ButtonComponent>(button);
  /*ready_button_c = GenerateButtonEntity(
      registry_lobby_, "READY", glob::window::Relative720(glm::vec2(1120, 40)),
      font_test_);*/
  b_c.text = "READY";
  b_c.font_size = 72;  // menu_settings::font_size;
  b_c.bounds = glm::vec2(b_c.font_size * b_c.text.size() / 2, b_c.font_size);
  b_c.f_handle = font_test_;
  glm::vec2 pos = glob::window::Relative720(glm::vec2(1120, 40));
  registry_lobby_.assign<TransformComponent>(button,
                                             glm::vec3(pos.x, pos.y, 0));
  b_c.text_current_color = glm::vec4(1.f, 0.f, 0.f, 1.f);
  b_c.text_normal_color = glm::vec4(1.f, 0.f, 0.f, 1.f);
  b_c.text_hover_color = glm::vec4(0.6f, 0.f, 0.f, 1.f);
  b_c.visible = true;
  registry_lobby_.assign<ReadyButtonComponent>(button);

  b_c.button_func = [&]() { this->ReadyButtonFunc(); };
}

void LobbyState::Init() {
  //
  auto& cli = engine_->GetClient();
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_lobby_);

  engine_->SetEnableChat(true);

  CreateBackgroundEntities();
  CreateGUIElements();
}

void LobbyState::Update(float dt) {
  //
  DrawTeamSelect();
}

void LobbyState::UpdateNetwork() {}

void LobbyState::Cleanup() {
  //
  registry_lobby_.reset();
}

void LobbyState::HandleUpdateLobbyTeamPacket(NetAPI::Common::Packet& packet) {
  int id = 0;
  unsigned int team = 0;
  bool ready = false;
  packet >> ready;
  packet >> team;
  packet >> id;

  LobbyPlayer plyr;
  plyr.ready = ready;
  plyr.team = team;
  lobby_players_[id] = plyr;
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
  registry_lobby_.assign<TransformComponent>(ball, glm::vec3(0, -4, 0),
                                             zero_vec, glm::vec3(1.0f));
  registry_lobby_.assign<BallComponent>(ball);

  // ladda in och skapa entity för robotar
  auto robot = registry_lobby_.create();
  auto& trans = registry_lobby_.assign<TransformComponent>(
      robot, zero_vec, zero_vec, glm::vec3(0.15f));
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
  cam_c.orientation = glm::quat(glm::vec3(0.f, -0.3f, 0.f));
}

void LobbyState::CreateGUIElements() {
  team_select_back_ =
      glob::GetGUIItem("Assets/GUI_elements/lobby_team_no_names.png");
  font_team_names_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  auto button_join_red = registry_lobby_.create();
  ButtonComponent* button_c = GenerateButtonEntity(
      registry_lobby_, "JOIN", glm::vec2(960, 700), font_team_names_, true, 22);
  button_c->text_current_color = glm::vec4(1, 1, 1, 1);
  button_c->text_normal_color = glm::vec4(1, 1, 1, 1);
  button_c->text_hover_color = glm::vec4(1.f, 0.3f, 0.3f, 1.f);

  button_c->button_func = [&] { SendJoinTeam(TEAM_RED); };

  auto button_join_blue = registry_lobby_.create();
  button_c = GenerateButtonEntity(registry_lobby_, "JOIN", glm::vec2(960, 490),
                                  font_team_names_, true, 22);
  button_c->text_current_color = glm::vec4(1, 1, 1, 1);
  button_c->text_normal_color = glm::vec4(1, 1, 1, 1);
  button_c->text_hover_color = glm::vec4(.3f, .3f, 1.f, 1.f);

  button_c->button_func = [&] { SendJoinTeam(TEAM_BLUE); };
}

void LobbyState::DrawTeamSelect() {
  glm::vec2 team_select_box_pos =
      glob::window::Relative720(glm::vec2(900, 270));
  glob::Submit(team_select_back_, team_select_box_pos, 1.f);

  glm::vec2 team_blue_start = glob::window::Relative720(glm::vec2(930, 460));
  glm::vec2 team_red_start = glob::window::Relative720(glm::vec2(930, 680));
  int blue_count = 0;
  int red_count = 0;

  for (auto& lobby_player : lobby_players_) {
    glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 1.f);
    if (lobby_player.second.ready) color = glm::vec4(.3f, 1.f, .3f, 1.f);
    if (lobby_player.first == my_id_) color = glm::vec4(1.f, 1.f, .3f, 1.f);
      if (lobby_player.second.team == TEAM_BLUE) {
        glob::Submit(font_team_names_,
                     team_blue_start - glm::vec2(0, 20 * blue_count), 32,
                     engine_->player_names_[lobby_player.first], color);
        blue_count++;
      } else {
        glob::Submit(font_team_names_,
                     team_red_start - glm::vec2(0, 20 * red_count), 32,
                     engine_->player_names_[lobby_player.first], color);
        red_count++;
      }
  }
}
