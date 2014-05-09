// Source file for the scene file viewer




////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////

#include "R3/R3.h"
#include "R3Scene.h"
#include "game.h"
#include "fglut/fglut.h"


///////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////


// Program arguments

static char *input_scene_name = NULL;


// Menu variables

enum { MAIN_MENU };
int menu = MAIN_MENU;
int menu_option = 0;

static const char* main_menu_text[] = {"START GAME", "OPTIONS", "QUIT"};
enum {
   START_GAME_SELECTED,
   OPTIONS_SELECTED,
   QUIT_SELECTED,
   NUM_MAIN_MENU_ITEMS
};


// Game variables

vector<Player> players;
static bool gameover = true;


// Display variables

static R3Scene *scene = NULL;
static R3Camera camera;
static int show_bboxes = 0;
static int quit = 0;


// GLUT variables

static int GLUTwindow = 0;
static int GLUTwindow_height = 512;
static int GLUTwindow_width = 512;



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

///////////
// Function Declarations
//////////

void GLUTStop(void);

////////////////////////////////////////////////////////////
// GAME DRAWING CODE
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
		   GLUTwindow_width/2 * 0.65,
		   GLUTwindow_height/2 - (i-1) * 40);
   }
}

void DrawMenu() 
{
   switch (menu) {
      case MAIN_MENU:
	 DrawMenuHelper(main_menu_text, NUM_MAIN_MENU_ITEMS);
   }
}

void DrawGame(R3Scene *scene)
{
  // Get current time (in seconds) since start of execution
  double current_time = GetTime();
  static double previous_time = 0;

  // program just started up?
  if (previous_time == 0) previous_time = current_time;

  // time passed since starting
  double delta_time = current_time - previous_time;

  int living = 0;
  for (unsigned int i = 0; i < players.size(); i++) {
     if (players[i].dead) { continue; }
     living++;

     // Move players
     UpdatePlayer(scene, &players[i], delta_time);

     // Update player point of view
     UpdateCamera(&players[i]);
  }

  // Gameover when only one player remaining
  gameover = (living == 0);

  // Remember previous time
  previous_time = current_time;
}

////////////////////////////////////////////////////////////
// SCENE DRAWING CODE
////////////////////////////////////////////////////////////

void DrawShape(R3Shape *shape)
{
   // Check shape type
   if (shape->type == R3_BOX_SHAPE) shape->box->Draw();
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

   // Draw Menu
   if (gameover) {
      DrawMenu();
   }
   // Draw Game
   else {
      DrawGame(scene);

      // Load scene lights
      LoadLights(scene);

      // Draw scene surfaces
      glEnable(GL_LIGHTING);
      DrawScene(scene);
   }

   // Quit here so that can save image before exit
   if (quit) {
      GLUTStop();
   }

   // Swap buffers
   glutSwapBuffers();
}



void GLUTSpecial(int key, int x, int y)
{
   // Process keyboard button event
   switch (key) {
      case GLUT_KEY_UP:
	 menu_option--; break;
      case GLUT_KEY_DOWN:
	 menu_option++; break;
      case GLUT_KEY_LEFT :
	 ToggleMovePlayer(&players[0], TURNING_LEFT); break;
      case GLUT_KEY_RIGHT :
	 ToggleMovePlayer(&players[0], TURNING_RIGHT); break;
   }

   // Redraw
   glutPostRedisplay();
}

// Handles all option toggling
void GLUTEnterPressed() {
   switch (menu) {
      case MAIN_MENU:
	 switch (menu_option) {
	    case START_GAME_SELECTED:
	       // Initialize game
	       InitLevel(1);
	       gameover = false;
	       break;
	    case OPTIONS_SELECTED:
	       break;
	    case QUIT_SELECTED:
	       quit = 1;
	       break;
	 }
	 break;
   }
}

void GLUTKeyboard(unsigned char key, int x, int y)
{
   // Process keyboard button event
   switch (key) {
      case 13: // ENTER
	 GLUTEnterPressed();
	 break;	 

      case 'B':
      case 'b':
         show_bboxes = !show_bboxes;
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


void GLUTKeyboadRelease(int key, int x, int y) {
   // Process keyboard button event
   switch (key) {
      case GLUT_KEY_LEFT : ToggleMovePlayer(&players[0], TURNING_LEFT); break;
      case GLUT_KEY_RIGHT : ToggleMovePlayer(&players[0], TURNING_RIGHT); break;
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
   glutIgnoreKeyRepeat(1);
   glutSpecialFunc(GLUTSpecial);
   glutSpecialUpFunc(GLUTKeyboadRelease);

   // Initialize graphics modes
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glEnable(GL_DEPTH_TEST);
   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
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








