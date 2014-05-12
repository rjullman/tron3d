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
   TURNING_RIGHT = -1
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
      Player(Color color, bool is_ai, R3Point position, R3Vector direction);

      bool IsAI();

      R3Point position;
      R3Vector direction;
      R3Mesh *mesh;
      Color color;
      bool dead;
      bool is_ai;
      int turn;

      vector<R3Point> trail;
};

inline bool Player::IsAI() { return is_ai; }


////////////////////////////////////////////////////////////
// API API
////////////////////////////////////////////////////////////

void InitGame();
void InitLevel(int human_players, int ai_players);

void GameUpdate(void);
void UpdateCamera(Player *player);
void UpdatePlayer(R3Scene *scene, Player *player, double delta_time);

void Check_Collisions(R3Scene *scene, Player *player);
bool Collide_Box(R3Scene *scene, R3Node *node, R3Point testpoint);
bool Collide_Scene(R3Scene *scene, R3Node *node, R3Point testpoint);
bool Collide_Trails(Player *player, R3Point testpoint);
bool Collide_Point(R3Point testpoint, R3Point trailpoint1, R3Point trailpoint2, R3Vector direction);

void DrawPlayer(Player *player);
void DrawTrail(Player *player);

void MovePlayer(int player_num, int turn_dir);

#endif
