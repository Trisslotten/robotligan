#include <ecs/components.hpp>
#include <engine.hpp>
#include <entitycreation.hpp>
#include "state.hpp"
#include <glob/window.hpp>

void SettingsState::Startup() {
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  
}

void SettingsState::Init() {
  engine_->SetCurrentRegistry(&registry_settings_);
  CreateSettingsMenu();
}

void SettingsState::Update() {
  //
}

void SettingsState::UpdateNetwork() {
  //
}
void SettingsState::Cleanup() { registry_settings_.reset(); }

void SettingsState::CreateSettingsMenu() {
  // BACK BUTTON
  ButtonComponent* b_c = GenerateButtonEntity(registry_settings_, "BACK",
                                              glm::vec2(100, 150), font_test_);
  b_c->button_func = [&]() {
    engine_->ChangeState(engine_->GetPreviousStateType());
  };
  // APPLY BUTTON
  b_c = GenerateButtonEntity(registry_settings_, "APPLY",
                                              glm::vec2(100, 200), font_test_);
  b_c->button_func = [=]() { SaveSettings();
  };

  glm::vec2 settings_start_pos =
      glm::vec2(100, glob::window::GetWindowDimensions().y - 100);
  glm::vec2 down_jump = glm::vec2(0, 75);
  // fov slider
  auto fov_slider = registry_settings_.create();
  auto& slider_c = registry_settings_.assign<SliderComponent>(fov_slider);
  slider_c.back_tex = glob::GetGUIItem("assets/GUI_Elements/slider_back.png");
  slider_c.front_tex = glob::GetGUIItem("assets/GUI_Elements/slider_front.png");
  slider_c.value = glob::GetCamera().GetFov();
  slider_c.position = settings_start_pos + down_jump * 0.f;
  slider_c.min_val = 60.f;
  slider_c.max_val = 120.f;
  slider_c.dimensions = glm::vec2(150, 50);
  slider_c.increment = 1.f;
  slider_c.text = "FOV";
  slider_c.value_to_write = &setting_fov_;
  slider_c.font_handle = font_test_;
}

void SettingsState::SaveSettings() { glob::GetCamera().SetFov(setting_fov_); }
