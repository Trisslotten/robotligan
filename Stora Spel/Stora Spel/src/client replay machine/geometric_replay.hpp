#ifndef GEOMETRIC_REPLAY_HPP_
#define GEOMETRIC_REPLAY_HPP_

#include <vector>

#include <entity/registry.hpp>

#include <shared/bit_pack.hpp>

#include <shared/id_component.hpp>
#include <shared/transform_component.hpp>

class GeometricReplay {
 private:
  // DATA TYPES
  struct ObjectData {
    EntityID id;
    float pos[3];
    float rot[4];
    // bool has_animation;	// Not required, just check
    // if animation vector has elements
    // bool has_particle_effect;  // Same here
    // particle vector
  };
  struct EventData {
    // id
    // other data
  };

  struct FrameData {
    std::vector<ObjectData> objects;
    std::vector<EventData> events;
  };

  struct InterpolationWindow {
    unsigned int a;
    unsigned int b;
  };

  // VARIABLES
  FrameData* frame_arr_;

  unsigned int num_of_frames_;

  unsigned int current_frame_write_;
  unsigned int current_frame_read_;

  unsigned int start_frame_;  // Variables for circular behaviour
  unsigned int end_frame_;

  unsigned int interpolation_interval_;
  unsigned int frames_since_last_interpolation_;

  InterpolationWindow interpolation_window_;

  // FUNCTIONS
  void SaveObjectsToFrame(entt::registry& in_registry);
  void WriteAnimationToBitpack();
  void WriteEventToBitpack();
  void WriteObjectToBitPack();
  void WriteParticleEffectToBitpack();

  void SetInterpolationWindow(unsigned int in_index);
  void InterpolateFrame(entt::registry& in_registry);
  void InterpolateTransformComponent(TransformComponent& in_tc,
                                     ObjectData& in_a, ObjectData& in_b);

 public:
  GeometricReplay(unsigned int in_num_of_frames,
                  unsigned int in_interpolation_interval_frames);
  ~GeometricReplay();

  bool SaveFrame(entt::registry& in_registry);
  bool LoadFrame(entt::registry& in_registry);
};

#endif  // !GEOMETRIC_REPLAY_HPP_
