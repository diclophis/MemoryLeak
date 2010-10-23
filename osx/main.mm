// Vanilla MacOSX OpenGL App

#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#import <QuartzCore/QuartzCore.h>

#include "MemoryLeak.h"

//#define kWindowWidth  480
//#define kWindowHeight 320

#define kWindowWidth  320
#define kWindowHeight 480

static std::vector<GLuint> textures;
static std::vector<foo*> models;
//static RaptorIsland *gameController;
static Engine *gameController;

GLuint loadTexture(NSBitmapImageRep *image) {
	GLuint text = 0;
	
	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glGenTextures(1, &text);
	glBindTexture(GL_TEXTURE_2D, text);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	

	GLuint width = CGImageGetWidth(image.CGImage);
	GLuint height = CGImageGetHeight(image.CGImage);
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	void *imageData = malloc( height * width * 4 );
	CGContextRef context2 = CGBitmapContextCreate( imageData, width, height, 8, 4 * width, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
	CGColorSpaceRelease( colorSpace );
	CGContextClearRect( context2, CGRectMake( 0, 0, width, height ) );
	CGContextTranslateCTM( context2, 0, height - height );
	CGContextDrawImage( context2, CGRectMake( 0, 0, width, height ), image.CGImage );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	CGContextRelease(context2);
	free(imageData);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	
	return text;
}


void draw(void) {
	gameController->draw(0);
  glutSwapBuffers();
}


void resize(int width, int height) {
  gameController->resizeScreen(width, height);
}


void processMouse(int button, int state, int x, int y) {
	gameController->hitTest(x, y);
}


void processMouseMotion(int x, int y) {
	gameController->hitTest(x, y);
}


int main(int argc, char** argv) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  //if (argc > 1) {
  //  glutGameModeString("1440x900:32@65");
  //  glutEnterGameMode();
  //} else {
    glutInitWindowSize (kWindowWidth, kWindowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
  //}

	NSArray *model_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../assets/models"];
	for (NSString *path in model_names) {
		FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
		fseek(fd, 0, SEEK_END);
		unsigned int len = ftell(fd);
		rewind(fd);

		foo *firstModel = new foo;
		firstModel->fp = fd;
		firstModel->off = 0;
		firstModel->len = len;
		
		models.push_back(firstModel);
	}

	NSArray *texture_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../assets/textures"];
	for (NSString *path in texture_names) {
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    NSBitmapImageRep *image = [NSBitmapImageRep imageRepWithData:texData];

    if (image == nil) {
      throw 1;
    }

	  textures.push_back(loadTexture(image));
    [image release];
    [texData release];
  }

  if (argc > 1) {
    gameController = new RaptorIsland(kWindowWidth, kWindowHeight, textures, models);
  } else {
    gameController = new RunAndJump(kWindowWidth, kWindowHeight, textures, models);
  }

  gameController->go();

	glutMouseFunc(processMouse);
	glutMotionFunc(processMouseMotion);
  glutDisplayFunc(draw);
	glutIdleFunc(draw);
  glutReshapeFunc(resize);
  glutMainLoop();

	[pool release];

  return 0;
}
