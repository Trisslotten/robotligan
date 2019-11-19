#include "debugoverlay.hpp"

#define DEBUG_OVERLAY
#ifdef DEBUG_OVERLAY

#include <glob/window.hpp>
#include "input.hpp"
#include "meminfo.hpp"

void UpdateTimes(std::vector<double> times, double& avg, double& min,
                 double& max) {
  avg = 0.;
  min = std::numeric_limits<double>::max();
  max = 0.;
  for (auto time : times) {
    avg += time;
    min = glm::min(min, time);
    max = glm::max(max, time);
  }
  avg /= (double)times.size();
}

void DebubOverlay::Init() {
  background_ = glob::GetGUIItem("assets/GUI_elements/transparent_pixel.png");
  font_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
}

void DebubOverlay::Update() {
  if (Input::IsKeyPressed(GLFW_KEY_F6)) {
    active_ = !active_;
  }

  double update_values_time = 0.5;

  if (update_values_timer.Elapsed() > update_values_time) {
    double elapsed = update_values_timer.Restart();

    UpdateTimes(network_times_, avg_network_time_, min_network_time_,
                max_network_time_);
    network_times_.clear();
    UpdateTimes(engine_times_, avg_engine_time_, min_engine_time_,
                max_engine_time_);
    engine_times_.clear();

    network_fps_ = num_network_updates_ / elapsed;
    engine_fps_ = num_engine_updates_ / elapsed;

    num_network_updates_ = 0;
    num_engine_updates_ = 0;
  }

  if (active_) {
    auto ws = glob::window::GetWindowDimensions();
    float box_height = 150;
    float box_width = 300;
    glm::vec2 box_offset(30, -30);
    glm::vec2 text_offset(13, -10);
    float spacing_y = 30;
    float spacing_x = 150;
    glob::Submit(background_, glm::vec2(0, ws.y - box_height) + box_offset,
                 box_height, box_width);
    glob::Submit(font_, glm::vec2(0, ws.y - 0) + text_offset + box_offset, 30,
                 "NetFPS: " + std::to_string((int)network_fps_));
    glob::Submit(font_,
                 glm::vec2(0, ws.y - 1 * spacing_y) + text_offset + box_offset,
                 30, "EngineFPS: " + std::to_string((int)engine_fps_));

    auto ram = util::MemoryInfo::GetInstance().GetUsedRAM();
    auto vram = util::MemoryInfo::GetInstance().GetUsedVRAM();
    glob::Submit(font_,
                 glm::vec2(0, ws.y - 2 * spacing_y) + text_offset + box_offset,
                 30, "RAM: " + std::to_string(ram) + " MB");
    glob::Submit(font_,
                 glm::vec2(0, ws.y - 3 * spacing_y) + text_offset + box_offset,
                 30, "VRAM: " + std::to_string(vram) + " MB");

    glob::Submit(font_,
                 glm::vec2(1 * spacing_x, ws.y - 0 * spacing_y) + text_offset +
                     box_offset,
                 30, "Network ms:");
    glob::Submit(font_,
                 glm::vec2(1 * spacing_x, ws.y - 1 * spacing_y) + text_offset +
                     box_offset,
                 30, "  avg: " + std::to_string((avg_network_time_ * 1000.0)));
    glob::Submit(font_,
                 glm::vec2(1 * spacing_x, ws.y - 2 * spacing_y) + text_offset +
                     box_offset,
                 30, "  min: " + std::to_string((min_network_time_ * 1000.0)));
    glob::Submit(font_,
                 glm::vec2(1 * spacing_x, ws.y - 3 * spacing_y) + text_offset +
                     box_offset,
                 30, "  max: " + std::to_string((max_network_time_ * 1000.0)));

    glob::Submit(font_,
                 glm::vec2(2 * spacing_x, ws.y - 0 * spacing_y) + text_offset +
                     box_offset,
                 30, "Engine ms:");
    glob::Submit(font_,
                 glm::vec2(2 * spacing_x, ws.y - 1 * spacing_y) + text_offset +
                     box_offset,
                 30, "  avg: " + std::to_string((avg_engine_time_ * 1000.0)));
    glob::Submit(font_,
                 glm::vec2(2 * spacing_x, ws.y - 2 * spacing_y) + text_offset +
                     box_offset,
                 30, "  min: " + std::to_string((min_engine_time_ * 1000.0)));
    glob::Submit(font_,
                 glm::vec2(2 * spacing_x, ws.y - 3 * spacing_y) + text_offset +
                     box_offset,
                 30, "  max: " + std::to_string((max_engine_time_ * 1000.0)));
  }
}

void DebubOverlay::BeforeNetworkUpdate() { network_timer_.Restart(); }

void DebubOverlay::AfterNetworkUpdate() {
  double elapsed = network_timer_.Elapsed();
  network_times_.push_back(elapsed);
  num_network_updates_++;
}

void DebubOverlay::BeforeEngineUpdate() { engine_timer_.Restart(); }

void DebubOverlay::AfterEngineUpdate() {
  double elapsed = engine_timer_.Elapsed();
  engine_times_.push_back(elapsed);
  num_engine_updates_++;
}

#else

void DebubOverlay::Init() {}

void DebubOverlay::Update() {}

void DebubOverlay::BeforeNetworkUpdate() {}

void DebubOverlay::AfterNetworkUpdate() {}

void DebubOverlay::BeforeEngineUpdate() {}

void DebubOverlay::AfterEngineUpdate() {}

#endif