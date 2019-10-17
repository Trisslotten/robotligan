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
  if (me_ready_) {
    ReadyButtonFunc();
  }
}

entt::entity LobbyState::GetAbilityButton(std::string find_string) {
  auto view_buttons = registry_lobby_.view<ButtonComponent>();
  for (auto button : view_buttons) {
    ButtonComponent& b_c = registry_lobby_.get<ButtonComponent>(button);
    if (b_c.find_name == find_string) {
      return button;
    }
  }
  return (entt::entity)NULL;
}
void LobbyState::SelectAbilityHandler(int id) {
  my_selected_ability_ = id;
  auto view_buttons = registry_lobby_.view<ButtonComponent>();
  for (auto button : view_buttons) {
    ButtonComponent& b_c = registry_lobby_.get<ButtonComponent>(button);
    b_c.gui_handle_normal = ability_back_normal_;
  }
  entt::entity selected_button =
      GetAbilityButton("ability_" + std::to_string((int)id));

  if ((int)selected_button != NULL) {
    ButtonComponent& b_c =
        registry_lobby_.get<ButtonComponent>(selected_button);
    b_c.gui_handle_normal = ability_back_selected_;
    b_c.gui_handle_current = ability_back_selected_;
  }

  //send selection to server
  auto& packet = engine_->GetPacket();
  packet << id;
  packet << PacketBlockType::LOBBY_SELECT_ABILITY;
}
bool LobbyState::IsAbilityBlackListed(int id) {
  for (auto b_ab : ability_blacklist) {
    if (b_ab == id) return true;
  }
  return false;
}
void LobbyState::Startup() {
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
}

void LobbyState::Init() {
  //
  glob::window::SetMouseLocked(false);
  auto& cli = engine_->GetClient();
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_lobby_);

  engine_->SetEnableChat(true);

  CreateBackgroundEntities();
  CreateGUIElements();
  SelectAbilityHandler(my_selected_ability_);

  engine_->GetChat()->SetPosition(glm::vec2(20, 140));

  engine_->GetAnimationSystem().Reset();
}

void LobbyState::Update() {
  //
  DrawTeamSelect();
  DrawAbilitySelect();
}

void LobbyState::UpdateNetwork() {}

void LobbyState::Cleanup() {
  me_ready_ = false;
  for (auto& l_p : lobby_players_) {
    l_p.second.ready = false;
  }
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
      glob::GetModel("assets/Map/Map_unified_TMP.fbx");
  registry_lobby_.assign<ModelComponent>(arena, model_arena);
  registry_lobby_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                             arena_scale);
  // ladda in och skapa entity för boll
  auto ball = registry_lobby_.create();
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/TestBall.fbx");
  registry_lobby_.assign<ModelComponent>(ball, model_ball);
  registry_lobby_.assign<TransformComponent>(ball, glm::vec3(0, -4, 0),
                                             zero_vec, glm::vec3(1.0f));
  registry_lobby_.assign<BallComponent>(ball);

  // ladda in och skapa entity för robotar
  auto robot = registry_lobby_.create();
  auto& trans = registry_lobby_.assign<TransformComponent>(
      robot, zero_vec, glm::vec3(0.f, 180.f, 0.f), glm::vec3(0.01f));
  glob::ModelHandle model_robot =
      glob::GetModel("assets/Mech/Mech.fbx");
  registry_lobby_.assign<ModelComponent>(robot, model_robot);
  //registry_lobby_.assign<AnimationComponent>(robot, glob::GetAnimationData(model_robot));
  trans.position = glm::vec3(10.f, -4.f, 0.f);

  // lägga ut en kamera i scenen
  auto camera = registry_lobby_.create();
  auto& cam_c = registry_lobby_.assign<CameraComponent>(camera);
  auto& cam_trans = registry_lobby_.assign<TransformComponent>(camera);
  cam_trans.position = glm::vec3(-10.f, 0.f, -3.f);
  glm::vec3 dir = glm::vec3(0) - trans.position;
  cam_c.orientation = glm::quat(glm::vec3(0.f, -0.3f, 0.f));
}

void LobbyState::CreateGUIElements() {
  //ability_blacklist.push_back((int)AbilityID::SWITCH_GOALS);
  team_select_back_ =
      glob::GetGUIItem("Assets/GUI_elements/lobby_team_no_names.png");
  font_team_names_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  ability_select_back_ =
      glob::GetGUIItem("Assets/GUI_elements/ability_select_back.png");
  ability_back_normal_ = glob::GetGUIItem(
      "Assets/GUI_elements/ability_icons/ability_back_normal.png");
  ability_back_selected_ = glob::GetGUIItem(
      "Assets/GUI_elements/ability_icons/ability_back_selected.png");
  ability_back_hover_ = glob::GetGUIItem(
      "Assets/GUI_elements/ability_icons/ability_back_hover.png");

  int num_abilites = (int)AbilityID::NUM_OF_ABILITY_IDS;
  ability_icons_.resize(num_abilites);
  for (int i = 0; i < num_abilites; i++) {
    ability_icons_[i] = glob::GetGUIItem("Assets/GUI_elements/ability_icons/" +
                                         std::to_string(i) + ".png");
  }

  //ready button
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
  registry_lobby_.assign<TransformComponent>(
      button, glm::vec3(glob::window::GetWindowDimensions().x - 200, 82, 0));
  b_c.text_current_color = glm::vec4(1.f, 0.f, 0.f, 1.f);
  b_c.text_normal_color = glm::vec4(1.f, 0.f, 0.f, 1.f);
  b_c.text_hover_color = glm::vec4(0.6f, 0.f, 0.f, 1.f);
  b_c.visible = true;
  registry_lobby_.assign<ReadyButtonComponent>(button);

  b_c.button_func = [&]() { this->ReadyButtonFunc(); };

  // auto button_join_red = registry_lobby_.create();
  ButtonComponent* button_c = GenerateButtonEntity(
      registry_lobby_, "JOIN",
      glob::window::GetWindowDimensions() - glm::vec2(320, 20),
      font_team_names_, true, 22);
  button_c->text_current_color = glm::vec4(1, 1, 1, 1);
  button_c->text_normal_color = glm::vec4(1, 1, 1, 1);
  button_c->text_hover_color = glm::vec4(1.f, 0.3f, 0.3f, 1.f);

  button_c->button_func = [&] { SendJoinTeam(TEAM_RED); };

  // auto button_join_blue = registry_lobby_.create();
  button_c = GenerateButtonEntity(
      registry_lobby_, "JOIN",
      glob::window::GetWindowDimensions() - glm::vec2(320, 230),
      font_team_names_, true, 22);
  button_c->text_current_color = glm::vec4(1, 1, 1, 1);
  button_c->text_normal_color = glm::vec4(1, 1, 1, 1);
  button_c->text_hover_color = glm::vec4(.3f, .3f, 1.f, 1.f);

  button_c->button_func = [&] { SendJoinTeam(TEAM_BLUE); };

  // ability buttons
  glm::vec2 ability_buttons_pos =
      glm::vec2(30, glob::window::GetWindowDimensions().y - 150);
  int xoffset = 100;
  int yoffset = -100;

  int columns = 4;

  int c = 0;
  for (int i = 1; i < num_abilites; i++) {
    if (!IsAbilityBlackListed(i)) {
      int row = c / columns;
      int column = c % columns;
      std::string ability_text = "";  //"Ability " + std::to_string(i);
      ButtonComponent* b_c = GenerateButtonEntity(
          registry_lobby_, ability_text,
          ability_buttons_pos + glm::vec2(column * xoffset, row * yoffset),
          font_team_names_, true, 18);
      b_c->button_func = [=] { SelectAbilityHandler(i); };
      b_c->gui_handle_icon = ability_icons_[i];
      b_c->gui_handle_normal = ability_back_normal_;
      b_c->gui_handle_current = ability_back_normal_;
      b_c->gui_handle_hover = ability_back_hover_;
      b_c->bounds = glm::vec2(82, 82);
      b_c->find_name = "ability_" + std::to_string(i);
      c++;
	}
  }
}
void LobbyState::DrawTeamSelect() {
  glm::vec2 team_select_box_pos =
      glob::window::GetWindowDimensions() - glm::vec2(390, 450);
  // glob::window::Relative720(glm::vec2(900, 270));
  glob::Submit(team_select_back_, team_select_box_pos, 1.f);

  glm::vec2 team_blue_start = team_select_box_pos + glm::vec2(25, 200);
  // glob::window::Relative720(glm::vec2(930, 460));
  glm::vec2 team_red_start = team_select_box_pos + glm::vec2(25, 410);
  // glob::window::Relative720(glm::vec2(930, 680));
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

void LobbyState::DrawAbilitySelect() {
  glm::vec2 ability_select_pos =
      glm::vec2(5, glob::window::GetWindowDimensions().y - 485);
  glob::Submit(ability_select_back_, ability_select_pos, 1.0f);
}