#include "state.hpp"

#include <GLFW/glfw3.h>
#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <util/asset_paths.hpp>
#include "..//entitycreation.hpp"
#include "engine.hpp"

// Private---------------------------------------------------------------------

void ReplayState::AddConstantStuff() {
  this->AddArenaStuff();
  this->AddBatmanLights();
  this->AddLights();
  //this->AddSpotlights();
  this->AddAudience();
  this->AddCamera(glm::vec3(0.f, 13.f, 42.f));  //  glm::vec3(60.f, 4.f, 38.f);
  glob::GetCamera().SetFov(60);
}

void ReplayState::AddArenaStuff() {
  // Map
  entt::entity arena = this->replay_registry_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);

  glob::ModelHandle model_arena = glob::GetModel(kModelPathArena);
  glob::ModelHandle model_arena_banner = glob::GetModel(kModelPathArenaSigns);
  glob::ModelHandle model_map = glob::GetModel(kModelPathMapWalls);
  glob::ModelHandle model_map_floor = glob::GetModel(kModelPathMapFloor);
  glob::ModelHandle model_map_projectors =
      glob::GetModel(kModelPathMapProjectors);
  glob::ModelHandle model_map_misc_laken = glob::GetModel(kModelPathMiscLaken);
  glob::ModelHandle model_map_misc_spectre =
      glob::GetModel(kModelPathMiscSpectre);
  glob::ModelHandle model_map_misc_starfighter =
      glob::GetModel(kModelPathMiscStarfighter);
  glob::ModelHandle model_map_misc_mig = glob::GetModel(kModelPathMiscMig);
  glob::ModelHandle model_map_misc1 = glob::GetModel(kModelPathMisc1);

  ModelComponent& model_c =
      this->replay_registry_.assign<ModelComponent>(arena);
  model_c.handles.push_back(model_arena);
  model_c.handles.push_back(model_arena_banner);
  model_c.handles.push_back(model_map_projectors);
  model_c.handles.push_back(model_map_misc1);
  model_c.handles.push_back(model_map_misc_laken);
  model_c.handles.push_back(model_map_misc_spectre);
  model_c.handles.push_back(model_map_misc_starfighter);
  model_c.handles.push_back(model_map_misc_mig);
  model_c.cast_shadow = false;

  this->replay_registry_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                    this->arena_scale_);

  entt::entity arena_floor = this->replay_registry_.create();
  ModelComponent& floor_model_c =
      this->replay_registry_.assign<ModelComponent>(arena_floor);
  floor_model_c.handles.push_back(model_map);
  floor_model_c.handles.push_back(model_map_floor);
  TransformComponent& trans_c =
      this->replay_registry_.assign<TransformComponent>(
          arena_floor, zero_vec, zero_vec, this->arena_scale_);
  // Ensure goals are correctly drawn
  if (goals_swapped_) {
    trans_c.rotation *= glm::quat(glm::vec3(0.f, glm::pi<float>(), 0.f));
  }
}

void ReplayState::AddBatmanLights() {
  // Batman lights
  glob::ModelHandle batman_light =
      glob::GetTransparentModel(kModelPathBatmanLight);
  int x = 1;
  int y = 1;
  for (int i = 0; i < 4; i++) {
    {
      auto entity = this->replay_registry_.create();
      auto& model_c = this->replay_registry_.assign<ModelComponent>(entity);
      model_c.handles.push_back(batman_light);

      glm::vec3 pos = glm::vec3(-50 * x, 0, 85 * y);
      this->replay_registry_.assign<TransformComponent>(
          entity, pos, glm::vec3(), glm::vec3(1, 250, 10));
      this->replay_registry_.assign<SkyLightComponent>(entity);
    }
    {
      auto entity = this->replay_registry_.create();
      auto& model_c = this->replay_registry_.assign<ModelComponent>(entity);
      model_c.handles.push_back(batman_light);

      glm::vec3 pos = glm::vec3(-95 * x, 0, 50 * y);
      this->replay_registry_.assign<TransformComponent>(
          entity, pos, glm::vec3(), glm::vec3(1, 250, 10));

      this->replay_registry_.assign<SkyLightComponent>(entity);
    }

    std::swap(x, y);
    y *= -1;
  }
}

void ReplayState::AddLights() {
  // Lights
  entt::entity light = this->replay_registry_.create();
  this->replay_registry_.assign<LightComponent>(
      light, glm::vec3(0.4f, 0.4f, 0.4f), 90.f, 0.2f);
  this->replay_registry_.assign<TransformComponent>(
      light, glm::vec3(0, 4.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f));
}

void ReplayState::AddSpotlights() {
  // Spotlights
  glm::vec3 pos_base;
  pos_base.x = GlobalSettings::Access()->ValueOf("SPOTLIGHT_POSITION_BASEX");
  pos_base.y = GlobalSettings::Access()->ValueOf("SPOTLIGHT_POSITION_BASEY");
  pos_base.z = GlobalSettings::Access()->ValueOf("SPOTLIGHT_POSITION_BASEZ");

  pos_base *= arena_scale_;

  float xrot = 1.f;
  float zrot = 1.f;
  for (int i = 0; i < 4; i++) {
    glm::vec3 temp_pos =
        glm::vec3(xrot * pos_base.x, pos_base.y, zrot * pos_base.z);

    // Add model entity for spotlight
    auto entity = this->replay_registry_.create();
    ModelComponent& model_c =
        this->replay_registry_.assign<ModelComponent>(entity);
    glob::ModelHandle m_hndl = glob::GetModel(kModelPathSpotlight);
    model_c.handles.push_back(m_hndl);
    TransformComponent& trans_c =
        this->replay_registry_.assign<TransformComponent>(entity);
    trans_c.position = temp_pos;
    // trans_c.rotation = glm::toQuat(
    //  glm::lookAt(temp_pos, glm::vec3(0, -40.0, 0), glm::vec3(0, 1, 0)));
    trans_c.rotation *= glm::quatLookAtRH(
        glm::normalize(glm::vec3(0.f, -40.0f, 0.f) - temp_pos),
        glm::vec3(0, 1, 0));
    trans_c.Rotate(glm::vec3(0, glm::radians(90.f), 0));

    std::swap(xrot, zrot);
    zrot *= -1.f;
  }
}

void ReplayState::AddAudience() {
  glm::vec3 zero_vec = glm::vec3(0.0f);

  glob::ModelHandle model_audience = glob::GetModel(kModelPathAudience);

  for (int i = 0; i < 4; i++) {
    auto audience = this->replay_registry_.create();

    auto& audience_mc = this->replay_registry_.assign<ModelComponent>(audience);
    audience_mc.handles.push_back(model_audience);

    this->replay_registry_.assign<TransformComponent>(
        audience, zero_vec, glm::vec3(0.f, (pi / 2.f) * i, 0.f),
        glm::vec3(0.02f, 0.02f, 0.02f));

    auto& animation_c = this->replay_registry_.assign<AnimationComponent>(
        audience, glob::GetAnimationData(model_audience));
    engine_->GetAnimationSystem().PlayAnimation(
        "WaveRowsUp", 1.f, &animation_c, 10, 1.f,
        engine_->GetAnimationSystem().LOOP);
  }
}

void ReplayState::AddCamera(glm::vec3 in_cam_pos) {
  // Camera
  entt::entity camera = this->replay_registry_.create();
  this->replay_registry_.assign<CameraComponent>(camera, glm::vec3(0.f),
                                                 glm::quat(glm::vec3(0.f)));

  glm::vec3 cam_pos = in_cam_pos;
  this->replay_registry_.assign<TransformComponent>(
      camera, cam_pos, glm::quat(), glm::vec3(0.f));
  this->replay_registry_.assign<PhysicsComponent>(camera);
  this->replay_registry_.assign<SoundComponent>(camera, engine_->GetSoundEngine().CreatePlayer());
}

void ReplayState::UpdateCamera() {
  float dt = 1.f / 128.f;

  // Default point of interest
  glm::vec3 point_of_interest = glm::vec3(0.0f);

  // Get target, set point of interest
  entt::basic_view target_view =
      this->replay_registry_.view<TargetComponent, TransformComponent>();
  for (entt::entity target : target_view) {
    TargetComponent& target_c =
        this->replay_registry_.get<TargetComponent>(target);
    TransformComponent& target_trans_c =
        this->replay_registry_.get<TransformComponent>(target);

    point_of_interest = target_trans_c.position;
  }

  // Update camera
  entt::basic_view camera_view =
      this->replay_registry_.view<CameraComponent, TransformComponent>();
  for (entt::entity camera : camera_view) {
    CameraComponent& cam_c =
        this->replay_registry_.get<CameraComponent>(camera);
    TransformComponent& cam_trans_c =
        this->replay_registry_.get<TransformComponent>(camera);

    // Create vector from camera to target
    glm::vec3 temp_dir =
        glm::normalize(point_of_interest - cam_trans_c.position);

    auto quarter_turn = glm::quat(glm::vec3(0, glm::pi<float>() * 0.5f, 0));

    cam_c.orientation =
        glm::quatLookAt(temp_dir, glm::vec3(0, 1, 0)) * quarter_turn;

    // Follow ball
    float speed =
        0.65f * glm::distance(cam_trans_c.position.x, point_of_interest.x);
    if (cam_trans_c.position.x < point_of_interest.x) {
      cam_trans_c.position.x += speed * dt;
    } else {
      cam_trans_c.position.x -= speed * dt;
    }
  }
}

void ReplayState::StartReplayMode() {
  // Do not start if already replaying
  if (this->replaying_) {
    return;
  }
  // Set replaying to true
  this->replaying_ = true;

  // Get number of replays
  this->num_of_replays_ =
      this->engine_->GetReplayMachinePtr()->NumberOfStoredReplays();
  this->replay_counter_ = 0;
  engine_->GetReplayMachinePtr()->SelectReplay(this->replay_counter_);

  // Set timer to [length] * [number] + 5 seconds
  this->replay_state_duration_ =
      (this->engine_->GetReplayMachinePtr()->ReplayLength() *
       this->num_of_replays_) +
      5;
  this->replay_state_timer_.Restart();

  // Add in stuff that is in all replays
  this->AddConstantStuff();

  // Play crowd cheers
  engine_->GetSoundSystem().PlayAmbientSound(this->replay_registry_);
}

void ReplayState::PlayReplay() {
  if (!replaying_) {
    return;
  }

  if (this->engine_->GetReplayMachinePtr()->LoadFrame(this->replay_registry_)) {
    // Once replay is done playing, clear the registry
    this->replay_registry_.reset();
    replay_counter_++;

    // Add the constant stuff back in again
    // Also prevents black-sceen when all replays are done
    this->AddConstantStuff();
    this->CreateInGameMenu();

    if (!engine_->GetReplayMachinePtr()->SelectReplay(replay_counter_)) {
      // And stop replaying
      this->replaying_ = false;
    }
  }

  this->UpdateCamera();
}

void ReplayState::ToggleInGameMenu() {
  show_in_game_menu_buttons_ = !show_in_game_menu_buttons_;
  glob::window::SetMouseLocked(!show_in_game_menu_buttons_);
  this->engine_->SetSendInput(!show_in_game_menu_buttons_);
  this->engine_->SetTakeInput(!show_in_game_menu_buttons_);
  this->UpdateInGameMenu(show_in_game_menu_buttons_);
}

void ReplayState::UpdateInGameMenu(bool show_menu) {
  // Set in_game buttons visibility
  auto view =
      this->replay_registry_.view<ButtonComponent, TransformComponent>();
  for (auto v : view) {
    auto& button_c = this->replay_registry_.get<ButtonComponent>(v);
    button_c.visible = show_menu;
  }
}

void ReplayState::CreateInGameMenu() {
  this->font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  glm::vec2 in_game_menu_pos = glob::window::GetWindowDimensions();
  in_game_menu_pos /= 2;
  in_game_menu_pos.x -= 110;
  in_game_menu_pos.y += 110;

  // CONTINUE BUTTON -- change registry to registry_gameplay_
  ButtonComponent* in_game_buttons_ = GenerateButtonEntity(
      this->replay_registry_, "CONTINUE", in_game_menu_pos + glm::vec2(0, 0),
      this->font_test_, false);
  in_game_buttons_->button_func = [&]() { ToggleInGameMenu(); };
  // SETTINGS BUTTON -- change registry to registry_settings_
  in_game_buttons_ = GenerateButtonEntity(this->replay_registry_, "LOBBY",
                                          in_game_menu_pos + glm::vec2(0, -100),
                                          this->font_test_, false);

  in_game_buttons_->button_func = [&] {
    this->engine_->ChangeState(StateType::LOBBY);
    this->ToggleInGameMenu();
  };

  in_game_buttons_ = GenerateButtonEntity(replay_registry_, "EXIT",
                                          in_game_menu_pos + glm::vec2(0, -200),
                                          font_test_, false);
  in_game_buttons_->button_func = [&] { exit(0); };
}

void ReplayState::SoundComponentDestroyed(entt::entity e,
                                          entt::registry& registry) {
  auto& sc = registry.get<SoundComponent>(e);
  engine_->GetSoundEngine().DestroyPlayer(sc.sound_player);
}

void ReplayState::ShowScoreboard() {
  this->engine_->DrawScoreboard();

  glm::vec2 pos = glob::window::GetWindowDimensions();
  pos /= 2;
  pos.y -= 160;

  std::string end_countdown_text = std::to_string((
      int)(this->replay_state_duration_ - this->replay_state_timer_.Elapsed()));

  std::string return_to_lobby_test =
      "Returning to lobby in: " + end_countdown_text;

  double width = glob::GetWidthOfText(font_test_, return_to_lobby_test, 48);
  pos.x = (glob::window::GetWindowDimensions().x / 2) - (width / 2);

  glob::Submit(font_test_, pos + glm::vec2(0, -40), 48, return_to_lobby_test);

  std::string best_team = "BLUE";
  glm::vec4 best_team_color = glm::vec4(0.13f, 0.13f, 1.f, 1.f);

  if (this->engine_->GetTeamScores()[0] > this->engine_->GetTeamScores()[1]) {
    best_team = "RED";
    best_team_color = glm::vec4(1.f, 0.13f, 0.13f, 1.f);
  }

  std::string winnin_team_text = best_team + " wins!";
  width = glob::GetWidthOfText(this->font_test_, winnin_team_text, 48);

  pos.x -= width / 2;

  glob::Submit(this->font_test_, pos + glm::vec2(1, -1), 48, winnin_team_text,
               glm::vec4(0, 0, 0, 0.7f));

  glob::Submit(font_test_, pos, 48, winnin_team_text, best_team_color);
}

void ReplayState::DrawFishingLines() {
  auto view_hooks = replay_registry_.view<HookComponent, TransformComponent>();
  for (auto hook : view_hooks) {
    TransformComponent hook_trans_c =
        replay_registry_.get<TransformComponent>(hook);
    EntityID owner = replay_registry_.get<HookComponent>(hook).creator;
    auto view_players =
        replay_registry_.view<TransformComponent, IDComponent>();
    for (auto player : view_players) {
      EntityID player_eid = replay_registry_.get<IDComponent>(player).id;
      if (player_eid == owner) {
        TransformComponent player_trans_c =
            replay_registry_.get<TransformComponent>(player);
        glm::vec3 player_pos = player_trans_c.position;
        glm::vec3 player_forward =
            replay_registry_.get<TransformComponent>(player).Forward();

        glm::vec3 right = glm::cross(player_forward, glm::vec3(0, 1, 0));
        player_pos -= right * 0.6f;
        player_pos += player_forward * 0.28f;
        glob::SubmitRope(player_pos, hook_trans_c.position);
        break;
      }
    }
  }
}

// Public----------------------------------------------------------------------

void ReplayState::Startup() {
  this->font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  this->in_game_menu_gui_ =
      glob::GetGUIItem("Assets/GUI_elements/ingame_menu_V1.png");

  this->replay_registry_.on_destroy<SoundComponent>()
      .connect<&ReplayState::SoundComponentDestroyed>(*this);
}

void ReplayState::Init() {
  // Set this to be the current registry of engine
  this->engine_->SetCurrentRegistry(&this->replay_registry_);

  this->StartReplayMode();

  this->CreateInGameMenu();
}

void ReplayState::Update(float dt) {
  // One function for the scoreboard
  // Highlight loop logic : NTS: Moved to network update
  // PlayReplay();

  // Draw scoreboard during highlight time
  this->ShowScoreboard();

  // Escape button: Show menu
  if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
    ToggleInGameMenu();
  }

  if (show_in_game_menu_buttons_) {
    glm::vec2 in_game_menu_pos = glob::window::GetWindowDimensions();
    in_game_menu_pos /= 2;
    in_game_menu_pos.x -= 165;
    in_game_menu_pos.y -= 180;
    glob::Submit(in_game_menu_gui_, in_game_menu_pos, 1.0f);
  }

  // Go to lobby once replay is done
  if (this->replay_state_timer_.Elapsed() > this->replay_state_duration_) {
    engine_->ChangeState(StateType::LOBBY);
  }

  DrawFishingLines();
}

void ReplayState::UpdateNetwork() {
  //
  this->PlayReplay();
}

void ReplayState::Cleanup() {
  // Stop playing crowd cheer
  engine_->GetSoundSystem().StopAmbientSound(this->replay_registry_);

  // Turn lights back on :)
  glob::SetBlackout(false);

  // Clear registry
  this->replay_registry_.reset();

  // Reset variables
  this->replaying_ = false;
  this->num_of_replays_ = 0;
  this->replay_counter_ = 0;

  this->replay_state_timer_.Pause();

  // Tell replay machine to clear stored data
  this->engine_->GetReplayMachinePtr()->ResetMachine();
}