#include "geometric_replay.hpp"

#include <map>

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------

GeometricReplay::GeometricReplay() {}

GeometricReplay::~GeometricReplay() {}

bool GeometricReplay::SaveFrame(entt::registry& in_registry) {
  /* Reverse strat
        



  // Create a map for sorting through entities based on id
  std::map<EntityID, entt::entity> map;

  // Add all entities with an IDComponent to a map
  in_registry.view<IDComponent>().each(
      [&](auto entity, IDComponent& id_c) { map[id_c.id] = entity; });

  // Go through all channels that exist and determine
  // if the objects they are tracking exists in the map
  */

  // Loop over all entries with an id component
  entt::basic_view view = in_registry.view<IDComponent>();
  for (entt::entity entity : view) {
    IDComponent& id_c = in_registry.get<IDComponent>(entity);

    // Check if entity with that id has a channel
    bool unfound = true;
    for (unsigned int i = 0; i < this->channels_.size && unfound; i++) {
      if (id_c.id == this->channels_.at(i).object_id) {
        unfound = false;
        // If it does we check if a new interpolation point should be added
        // dependent on the object's type

        /*
                WIP:
                Add switch case for object types(?)
                Add call to threshold function
                If object should be added save frame number as well
        */
      }
    }

    // If after looping through an object still hasn't been found
    // it should be added to its own channel
    if (unfound) {
		/*
				WIP:
				Add new channel
		*/
	}
  }

  return false;
}