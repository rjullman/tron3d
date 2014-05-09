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
   R3Point eye = player.position - 4 * player.direction;
   float x = eye.X();
   float y = eye.Y();
   float dx = player.direction.X();
   float dy = player.direction.Y();

   gluLookAt(	x, y, 1.0f,
              x+dx, y+dy,  1.0f,
              0.0f, 0.0f,  1.0f);

   float angle = acos(R3xaxis_vector.Dot(player.direction));
   angle *= 180.0 / M_PI;
   if (player.direction.Y() < 0.0)
     angle = 360 - angle;

   // Follow player
   glColor3f(1.0f, 0.2f, 0.2f);
   glTranslatef(player.position.X(), player.position.Y(), 0.0f);
   glRotatef(angle, 0.0f, 0.0f, 1.0f);
   glutSolidCube(0.8f);
   glRotatef(-angle, 0.0f, 0.0f, 1.0f);
   glTranslatef(-(player.position.X()), -(player.position.Y()),0.0f);
}

bool UpdatePlayers(R3Scene *scene, double delta_time) {
   // Turn the player
   player.direction.Rotate(R3zaxis_vector, player.turn * TURN_ANGLE);
   // Move the player
   player.position += player.direction * PLAYER_SPEED * delta_time * 0.1f;

   // Check if inside boundaries
   float x = player.position.X();
   float y = player.position.Y();
   if (x <= -23 || x >= 23 || y <= -23 || y >= 23) {
    return false;
   }

   // Check for collisions with scene
   /*
   ////// Kill collisions (head on)
   R3Ray kill_ray(player.position, player.direction, normalized=true);
   if (CheckForSceneCollisions(scene, kill_ray).hit)
    return false;

   ////// On top of object collisions (beneath)
   R3Ray top_ray(player.position, R3Vector(0,0,-1), normalized=true);
   if (CheckForSceneCollisions(scene, top_ray).hit && CheckForSceneCollisions(scene, top_ray).position.Z() > 0.1)
    player.position.SetZ(4.0f);
   else
    player.position.SetZ(1.0f);
    */

  return true;
}

void ToggleMovePlayer(int turn_dir) {
   if (player.turn == turn_dir) { player.turn = NOT_TURNING; }
   else { player.turn = turn_dir; }
}
