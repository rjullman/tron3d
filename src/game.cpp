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

extern vector<Player> players;

void InitLevel(int num_players) {
   players.clear();
   for (int i = 0; i < num_players; i++) {
      players.push_back(Player());
   }
}

void UpdateCamera(Player *player) {
   // Set projection transformation
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(360.0/M_PI*0.25, (GLdouble) 1, 0.01, 10000);

   // Set the camera direction
   float x = player->position.X();
   float y = player->position.Y();
   float dx = player->direction.X();
   float dy = player->direction.Y();
   gluLookAt(	x, y, 1.0f,
              x+dx, y+dy,  1.0f,
              0.0f, 0.0f,  1.0f);
}

void UpdatePlayer(R3Scene *scene, Player *player, double delta_time) {
   // Turn the player
   player->direction.Rotate(R3zaxis_vector, player->turn * TURN_ANGLE);
   // Move the player
   player->position += player->direction * PLAYER_SPEED * delta_time * 0.1f;

   // TODO: Check for collisions
   // On death set player->dead = true
}

void ToggleMovePlayer(Player *player, int turn_dir) {
   if (player->turn == turn_dir) { player->turn = NOT_TURNING; }
   else { player->turn = turn_dir; }
}
