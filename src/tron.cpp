// Source file for the scene file viewer




////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "R3/R3.h"
#include "R3Scene.h"
#include "game.h"
#include "fglut/fglut.h"
#include <fstream>
#include <string>
#include <iostream>

///////////////////////////////////////////////////////////
// GLOBAL CONSTANTS
////////////////////////////////////////////////////////////

static double MAX_FPS = 60;

static int MAX_SOUND_FX_VOL = 4;
static int MAX_MUSIC_VOL = 4;

///////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////


// Program arguments

static char *input_scene_name = NULL;
GLuint texture;


// Menu variables

enum { MAIN_MENU, OPTIONS_MENU };
enum { MENU_ENTER, MENU_LEFT, MENU_RIGHT };
int menu = MAIN_MENU;
int menu_option = 0;

static const char* main_menu_text[] = {"START GAME", "OPTIONS", "QUIT"};
enum {
   START_GAME_SELECTED,
   OPTIONS_SELECTED,
   QUIT_SELECTED,
   NUM_MAIN_MENU_ITEMS
};

static const char* options_menu_text[] = {
   "PLAYERS:       %d",
   "VIEW:              %s",
//   "MUSIC:           %d/%d",
//   "SOUND FX:    %d/%d",
   "P1 LEFT:         %s",
   "P1 RIGHT:      %s",
   "P2 LEFT:         %s",
   "P2 RIGHT:      %s",
   "BACK"
};
enum {
   NUM_PLAYERS_SELECTED,
   VIEW_SELECTED,
   PLAYER_1_LEFT_SELECTED,
   PLAYER_1_RIGHT_SELECTED,
   PLAYER_2_LEFT_SELECTED,
   PLAYER_2_RIGHT_SELECTED,
   BACK_SELECTED,
   NUM_OPTIONS_MENU_ITEMS,
   MUSIC_SELECTED,
   SOUND_FX_SELECTED,
};


// Game variables

vector<Player> players;
vector<R3Point> init_positions;
vector<R3Vector> init_directions;
static bool gameover = true;

static int num_humans = 1;
static int num_ai = 1;

static int view = OVER_THE_SHOULDER;
static int sound_fx = MAX_SOUND_FX_VOL / 2;
static int music = MAX_MUSIC_VOL / 2;

static double game_start_time = 0;

// Display variables

static R3Scene *scene = NULL;
static R3Camera camera;
static int show_bboxes = 0;
static int quit = 0;


// GLUT variables

static int GLUTwindow = 0;
int GLUTwindow_height = 512;
int GLUTwindow_width = 512;



// GLUT command list

enum {
   DISPLAY_BBOXES_TOGGLE_COMMAND,
   QUIT_COMMAND,
};



////////////////////////////////////////////////////////////
// TIMER CODE
////////////////////////////////////////////////////////////

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/time.h>
#endif

static double GetTime(void)
{
#ifdef _WIN32
  // Return number of seconds since start of execution
  static int first = 1;
  static LARGE_INTEGER timefreq;
  static LARGE_INTEGER start_timevalue;

  // Check if this is the first time
  if (first) {
    // Initialize first time
    QueryPerformanceFrequency(&timefreq);
    QueryPerformanceCounter(&start_timevalue);
    first = 0;
    return 0;
  }
  else {
    // Return time since start
    LARGE_INTEGER current_timevalue;
    QueryPerformanceCounter(&current_timevalue);
    return ((double) current_timevalue.QuadPart -
            (double) start_timevalue.QuadPart) /
            (double) timefreq.QuadPart;
  }
#else
  // Return number of seconds since start of execution
  static int first = 1;
  static struct timeval start_timevalue;

  // Check if this is the first time
  if (first) {
    // Initialize first time
    gettimeofday(&start_timevalue, NULL);
    first = 0;
    return 0;
  }
  else {
    // Return time since start
    struct timeval current_timevalue;
    gettimeofday(&current_timevalue, NULL);
    int secs = current_timevalue.tv_sec - start_timevalue.tv_sec;
    int usecs = current_timevalue.tv_usec - start_timevalue.tv_usec;
    return (double) (secs + 1.0E-6F * usecs);
  }
#endif
}

////////////////
// Typing
////////////////

void renderBitmapString(
    float x,
    float y,
    float z,
    void *font,
    char *string) {

  char *c;
  glRasterPos3f(x, y,z);
  for (c=string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}

///////////
// Function Declarations
//////////

void GLUTStop(void);

void LoadLights(R3Scene *scene);
void DrawScene(R3Scene *scene);
void SwitchMenu(int new_menu);

////////////////////////////////////////////////////////////
// GAME STATE
////////////////////////////////////////////////////////////

bool Playing() {
   return !gameover;
}

////////////////////////////////////////////////////////////
// GAME DRAWING
////////////////////////////////////////////////////////////

void DrawMenuText(const char *text, bool select, double px, double py) {
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
   glRasterPos2i(px, py);

   // Font choice
   void * font = GLUT_BITMAP_TIMES_ROMAN_24;

   // Indicate selecteted char via '*'
   if (select) { glutBitmapCharacter(font, '*'); }
   int num_spaces = select ? 2 : 4;
   for (int i = 0; i < num_spaces; i++)
      glutBitmapCharacter(font, ' ');

      // Display characters
      glColor3d(1.0, 1.0, 1.0);
      while (*text) {
	 glutBitmapCharacter(font, *text);
	 text++;
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

void DrawMenuHelper(const char* text[], int items) {
   // Keep menu item within bounds
   if (menu_option < 0) { menu_option = 0; }
   else if (menu_option >= items) { menu_option = items-1; }
   for (int i = 0; i < items; i++) {
      bool selected = (menu_option % items) == i;
      DrawMenuText(text[i],
		   selected,
		   GLUTwindow_width/2 * 0.55,
		   GLUTwindow_height/2 - (i-items/2) * 40);
   }
}

void DrawMenu()
{
   switch (menu) {
      case MAIN_MENU:
	 DrawMenuHelper(main_menu_text, NUM_MAIN_MENU_ITEMS);
	 break;
      case OPTIONS_MENU:
	 int MAX_LINE_LEN = 50;

	 // Created formatted options
	 char** formatted = new char *[NUM_OPTIONS_MENU_ITEMS];
	 for (int i = 0; i < NUM_OPTIONS_MENU_ITEMS; i++) {
	    char* s = new char[MAX_LINE_LEN];
	    const char* cur = options_menu_text[i];
	    switch (i) {
	       case NUM_PLAYERS_SELECTED:
		  snprintf(s, MAX_LINE_LEN, cur, num_humans);
		  break;
	       case VIEW_SELECTED:
		  if (view == OVER_THE_SHOULDER)
		     snprintf(s, MAX_LINE_LEN, cur, "shoulder");
		  else if (view == FIRST_PERSON)
		     snprintf(s, MAX_LINE_LEN, cur, "first person");
		  break;
	       case MUSIC_SELECTED:
		  snprintf(s, MAX_LINE_LEN, cur, music, MAX_MUSIC_VOL);
		  break;
	       case SOUND_FX_SELECTED:
		  snprintf(s, MAX_LINE_LEN, cur, sound_fx, MAX_SOUND_FX_VOL);
		  break;
	       case PLAYER_1_LEFT_SELECTED:
		  snprintf(s, MAX_LINE_LEN, cur, "<-");
		  break;
	       case PLAYER_1_RIGHT_SELECTED:
		  snprintf(s, MAX_LINE_LEN, cur, "->");
		  break;
	       case PLAYER_2_LEFT_SELECTED:
		  snprintf(s, MAX_LINE_LEN, cur, "a");
		  break;
	       case PLAYER_2_RIGHT_SELECTED:
		  snprintf(s, MAX_LINE_LEN, cur, "d");
		  break;
	       case BACK_SELECTED:
		  snprintf(s, MAX_LINE_LEN, "%s", cur);

	    }
	    formatted[i] = s;
	 }

	 // Draw formatted menu
	 DrawMenuHelper((const char**) formatted, NUM_OPTIONS_MENU_ITEMS);

	 // Clean up
	 for (int i = 0; i < NUM_OPTIONS_MENU_ITEMS; i++) {
	    delete[] formatted[i];
	 }
	 delete[] formatted;

	 break;
   }
}

void DrawGameText() {
   // Disable lighting
   GLboolean lighting = glIsEnabled(GL_LIGHTING);
   glDisable(GL_LIGHTING);

   // Save matrices
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0, GLUTwindow_width, GLUTwindow_height, 0);
   glMatrixMode(GL_MODELVIEW);


    ifstream scores;
    scores.open ("scores.txt");
   std::string high_score;
   std::getline(scores, high_score);

   // Draw game text
   glColor3f(1.0f,1.0f,1.0f);
   char s[50];
   sprintf(s,"-------  Tron 3D --------");
   glPushMatrix();
   glLoadIdentity();
   renderBitmapString(310,30,0,GLUT_BITMAP_HELVETICA_12,s);
   glPopMatrix();
   sprintf(s,"Round Duration: %f", GetTime() - game_start_time);
   renderBitmapString(310,50,0,GLUT_BITMAP_HELVETICA_12,s);
   glPopMatrix();

  sprintf(s,"High Score: %s", high_score.c_str());
   renderBitmapString(310,70,0,GLUT_BITMAP_HELVETICA_12,s);
   glPopMatrix();



   // Restore matrices
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);

   // Restore lighting
   if (lighting) glEnable(GL_LIGHTING);
}

void SetupViewport(int player_num, int total_players) {
   switch(total_players) {
      case 1:
	 glViewport(0, 0, GLUTwindow_width, GLUTwindow_height);
	 break;
      case 2:
	 int y = player_num == 0 ? GLUTwindow_height/2 : 0;
	 glViewport(0, y, GLUTwindow_width, GLUTwindow_height/2);
	 break;
   }
}

void DrawGame(R3Scene *scene)
{
   int viewports_drawn = 0;
   for (unsigned int i = 0; i < players.size(); i++) {
      if (players[i].IsAI()) { continue; }

      // Setup the viewport
      SetupViewport(viewports_drawn++, num_humans);

      // Update player point of view

      UpdateCamera(&players[i], players[i].view);

      // Draw scene surfaces
      glEnable(GL_LIGHTING);
      DrawScene(scene);

      // Draw players
      for (unsigned int j = 0; j < players.size(); j++) {
	 DrawPlayer(&players[j]);
	 DrawTrail(&players[j], &players[i], camera.xfov);
      }

      DrawFuel(&players[i]);
    }



  glViewport(0, GLUTwindow_height- 128, 128, 128);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(360.0/M_PI*0.25, (GLdouble) 1, 0.01, 10000);
  gluLookAt(  0, 0, 40,
           0, 0,  0.0f,
           1.0f, 0.0f,  0.0f);
  // Draw scene surfaces
  //glEnable(GL_LIGHTING);
  //DrawScene(scene);

  glClearDepth(1);
  glEnable(GL_DEPTH_TEST);
  GLfloat AmbientLight[] = {1.0, 1.0, 1.0};
  glLightfv (GL_LIGHT0, GL_AMBIENT, AmbientLight);
  // Draw players
  for (unsigned int i = 0; i < players.size(); i++) {
    for (unsigned int j = 0; j < players.size(); j++) {
      DrawPlayer(&players[j]);
      DrawTrail(&players[j], &players[i], camera.xfov);
    }
  }

  // Return to full screen viewport
   glViewport(0, 0, GLUTwindow_width, GLUTwindow_height);

   DrawGameText();
}

void StartGame() {
   // Initialize game
   InitLevel(num_humans, num_ai,
	     view, scene->BBox().DiagonalLength(), init_positions, init_directions);
   game_start_time = GetTime();
   gameover = false;
}

void UpdateGame(R3Scene *scene)
{
  // Get current time (in seconds) since start of execution
  double current_time = GetTime();
  static double previous_time = 0;

  // program just started up?
  if (previous_time == 0) previous_time = current_time;

  // time passed since starting
  double delta_time = current_time - previous_time;

  // Limit frames drawn per second to MAX_FPS
  if (delta_time > 1.0 / MAX_FPS) {
      // Remember previous time
      previous_time = current_time;
  }
  else { return; }

  // Check for any collisions
  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i].dead) { continue; }

    Check_Collisions(scene, &players[i], delta_time, NORMAL, 1);
  }

  int living = 0;
  for (unsigned int i = 0; i < players.size(); i++) {
     if (players[i].dead) { continue; }
     living++;

     // Move players
     UpdatePlayer(scene, &players[i], delta_time);
  }

  // Gameover when only one player remaining
  gameover = (living == 0);
  if (gameover) {
     // Handle high scores
     ifstream get_scores;
     get_scores.open ("scores.txt");
     std::string high_score;
     std::getline(get_scores, high_score);
     double h_s = atof(high_score.c_str());
     double new_score = GetTime() - game_start_time;
     if (new_score > h_s){
        std::ofstream scores;
        scores.open("scores.txt", std::ios::out);
        scores << new_score;
        scores << "\n";
     }

     // Reset to the main menu
     previous_time = 0;
     SwitchMenu(MAIN_MENU);
  }
}



////////////////////////////////////////////////////////////
// SCENE DRAWING CODE
////////////////////////////////////////////////////////////


// load a 256x256 RGB .RAW file as a texture
GLuint LoadTextureRaw( const char * filename, int wrap )
{
    GLuint texture;
    int width, height;
    char * data;
    FILE * file;

    // open texture data
    file = fopen( filename, "rb" );
    if ( file == NULL ) return 0;

    // allocate buffer
    width = 420;
    height = 420;
    data = (char*)malloc( width * height * 3 );

    // read texture data
    fread( data, width * height * 3, 1, file );
    fclose( file );

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     wrap ? GL_REPEAT : GL_CLAMP );

    // build our texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
                       GL_RGB, GL_UNSIGNED_BYTE, data );

    // free buffer
    free( data );

    return texture;
}

void DrawShape(R3Shape *shape)
{
   // Check shape type

   if (shape->type == R3_BOX_SHAPE) {
    glEnable( GL_TEXTURE_2D );
   glBindTexture( GL_TEXTURE_2D, texture);
    shape->box->Draw();
    glDisable( GL_TEXTURE_2D );
  }
   else if (shape->type == R3_SPHERE_SHAPE) shape->sphere->Draw();
   else if (shape->type == R3_CYLINDER_SHAPE) shape->cylinder->Draw();
   else if (shape->type == R3_CONE_SHAPE) shape->cone->Draw();
   else if (shape->type == R3_MESH_SHAPE) shape->mesh->Draw();
   else if (shape->type == R3_SEGMENT_SHAPE) shape->segment->Draw();
   else if (shape->type == R3_CIRCLE_SHAPE) shape->circle->Draw();
   else fprintf(stderr, "Unrecognized shape type: %d\n", shape->type);
}



void LoadMatrix(R3Matrix *matrix)
{
   // Multiply matrix by top of stack
   // Take transpose of matrix because OpenGL represents vectors with
   // column-vectors and R3 represents them with row-vectors
   R3Matrix m = matrix->Transpose();
   glMultMatrixd((double *) &m);
}



void LoadMaterial(R3Material *material)
{
   GLfloat c[4];

   // Check if same as current
   static R3Material *current_material = NULL;
   if (material == current_material) return;
   current_material = material;

   // Compute "opacity"
   double opacity = 1 - material->kt.Luminance();

   // Load ambient
   c[0] = material->ka[0];
   c[1] = material->ka[1];
   c[2] = material->ka[2];
   c[3] = opacity;
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

   // Load diffuse
   c[0] = material->kd[0];
   c[1] = material->kd[1];
   c[2] = material->kd[2];
   c[3] = opacity;
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

   // Load specular
   c[0] = material->ks[0];
   c[1] = material->ks[1];
   c[2] = material->ks[2];
   c[3] = opacity;
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

   // Load emission
   c[0] = material->emission.Red();
   c[1] = material->emission.Green();
   c[2] = material->emission.Blue();
   c[3] = opacity;
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

   // Load shininess
   c[0] = material->shininess;
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, c[0]);

   // Load texture
   if (material->texture) {
      if (material->texture_index <= 0) {
         // Create texture in OpenGL
         GLuint texture_index;
         glGenTextures(1, &texture_index);
         material->texture_index = (int) texture_index;
         glBindTexture(GL_TEXTURE_2D, material->texture_index);
         R2Image *image = material->texture;
         int npixels = image->NPixels();
         R2Pixel *pixels = image->Pixels();
         GLfloat *buffer = new GLfloat [ 4 * npixels ];
         R2Pixel *pixelsp = pixels;
         GLfloat *bufferp = buffer;
         for (int j = 0; j < npixels; j++) {
            *(bufferp++) = pixelsp->Red();
            *(bufferp++) = pixelsp->Green();
            *(bufferp++) = pixelsp->Blue();
            *(bufferp++) = pixelsp->Alpha();
            pixelsp++;
         }
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
         glTexImage2D(GL_TEXTURE_2D, 0, 4, image->Width(), image->Height(), 0, GL_RGBA, GL_FLOAT, buffer);
         delete [] buffer;
      }

      // Select texture
      glBindTexture(GL_TEXTURE_2D, material->texture_index);
      glEnable(GL_TEXTURE_2D);
   }
   else {
      glDisable(GL_TEXTURE_2D);
   }

   // Enable blending for transparent surfaces
   if (opacity < 1) {
      glDepthMask(false);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
   }
   else {
      glDisable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ZERO);
      glDepthMask(true);
   }
}



void LoadLights(R3Scene *scene)
{
   GLfloat buffer[4];

   // Load ambient light
   static GLfloat ambient[4];
   ambient[0] = scene->ambient[0];
   ambient[1] = scene->ambient[1];
   ambient[2] = scene->ambient[2];
   ambient[3] = 1;
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

   // Load scene lights
   for (int i = 0; i < (int) scene->lights.size(); i++) {
      R3Light *light = scene->lights[i];
      int index = GL_LIGHT0 + i;

      // Temporarily disable light
      glDisable(index);

      // Load color
      buffer[0] = light->color[0];
      buffer[1] = light->color[1];
      buffer[2] = light->color[2];
      buffer[3] = 1.0;
      glLightfv(index, GL_DIFFUSE, buffer);
      glLightfv(index, GL_SPECULAR, buffer);

      // Load attenuation with distance
      buffer[0] = light->constant_attenuation;
      buffer[1] = light->linear_attenuation;
      buffer[2] = light->quadratic_attenuation;
      glLightf(index, GL_CONSTANT_ATTENUATION, buffer[0]);
      glLightf(index, GL_LINEAR_ATTENUATION, buffer[1]);
      glLightf(index, GL_QUADRATIC_ATTENUATION, buffer[2]);

      // Load spot light behavior
      buffer[0] = 180.0 * light->angle_cutoff / M_PI;
      buffer[1] = light->angle_attenuation;
      glLightf(index, GL_SPOT_CUTOFF, buffer[0]);
      glLightf(index, GL_SPOT_EXPONENT, buffer[1]);

      // Load positions/directions
      if (light->type == R3_DIRECTIONAL_LIGHT) {
         // Load direction
         buffer[0] = -(light->direction.X());
         buffer[1] = -(light->direction.Y());
         buffer[2] = -(light->direction.Z());
         buffer[3] = 0.0;
         glLightfv(index, GL_POSITION, buffer);
      }
      else if (light->type == R3_POINT_LIGHT) {
         // Load position
         buffer[0] = light->position.X();
         buffer[1] = light->position.Y();
         buffer[2] = light->position.Z();
         buffer[3] = 1.0;
         glLightfv(index, GL_POSITION, buffer);
      }
      else if (light->type == R3_SPOT_LIGHT) {
         // Load position
         buffer[0] = light->position.X();
         buffer[1] = light->position.Y();
         buffer[2] = light->position.Z();
         buffer[3] = 1.0;
         glLightfv(index, GL_POSITION, buffer);

         // Load direction
         buffer[0] = light->direction.X();
         buffer[1] = light->direction.Y();
         buffer[2] = light->direction.Z();
         buffer[3] = 1.0;
         glLightfv(index, GL_SPOT_DIRECTION, buffer);
      }
      else if (light->type == R3_AREA_LIGHT) {
         // Load position
         buffer[0] = light->position.X();
         buffer[1] = light->position.Y();
         buffer[2] = light->position.Z();
         buffer[3] = 1.0;
         glLightfv(index, GL_POSITION, buffer);

         // Load direction
         buffer[0] = light->direction.X();
         buffer[1] = light->direction.Y();
         buffer[2] = light->direction.Z();
         buffer[3] = 1.0;
         glLightfv(index, GL_SPOT_DIRECTION, buffer);
      }
      else {
         fprintf(stderr, "Unrecognized light type: %d\n", light->type);
         return;
      }

      // Enable light
      glEnable(index);
   }
}



void DrawNode(R3Scene *scene, R3Node *node)
{
   // Push transformation onto stack
   glPushMatrix();
   LoadMatrix(&node->transformation);

   // Load material
   if (node->material) LoadMaterial(node->material);

   // Draw shape

   if (node->shape) DrawShape(node->shape);

   // Draw children nodes
   for (int i = 0; i < (int) node->children.size(); i++)
      DrawNode(scene, node->children[i]);

   // Restore previous transformation
   glPopMatrix();

   // Show bounding box
   if (show_bboxes) {
      GLboolean lighting = glIsEnabled(GL_LIGHTING);
      glDisable(GL_LIGHTING);
      node->bbox.Outline();
      if (lighting) glEnable(GL_LIGHTING);
   }
}


void DrawScene(R3Scene *scene)
{
   // Draw nodes recursively
   DrawNode(scene, scene->root);
}




////////////////////////////////////////////////////////////
// GLUT USER INTERFACE CODE
////////////////////////////////////////////////////////////

void GLUTMainLoop(void)
{
   // Run main loop -- never returns
   glutMainLoop();
}



void GLUTDrawText(const R3Point& p, const char *s)
{
   // Draw text string s and position p
   glRasterPos3d(p[0], p[1], p[2]);
   while (*s) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *(s++));
}



void GLUTStop(void)
{
   // Destroy window
   glutDestroyWindow(GLUTwindow);

   // Delete scene
   delete scene;

   // Exit
   exit(0);
}



void GLUTIdle(void)
{
   // Set current window
   if ( glutGetWindow() != GLUTwindow )
      glutSetWindow(GLUTwindow);

   // Redraw
   glutPostRedisplay();
}



void GLUTResize(int w, int h)
{
   // Resize window
   glViewport(0, 0, w, h);

   // Resize camera vertical field of view to match aspect ratio of viewport
   camera.yfov = atan(tan(camera.xfov) * (double) h/ (double) w);

   // Remember window size
   GLUTwindow_width = w;
   GLUTwindow_height = h;

   // Redraw
   glutPostRedisplay();
}



void GLUTRedraw(void)
{
   // Initialize OpenGL drawing modes
   glEnable(GL_LIGHTING);
   glDisable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ZERO);
   glDepthMask(true);

   // Clear window
   R3Rgb background = scene->background;
   glClearColor(background[0], background[1], background[2], background[3]);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Menu
   if (!Playing()) {
      DrawMenu();
   }
   // Game
   else {
      UpdateGame(scene);
      DrawGame(scene);
   }

   // Load scene lights
   LoadLights(scene);

   // Quit here so that can save image before exit
   if (quit) {
      GLUTStop();
   }

   // Swap buffers
   glutSwapBuffers();
}

void SwitchMenu(int new_menu) {
   menu = new_menu;
   menu_option = 0;
}

void MainMenuToggle(int action) {
   switch (menu_option) {
      case START_GAME_SELECTED:
	 if (action == MENU_ENTER)
	    StartGame();
	 break;
      case OPTIONS_SELECTED:
	 if (action == MENU_ENTER)
	    SwitchMenu(OPTIONS_MENU);
	 break;
      case QUIT_SELECTED:
	 if (action == MENU_ENTER)
	    quit = 1;
	 break;
   }
}

void OptionsMenuToggle(int action) {
   switch (menu_option) {
      case NUM_PLAYERS_SELECTED:
	 if (action == MENU_LEFT  ||
	     action == MENU_RIGHT ||
	     action == MENU_ENTER) {
	    num_humans = 3 - num_humans;
	    num_ai = 1 - num_ai;
	 }
	 break;
      case VIEW_SELECTED:
	 if (action == MENU_LEFT)
	    view--;
	 else if (action == MENU_RIGHT ||
		  action == MENU_ENTER)
	    view++;
	 view = (view + NUM_VIEWS) % NUM_VIEWS;
	 break;
      case SOUND_FX_SELECTED:
	 if (action == MENU_LEFT)
	    sound_fx--;
	 else if (action == MENU_RIGHT ||
		  action == MENU_ENTER)
	    sound_fx++;
	 sound_fx = (sound_fx + MAX_SOUND_FX_VOL + 1) % (MAX_SOUND_FX_VOL + 1);
	 break;
      case MUSIC_SELECTED:
	 if (action == MENU_LEFT)
	    music--;
	 else if (action == MENU_RIGHT ||
		  action == MENU_ENTER)
	    music++;
	 music = (music + MAX_MUSIC_VOL + 1) % (MAX_MUSIC_VOL + 1);
	 break;
      case BACK_SELECTED:
	 if (action == MENU_ENTER)
	    SwitchMenu(MAIN_MENU);
	 break;
      case PLAYER_1_LEFT_SELECTED:
      case PLAYER_1_RIGHT_SELECTED:
      case PLAYER_2_LEFT_SELECTED:
      case PLAYER_2_RIGHT_SELECTED:
	 break;
   }
}

// Handles all option toggling
void MenuToggle(int action) {
   if (Playing()) { return; }

   switch (menu) {
      case MAIN_MENU:
	 MainMenuToggle(action);
	 break;
      case OPTIONS_MENU:
	 OptionsMenuToggle(action);
	 break;
   }
}

void GLUTSpecial(int key, int x, int y)
{
   // Process keyboard button event
   switch (key) {
      case GLUT_KEY_UP:
	 if (Playing())
	    MovePlayer(0, JUMPING);
	 else
	    menu_option--;
	 break;
      case GLUT_KEY_DOWN:
	 menu_option++; break;
      case GLUT_KEY_LEFT :
	 if (Playing())
	    MovePlayer(0, TURNING_LEFT);
	 else
	    MenuToggle(MENU_LEFT);
	 break;
      case GLUT_KEY_RIGHT :
	 if (Playing())
	    MovePlayer(0, TURNING_RIGHT);
	 else
	    MenuToggle(MENU_RIGHT);
	 break;
   }

   // Redraw
   glutPostRedisplay();
}

void GLUTKeyboard(unsigned char key, int x, int y)
{
   // Process keyboard button event
   switch (key) {
      case 'W':
      case 'w':
	 if (Playing())
	    MovePlayer(1, JUMPING);
	 break;

      case 'A':
      case 'a':
	 if (Playing())
	    MovePlayer(1, TURNING_LEFT);
	 break;

      case 'D':
      case 'd':
	 if (Playing())
	    MovePlayer(1, TURNING_RIGHT);
	 break;

      case 'B':
      case 'b':
         show_bboxes = !show_bboxes;
         break;

      case 13: // ENTER
	 MenuToggle(MENU_ENTER);
	 break;

      case 'Q':
      case 'q':
      case 27: // ESCAPE
         quit = 1;
         break;
   }

   // Redraw
   glutPostRedisplay();
}

void GLUTKeyboardRelease(unsigned char key, int x, int y) {
   // Process keyboard button event
   switch (key) {
      case 'W':
      case 'w':
	 if (Playing())
	    MovePlayer(1, NOT_JUMPING);
	 break;
      case 'A':
      case 'a':
      case 'D':
      case 'd':
	 if (Playing())
	    MovePlayer(1, NOT_TURNING);
	 break;
   }

   // Redraw
   glutPostRedisplay();
}

void GLUTSpecialRelease(int key, int x, int y) {
   // Process keyboard button event
   switch (key) {
      case GLUT_KEY_UP:
	 if (Playing())
	    MovePlayer(0, NOT_JUMPING);
	 break;
      case GLUT_KEY_LEFT:
      case GLUT_KEY_RIGHT:
	 if (Playing())
	    MovePlayer(0, NOT_TURNING);
	 break;
   }

   // Redraw
   glutPostRedisplay();
}


void GLUTCommand(int cmd)
{
   // Execute command
   switch (cmd) {
      case DISPLAY_BBOXES_TOGGLE_COMMAND: show_bboxes = !show_bboxes; break;
      case QUIT_COMMAND: quit = 1; break;
   }

   // Mark window for redraw
   glutPostRedisplay();
}





void GLUTInit(int *argc, char **argv)
{
   // Open window
   glutInit(argc, argv);
   glutInitWindowPosition(100, 100);
   glutInitWindowSize(GLUTwindow_width, GLUTwindow_height);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // | GLUT_STENCIL
   GLUTwindow = glutCreateWindow("Tron 3D");

   // Initialize GLUT callback functions
   glutIdleFunc(GLUTIdle);
   glutReshapeFunc(GLUTResize);
   glutDisplayFunc(GLUTRedraw);
   glutKeyboardFunc(GLUTKeyboard);
   glutKeyboardUpFunc(GLUTKeyboardRelease);
   glutIgnoreKeyRepeat(1);
   glutSpecialFunc(GLUTSpecial);
   glutSpecialUpFunc(GLUTSpecialRelease);

   // Initialize graphics modes
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glEnable(GL_DEPTH_TEST);
   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

   // Initialize game
   texture = LoadTextureRaw("../textures/grid.bmp", 1);
   InitGame();
}




////////////////////////////////////////////////////////////
// SCENE READING
////////////////////////////////////////////////////////////


R3Scene *
ReadScene(const char *filename)
{
   // Allocate scene
   R3Scene *scene = new R3Scene();
   if (!scene) {
      fprintf(stderr, "Unable to allocate scene\n");
      return NULL;
   }

   // Read file
   if (!scene->Read(filename)) {
      fprintf(stderr, "Unable to read scene from %s\n", filename);
      return NULL;
   }

   // Remember initial camera
   camera = scene->camera;

    FILE *fp;
    if (!(fp = fopen(filename, "r"))) {
      fprintf(stderr, "Unable to open file %s", filename);
      return 0;
    }

    char cmd[128];
    vector<R3Particle *> particles_for_springs;
    while (fscanf(fp, "%s", cmd) == 1) {
      if (cmd[0] == '#') {
          // Comment -- read everything until end of line
          do { cmd[0] = fgetc(fp); } while ((cmd[0] >= 0) && (cmd[0] != '\n'));
        }
      else if (!strcmp(cmd, "player_position")) {
        R3Point position;
        R3Vector direction;
        if(fscanf(fp, "%lf%lf%lf%lf%lf%lf", &position[0], &position[1], &position[2],
          &direction[0], &direction[1], &direction[2]) != 6){
          fprintf(stderr, "Unable to read player position\n");
        }
        if(direction[2] != 0){
          direction[2] = 0;
        }
        direction.Normalize();
        init_directions.push_back(direction);
        init_positions.push_back(position);
      }
    }
   // Return scene
   return scene;
}



////////////////////////////////////////////////////////////
// PROGRAM ARGUMENT PARSING
////////////////////////////////////////////////////////////

int
ParseArgs(int argc, char **argv)
{
   // Innocent until proven guilty
   int print_usage = 0;

   // Parse arguments
   argc--; argv++;
   while (argc > 0) {
      if ((*argv)[0] == '-') {
         if (!strcmp(*argv, "-help")) { print_usage = 1; }
         else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
         argv++; argc--;
      }
      else {
         if (!input_scene_name) input_scene_name = *argv;
         else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
         argv++; argc--;
      }
   }

   // Check input_scene_name
   if (!input_scene_name || print_usage) {
      printf("Usage: tron <level.scn>\n");
      return 0;
   }

   // Return OK status
   return 1;
}



////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
   // Parse program arguments
   if (!ParseArgs(argc, argv)) exit(1);

   // Initialize GLUT
   GLUTInit(&argc, argv);

   // Read scene
   scene = ReadScene(input_scene_name);
   if (!scene) exit(-1);

   // Run GLUT interface
   GLUTMainLoop();

   // Return success
   return 0;
}









