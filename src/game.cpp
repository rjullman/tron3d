////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "game.h"
#include "irrKlang/include/irrKlang.h"
#include "fglut/fglut.h"

////////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////////////////////////

static const float PLAYER_SPEED = 3.0f;
static const float TURN_SPEED = 3.0f;
static const float AI_TURN_SPEED = M_PI/2.0;
static const float PATH_WIDTH = 0.01f;

static const float TRAIL_DIAMETER = 0.05;
static const float MAX_PIPE_DETAIL = 32;
static const float MIN_PIPE_DETAIL = 3;

static const double BIKE_HEIGHT = 1.0;

static const double FULL_FUEL = 2.0;
static const double USE_FUEL_RATE = 2.0;
static const double CHARGE_FUEL_RATE = 1.0;

static Color PLAYER_COLORS[] = {
   Color(1.0,0.5,0.0),
   Color(0.0,0.0,.75),
   Color(0.0,1.0,0.0),
   Color(1.0,0.0,0.0)
};

static const char* BIKE_MESH_LOC = "../bikes/m1483.off";
R3Mesh bike;

////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////

extern vector<Player> players;
extern int GLUTwindow_height;
extern int GLUTwindow_width;
static double level_size;

// MUSIC: irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();

////////////////////////////////////////////////////////////
// PLAYER IMPLEMENTATION
////////////////////////////////////////////////////////////

Player::
Player(Color color, bool is_ai, R3Point position, R3Vector direction, int view)
    : position(position),
      direction(direction),
      view(view),
      mesh(NULL),
      color(color),
      dead(false),
      is_ai(is_ai),
      turn(NOT_TURNING),
      jumping(false),
      fuel_time(FULL_FUEL),
      trail(vector<R3Point>())
{
   // Currently no bike mesh options
   mesh = &bike;
}


////////////////////////////////////////////////////////////
// GAME IMPLEMENTATION
////////////////////////////////////////////////////////////

void InitGame() {
   // Load Sound
   // MUSIC: engine->play2D("ridindirty.mp3", true);

   // Load bike mesh
   bike.Read(BIKE_MESH_LOC);
   bike.Translate(-0.19,0,0);
   bike.Rotate(-M_PI/2, R3yaxis_line);
   bike.Rotate(M_PI/2, R3xaxis_line);
   bike.Rotate(M_PI, R3zaxis_line);
}


void InitLevel(int human_players, int ai_players,
	       int view, double size, vector<R3Point> init_positions,
          vector<R3Vector> init_directions) {
   level_size = size;

   players.clear();
   for (int i = 0; i < human_players + ai_players; i++) {
      bool ai = i >= (human_players);
      R3Point startposition = init_positions[i];
      R3Vector startdirection = init_directions[i];

      players.push_back(Player(PLAYER_COLORS[i], ai, startposition,
			       startdirection, view));
   }
}

R3Point ComputeEye(Player *player, int camera_perspective) {
   if (camera_perspective == OVER_THE_SHOULDER)
      return player->position - 5 * player->direction;
   else if (camera_perspective == FIRST_PERSON)
      return player->position + 0.85 * player->direction;

   assert(false);
   return NULL;
}

void UpdateCamera(Player *player, int camera_perspective) {
   // Set projection transformation
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(360.0/M_PI*0.25, (GLdouble) 1, 0.01, 10000);

   // Set the camera direction
   R3Point eye = ComputeEye(player, camera_perspective);
   float x = eye.X();
   float y = eye.Y();
   float dx = player->direction.X();
   float dy = player->direction.Y();

   if (camera_perspective == OVER_THE_SHOULDER) {
      gluLookAt(  x, y, 1.4f,
           x+dx, y+dy,  1.4f,
           0.0f, 0.0f,  1.0f);
   }
   else if (camera_perspective == FIRST_PERSON) {
      gluLookAt(  x, y, 0.5f,
           x+dx, y+dy,  0.5f,
           0.0f, 0.0f,  1.0f);
   }
}


void UpdatePlayer(R3Scene *scene, Player *player, double delta_time) {
   if (player->is_ai) {
      if (Check_Collisions(scene, player, delta_time, CHECK_FRONT, 100)) {
         if (Check_Collisions(scene, player, delta_time, CHECK_LEFT, 50) && !(Check_Collisions(scene, player, delta_time, CHECK_RIGHT, 50))) {
            player->direction.Rotate(R3zaxis_vector,
                1 * AI_TURN_SPEED);
            player->position += player->direction * PLAYER_SPEED * delta_time;
         }
         else if (!(Check_Collisions(scene, player, delta_time, CHECK_LEFT, 50)) && (Check_Collisions(scene, player, delta_time, CHECK_RIGHT, 50))) {
            player->direction.Rotate(R3zaxis_vector,
                -1 * AI_TURN_SPEED);
            player->position += player->direction * PLAYER_SPEED * delta_time;
         }
         else {
            player->direction.Rotate(R3zaxis_vector,
                1 * AI_TURN_SPEED);
            player->position += player->direction * PLAYER_SPEED * delta_time;
         }
      }
      else {
         player->position += player->direction * PLAYER_SPEED * delta_time;
      }
   }
   else {
      // Jump the player between 2 heights (ground and levitate)
      if (player->jumping) {
	 player->fuel_time -= delta_time * USE_FUEL_RATE;
	 player->position.SetZ(BIKE_HEIGHT);
	 if (player->fuel_time <= 0) {
	    player->fuel_time = 0;
	    player->jumping = false;
	 }
      } else {
	 player->fuel_time += delta_time * CHARGE_FUEL_RATE;
	 player->fuel_time = MIN(FULL_FUEL, player->fuel_time);
	 player->position.SetZ(BIKE_HEIGHT/2);
      }
      
      // Turn the player
      player->direction.Rotate(R3zaxis_vector,
   			    player->turn * TURN_SPEED * delta_time);
      // Move the player
      player->position += player->direction * PLAYER_SPEED * delta_time;
   }

   // Continue the trail
   player->trail.push_back(player->position);
}

bool Check_Collisions(R3Scene *scene, Player *player, double delta_time, int for_decisions, int precision) {
   R3Point nextpoint(0,0,0.5);

   // Test for Collisions
   R3Point testpoint = player->position + 0.85 * player->direction;
   if (for_decisions == NORMAL) {
      nextpoint = player->position + 0.85 * player->direction + delta_time * PLAYER_SPEED * player->direction;
   }
   else if (for_decisions == CHECK_FRONT)  {
      nextpoint = player->position + delta_time * PLAYER_SPEED * player->direction * 10;
   }
   else if (for_decisions == CHECK_LEFT) {
      R3Vector side = R3zaxis_vector;
      side.Cross(player->direction);
      nextpoint = player->position + delta_time * PLAYER_SPEED * side * precision;
   }
   else if (for_decisions == CHECK_RIGHT) {
      R3Vector side = R3zaxis_vector;
      side.Cross(player->direction);
      nextpoint = player->position - delta_time * PLAYER_SPEED * side * precision;
   }

   // Check for collisions in scene
   if (for_decisions == CHECK_FRONT)
      testpoint = nextpoint;
   if (Collide_Scene(scene, scene->root, testpoint)) {
      if (for_decisions == NORMAL) {
         player->dead = true;
         //MUSIC: engine->play2D("crash.wav");
      }
      return true;
   }
   else {
      if (for_decisions == CHECK_FRONT)
         testpoint = player->position;

      // Check for collisions with laid paths
      for (unsigned int j = 0; j < players.size(); j++) {
         if (Collide_Trails(&players[j], testpoint, nextpoint)) {
            if (for_decisions == NORMAL) {
               player->dead = true;
               // MUSIC: engine->play2D("crash.wav");
            }
            return true;
         }
      }
   }
   return false;
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

bool Segment_Intersection(R3Point p1, R3Point p2, R3Point p3, R3Point p4) {
   double x1 = p1.X();
   double x2 = p2.X();
   double x3 = p3.X();
   double x4 = p4.X();

   double y1 = p1.Y();
   double y2 = p2.Y();
   double y3 = p3.Y();
   double y4 = p4.Y();

   double z1 = (p1 + (p2-p1)/2).Z();
   double z2 = (p3 + (p4-p3)/2).Z();

   double d = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
   if (d == 0) {
      R3Vector dir = p2 - p1;
      float t2 = 1.f;
      float t3 = abs((p3 - p1).Dot(dir) / dir.Dot(dir));
      float t4 = abs((p4 - p1).Dot(dir) / dir.Dot(dir));

      if (t3 <= t2 || t4 <= t2) {
         printf("%f t3\n",t3);
         printf("%f t4\n",t4);
         p1.Print();
         printf("\n");
         p4.Print();
         printf("\n");

         return true;
      }
      else
         return false;
   }

   double xi = ((x3-x4)*(x1*y2-y1*x2)-(x1-x2)*(x3*y4-y3*x4))/d;
   double yi = ((y3-y4)*(x1*y2-y1*x2)-(y1-y2)*(x3*y4-y3*x4))/d;

   if (xi < MIN(x1,x2) || xi > MAX(x1,x2)) return false;
   if (xi < MIN(x3,x4) || xi > MAX(x3,x4)) return false;

   return abs(z1 - z2) < BIKE_HEIGHT/4;
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
   glTranslatef(player->position.X(), player->position.Y(), player->position.Z() - BIKE_HEIGHT/2);
   glRotatef(angle, 0.0f, 0.0f, 1.0f);
   //glutSolidCube(0.5f);
   player->mesh->Draw();
   glPopMatrix();

   glDisable(GL_COLOR_MATERIAL);

}

void DrawFuel(Player *player) {
   // Disable lighting
   GLboolean lighting = glIsEnabled(GL_LIGHTING);
   glDisable(GL_LIGHTING);

   // Save matrices and setup projection
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0.0, GLUTwindow_width, 0.0, GLUTwindow_height);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   
   // Font choice
   void * font = GLUT_BITMAP_TIMES_ROMAN_24;

   // Determine height of fuel bar on screen
   int pad = (int) GLUTwindow_height * .4;
   double percent_fuel = player->fuel_time / FULL_FUEL;
   int top = (int) ((GLUTwindow_height - pad) * percent_fuel);

   // Display characters
   for (int i = 0; i < top; i++) {
      // Make a gradient of colors
      double n = (1 - i * 1.0 / (GLUTwindow_height - pad)) * 100;
      double R=(255.0*n)/100.0;
      double G=(255.0*(100.0-n))/100.0; 
      glColor3d(R/255.0, G/255.0, 0.0);

      // Draw '*' to indicate fuel bar
      glRasterPos2i(GLUTwindow_width * .9, i + pad/2);
      glutBitmapCharacter(font, '*');
   }

   // Restore matrices
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glFlush();

   // Restore lighting
   if (lighting) glEnable(GL_LIGHTING);
}

void DrawTrail(Player *player, Player *perspective, double xfov) {
   static GLUquadricObj *glu_sphere = gluNewQuadric();
   gluQuadricTexture(glu_sphere, GL_TRUE);
   gluQuadricNormals(glu_sphere, (GLenum) GLU_SMOOTH);
   gluQuadricDrawStyle(glu_sphere, (GLenum) GLU_FILL);

   R3Vector normal = perspective->direction;
   R3Point pos = ComputeEye(perspective, perspective->view);

   R3Vector nor1 = normal;
   R3Vector nor2 = normal;
   nor1.Rotate(R3posz_vector, ((xfov-.01)/2)*(180/M_PI));
   nor2.Rotate(R3posz_vector, -((xfov-.01)/2)*(180/M_PI));

   for (unsigned int i = 1; i < player->trail.size(); i++) {
      // Make cylinder between p1 and p2
      // See: http://www.thjsmith.com/40/cylinder-between-two-points-opengl-c
      R3Point p1 = player->trail[i-1];
      R3Point p2 = player->trail[i];

      R3Vector v = p1 - pos;
      R3Vector u = p2 - pos;

      if (v.Dot(normal) > 0 && u.Dot(normal) > 0){
         if((v.Dot(nor1) > 0 && u.Dot(nor1) > 0) && (v.Dot(nor2) > 0 && u.Dot(nor2) > 0)){

      	 // Compute direction and angle of rotation from standard
      	 R3Vector p = (p2 - p1);
      	 R3Vector t = R3Vector(R3zaxis_vector);
      	 t.Cross(p);
      	 double angle = 180 / M_PI * acos (R3zaxis_vector.Dot(p) / p.Length());

      	 // Level of detail
      	 R3Point pipe_center = (p2 - p1)/2 + p1;
      	 double percent = R3Distance(pos, pipe_center) / level_size * 1.25;
      	 percent = 1 - sqrt(percent);
      	 int detail = (int) (MAX_PIPE_DETAIL * percent);
      	 detail = MAX(detail, MIN_PIPE_DETAIL);

      	 // Draw cylinder

      	 glPushMatrix();
      	 glTranslated(p1.X(),p1.Y(),p1.Z());
      	 glRotated(angle,t.X(),t.Y(),t.Z());
      	 gluCylinder(glu_sphere, TRAIL_DIAMETER, TRAIL_DIAMETER, p.Length(), detail, detail);
      	 glPopMatrix();
         }
      }
   }

}

void MovePlayer(int player_num, int move) {
   if ((unsigned int) player_num >= players.size()) { return; }

   switch (move) {
      case JUMPING:
	 players[player_num].jumping = true;
	 break;
      case NOT_JUMPING:
	 players[player_num].jumping = false;
	 break;
      case TURNING_LEFT:
      case TURNING_RIGHT:
      case NOT_TURNING:
      default:
	 players[player_num].turn = move;
	 break;
   }
}


