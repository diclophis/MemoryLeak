// Vanilla Linux OpenGL+GLUT+SOIL App

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

#include "MemoryLeak.h"

#include <SOIL/SOIL.h>
#include <dirent.h>
#include <alsa/asoundlib.h>
#include <vector>

#define kWindowWidth 1024
#define kWindowHeight 1024

static pthread_t audio_thread;

static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> sounds;
static std::vector<foo*> levels;

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

static int game_index = 1;
static bool left_down = false;
static bool right_down = false;
static bool reset_down = false;
static bool debug_down = false;


void *pump_audio(void *) {
  snd_pcm_sframes_t foo;

  LOGV("%d min_buffer\n", min_buffer);

  short *buffer;
  buffer = new short[min_buffer];
  memset(buffer, 0, min_buffer * sizeof(short));

  while (true) {
    Engine::CurrentGameDoAudio(buffer, min_buffer * sizeof(short));
    foo = snd_pcm_writei(pcm_handle, buffer, min_buffer / sizeof(short));
    if (foo < 0) {
      LOGV("wtf: %s\n", snd_strerror(foo));
    } else if (foo > 0) {
      //LOGV("wrote: %lu\n", foo);
    }
  }
}


void draw(void) {
  Engine::CurrentGameDrawScreen(0);
  glutSwapBuffers();
  glutPostRedisplay();
}


void resize(int width, int height) {
  Engine::CurrentGameResizeScreen(width, height);
}


void processMouse(int button, int state, int x, int y) {
  switch (state) {
    case GLUT_DOWN:
      Engine::CurrentGameHit(x, y, 0);
      break;
    case GLUT_UP:
      Engine::CurrentGameHit(x, y, 2);
      break;
  }
}


void processMouseMotion(int x, int y) {
  Engine::CurrentGameHit(x, y, 1);
}


void processNormalKeys(unsigned char key, int x, int y) {
  //LOGV("key: %d %c\n", key, key);
  if (key == 49) {
    if (debug_down) {
      Engine::CurrentGameHit(0, 0, 2);
    } else {
      Engine::CurrentGameHit(0, 0, 0);
    }
    debug_down = !debug_down;
  } else if (key == 110) {
    if (left_down) {
      Engine::CurrentGameHit(0, 1024, 2);
    } else {
      Engine::CurrentGameHit(0, 1024, 0);
    }
    left_down = !left_down;
  } else if (key == 109) {
    if (right_down) {
      Engine::CurrentGameHit(1024, 1024, 2);
    } else {
      Engine::CurrentGameHit(1024, 1024, 0);
    }
    right_down = !right_down;
  } else {
    if (reset_down) {
    } else {
      Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, NULL);
    }
    reset_down = !reset_down;
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

//int scandir(const char *dirp, struct dirent ***namelist,
//int (*filter)(const struct dirent *),
//int (*compar)(const struct dirent **, const struct dirent **));

int is_not_dot_or_dot_dot(const struct dirent *dp) {
  if (strcmp(".", dp->d_name) == 0 || strcmp("..", dp->d_name) == 0) {
    return 0;
  } else {
    return 1;
  }
}


int main(int argc, char** argv) {

  glutInit(&argc,argv);
  glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(kWindowWidth, kWindowHeight);
  glutInitWindowPosition(0,0);
  glutCreateWindow("simple");
  glutKeyboardFunc(processNormalKeys);
  glutKeyboardUpFunc(processNormalKeys);
  glutIgnoreKeyRepeat(true);
  glutMouseFunc(processMouse);
  glutMotionFunc(processMouseMotion);
  glutDisplayFunc(draw);
  glutReshapeFunc(resize);

  struct dirent *dp;
  struct dirent **dps;

  const char *dir_path="../../assets/textures/";
  DIR *dir; // = opendir(dir_path);
  int dir_ents = scandir(dir_path, &dps, is_not_dot_or_dot_dot, alphasort);

  if (dir_ents == -1) {
    LOGV("wtf\n");
    exit(1);
  }

  for (int i=0; i<dir_ents; i++) {
    char *tmp;
    tmp = path_cat(dir_path, dps[i]->d_name);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    GLuint tex_2d = SOIL_load_OGL_texture(
      tmp,
      SOIL_LOAD_AUTO,
      SOIL_CREATE_NEW_ID,
      SOIL_FLAG_MULTIPLY_ALPHA
    );
    textures.push_back(tex_2d);

    free(tmp);
    tmp=NULL;
  }

  dir_path="../../assets/models/";
  dir = opendir(dir_path);
  if (dir) {
    while ((dp=readdir(dir)) != NULL) {
      char *tmp;
      tmp = path_cat(dir_path, dp->d_name);
      if (strcmp(".", dp->d_name) == 0 || strcmp("..", dp->d_name) == 0) {
      } else {
        FILE *fd = fopen(tmp, "rb");
        fseek(fd, 0, SEEK_END);
        unsigned int len = ftell(fd);
        rewind(fd);

        foo *firstModel = new foo;
        firstModel->fp = fd;
        firstModel->off = 0;
        firstModel->len = len;

        models.push_back(firstModel);
      }

      free(tmp);
      tmp=NULL;
    }
    closedir(dir);
  }

  dir_path="../../assets/sounds/";
  dir = opendir(dir_path);
  if (dir) {
    while ((dp=readdir(dir)) != NULL) {
      char *tmp;
      tmp = path_cat(dir_path, dp->d_name);
      if (strcmp(".", dp->d_name) == 0 || strcmp("..", dp->d_name) == 0) {
      } else {
        FILE *fd = fopen(tmp, "rb");
        fseek(fd, 0, SEEK_END);
        unsigned int len = ftell(fd);
        rewind(fd);

        foo *firstModel = new foo;
        firstModel->fp = fd;
        firstModel->off = 0;
        firstModel->len = len;

        sounds.push_back(firstModel);
      }
    
      free(tmp);
      tmp=NULL;
    }
    closedir(dir);
  }

  dir_path="../../assets/levels/";
  dir = opendir(dir_path);
  if (dir) {
    while ((dp=readdir(dir)) != NULL) {
      char *tmp;
      tmp = path_cat(dir_path, dp->d_name);
      if (strcmp(".", dp->d_name) == 0 || strcmp("..", dp->d_name) == 0) {
      } else {
      FILE *fd = fopen(tmp, "rb");
      fseek(fd, 0, SEEK_END);
      unsigned int len = ftell(fd);
      rewind(fd);

      foo *firstModel = new foo;
      firstModel->fp = fd;
      firstModel->off = 0;
      firstModel->len = len;

      levels.push_back(firstModel);
      }

      free(tmp);
      tmp=NULL;
    }
    closedir(dir);
  }

  /* Init pcm_name. Of course, later you */
  /* will make this configurable ;-)     */
  pcm_name = strdup("plughw:0,0");

  /* Allocate the snd_pcm_hw_params_t structure on the stack. */
  snd_pcm_hw_params_alloca(&hwparams);

  /* Open PCM. The last parameter of this function is the mode. */
  /* If this is set to 0, the standard mode is used. Possible   */
  /* other values are SND_PCM_NONBLOCK and SND_PCM_ASYNC.       */ 
  /* If SND_PCM_NONBLOCK is used, read / write access to the    */
  /* PCM device will return immediately. If SND_PCM_ASYNC is    */
  /* specified, SIGIO will be emitted whenever a period has     */
  /* been completely processed by the soundcard.                */
  if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0) < 0) {
    fprintf(stderr, "Error opening PCM device %s\n", pcm_name);
    return(-1);
  }

  /* Init hwparams with full configuration space */
  if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
    fprintf(stderr, "Can not configure this PCM device.\n");
    return(-1);
  }

  unsigned int rate = 44100; /* Sample rate */
  //min_buffer = 64;
  //min_buffer = 128 * 64;

  int periods = 4;       /* Number of periods */
  snd_pcm_uframes_t periodsize = 1024; /* Periodsize (bytes) */

  /* Set access type. This can be either    */
  /* SND_PCM_ACCESS_RW_INTERLEAVED or       */
  /* SND_PCM_ACCESS_RW_NONINTERLEAVED.      */
  /* There are also access types for MMAPed */
  /* access, but this is beyond the scope   */
  /* of this introduction.                  */
  if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
    fprintf(stderr, "Error setting access.\n");
    return(-1);
  }

  /* Set sample format */
  //if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S32_LE) < 0) {
  //if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_U16_LE) < 0) {
  if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16_LE) < 0) {
    fprintf(stderr, "Error setting format.\n");
    return(-1);
  }

  /* Set sample rate. If the exact rate is not supported */
  /* by the hardware, use nearest possible rate.         */ 
  //exact_rate = rate;
  if (snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &rate, 0) < 0) {
    fprintf(stderr, "Error setting rate.\n");
    return(-1);
  }

  /* Set number of channels */
  if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2) < 0) {
    fprintf(stderr, "Error setting channels.\n");
    return(-1);
  }

  /* Set number of periods. Periods used to be called fragments. */ 
  if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, periods, 0) < 0) {
    fprintf(stderr, "Error setting periods.\n");
    return(-1);
  }

  /* Set buffer size (in frames). The resulting latency is given by */
  /* latency = periodsize * periods / (rate * bytes_per_frame)     */
  if (snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, (periodsize * periods)>>2) < 0) {
    fprintf(stderr, "Error setting buffersize.\n");
    return(-1);
  }

  /* Apply HW parameter settings to */
  /* PCM device and prepare device  */
  if (snd_pcm_hw_params(pcm_handle, hwparams) < 0) {
    fprintf(stderr, "Error setting HW params.\n");
    return(-1);
  }

  if (snd_pcm_prepare(pcm_handle) < 0) {
    fprintf(stderr, "cannot prepare audio interface for use\n");
    return(-1);
  }

  snd_pcm_uframes_t buffersize2, periodsize2;
  if (snd_pcm_get_params(pcm_handle, &buffersize2, &periodsize2) < 0) {
    fprintf(stderr, "Error cant get num frames\n");
    return(-1);
  }

  min_buffer = periodsize2 * 16;

  Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, NULL);
  pthread_create(&audio_thread, 0, pump_audio, NULL);

  glutMainLoop();
  return 0;
}
