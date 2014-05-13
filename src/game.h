#ifndef GAME_H
#define GAME_H

////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "R2/R2.h"
#include "R3/R3.h"
#include "R3Scene.h"
#include "fglut/fglut.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

////////////////////////////////////////////////////////////
// PLAYER DEFINITION
////////////////////////////////////////////////////////////

enum {
   NOT_TURNING = 0,
   TURNING_LEFT = 1,
   TURNING_RIGHT = -1,
   JUMPING = 2,
   NOT_JUMPING
};

enum {
   OVER_THE_SHOULDER,
   FIRST_PERSON,
   NUM_VIEWS
};

enum {
   NORMAL = 0,
   CHECK_FRONT = 1,
   CHECK_LEFT = 2,
   CHECK_RIGHT = 3
};

struct Color {
   public:
      Color(double R, double G, double B);
      double R(void);
      double G(void);
      double B(void);

   private:
      double r;
      double g;
      double b;
};

inline Color::
Color(double R, double G, double B) : r(R), g(G), b(B) {
   r = MIN(r, 1.0); r = MAX(r, 0.0);
   g = MIN(g, 1.0); g = MAX(g, 0.0);
   b = MIN(b, 1.0); b = MAX(b, 0.0);
}
inline double Color::R(void) { return r; }
inline double Color::G(void) { return g; }
inline double Color::B(void) { return b; }

struct Player {
   public:
      Player(Color color, bool is_ai, R3Point position, R3Vector direction, int view);

      bool IsAI();

      R3Point position;
      R3Vector direction;
      int view;
      R3Mesh *mesh;
      Color color;
      bool dead;
      bool is_ai;
      int turn;
      bool jumping;
      double fuel_time;

      vector<R3Point> trail;
};

inline bool Player::IsAI() { return is_ai; }


////////////////////////////////////////////////////////////
// API API
////////////////////////////////////////////////////////////

void InitGame();
void InitLevel(int human_players, int ai_players,
               int view, double size, vector<R3Point> init_positions, vector<R3Vector> init_directions);

void GameUpdate(void);
void UpdateCamera(Player *player, int camera_perspective);
void UpdatePlayer(R3Scene *scene, Player *player, double delta_time);

bool Check_Collisions(R3Scene *scene, Player *player, double delta_time, int for_decisions, int precision);
bool Collide_Box(R3Scene *scene, R3Node *node, R3Point testpoint);
bool Collide_Scene(R3Scene *scene, R3Node *node, R3Point testpoint);
bool Collide_Trails(Player *player, R3Point testpoint, R3Point nextpoint);
//bool Collide_Point(R3Point testpoint, R3Point trailpoint1, R3Point trailpoint2, R3Vector direction);
//bool Collide_Point(Player *player, R3Point testpoint, R3Point trailpoint1, R3Point trailpoint2);
bool Segment_Intersection(R3Point p1, R3Point p2, R3Point p3, R3Point p4);

void DrawPlayer(Player *player);
void DrawFuel(Player *player);
void DrawTrail(Player *player, Player *perspective, double xfov);

void MovePlayer(int player_num, int move);

#endif
