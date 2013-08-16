// Vanilla Linux OpenGL+GLUT+SOIL App

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

#include "MemoryLeak.h"

#include <dirent.h>
#include <vector>

//#include "SDL.h"
//#include "SDL_audio.h"

//#define kWindowWidth 1024
//#define kWindowHeight 1024


static int game_index = 0;
static bool left_down = false;
static bool right_down = false;
static bool reset_down = false;
static bool debug_down = false;

int kWindowWidth = 0;
int kWindowHeight = 0;

extern "C" {


int __attribute__((used)) start_game (int i);
int __attribute__((used)) start_game (int i) {
  game_index = i;
  Engine::Start(game_index, kWindowWidth, kWindowHeight);
  return game_index;
}

//int __attribute__((used)) command (int i, const char *s);
//int __attribute__((used)) command (int i, const char *s) {
//  return Engine::CurrentGameCommand(i, s);
//}

//typedef void (__cdecl * SinkJs_writeCallback) (void *buffer, int size, int channels);
//void sinkJsInit(SinkJs_writeCallback writeFunc, int frames, int sizeOfFrames, int channels);


}

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen) {
  //m_Socket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon
  if (SO_ERROR == optname) {
    optval = 0;
    return 0;
  }

  return 0;
}


//void sinkJsWriteFunc(void *buffer, int size, int channels) {
//  Engine::CurrentGameDoAudio(buffer, size);
//}


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
      Engine::Start(game_index, kWindowWidth, kWindowHeight);
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


int main(int argc, char** argv) {
  char *wh;
  
  wh = emscripten_run_script_string("(function(){return document.body.offsetWidth;})()");
  kWindowWidth = atoi(wh);

  wh = emscripten_run_script_string("(function(){return document.body.offsetHeight;})()");
  kWindowHeight = atoi(wh);

  LOGV("%s %d %d\n", wh, kWindowWidth, kWindowHeight);


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
        Engine::PushBackFileHandle(TEXTURES, fd, 0, len, tmp);
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
        Engine::PushBackFileHandle(MODELS, fd, 0, len, tmp);
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
        Engine::PushBackFileHandle(SOUNDS, fd, 0, len, tmp);
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
        Engine::PushBackFileHandle(LEVELS, fd, 0, len, tmp);
      }

      free(tmp);
      tmp=NULL;
    }
    closedir(dir);
  }

  //sinkJsInit(sinkJsWriteFunc, 4096, sizeof(short), 2);


  Engine::Start(game_index, kWindowWidth, kWindowHeight);

  glutMainLoop();

  return 0;
}
