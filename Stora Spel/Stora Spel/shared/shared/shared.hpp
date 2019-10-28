#ifndef SHARED_HPP_
#define SHARED_HPP_

#define TEAM_RED (unsigned int)0
#define TEAM_BLUE (unsigned int)1
#define TEAM_NONE (unsigned int)3

#define POINTS_GOAL 10
#define POINTS_ASSIST 2
#define POINTS_SAVE 4

const double kClientUpdateRate = 64;
const double kServerUpdateRate = 64;
const unsigned kServerTimeout = 6;

enum class ServerStateType {
  LOBBY = 0,
  PLAY,
};

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
  SECONDARY_USED,
  MESSAGE,
  UPDATE_POINTS,
  CREATE_WALL,
  CREATE_PICK_UP,
  DESTROY_PICK_UP,
  RECEIVE_PICK_UP,
  LOBBY_UPDATE_TEAM,
  PLAYER_LOBBY_DISCONNECT,
  LOBBY_SELECT_TEAM,
  LOBBY_YOUR_ID,
  PING,
  PING_RECIEVE,
  LOBBY_SELECT_ABILITY,
  CREATE_PROJECTILE,
  DESTROY_ENTITIES,
  MATCH_TIMER,
  GAME_EVENT,
  PHYSICS_DATA,
  GAME_END,
  YOUR_TARGET,
  FRAME_ID,
  SERVER_CAN_JOIN,
  CREATE_BALL,
  CREATE_FAKE_BALL,
  SERVER_STATE,
  MY_NAME,
  PLAYER_LOOK_DIR,
  PLAYER_MOVE_DIR,
  TO_CLIENT_NAME,
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

struct MenuEvent {
  enum { HOVER, CLICK, NUM_EVENTS } type;
};

struct GameEvent {
  enum {
    GOAL = 0,
    KICK,
    HIT,
    NUDGE,
    BOUNCE,
    LAND,
    JUMP,
    SHOOT,
    GRAVITY_DROP,
    SUPER_KICK,
    MISSILE_FIRE,
    MISSILE_IMPACT,
    TELEPORT_CAST,
    TELEPORT_IMPACT,
    HOMING_BALL,
    FORCE_PUSH,
    FORCE_PUSH_IMPACT,
    SWITCH_GOALS,
    SWITCH_GOALS_DONE,
    SPRINT_START,
    SPRINT_END,
    RUN_START,
    RUN_END,
    RESET,
    BUILD_WALL,
    PRIMARY_USED,
    SECONDARY_USED,
    FAKE_BALL_CREATED,
    FAKE_BALL_POOF,
    NUM_EVENTS
  } type;
  union {
    // Goal
    struct {
    } goal;

    // Kick
    struct {
      EntityID player_id;
    } kick;

    // Hit
    struct {
      EntityID player_id;
    } hit;

    // Nudge
    struct {
      EntityID ball_id;
    } nudge;

    // Ball bounce
    struct {
      EntityID ball_id;
    } bounce;

    // Player Land
    struct {
      EntityID player_id;
    } land;

    // Player Jump
    struct {
      EntityID player_id;
    } jump;

    // Ability Gravity Change
    struct {
    } gravity;

    // Ability Missile Fire
    struct {
      EntityID projectile_id;
    } missile_fire;

    // Ability Missile Impact
    struct {
      EntityID projectile_id;
    } missile_impact;

    // Ability Teleport Cast
    struct {
      EntityID player_id;
    } teleport_cast;

    // Ability Teleport Impact
    struct {
      EntityID player_id;
    } teleport_impact;

    // Ability Super Kick
    struct {
      EntityID player_id;
    } super_kick;

    // Ability Homing Ball
    struct {
      EntityID ball_id;
    } homing_ball;

    // Ability Force Push
    struct {
      EntityID player_id;
    } force_push;

    // Ability Force Push Impact
    struct {
      EntityID projectile_id;
    } force_push_impact;

    // Ability Switch Goals
    struct {
    } switch_goals;

    // Player Sprint start
    struct {
      EntityID player_id;
    } sprint_start;

    // Player Sprint end
    struct {
      EntityID player_id;
    } sprint_end;

    // Player Run start
    struct {
      EntityID player_id;
    } run_start;

    // Player Run end
    struct {
      EntityID player_id;
    } run_end;

    // Player shoots a bullet
    struct {
      EntityID player_id;
    } shoot;

    // RESET
    struct {
    } reset;

    // BUILD WALL
    struct {
      EntityID wall_id;
    } build_wall;

    // Primary ability used
    struct {
      EntityID player_id;
      float cd;
    } primary_used;

    // Secondary ability used
    struct {
      EntityID player_id;
    } secondary_used;

    // ability fake ball created
    struct {
      EntityID ball_id;
    } fake_ball_created;
    // ability fake ball poofed
    struct {
      EntityID ball_id;
    } fake_ball_poofed;
  };
};

enum class ProjectileID {
  CANNON_BALL,
  TELEPORT_PROJECTILE,
  FORCE_PUSH_OBJECT,
  MISSILE_OBJECT,
  NUM_PROJECTILE_IDS
};

struct Projectile {
  EntityID entity_id;
  ProjectileID projectile_id;
};
#endif  // SHARED_HPP_