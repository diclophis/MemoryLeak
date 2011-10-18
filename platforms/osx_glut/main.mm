// Vanilla MacOSX OpenGL App

#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#import <QuartzCore/QuartzCore.h>
#include <CoreAudio/AudioHardware.h>

#include "MemoryLeak.h"

#define kWindowWidth 320
#define kWindowHeight 480

static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> sounds;
static std::vector<foo*> levels;

static int game_index = 0;

bool pushMessageToWebView(const char *theMessage) {
	return true;
}

const char *popMessageFromWebView() {
  return "";
}

void SimulationThreadCleanup() {
}

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
  Engine::CurrentGameDrawScreen(0);
  glutSwapBuffers();
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
  printf("key: %d %c\n", key, key);

  game_index += 1;
  if (game_index > 2) {
    game_index = 0;
  }

  Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, pushMessageToWebView, popMessageFromWebView, SimulationThreadCleanup);
/*
  switch (key) {
    case 27:
    gameController->pause();
    break;

    case 13:
    gameController->hitTest(x, y, 0);
    gameController->hitTest(x, y, 2);
    break;

    case 113:
    glDepthFunc(GL_NEVER);
    break;

    case 119:
    glDepthFunc(GL_LESS);
    break;

    case 101:
    glDepthFunc(GL_EQUAL);
    break;

    case 114:
    glDepthFunc(GL_LEQUAL);
    break;

    case 116:
    glDepthFunc(GL_GREATER);
    break;

    case 121:
    glDepthFunc(GL_NOTEQUAL);
    break;

    case 117:
    glDepthFunc(GL_GEQUAL);
    break;

    case 105:
    glDepthFunc(GL_ALWAYS);
    break;

    case 111:
    glDisable(GL_BLEND);
    break;

    case 112:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    break;
  }

  LOGV("the fuck: %d\n", key);
*/
}

int main(int argc, char** argv) {

  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  glutInitWindowSize(kWindowWidth, kWindowHeight);
  glutInitWindowPosition(1000, 500);
  glutCreateWindow("main");

	NSArray *model_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../../assets/models"];
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

	NSArray *texture_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../../assets/textures"];
	for (NSString *path in texture_names) {
    NSLog(@"path: %@", path);
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    NSBitmapImageRep *image = [NSBitmapImageRep imageRepWithData:texData];
    
    if (image == nil) {
      throw 1;
    }

	  textures.push_back(loadTexture(image));
    [image release];
    [texData release];
  }

	NSArray *level_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../../assets/levels"];
	for (NSString *path in level_names) {
		FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
		fseek(fd, 0, SEEK_END);
		unsigned int len = ftell(fd);
		rewind(fd);

		foo *firstModel = new foo;
		firstModel->fp = fd;
		firstModel->off = 0;
		firstModel->len = len;
		
		levels.push_back(firstModel);
	}

	NSArray *sound_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../../assets/sounds"];
	for (NSString *path in sound_names) {
		FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
		fseek(fd, 0, SEEK_END);
		unsigned int len = ftell(fd);
		rewind(fd);

		foo *firstModel = new foo;
		firstModel->fp = fd;
		firstModel->off = 0;
		firstModel->len = len;
		
		sounds.push_back(firstModel);
	}

  Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, pushMessageToWebView, popMessageFromWebView, SimulationThreadCleanup);

  glutKeyboardFunc(processNormalKeys);
  glutMouseFunc(processMouse);
  glutMotionFunc(processMouseMotion);
  glutDisplayFunc(draw);
  glutIdleFunc(draw);
  glutReshapeFunc(resize);
  glutMainLoop();

  [pool release];

  return 0;
}
