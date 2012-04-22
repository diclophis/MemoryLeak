// Vanilla Linux OpenGL+GLUT+SOIL App

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

#include "MemoryLeak.h"

#include <dirent.h>
#include <vector>

#include "SDL.h"
#include "SDL_audio.h"

#define kWindowWidth 320
#define kWindowHeight 480


static int game_index = 1;
static bool left_down = false;
static bool right_down = false;
static bool reset_down = false;
static bool debug_down = false;
static short int *outData;


__attribute__((used)) int start_game (int i);


int start_game (int i) {
  game_index = i;
  Engine::Start(game_index, kWindowWidth, kWindowHeight); //, textures, models, levels, sounds, NULL);
  return game_index;
}


void mixaudio(void *unused, Uint8 *stream, int len) {
  Engine::CurrentGameDoAudio((short *)stream, len);
}


void draw(void) {
  Engine::CurrentGameDrawScreen(0);
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
  LOGV("key: %d %c\n", key, key);
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
      Engine::Start(game_index, kWindowWidth, kWindowHeight); //, textures, models, levels, sounds, NULL);
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


int is_not_dot_or_dot_dot(const struct dirent *dp) {
  if (strcmp(".", dp->d_name) == 0 || strcmp("..", dp->d_name) == 0) {
    return 0;
  } else {
    return 1;
  }
}


int alphasort(const struct dirent **a, const struct dirent **b) {
  return strcmp((*a)->d_name,(*b)->d_name);
}


GLuint LoadTexture(const char *path) {
  png_t tex;
  unsigned char* data;
  GLuint textureHandle;

  png_init(0, 0);
  png_open_file_read(&tex, path);
  data = (unsigned char*) malloc(tex.width * tex.height * tex.bpp);
  png_get_data(&tex, data);

  glGenTextures(1, &textureHandle);
  glBindTexture(GL_TEXTURE_2D, textureHandle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  png_close_file(&tex);
  free(data);

  return textureHandle;
}


int main(int argc, char** argv) {

  glutInit(&argc,argv);
  glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(kWindowWidth, kWindowHeight);
  glutCreateWindow("does emscripten GLUT wrapper set window.title?, do I need a title? //TODO");
  glutKeyboardFunc(processNormalKeys);
  glutKeyboardUpFunc(processNormalKeys);
  glutMouseFunc(processMouse);
  glutMotionFunc(processMouseMotion);
  glutDisplayFunc(draw);
  glutReshapeFunc(resize);

  struct dirent *dp;
  struct dirent **dps;

  const char *dir_path="../../assets/textures/";
  DIR *dir;

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
        Engine::PushBackFileHandle(TEXTURES, fd, 0, len);
      }
      free(tmp);
      tmp=NULL;
    }
    closedir(dir);
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
        Engine::PushBackFileHandle(MODELS, fd, 0, len);
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
        Engine::PushBackFileHandle(SOUNDS, fd, 0, len);
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
        Engine::PushBackFileHandle(LEVELS, fd, 0, len);
      }

      free(tmp);
      tmp=NULL;
    }
    closedir(dir);
  }

  outData = (short int *)calloc(512, sizeof(short int));

  // Set 16-bit stereo audio at 44.1Khz
  SDL_AudioSpec fmt;
  fmt.freq = 44100;
  fmt.format = AUDIO_S16;
  fmt.channels = 2;
  fmt.samples = 512;
  fmt.callback = mixaudio;
  fmt.userdata = NULL;

  // Open the audio device and start playing sound!
  if (SDL_OpenAudio(&fmt, NULL) < 0) {
    LOGV("No audio: %s\n", SDL_GetError());
  } else {
    SDL_PauseAudio(0);
  }

  Engine::Start(game_index, kWindowWidth, kWindowHeight); //, textures, models, levels, sounds, NULL);

  glutMainLoop();

  return 0;
}
