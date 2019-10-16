#include "geometric_replay.hpp"

// Private---------------------------------------------------------------------

void GeometricReplay::SaveObjectsToFrame(entt::registry& in_registry) {
  // Start by getting all entities
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
    this->frame_arr_[this->current_frame_write_].objects.push_back(temp_data);
  }
}

void GeometricReplay::WriteAnimationToBitpack() {}

void GeometricReplay::WriteEventToBitpack() {}

void GeometricReplay::WriteObjectToBitPack() {}

void GeometricReplay::WriteParticleEffectToBitpack() {}

void GeometricReplay::SetInterpolationWindow(unsigned int in_index) {
  unsigned int mod_val = in_index % this->interpolation_interval_;
  this->interpolation_window_.a = in_index - mod_val;
  this->interpolation_window_.b =
      in_index + (this->interpolation_interval_ - mod_val);
}

void GeometricReplay::InterpolateFrame(entt::registry& in_registry) {
  // Start by getting all entities
  // with an id and a transformation
  // from the registry entt::basic_view object_view =
  entt::basic_view object_view =
      in_registry.view<IDComponent, TransformComponent>();

  // Loop over each entity
  for (entt::entity object_entity : object_view) {
    IDComponent& id_c = in_registry.get<IDComponent>(object_entity);
    TransformComponent& transform_c =
        in_registry.get<TransformComponent>(object_entity);

    // Check if there is ObjectData in
    // interpolation window a and b
    // for that id.
    // If there is save the index in
    // the ObjectData vector it was
    // encountered on.
    std::vector<ObjectData>& a_vector =
        this->frame_arr_[this->interpolation_window_.a].objects;
    int a_index = -1;
    std::vector<ObjectData>& b_vector =
        this->frame_arr_[this->interpolation_window_.b].objects;
    int b_index = -1;
    // Loop for a
    for (unsigned int i = 0; i < a_vector.size(); i++) {
      if (a_vector.at(i).id == id_c.id) {
        a_index = i;
        i = a_vector.size();
      }
    }
    // Loop for b
    for (unsigned int i = 0; i < b_vector.size(); i++) {
      if (b_vector.at(i).id == id_c.id) {
        b_index = i;
        i = a_vector.size();
      }
    }

    // If indexation values were found for both
    // a and b we interpolate
    if ((a_index != -1) && (b_index != -1)) {
      this->InterpolateTransformComponent(transform_c, a_vector.at(a_index),
                                          b_vector.at(b_index));
    }

    // --- BIG WIP ---

    // Consider how to handle a object that does not
    // exist in a, butr is created to exist in b

    // Consider how to handle a object that exists
    // in a but disappears before b

    // Do other interpolations here

    // --- BIG WIP ---
  }
}

void GeometricReplay::InterpolateTransformComponent(TransformComponent& in_tc,
                                                    ObjectData& in_a,
                                                    ObjectData& in_b) {
  // Calculate how far we are (percentually)
  // through the interpolation window
  float coeff = (this->current_frame_read_ % this->interpolation_interval_) /
                this->interpolation_interval_;

  // - Calculate the difference between a and b
  // for each value
  // - Adjust the difference with the percentage
  // - Add the adjusted value to the corresponding
  // value of a
  // - Set the value to the transform component
  in_tc.position.x = in_a.pos[0] + coeff * (in_b.pos[0] - in_a.pos[0]);
  in_tc.position.y = in_a.pos[1] + coeff * (in_b.pos[1] - in_a.pos[1]);
  in_tc.position.z = in_a.pos[2] + coeff * (in_b.pos[2] - in_a.pos[2]);
  in_tc.rotation.x = in_a.rot[0] + coeff * (in_b.rot[0] - in_a.rot[0]);
  in_tc.rotation.y = in_a.rot[1] + coeff * (in_b.rot[1] - in_a.rot[1]);
  in_tc.rotation.z = in_a.rot[2] + coeff * (in_b.rot[2] - in_a.rot[2]);
  in_tc.rotation.w = in_a.rot[3] + coeff * (in_b.rot[3] - in_a.rot[3]);
}

// Public----------------------------------------------------------------------

GeometricReplay::GeometricReplay(
    unsigned int in_num_of_frames,
    unsigned int in_interpolation_interval_frames) {
  // Allocate space for the designated number of frames
  this->frame_arr_ = new FrameData[in_num_of_frames];
  this->num_of_frames_ = in_num_of_frames;
  this->current_frame_write_ = 0;
  this->current_frame_read_ = 0;
  this->start_frame_ = 0;
  this->end_frame_ = 0;

  // Save the interpolation interval for objects
  this->interpolation_interval_ = in_interpolation_interval_frames;
  this->frames_since_last_interpolation_ = in_interpolation_interval_frames;

  // ---
  this->interpolation_window_.a = 0;
  this->interpolation_window_.b = 0;
}

GeometricReplay::~GeometricReplay() {
  if (this->frame_arr_ != nullptr) {
    delete[] this->frame_arr_;
  }
}

bool GeometricReplay::SaveFrame(entt::registry& in_registry) {
  // Return true if the last frame has been reached
  if (this->current_frame_write_ == this->num_of_frames_) {
    return true;
  }

  // Check if we are to save data required for interpolation
  this->frames_since_last_interpolation_++;
  if (this->frames_since_last_interpolation_ >= this->interpolation_interval_) {
    this->frames_since_last_interpolation_ = 0;

    // If so, find all objects and save them
    // to the frame structure
    this->SaveObjectsToFrame(in_registry);
  }

  // Set last written frame to be the last frame of the replay
  this->end_frame_ = this->current_frame_write_;

  // Move forward until next write
  this->current_frame_write_++;

  // Return
  return false;
}

bool GeometricReplay::LoadFrame(entt::registry& in_registry) {
  // Return true if the last frame of the replay has been passed
  if (this->current_frame_read_ == this->end_frame_ + 1) {
    return true;
  }

  // Set up the interpolation window
  this->SetInterpolationWindow(this->current_frame_read_);

  // Do interpolation
  this->InterpolateFrame(in_registry);

  return false;
}
