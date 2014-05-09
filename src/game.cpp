////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "game.h"

////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////////////////////////

static const float PLAYER_SPEED = 35.0f;
static const float TURN_SPEED = 5.0f;


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

   // Follow player
   float angle = acos(R3xaxis_vector.Dot(player->direction));
   angle *= 180.0 / M_PI;
   if (player->direction.Y() < 0.0)
     angle = 360 - angle;

   glEnable(GL_COLOR_MATERIAL);
   glColor3f(0.5f, 0.4f, 0.0f);
   glTranslatef(player->position.X(), player->position.Y(), 0.0f);
   glRotatef(angle, 0.0f, 0.0f, 1.0f);
   //glutSolidCube(0.8f);
   //R3Box(-.5,-.5,0,.5,.5,.5).Draw();
   player->mesh.Draw();
   glRotatef(-angle, 0.0f, 0.0f, 1.0f);
   glTranslatef(-(player->position.X()), -(player->position.Y()),0.0f);
   glDisable(GL_COLOR_MATERIAL);
}

void UpdatePlayer(R3Scene *scene, Player *player, double delta_time) {
   // Turn the player
   player->direction.Rotate(R3zaxis_vector,
			    player->turn * TURN_SPEED * delta_time);
   // Move the player
   player->position += player->direction * PLAYER_SPEED * delta_time;

   // TODO: Check for collisions
   // On death set player->dead = true
}

void ToggleMovePlayer(Player *player, int turn_dir) {
   if (player->turn == turn_dir) { player->turn = NOT_TURNING; }
   else { player->turn = turn_dir; }
}
