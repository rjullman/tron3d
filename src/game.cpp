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
   R3Point eye = player->position - 4 * player->direction;
   float x = eye.X();
   float y = eye.Y();
   float dx = player->direction.X();
   float dy = player->direction.Y();

   gluLookAt(	x, y, 1.0f,
              x+dx, y+dy,  1.0f,
              0.0f, 0.0f,  1.0f);

   float angle = acos(R3xaxis_vector.Dot(player->direction));
   angle *= 180.0 / M_PI;
   if (player->direction.Y() < 0.0)
     angle = 360 - angle;

   // Follow player
   glColor3f(1.0f, 0.2f, 0.2f);
   glTranslatef(player->position.X(), player->position.Y(), 0.0f);
   glRotatef(angle, 0.0f, 0.0f, 1.0f);
   //glutSolidCube(0.8f);
   //R3Box(-.5,-.5,0,.5,.5,.5).Draw();
   player->mesh.Draw();
   glRotatef(-angle, 0.0f, 0.0f, 1.0f);
   glTranslatef(-(player->position.X()), -(player->position.Y()),0.0f);
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
