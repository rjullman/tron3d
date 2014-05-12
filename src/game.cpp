////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "game.h"
#include "irrKlang/include/irrKlang.h"

////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////////////////////////

static const float PLAYER_SPEED = 7.0f;
static const float TURN_SPEED = 3.0f;
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
   bike.Read(BIKE_MESH_LOC);
   bike.Translate(-0.19,0,0);
   bike.Rotate(-M_PI/2, R3yaxis_line);
   bike.Rotate(M_PI/2, R3xaxis_line);
   bike.Rotate(M_PI, R3zaxis_line);
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

void Check_Collisions(R3Scene *scene, Player *player, double delta_time) {
   // Test for Collisions
   R3Point testpoint = player->position;
   R3Point nextpoint = player->position + delta_time * PLAYER_SPEED * player->direction;

   // Check for collisions in scene
   if (Collide_Scene(scene, scene->root, testpoint)) {
      player->dead = true;
   }
   else {
      // Check for collisions with laid paths
      for (unsigned int j = 0; j < players.size(); j++) {
         if (Collide_Trails(&players[j], testpoint, nextpoint)) {
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

bool Collide_Trails(Player *player, R3Point testpoint, R3Point nextpoint) {
   for (unsigned int i = 1; i < player->trail.size(); i++) {
      if (Segment_Intersection(testpoint, nextpoint, player->trail[i-1], player->trail[i]))
         return true;
   }
   return false;
}

/*bool Collide_Point(R3Point testpoint, R3Point trailpoint1, R3Point trailpoint2, R3Vector direction) {
   R3Vector to = trailpoint2 - trailpoint1;
   R3Vector d = testpoint - trailpoint1;

   R3Vector normal = R3zaxis_vector;
   normal.Cross(to);

  // if (d.Dot(normal) > 0)
   //   normal = -normal;

   float distance = abs((d).Dot(normal));

   //R3Point p = testpoint + direction * distance;
   double interval = R3Distance(trailpoint1, trailpoint2);
   R3Point testpoint2 = testpoint + 1 * direction;
   R3Vector d2 = testpoint2 - trailpoint1;



      //if (R3Distance(testpoint, trailpoint1) <= interval && R3Distance(testpoint, trailpoint2) <= interval)

   if ((distance <= PATH_WIDTH && R3Distance(testpoint, trailpoint1) <= interval) || (d.Dot(normal) * d2.Dot(normal) < 0) && R3Distance(testpoint, trailpoint1) <= interval)
      return true;
   else
      return false;
}*/

/*bool Collide_Point(Player *player, R3Point testpoint, R3Point trailpoint1, R3Point trailpoint2) {

   // 1)
   R3Point nextpoint = testpoint + player->direction * PLAYER_SPEED * 0.05; // need to pass delta_time

   return Segment_Intersection(testpoint, nextpoint, trailpoint1, trailpoint2);

   // 2)
   R3Vector to = trailpoint2 - trailpoint1;
   R3Vector normal = R3zaxis_vector;
   normal.Cross(to);

   R3Vector d1 = trailpoint1 - testpoint;
   R3Vector d2 = trailpoint1 - nextpoint;

   // 3)
   if (d1.Dot(normal) * d2.Dot(normal) < -.00001) {
      // printf("here\n");
      // printf("%f\n", d1.Dot(normal)*d2.Dot(normal));
      // 4)
      // trailpoint1 = planePoint
      // d1 = planeVector
      double t = (d1.Dot(normal)) / (player->direction.Dot(normal));
      printf("%f\n", t);
      R3Point projected = testpoint + t * player->direction;

      // 5)
      R3Vector a = trailpoint1 - projected;
      R3Vector b = trailpoint2 - projected;

      if (a.Dot(b) < 0)
         return true;
   }

   return false;
}*/

bool Segment_Intersection(R3Point p1, R3Point p2, R3Point p3, R3Point p4) {
   double x1 = p1.X();
   double x2 = p2.X();
   double x3 = p3.X();
   double x4 = p4.X();

   double y1 = p1.Y();
   double y2 = p2.Y();
   double y3 = p3.Y();
   double y4 = p4.Y();

   double d = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
   if (d == 0) return false;

   double xi = ((x3-x4)*(x1*y2-y1*x2)-(x1-x2)*(x3*y4-y3*x4))/d;
   //double yi = ((y3-y4)*(x1*y2-y1*x2)-(y1-y2)*(x3*y4-y3*x4))/d;

   if (xi < MIN(x1,x2) || xi > MAX(x1,x2)) return false;
   if (xi < MIN(x3,x4) || xi > MAX(x3,x4)) return false;

   return true;
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
   //glutSolidCube(0.5f);
   player->mesh->Draw();
   glPopMatrix();

   glDisable(GL_COLOR_MATERIAL);

}

void DrawTrail(Player *player) {
//   glColor3d(player->color.R(), player->color.G(), player->color.B());

   static GLUquadricObj *glu_sphere = gluNewQuadric();
   gluQuadricTexture(glu_sphere, GL_TRUE);
   gluQuadricNormals(glu_sphere, (GLenum) GLU_SMOOTH);
   gluQuadricDrawStyle(glu_sphere, (GLenum) GLU_FILL);

   for (unsigned int i = 1; i < player->trail.size(); i++) {
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


