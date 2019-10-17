#ifndef PICK_UP_EVENT_HPP_
#define PICK_UP_EVENT_HPP_

#include "shared/shared.hpp"

struct PickUpEvent {
  EntityID pick_up_id;
  long client_id;
  AbilityID ability_id;
};

#endif  // PICK_UP_EVENT_HPP_