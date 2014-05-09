////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "game.h"

////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////////////////////////

static const float PLAYER_SPEED = 40.0f;
static const float TURN_ANGLE = 0.05f;


////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////

Player player;

void InitLevel(void) {
   player = Player();
}

void UpdateCamera() {
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

void UpdatePlayers(R3Scene *scene, double delta_time) {
   // Turn the player
   player.direction.Rotate(R3zaxis_vector, player.turn * TURN_ANGLE);
   // Move the player
   player.position += player.direction * PLAYER_SPEED * delta_time * 0.1f;
}

void ToggleMovePlayer(int turn_dir) {
   if (player.turn == turn_dir) { player.turn = NOT_TURNING; }
   else { player.turn = turn_dir; }
}
