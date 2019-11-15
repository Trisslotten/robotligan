#ifndef ASSERT_MODULE_HPP_
#define ASSERT_MODULE_HPP_

#include <shared/reg_pack.hpp>

enum AssertMode {
  ASSERT_NOTHING,               //0
  ASSERT_ABILITY_COMPONENTS,    //1
  ASSERT_PLAYER_COMPONENTS,     //2
  ASSERT_TRANSFORM_COMPONENTS,  //3
  ASSERT_ALL                    //4
};

class AssertModule {
 private:
  RegPack* assert_log_;
  AssertMode mode_;

  // Functions	:	Assert mode comparasions
  bool AssertAbilityComponents(entt::registry& in_registry);
  bool AssertPlayerComponents(entt::registry& in_registry);
  bool AssertTransformComponents(entt::registry& in_registry);

 public:
  AssertModule(unsigned int in_num_of_frames);
  ~AssertModule();

  void SnapshotRegistry(entt::registry& in_registry);
  bool AssertRegistry(entt::registry& in_registry);
};

#endif  // !ASSERT_MODULE_HPP_
