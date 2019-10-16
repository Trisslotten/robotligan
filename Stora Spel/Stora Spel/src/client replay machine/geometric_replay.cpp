#include "geometric_replay.hpp"

// Private---------------------------------------------------------------------

void GeometricReplay::SaveObjectsToFrame(entt::registry& in_registry) {
  // Start by getting all the objects
  // with an id and a transformation
  // from the registry
  entt::basic_view object_view =
      in_registry.view<IDComponent, TransformComponent>();

  for (entt::entity object_entity : object_view) {
    IDComponent& id_c = in_registry.get<IDComponent>(object_entity);
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(object_entity);

    // Save data to a ObjectData struct
    ObjectData temp_data;
    temp_data.id = id_c.id;
    temp_data.pos[0] = transform_c.position.x;
    temp_data.pos[1] = transform_c.position.y;
    temp_data.pos[2] = transform_c.position.z;
    temp_data.rot[0] = transform_c.rotation.x;
    temp_data.rot[1] = transform_c.rotation.y;
    temp_data.rot[2] = transform_c.rotation.z;
    temp_data.rot[3] = transform_c.rotation.w;

	// --- BIG WIP ---

	// Check if 'object_entity' has any animation component
    // and if so add it to the temp_data

	// Check if 'object_entity' has any particle component
	// and if so add it to the temp_data

	// --- BIG WIP ---

    // Add the extracted ObjectData to the
    // FrameData array position for the
    // currenmt frame
    this->frame_arr_[this->current_frame_].objects.push_back(temp_data);
  }
}

void GeometricReplay::WriteAnimationToBitpack() {}

void GeometricReplay::WriteEventToBitpack() {}

void GeometricReplay::WriteObjectToBitPack() {}

void GeometricReplay::WriteParticleEffectToBitpack() {}

// Public----------------------------------------------------------------------

GeometricReplay::GeometricReplay(
    unsigned int in_num_of_frames,
    unsigned int in_interpolation_interval_frames) {
  // Allocate space for the designated number of frames
  this->frame_arr_ = new FrameData[in_num_of_frames];
  this->num_of_frames_ = in_num_of_frames;
  this->current_frame_ = 0;

  // Save the interpolation interval for objects
  this->interpolation_interval_ = in_interpolation_interval_frames;
  this->frames_since_last_interpolation_ = in_interpolation_interval_frames;
}

GeometricReplay::~GeometricReplay() {
  if (this->frame_arr_ != nullptr) {
    delete[] this->frame_arr_;
  }
}

bool GeometricReplay::SaveFrame(entt::registry& in_registry) {
  // Return true if the last frame has been reached
  if (this->current_frame_ == this->num_of_frames_) {
    return true;
  }

  // Check if we are to save data required for interpolation
  this->frames_since_last_interpolation_++;
  if (this->frames_since_last_interpolation_ > this->interpolation_interval_) {
    this->frames_since_last_interpolation_ = 0;

    // If so, find all objects and save them
    // to the frame structure
    this->SaveObjectsToFrame(in_registry);
  }

  // Return
  return false;
}

bool GeometricReplay::LoadFrame(entt::registry& in_registry) { return false; }
