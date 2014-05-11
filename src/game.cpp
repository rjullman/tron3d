////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "game.h"

////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////////////////////////

static const float PLAYER_SPEED = 15.0f;
static const float TURN_SPEED = 5.0f;

static const char* BIKE_MESH_LOC = "../bikes/m1483.off";
R3Mesh bike;

////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////

extern vector<Player> players;


////////////////////////////////////////////////////////////
// PLAYER IMPLEMENTATION
////////////////////////////////////////////////////////////

Player::
Player(bool is_ai)
    : position(R3Point(0,0,0)),
      direction(R3Vector(1.0f, 0.0f, 0.0f)),
      mesh(NULL),
      dead(false),
      is_ai(is_ai),
      turn(NOT_TURNING)
{
   // Currently no bike mesh options
   mesh = &bike;
}


////////////////////////////////////////////////////////////
// GAME IMPLEMENTATION
////////////////////////////////////////////////////////////

void InitGame() {
   // Load bike mesh
   bike.Read(BIKE_MESH_LOC);
   bike.Rotate(-M_PI/2, R3yaxis_line);
   bike.Rotate(M_PI/2, R3xaxis_line);
   bike.Rotate(M_PI, R3zaxis_line);
   bike.Translate(0,-0.3,0);
}

void InitLevel(int human_players, int ai_players) {
   players.clear();
   for (int i = 0; i < human_players + ai_players; i++) {
      bool ai = i >= (human_players);
      players.push_back(Player(ai));
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

void DrawPlayer(Player *player) {
   // Follow player
   float angle = acos(R3xaxis_vector.Dot(player->direction));
   angle *= 180.0 / M_PI;
   if (player->direction.Y() < 0.0)
     angle = 360 - angle;

   // TODO: Set player color
   glEnable(GL_COLOR_MATERIAL);
   glColor3f(0.5f, 0.4f, 0.0f);

   // Display bike
   glPushMatrix();
   glTranslatef(player->position.X(), player->position.Y(), 0.0f);
   glRotatef(angle, 0.0f, 0.0f, 1.0f);
   player->mesh->Draw();
   glPopMatrix();

   glDisable(GL_COLOR_MATERIAL);
}

void ToggleMovePlayer(int player_num, int turn_dir) {
   if ((unsigned int)player_num >= players.size()) { return; }
   if (players[player_num].turn == turn_dir) {
      players[player_num].turn = NOT_TURNING;
   }
   else { players[player_num].turn = turn_dir; }
}

void MovePlayer(int player_num, int turn_dir) {
   if (player_num >= players.size()) { return; }
   players[player_num].turn = turn_dir;
}
