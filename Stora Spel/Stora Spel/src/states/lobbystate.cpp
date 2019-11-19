#include "state.hpp"

#include <glob/window.hpp>
#include <util/asset_paths.hpp>
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
      auto& b_c = registry_lobby_.get<ButtonComponent>(button);
      b_c.gui_handle_icon = ready_icon_;
      break;
    }
    // b_c.text_current_color = glm::vec4(0.f, 1.f, 0.f, 1.f);
    // b_c.text_hover_color = glm::vec4(0.f, 1.f, 0.f, 1.f);
  } else {
    auto& packet = engine_->GetPacket();
    packet << PacketBlockType::CLIENT_NOT_READY;

    for (auto button : view_ready_button) {
      auto& b_c = registry_lobby_.get<ButtonComponent>(button);
      b_c.gui_handle_icon = ready_empty_icon_;
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

  // reset all ability buttons
  for (auto button : view_buttons) {
    ButtonComponent& b_c = registry_lobby_.get<ButtonComponent>(button);
    if (b_c.find_name.find("ability") != std::string::npos) {
      b_c.gui_handle_normal = ability_back_normal_;
    }
  }
  entt::entity selected_button =
      GetAbilityButton("ability_" + std::to_string((int)id));

  if ((int)selected_button != NULL) {
    ButtonComponent& b_c =
        registry_lobby_.get<ButtonComponent>(selected_button);
    b_c.gui_handle_normal = ability_back_selected_;
    b_c.gui_handle_current = ability_back_selected_;
  }

  // send selection to server
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
void LobbyState::SendMyName() {
  auto& packet = engine_->GetPacket();
  std::string name = GlobalSettings::Access()->StringValueOf("USERNAME");
  packet.Add(name.c_str(), name.size());
  packet << name.size();
  packet << PacketBlockType::MY_NAME;
}

void LobbyState::Startup() {
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  ws_ = glob::window::GetWindowDimensions();
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
  SendMyName();

  engine_->GetChat()->SetPosition(glm::vec2(20, 140));

  engine_->GetAnimationSystem().Reset(registry_lobby_);

  engine_->GetChat()->SetShowChat();
}

void LobbyState::Update(float dt) {
  server_state_ = engine_->GetServerState();
  DrawTeamSelect();
  DrawAbilitySelect();

  // draw ready string
  glm::vec2 pos = glm::vec2(glob::window::GetWindowDimensions().x - 330, 120);
  glob::Submit(font_test_, pos, 72, "Ready: ");

  bool everyone_ready = true;
  for (auto lp : lobby_players_) {
    if (!lp.second.ready) {
      everyone_ready = false;
      break;
    }
  }
  if (everyone_ready) {
    glm::vec2 bottom_pos =
        glm::vec2((glob::window::GetWindowDimensions().x / 2) - 235, 30);

    if (engine_->GetServerState() == ServerStateType::LOBBY) {
      glob::Submit(font_test_, bottom_pos, 28,
                   "All players are ready. Match will start soon.");
    } else {
      glob::Submit(font_test_, bottom_pos, 28, "Match is currently in session");
    }

    // auto game_clients = engine_->GetPlayingPlayers();

    /*
    if (engine_->GetStateType() == 1)
    {
            if (game_clients && std::find(game_clients->begin(),
    game_clients->end(), this->my_id_) != game_clients->end())
            {
                    engine_->ChangeState(StateType::PLAY);
            }
    }*/
  }
  glob::Submit(chatbox_back_, glm::vec2(10, 24), 1.0f);
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
  int id = -1;
  unsigned int team = 0;
  bool ready = false;
  size_t len;
  std::string name = "";
  packet >> ready;
  packet >> team;
  packet >> id;
  packet >> len;
  name.resize(len);
  packet.Remove(name.data(), len);
  std::cout << "Lobby: name: " << name << "\n";
  if (id != -1) {
    LobbyPlayer plyr;
    plyr.ready = ready;
    plyr.team = team;
    lobby_players_[id] = plyr;
  }
  engine_->player_names_[id] = name;
}

void LobbyState::HandlePlayerDisconnect(NetAPI::Common::Packet& packet) {
  unsigned short id = -1;
  packet >> id;
  lobby_players_.erase(id);
}

void LobbyState::CreateBackgroundEntities() {
  // add the lights to scene
  auto light_test = registry_lobby_.create();
  registry_lobby_.assign<LightComponent>(
      light_test, glm::vec3(0.1f, 0.1f, 1.0f), 30.f, 0.2f);
  registry_lobby_.assign<TransformComponent>(
      light_test, glm::vec3(12.f, -16.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(2.0f);
  {
    // ladda in och skapa entity för bana
    auto arena = registry_lobby_.create();
    glm::vec3 zero_vec = glm::vec3(0.0f);
    glob::ModelHandle model_arena =
        glob::GetModel("assets/Arena/Map_V3_ARENA.fbx");
    glob::ModelHandle model_arena_banner =
        glob::GetModel("assets/Arena/Map_V3_ARENA_SIGNS.fbx");
    glob::ModelHandle model_map = glob::GetModel("assets/MapV3/Map_Walls.fbx");
    glob::ModelHandle model_map_floor =
        glob::GetModel("assets/MapV3/Map_Floor.fbx");
    glob::ModelHandle model_map_projectors =
        glob::GetModel("assets/MapV3/Map_Projectors.fbx");

    // glob::GetModel(kModelPathMapSingular);
    auto& model_c = registry_lobby_.assign<ModelComponent>(arena);
    model_c.handles.push_back(model_arena);
    model_c.handles.push_back(model_arena_banner);
    model_c.handles.push_back(model_map_projectors);

    registry_lobby_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                               arena_scale);

    arena = registry_lobby_.create();
    auto& model_c2 = registry_lobby_.assign<ModelComponent>(arena);
    model_c2.handles.push_back(model_map);
    model_c2.handles.push_back(model_map_floor);
    registry_lobby_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                               arena_scale);
  }

  {
    auto arena = registry_lobby_.create();
    glob::ModelHandle model_map_walls =
        glob::GetTransparentModel("assets/MapV3/Map_EnergyWall.fbx");

    auto& model_c = registry_lobby_.assign<ModelComponent>(arena);
    model_c.handles.push_back(model_map_walls);
    registry_lobby_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                               arena_scale);
  }

  {
    // ladda in och skapa entity för boll
    auto ball = registry_lobby_.create();
    glob::ModelHandle model_ball_projectors_p =
        glob::GetModel("Assets/Ball_new/Ball_projectors.fbx");
    glob::ModelHandle model_ball_sphere_p =
        glob::GetTransparentModel("Assets/Ball_new/Ball_Sphere.fbx");
    // glob::GetModel("assets/Ball_new/Ball_Comb_tmp.fbx");
    auto& model_c = registry_lobby_.assign<ModelComponent>(ball);
    model_c.handles.push_back(model_ball_sphere_p);
    model_c.handles.push_back(model_ball_projectors_p);
    registry_lobby_.assign<TransformComponent>(ball, glm::vec3(0, 1, 0),
                                               zero_vec, glm::vec3(1.0f));
    registry_lobby_.assign<BallComponent>(ball);
  }

  {
    // ladda in och skapa entity för robotar
    auto robot = registry_lobby_.create();
    auto& trans = registry_lobby_.assign<TransformComponent>(
        robot, zero_vec, glm::vec3(0.f, 180.f, 0.f), glm::vec3(0.01f));
    glob::ModelHandle model_robot = glob::GetModel(kModelPathMech);
    auto& model_c = registry_lobby_.assign<ModelComponent>(robot);
    model_c.handles.push_back(model_robot);
    // registry_lobby_.assign<AnimationComponent>(robot,
    // glob::GetAnimationData(model_robot));
    trans.position = glm::vec3(10.f, -4.f, 0.f);
  }

  {
    // lägga ut en kamera i scenen
    auto camera = registry_lobby_.create();
    auto& cam_c = registry_lobby_.assign<CameraComponent>(camera);
    auto& cam_trans = registry_lobby_.assign<TransformComponent>(camera);
    cam_trans.position = glm::vec3(-10.f, 5.f, -3.f);
    glm::vec3 dir = glm::vec3(0) - cam_trans.position;
    cam_c.orientation = glm::quat(glm::vec3(0.f, -0.3f, 0.f));
  }
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

  ready_back_normal_ =
      glob::GetGUIItem("Assets/GUI_elements/lobby/ready_unchecked.png");
  ready_back_hover_ =
      glob::GetGUIItem("Assets/GUI_elements/lobby/ready_hover.png");

  ready_icon_ = glob::GetGUIItem("Assets/GUI_elements/lobby/ready_icon.png");
  chatbox_back_ = glob::GetGUIItem("Assets/GUI_elements/chat_back.png");
  ready_empty_icon_ =
      glob::GetGUIItem("Assets/GUI_elements/lobby/dummy_icon.png");

  int num_abilites = (int)AbilityID::NUM_OF_ABILITY_IDS;
  ability_icons_.resize(num_abilites);
  for (int i = 0; i < num_abilites; i++) {
    ability_icons_[i] = glob::GetGUIItem("Assets/GUI_elements/ability_icons/" +
                                         std::to_string(i) + ".png");
  }
  ability_tooltips_.resize(num_abilites);
  ability_tooltips_[1] = "BUILD WALL: construct a wall on the field.";
  ability_tooltips_[2] =
      "FAKE BALL: Spawn a number of fake balls around the ball.";
  ability_tooltips_[3] =
      "FORCE PUSH: Throw an explosive projectile that pushes opponents back.";
  ability_tooltips_[4] = "GRAVITY: Lower the gravity of the arena.";
  ability_tooltips_[5] =
      "HOMING BALL: Kick the ball and guide it with your aim.";
  ability_tooltips_[6] =
      "INVISIBILITY: Turn invisible for a short amount of time.";
  ability_tooltips_[7] = "MISSILE: Shoot a guided missile at your target.";
  ability_tooltips_[8] =
      "SUPER STRIKE: Kick the ball with an insane amount of force.";
  ability_tooltips_[9] = "SWITCH GOALS: Flip both teams goals around.";
  ability_tooltips_[10] =
      "TELEPORT: Fire a projectile that teleports you to the point of impact.";

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
      b_c->hover_text = ability_tooltips_[i];
      c++;
    }
  }

  // ready button
  auto button = registry_lobby_.create();
  registry_lobby_.assign<ReadyButtonComponent>(button);
  ButtonComponent& button_comp =
      registry_lobby_.assign<ButtonComponent>(button);
  /*ready_button_c = GenerateButtonEntity(
      registry_lobby_, "READY", glob::window::Relative720(glm::vec2(1120, 40)),
      font_test_);*/
  button_comp.text = "";
  button_comp.font_size = 0;  // menu_settings::font_size;
  button_comp.f_handle = font_test_;
  registry_lobby_.assign<TransformComponent>(
      button, glm::vec3(glob::window::GetWindowDimensions().x - 200, 82, 0));
  button_comp.visible = true;
  button_comp.gui_handle_normal = ready_back_normal_;
  button_comp.gui_handle_current = ready_back_normal_;
  button_comp.gui_handle_hover = ready_back_hover_;
  button_comp.gui_handle_icon = ready_empty_icon_;
  button_comp.click_offset = glm::vec2(-150, 0);
  button_comp.bounds = glm::vec2(200, 50);
  button_comp.button_func = [&] {
    if (engine_->GetServerState() == ServerStateType::LOBBY) {
      ReadyButtonFunc();
    }
  };
  ButtonComponent* b_c = GenerateButtonEntity(
      registry_lobby_, "DISCONNECT",
      glm::vec2(glob::window::GetWindowDimensions().x - 330, 65), font_test_);
  b_c->button_func = [&]() {
    engine_->GetClient().Disconnect();
    engine_->ChangeState(StateType::MAIN_MENU);
  };
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