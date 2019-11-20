#ifndef MINE_COMPONENT_HPP_
#define MINE_COMPONENT_HPP_

typedef int EntityID;

struct MineComponent {
  EntityID owner_id;
};

#endif  // MINE_COMPONENT_HPP_