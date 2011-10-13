// Vanilla Linux OpenGL+GLUT+SOIL App

//#include "MemoryLeak.h"

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

#include "MemoryLeak.h"

#include <SOIL/SOIL.h>
#include <dirent.h>
#include <alsa/asoundlib.h>
#include <vector>

#define kWindowWidth 320
#define kWindowHeight 480

static pthread_t audio_thread;

static std::vector<GLuint> textures;

//static std::vector<foo*> models;
//static std::vector<foo*> sounds;
//static std::vector<foo*> levels;

static int min_buffer;

// Handle for the PCM device
snd_pcm_t *pcm_handle;          

// Playback stream
snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

// This structure contains information about
// the hardware and can be used to specify the
// configuration to be used for the PCM stream.
snd_pcm_hw_params_t *hwparams;

// Name of the PCM device, like plughw:0,0
// The first number is the number of the soundcard,
// the second number is the number of the device.
char *pcm_name;

  
void *pump_audio(void *) {
  snd_pcm_sframes_t foo;

  short *buffer;
  buffer = new short[min_buffer];
  memset(buffer, 0, min_buffer * sizeof(short));

  while (true) {
    //#TODO game->DoAudio(buffer, min_buffer);
    //Engine::CurrentGameDoAudio(buffer, min_buffer / sizeof(short));
    foo = snd_pcm_writei(pcm_handle, buffer, min_buffer);
    if (foo < 0) {
      //LOGV("wtf: %s\n", snd_strerror(foo));
    }
  }
}


void draw(void) {
  //#TODO game->DrawScreen(0);
  glutSwapBuffers();
}


void resize(int width, int height) {
  //#TODO game->ResizeScreen(width, height);
}


void processMouse(int button, int state, int x, int y) {
  switch (state) {
    case GLUT_DOWN:
      //#TODO game->Hit(x, y, 0);
      break;
    case GLUT_UP:
      //#TODO game->Hit(x, y, 2);
      break;
  }
}


void processMouseMotion(int x, int y) {
  //#TODO game->Hit(x, y, 1);
}


void processNormalKeys(unsigned char key, int x, int y) {
  switch (key) {
    case 27:
      int save_result = SOIL_save_screenshot("/tmp/awesomenessity.png", SOIL_SAVE_TYPE_BMP, 0, 0, 320, 480);
      exit(0);
    break;
  }
}

char *path_cat (const char *str1, char *str2) {
	size_t str1_len = strlen(str1);
	size_t str2_len = strlen(str2);
	char *result;
	result = (char*)malloc((str1_len+str2_len+1)*sizeof *result);
	strcpy (result,str1);
	int i,j;
	for(i=str1_len, j=0; ((i<(str1_len+str2_len)) && (j<str2_len));i++, j++) {
		result[i]=str2[j];
	}
	result[str1_len+str2_len]='\0';
	return result;
}

void display(void)
{
/* clear window */
glClear(GL_COLOR_BUFFER_BIT);

/* draw unit square polygon */
glBegin(GL_POLYGON);
glVertex2f(-0.5, -0.5);
glVertex2f(-0.5, 0.5);
glVertex2f(0.5, 0.5);
glVertex2f(0.5, -0.5);
glEnd();

/* flush GL buffers */
glFlush();
}


void init()
{
/* set clear color to black */
glClearColor (0.0, 0.0, 0.0, 0.0);

/* set fill color to white */
glColor3f(1.0, 1.0, 1.0);

/* set up standard orthogonal view with clipping */
/* box as cube of side 2 centered at origin */
/* This is default view and these statement could be removed */
glMatrixMode (GL_PROJECTION);
glLoadIdentity ();
glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
}

int main(int argc, char** argv) {
glutInit(&argc,argv);
glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
glutInitWindowSize(500,500);
glutInitWindowPosition(0,0);
glutCreateWindow("simple");
glutDisplayFunc(display);
//glutIdleFunc(display);
glutReshapeFunc(resize);
init();
printf("run\n\n\n\n\n\n\n\n\n\n\n\n");
glutMainLoop();

/*
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  glutInitWindowSize(kWindowWidth, kWindowHeight);
  glutInitWindowPosition(1000, 500);
  glutCreateWindow("main");

  glutKeyboardFunc(processNormalKeys);
  glutMouseFunc(processMouse);
  glutMotionFunc(processMouseMotion);
  glutDisplayFunc(draw);
  glutIdleFunc(draw);
  glutReshapeFunc(resize);
  glutMainLoop();
*/
  return 0;
}
