// Vanilla Linux OpenGL+GLUT+SOIL App

/*
#include <stdio.h>

int main(int argc, char** argv) {

  printf("fuck yea\n");

  return 0;
}
*/

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

#include "MemoryLeak.h"

#include <dirent.h>
#include <vector>

#define kWindowWidth 320
#define kWindowHeight 480

static pthread_t audio_thread;

static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> sounds;
static std::vector<foo*> levels;

static int game_index = 1;
static bool left_down = false;
static bool right_down = false;
static bool reset_down = false;
static bool debug_down = false;


void *pump_audio(void *) {
  LOGV("pump audio\n");
  return NULL;
}


void draw(void) {
  LOGV("Draw\n");
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

  glutInit(&argc,argv);
  glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(kWindowWidth, kWindowHeight);
  //glutInitWindowPosition(0,0);
  glutCreateWindow("simple");
  glutKeyboardFunc(processNormalKeys);
  glutKeyboardUpFunc(processNormalKeys);
  //glutIgnoreKeyRepeat(true);
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
        GLuint tex_2d = 0;
        //SOIL_load_OGL_texture(
        //  tmp,
        //  SOIL_LOAD_AUTO,
        //  SOIL_CREATE_NEW_ID,
        //  SOIL_FLAG_MIPMAPS | SOIL_FLAG_MULTIPLY_ALPHA
        //);
        textures.push_back(tex_2d);
      }
    }
  }

  /*
  int dir_ents = 0; //scandir(dir_path, &dps, is_not_dot_or_dot_dot, alphasort);

  if (dir_ents == -1) {
    LOGV("wtf\n");
    exit(1);
  }

  for (int i=0; i<dir_ents; i++) {
    char *tmp;
    tmp = path_cat(dir_path, dps[i]->d_name);
    GLuint tex_2d = 0;
    //SOIL_load_OGL_texture(
    //  tmp,
    //  SOIL_LOAD_AUTO,
    //  SOIL_CREATE_NEW_ID,
    //  SOIL_FLAG_MIPMAPS | SOIL_FLAG_MULTIPLY_ALPHA
    //);
    textures.push_back(tex_2d);
    free(tmp);
    tmp=NULL;
  }
  */

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

  Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, NULL);
  pthread_create(&audio_thread, 0, pump_audio, NULL);

  glutMainLoop();

  return 0;
}
