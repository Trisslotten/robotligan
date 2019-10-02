#ifndef SHARED_HPP_
#define SHARED_HPP_

#define TEAM_RED (unsigned int)0
#define TEAM_BLUE (unsigned int)1

typedef int PlayerID;
typedef int EntityID;

namespace PlayerAction {
enum : int16_t {
  WALK_FORWARD = 0,
  WALK_BACKWARD,
  WALK_LEFT,
  WALK_RIGHT,
  ABILITY_PRIMARY,
  ABILITY_SECONDARY,
  SPRINT,
  JUMP,
  SHOOT,
  KICK,
  NUM_ACTIONS,
};
}  // namespace PlayerAction

namespace PacketBlockType {
enum : int16_t {
  INPUT = 0,
  SET_CLIENT_PLAYER_ID,
  CREATE_PLAYER,
  PLAYER_STAMINA,
  PLAYERS_TRANSFORMS,
  CAMERA_TRANSFORM,
  BALL_TRANSFORM,
  TEST_STRING,
  TEAM_SCORE,
  SWITCH_GOALS,
  NUM_BLOCK_TYPES,
  MESSAGE,
};
} // namespace PacketType

#endif  // SHARED_HPP_