////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "game.h"

////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////////////////////////

static const float PLAYER_SPEED = 40.0f;
static const float TURN_ANGLE = 0.25f;


////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////

Player player;

void InitLevel(void) {
   player = Player();
}

void GameUpdate() {
   // Set projection transformation
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(2*180.0*0.25/M_PI, (GLdouble) 1, 0.01, 10000);

   // Set the camera direction
   float x = player.position.X();
   float y = player.position.Y();
   float dx = player.direction.X();
   float dy = player.direction.Y();
   gluLookAt(	x, y, 1.0f,
              x+dx, y+dy,  1.0f,
              0.0f, 0.0f,  1.0f);
}

void MoveForward(void) {
   player.position += player.direction * PLAYER_SPEED * 0.01f;
}

void MoveLeft(void) {
   player.direction.Rotate(R3zaxis_vector, TURN_ANGLE);
}

void MoveRight(void) {
   player.direction.Rotate(R3zaxis_vector, -TURN_ANGLE);
}
