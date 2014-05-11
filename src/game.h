#ifndef GAME_H
#define GAME_H

////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "R2/R2.h"
#include "R3/R3.h"
#include "R3Scene.h"
#include "fglut/fglut.h"


////////////////////////////////////////////////////////////
// PLAYER DEFINITION
////////////////////////////////////////////////////////////

enum {
   NOT_TURNING = 0,
   TURNING_LEFT = 1,
   TURNING_RIGHT = -1
};

struct Player {
   public:
      Player(bool is_ai);

      bool IsAI();

      R3Point position;
      R3Vector direction;
      R3Mesh *mesh;
      bool dead;
      bool is_ai;
      int turn;
};

inline bool Player::IsAI() { return is_ai; }


////////////////////////////////////////////////////////////
// API API
////////////////////////////////////////////////////////////

void InitGame();
void InitLevel(int human_players, int ai_players);

void GameUpdate(void);
void UpdateCamera(Player *player);
void UpdatePlayer(R3Scene *scene, Player *player, double delta_time);

bool Collide_Box(R3Scene *scene, R3Node *node, R3Point testpoint);
bool Collide_Scene(R3Scene *scene, R3Node *node, R3Point testpoint);
bool Collide_Trails(Player *player, R3Point testpoint);
bool Collide_Point(R3Point testpoint, R3Point trailpoint);

void DrawPlayer(Player *player);

void ToggleMovePlayer(int player_num, int turn_dir);
void MovePlayer(int player_num, int turn_dir);

#endif
