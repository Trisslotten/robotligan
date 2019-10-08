#ifndef SHARED_HPP_
#define SHARED_HPP_

#define TEAM_RED (unsigned int)0
#define TEAM_BLUE (unsigned int)1

#define POINTS_GOAL		10
#define POINTS_ASSIST	2
#define POINTS_SAVE		4

const double kClientUpdateRate = 64;
const double kServerUpdateRate = 64;

typedef int EntityID;

// deprecated
typedef int PlayerID;

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
  ENTITY_TRANSFORMS,
  PLAYER_STAMINA,
  CAMERA_TRANSFORM,
  CLIENT_READY,      // client is ready in lobby
  CLIENT_NOT_READY,  // client is not ready in lobby
  GAME_START,        // game start after lobby
  CLIENT_RECEIVE_UPDATES, 
  TEST_STRING,
  TEST_REPLAY_KEYS,
  TEAM_SCORE,
  CHOOSE_TEAM,
  SWITCH_GOALS,
  MESSAGE,
  UPDATE_POINTS,
  CREATE_PICK_UP,
  DESTROY_PICK_UP,
  RECEIVE_PICK_UP,
  LOBBY_UPDATE_TEAM,
  LOBBY_SELECT_TEAM,
  LOBBY_YOUR_ID,
  NUM_BLOCK_TYPES,
};

}  // namespace PacketBlockType

enum class AbilityID {
  NULL_ABILITY,
  BUILD_WALL,
  FAKE_BALL,
  FORCE_PUSH,
  GRAVITY_CHANGE,
  HOMING_BALL,
  INVISIBILITY,
  MISSILE,
  SUPER_STRIKE,
  SWITCH_GOALS,
  TELEPORT,
  // Fill with more abilities and passive boosts
  NUM_OF_ABILITY_IDS
};

#endif  // SHARED_HPP_