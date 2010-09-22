
#include <stdio.h>
#include <OpenGL/gl.h>    // Header File For The OpenGL32 Library
#include <OpenGL/glu.h>   // Header File For The GLu32 Library
#include <GLUT/glut.h>    // Header File For The GLut Library

#include "OpenSteer/Vec3.h"
#include "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/Color.h"
#include "CaptureTheFlag.h"
#include "RaptorIsland.h"


#define kWindowWidth  400
#define kWindowHeight 300

static FILE *myFile1;
static unsigned int fileOffset1;
static unsigned int fileLength1;

static FILE *myFile2;
static unsigned int fileOffset2;
static unsigned int fileLength2;

static FILE *myFile3;
static unsigned int fileOffset3;
static unsigned int fileLength3;

static std::vector<GLuint> sPlayerTextures;
static RaptorIsland *gameController;

void InitGL(void) {
}

void DrawGLScene(void) {
  printf("draw\n");
  glutSwapBuffers();
}

void ReSizeGLScene(int width, int height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize (kWindowWidth, kWindowHeight);
  glutInitWindowPosition(100, 100);
  glutCreateWindow(argv[0]);

  //InitGL();
	//for (int i=0; i<13; i++) {
	//	sPlayerTextures.push_back(env->GetIntArrayElements(arr, 0)[i]);
	//}

	std::vector<foo*> models;
		
	foo firstModel; // = new foo;
	firstModel.fp = myFile1;
	firstModel.off = fileOffset1;
	firstModel.len = fileLength1;
	
	models.push_back(&firstModel);

	foo secondModel; // = new foo;
	secondModel.fp = myFile2;
	secondModel.off = fileOffset2;
	secondModel.len = fileLength2;
	
	models.push_back(&secondModel);

	foo thirdModel; // = new foo;
	thirdModel.fp = myFile3;
	thirdModel.off = fileOffset3;
	thirdModel.len = fileLength3;
	
	models.push_back(&thirdModel);

  gameController = new RaptorIsland();
  gameController->build(kWindowWidth, kWindowHeight, sPlayerTextures, models);
	models.clear();
	sPlayerTextures.clear();


  glutDisplayFunc(DrawGLScene);
  glutReshapeFunc(ReSizeGLScene);

  glutMainLoop();

  return 0;
}


