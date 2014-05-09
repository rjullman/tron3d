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
   R3Mesh mesh;
   bool dead;
   int turn;
};

inline Player::
Player(void)
   : position(R3Point(0,0,0)),
     direction(R3Vector(1.0f, 0.0f, 0.0f)),
     mesh(R3Mesh()),
     dead(false),
     turn(NOT_TURNING)
{
  mesh.Read("../bikes/m1483.off");
  R3Point middle(0,0,0);
  R3Line axisy(middle, R3yaxis_vector, true);
  mesh.Rotate(-M_PI/2, axisy);
  R3Line axisx(middle, R3xaxis_vector, true);
  mesh.Rotate(M_PI/2, axisx);
  R3Line axisz(middle, R3zaxis_vector, true);
  mesh.Rotate(M_PI, axisz);
  mesh.Translate(0,-0.3,0);
}
////////////////////////////////////////////////////////////
// Game API
////////////////////////////////////////////////////////////

void InitLevel(int num_players);
void GameUpdate(void);
void UpdateCamera(Player *player);
void UpdatePlayer(R3Scene *scene, Player *player, double delta_time);

void ToggleMovePlayer(Player *player, int turn_dir);

#endif
