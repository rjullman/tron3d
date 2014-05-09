#ifndef GAME_H
#define GAME_H

////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "R2/R2.h"
#include "R3/R3.h"
#include "R3Scene.h"


////////////////////////////////////////////////////////////
// Player Defintiion
////////////////////////////////////////////////////////////

enum {
   NOT_TURNING = 0,
   TURNING_LEFT = 1,
   TURNING_RIGHT = -1
};

struct Player {
 public:
   Player(void);

   R3Point position;
   R3Vector direction;
   int turn;
};

inline Player::
Player(void)
   : position(R3Point(0,0,0)),
     direction(R3Vector(1.0f, 0.0f, 0.0f)),
     turn(NOT_TURNING)
{
}

////////////////////////////////////////////////////////////
// Game API
////////////////////////////////////////////////////////////

void InitLevel(void);
void GameUpdate(void);
void UpdateCamera(void);
bool UpdatePlayers(R3Scene *scene, double delta_time);


void ToggleMovePlayer(int turn_dir);

#endif
