////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "game.h"
#include "irrKlang/include/irrKlang.h"

////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////////////////////////

static const float PLAYER_SPEED = 7.0f;
static const float TURN_SPEED = 2.0f;
static const float PATH_WIDTH = 0.01f;

static const float TRAIL_DIAMETER = 0.05;

static Color PLAYER_COLORS[] = {
   Color(1.0,0.5,0.0),
   Color(0.0,0.0,1.0),
   Color(0.0,1.0,0.0),
   Color(1.0,0.0,0.0)
};

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
Player(Color color, bool is_ai, R3Point position, R3Vector direction)
    : position(position),
      direction(direction),
      mesh(NULL),
      color(color),
      dead(false),
      is_ai(is_ai),
      turn(NOT_TURNING),
      trail(vector<R3Point>())
{
   // Currently no bike mesh options
   mesh = &bike;
}


////////////////////////////////////////////////////////////
// GAME IMPLEMENTATION
////////////////////////////////////////////////////////////

void InitGame() {
   // Load bike mesh
   irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();
   //engine->play2D("ridindirty.mp3", true);
   /*bike.Read(BIKE_MESH_LOC);
   bike.Translate(-0.19,0,0);
   bike.Rotate(-M_PI/2, R3yaxis_line);
   bike.Rotate(M_PI/2, R3xaxis_line);
   bike.Rotate(M_PI, R3zaxis_line);*/
}


void InitLevel(int human_players, int ai_players) {
   players.clear();
   for (int i = 0; i < human_players + ai_players; i++) {
      bool ai = i >= (human_players);
      R3Point startposition(0,0,0.5);
      R3Vector startdirection(1,0,0);

      if (i == 1) {
         startposition = R3Point(18,0,0.5);
         startdirection = R3Vector(-1,0,0);
      }



      players.push_back(Player(PLAYER_COLORS[i], ai, startposition, startdirection));
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

   if (delta_time > 1) {
   fprintf(stderr, "%g\n", delta_time);
   fprintf(stderr, "%g %g %g\n", player->position[0], player->position[1], player->position[2]);
   }

   // Continue the trail
   player->trail.push_back(player->position);
}

void Check_Collisions(R3Scene *scene, Player *player) {
   // Test for Collisions
   vector<R3Point> testpoints;
   testpoints.push_back(player->position + 0.2 * player->direction);

   //R3Vector side_direction = R3zaxis_vector;
   //side_direction.Cross(player->direction);
   //testpoints.push_back(player->position + 1.5 * player->direction + 0.3 * side_direction);
   //testpoints.push_back(player->position + 1.5 * player->direction - 0.3 * side_direction);

   // Check for collisions in scene
   for (unsigned int i = 0; i < testpoints.size(); i++) {
      if (Collide_Scene(scene, scene->root, testpoints[i]))
         player->dead = true;
      else {
      // Check for collisions with laid paths
         for (unsigned int j = 0; j < players.size(); j++) {
            if (Collide_Trails(&players[j], testpoints[i]))
               player->dead = true;
         }
      }
   }
}

bool Collide_Box(R3Scene *scene, R3Node *node, R3Point testpoint) {
   R3Box scene_box = *node->shape->box;

   if (testpoint.X() >= scene_box.XMin()
          && testpoint.X() <= scene_box.XMax()
          && testpoint.Y() >= scene_box.YMin()
          && testpoint.Y() <= scene_box.YMax()
          && testpoint.Z() >= scene_box.ZMin()
          && testpoint.Z() <= scene_box.ZMax())
      return true;
   else
      return false;
}

bool Collide_Scene(R3Scene *scene, R3Node *node, R3Point testpoint) {

   if (node->shape != NULL && node->shape->type == R3_BOX_SHAPE) {
      if (Collide_Box(scene, node, testpoint))
         return true;
   }

   // Check for collision with children nodes
   for (unsigned int i = 0; i < node->children.size(); i++) {
      if (Collide_Scene(scene, node->children[i], testpoint))
         return true;
   }

   return false;
}

bool Collide_Trails(Player *player, R3Point testpoint) {
   for (unsigned int i = 1; i < player->trail.size(); i++) {
      if (Collide_Point(testpoint, player->trail[i-1], player->trail[i], player->direction))
         return true;
   }
   return false;
}

bool Collide_Point(R3Point testpoint, R3Point trailpoint1, R3Point trailpoint2, R3Vector direction) {
   R3Vector to = trailpoint2 - trailpoint1;
   R3Vector d = testpoint - trailpoint1;

   R3Vector normal = R3zaxis_vector;
   normal.Cross(to);

   if (d.Dot(normal) > 0)
      normal = -normal;

   float distance = (d).Dot(normal);

   R3Point p = testpoint + direction * distance;

   if (distance <= PATH_WIDTH && R3Distance(p, trailpoint1) <= R3Distance(trailpoint1, trailpoint2))
      return true;
   else
      return false;
}

void DrawPlayer(Player *player) {
   // Follow player
   float angle = acos(R3xaxis_vector.Dot(player->direction));
   angle *= 180.0 / M_PI;
   if (player->direction.Y() < 0.0)
     angle = 360 - angle;

   glEnable(GL_COLOR_MATERIAL);
   glColor3d(player->color.R(), player->color.G(), player->color.B());

   // Display bike
   glPushMatrix();
   glTranslatef(player->position.X(), player->position.Y(), 0.0f);
   glRotatef(angle, 0.0f, 0.0f, 1.0f);
   glutSolidCube(0.5f);
   //player->mesh->Draw();
   glPopMatrix();

   glDisable(GL_COLOR_MATERIAL);

}

void DrawTrail(Player *player) {
//   glColor3d(player->color.R(), player->color.G(), player->color.B());

   static GLUquadricObj *glu_sphere = gluNewQuadric();
   gluQuadricTexture(glu_sphere, GL_TRUE);
   gluQuadricNormals(glu_sphere, (GLenum) GLU_SMOOTH);
   gluQuadricDrawStyle(glu_sphere, (GLenum) GLU_FILL);

   for (int i = 1; i < player->trail.size(); i++) {
      // Make cylinder between p1 and p2
      // See: http://www.thjsmith.com/40/cylinder-between-two-points-opengl-c
      R3Point p1 = player->trail[i-1];
      R3Point p2 = player->trail[i];

      // Compute direction and angle of rotation from standard
      R3Vector p = (p2 - p1);
      R3Vector t = R3Vector(R3zaxis_vector);
      t.Cross(p);
      double angle = 180 / M_PI * acos (R3zaxis_vector.Dot(p) / p.Length());

      // Draw cylinder
      glPushMatrix();
      glTranslated(p1.X(),p1.Y(),p1.Z());
      glRotated(angle,t.X(),t.Y(),t.Z());
      gluCylinder(glu_sphere, TRAIL_DIAMETER, TRAIL_DIAMETER, p.Length(), 32, 32);
      glPopMatrix();
   }
}

void MovePlayer(int player_num, int turn_dir) {
   if ((unsigned int) player_num >= players.size()) { return; }
   players[player_num].turn = turn_dir;
}


