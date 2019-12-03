#include "state.hpp"

#include <ecs\components\follow_bone_component.hpp>
#include <glob/window.hpp>
#include <util\asset_paths.hpp>
#include "../ecs/components.hpp"
#include "../ecs/systems/animation_system.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"

void MainMenuState::Startup() {
  CreateMainMenu();
  CreateInformationMenu();
  CreateBackgroundEnitites();

  information_image_ = glob::GetGUIItem("assets/GUI_elements/info_menu_v2.png");

  loggo_image_ = glob::GetGUIItem("assets/GUI_elements/logga.png");
}

void MainMenuState::Init() {
  auto& client = engine_->GetClient();
  //
  glob::window::SetMouseLocked(false);
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_mainmenu_);
  engine_->SetEnableChat(false);

  if (client.IsConnected()) {
    client.Disconnect();
  }
  glob::GetCamera().SetFov(90.f);
}

void MainMenuState::Update(float dt) {
  //
  if (engine_->GetCurrentRegistry() == &registry_information_) {
    auto ws = glob::window::GetWindowDimensions();
    glm::vec2 pos;
    pos.x = ws.x - 1152.0f;
    pos.y = ws.y - 648.0f;
    glob::Submit(information_image_, pos, 1.0f);
  } else {
    auto ws = glob::window::GetWindowDimensions();
    float scale = 0.8f;
    glm::vec2 pos;
    pos.x = ws.x / 2.f - scale * 1280.f / 2.f;
    pos.y = ws.y - scale * 313 - 100;
    glob::Submit(loggo_image_, pos, scale);
  }
}

void MainMenuState::UpdateNetwork() {
  //
}

void MainMenuState::Cleanup() {
  //
}

void MainMenuState::CreateMainMenu() {
  glob::window::SetMouseLocked(false);
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  // PLAY BUTTON - change registry to registry_mainmenu_
  ButtonComponent* b_c = GenerateButtonEntity(registry_mainmenu_, "JOIN SERVER",
                                              glm::vec2(100, 290), font_test_);
  b_c->button_func = [&]() { engine_->ChangeState(StateType::CONNECT_MENU); };

  // Create Server
  b_c = GenerateButtonEntity(registry_mainmenu_, "CREATE SERVER",
                             glm::vec2(100, 230), font_test_);
  b_c->button_func = [&]() { engine_->ChangeState(StateType::CREATE_SERVER); };

  // SETTINGS BUTTON - change registry to registry_settings_
  b_c = GenerateButtonEntity(registry_mainmenu_, "SETTINGS",
                             glm::vec2(100, 170), font_test_);
  b_c->button_func = [&]() { engine_->ChangeState(StateType::SETTINGS); };

  b_c = GenerateButtonEntity(registry_mainmenu_, "INFORMATION",
                             glm::vec2(100, 110), font_test_);
  b_c->button_func = [&]() {
    engine_->SetCurrentRegistry(&registry_information_);
  };

  // EXIT BUTTON - close the game
  // Exit() is bad, does not call destructors
  b_c = GenerateButtonEntity(registry_mainmenu_, "EXIT", glm::vec2(100, 50),
                             font_test_);
  b_c->button_func = [&]() { engine_->should_quit = true; };
}

void MainMenuState::CreateInformationMenu() {
  // BACK BUTTON in Information - go back to main menu
  ButtonComponent* b_c = GenerateButtonEntity(registry_information_, "BACK",
                                              glm::vec2(60, 50), font_test_);
  b_c->button_func = [&]() {
    engine_->SetCurrentRegistry(&registry_mainmenu_);
  };

  // Background scene
  // add the lights to scene

  auto light_test = registry_information_.create();  // Get from engine
  registry_information_.assign<LightComponent>(light_test, glm::vec3(0.05f),
                                               30.f, 0.1f);
  registry_information_.assign<TransformComponent>(
      light_test, glm::vec3(0.f, 16.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  glm::vec3 zero_vec = glm::vec3(0.0f);

  auto light_test2 = registry_information_.create();  // Get from engine
  registry_information_.assign<LightComponent>(
      light_test2, glm::vec3(1.f, 1.f, 1.0f), 50.f, 0.1f);
  registry_information_.assign<TransformComponent>(
      light_test2, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));
  {
    // ladda in och skapa entity för bana
    auto arena = registry_information_.create();
    glm::vec3 arena_scale = glm::vec3(2.0f);

    glob::ModelHandle model_arena =
        glob::GetModel("assets/Arena/Map_V3_ARENA.fbx");
    glob::ModelHandle model_arena_banner =
        glob::GetModel("assets/Arena/Map_V3_ARENA_SIGNS.fbx");
    glob::ModelHandle model_map = glob::GetModel("assets/MapV3/Map_Walls.fbx");
    glob::ModelHandle model_map_floor =
        glob::GetModel("assets/MapV3/Map_Floor.fbx");
    glob::ModelHandle model_map_projectors =
        glob::GetModel("assets/MapV3/Map_Projectors.fbx");

    auto& model_c = registry_information_.assign<ModelComponent>(arena);
    model_c.handles.push_back(model_arena);
    model_c.handles.push_back(model_arena_banner);
    model_c.handles.push_back(model_map);
    model_c.handles.push_back(model_map_floor);
    model_c.handles.push_back(model_map_projectors);
    // model_c.cast_shadow = false;

    registry_information_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                     arena_scale);
  }
  {
    glm::vec3 zero_vec = glm::vec3(0.0f);
    glm::vec3 arena_scale = glm::vec3(1.f);
    auto arena = registry_information_.create();
    glob::ModelHandle model_map_walls =
        glob::GetTransparentModel("assets/MapV3/Map_EnergyWall.fbx");

    auto& model_c = registry_information_.assign<ModelComponent>(arena);
    model_c.handles.push_back(model_map_walls);
    model_c.cast_shadow = false;

    registry_information_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                     arena_scale);
  }
  {
    // lägga ut en kamera i scenen
    auto camera = registry_information_.create();
    auto& cam_c = registry_information_.assign<CameraComponent>(camera);
    auto& cam_trans = registry_information_.assign<TransformComponent>(camera);
    cam_trans.position = glm::vec3(15.f, 3.f, 0.f);
    glm::vec3 dir = glm::vec3(0) - cam_trans.position;
    cam_c.orientation =
        glm::quat(glm::vec3(0.f, 0.f, 0.f * glm::radians(-86.f)));
  }
}

void MainMenuState::CreateBackgroundEnitites() {
  // add the lights to scene

  auto light_test = registry_mainmenu_.create();  // Get from engine
  registry_mainmenu_.assign<LightComponent>(light_test, glm::vec3(0.05f), 30.f,
                                            0.1f);
  registry_mainmenu_.assign<TransformComponent>(
      light_test, glm::vec3(0.f, 16.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  glm::vec3 zero_vec = glm::vec3(0.0f);

  auto light_test2 = registry_mainmenu_.create();  // Get from engine
  registry_mainmenu_.assign<LightComponent>(
      light_test2, glm::vec3(1.f, 1.f, 1.0f), 50.f, 0.1f);
  registry_mainmenu_.assign<TransformComponent>(
      light_test2, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));
  {
    // ladda in och skapa entity för bana
    auto arena = registry_mainmenu_.create();
    glm::vec3 arena_scale = glm::vec3(2.0f);

    glob::ModelHandle model_arena =
        glob::GetModel("assets/Arena/Map_V3_ARENA.fbx");
    glob::ModelHandle model_arena_banner =
        glob::GetModel("assets/Arena/Map_V3_ARENA_SIGNS.fbx");
    glob::ModelHandle model_map = glob::GetModel("assets/MapV3/Map_Walls.fbx");
    glob::ModelHandle model_map_floor =
        glob::GetModel("assets/MapV3/Map_Floor.fbx");
    glob::ModelHandle model_map_projectors =
        glob::GetModel("assets/MapV3/Map_Projectors.fbx");

    auto& model_c = registry_mainmenu_.assign<ModelComponent>(arena);
    model_c.handles.push_back(model_arena);
    model_c.handles.push_back(model_arena_banner);
    model_c.handles.push_back(model_map);
    model_c.handles.push_back(model_map_floor);
    model_c.handles.push_back(model_map_projectors);
    // model_c.cast_shadow = false;

    registry_mainmenu_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                  arena_scale);
  }
  {
    glm::vec3 zero_vec = glm::vec3(0.0f);
    glm::vec3 arena_scale = glm::vec3(1.f);
    auto arena = registry_mainmenu_.create();
    glob::ModelHandle model_map_walls =
        glob::GetTransparentModel("assets/MapV3/Map_EnergyWall.fbx");

    auto& model_c = registry_mainmenu_.assign<ModelComponent>(arena);
    model_c.handles.push_back(model_map_walls);
    model_c.cast_shadow = false;

    registry_mainmenu_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                  arena_scale);
  }

  // Modeller
  {
    // ladda in och skapa entity för robotar
    auto robot = registry_mainmenu_.create();
    auto& trans_c = registry_mainmenu_.assign<TransformComponent>(
        robot, glm::vec3(22, 0, -8.f),
        glm::vec3(0.f, glm::radians(-135.0f), 0.f), glm::vec3(0.0033f));
    glob::ModelHandle model_robot = glob::GetModel("assets/Mech/Mech.fbx");
    auto& model_c = registry_mainmenu_.assign<ModelComponent>(robot);
    model_c.handles.push_back(model_robot);

    // Animation
    auto& animation_c = registry_mainmenu_.assign<AnimationComponent>(
        robot, glob::GetAnimationData(model_robot));

    engine_->GetAnimationSystem().PlayAnimation(
        "Emote2", 1.f, &animation_c, 10, 1.f,
        engine_->GetAnimationSystem().LOOP);

    robot = registry_mainmenu_.create();
    auto& trans_c2 = registry_mainmenu_.assign<TransformComponent>(
        robot, glm::vec3(22, 0, 0.f), glm::vec3(0.f, glm::radians(180.0f), 0.f),
        glm::vec3(0.0033f));
    auto& model_c2 = registry_mainmenu_.assign<ModelComponent>(robot);
    model_c2.handles.push_back(model_robot);

    // Animation
    auto& animation_c2 = registry_mainmenu_.assign<AnimationComponent>(
        robot, glob::GetAnimationData(model_robot));

    engine_->GetAnimationSystem().PlayAnimation(
        "Run", 1.f, &animation_c2, 10, 1.f, engine_->GetAnimationSystem().LOOP);

    // ladda in och skapa entity för robotar
    robot = registry_mainmenu_.create();
    auto& trans_c3 = registry_mainmenu_.assign<TransformComponent>(
        robot, glm::vec3(22.f, 0, 8.f),
        glm::vec3(0.f, glm::radians(135.0f), 0.f), glm::vec3(0.0033f));
    auto& model_c3 = registry_mainmenu_.assign<ModelComponent>(robot);
    model_c3.handles.push_back(model_robot);

    // Animation
    auto& animation_c3 = registry_mainmenu_.assign<AnimationComponent>(
        robot, glob::GetAnimationData(model_robot));

    engine_->GetAnimationSystem().PlayAnimation(
        "Emote1", 2.f, &animation_c3, 10, 1.f,
        engine_->GetAnimationSystem().LOOP);
  }
  {
    // lägga ut en kamera i scenen
    auto camera = registry_mainmenu_.create();
    auto& cam_c = registry_mainmenu_.assign<CameraComponent>(camera);
    auto& cam_trans = registry_mainmenu_.assign<TransformComponent>(camera);
    cam_trans.position = glm::vec3(15.f, 3.f, 0.f);
    glm::vec3 dir = glm::vec3(0) - cam_trans.position;
    cam_c.orientation =
        glm::quat(glm::vec3(0.f, 0.f, 0.f * glm::radians(-86.f)));

  }
}
