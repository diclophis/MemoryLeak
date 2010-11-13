// Vanilla MacOSX OpenGL App

#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#import <QuartzCore/QuartzCore.h>
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

#import "GLView.h"

#include "MemoryLeak.h"
#include "Audio.h"
#include "Model.h"
#include "Interpretator.h"
#include "Engine.h"
#include "MachineGun.h"
#include "RunAndJump.h"

//#define kWindowWidth  480
//#define kWindowHeight 320
#define kWindowWidth  320
#define kWindowHeight 480


@interface Skeleton : NSObject {
  Engine *game;
  std::vector<GLuint> textures;
  std::vector<foo*> models;
}

-(id)init;
-(GLuint)loadTexture:(NSBitmapImageRep *)image;
-(void)draw:(id)userInfo;

@end

@implementation Skeleton

-(id)init {
  self = [super init];

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  id menubar = [[NSMenu new] autorelease];
  id appMenuItem = [[NSMenuItem new] autorelease];
  [menubar addItem:appMenuItem];
  [NSApp setMainMenu:menubar];
  id appMenu = [[NSMenu new] autorelease];
  id appName = [[NSProcessInfo processInfo] processName];
  id quitTitle = [@"Quit " stringByAppendingString:appName];
  id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
  [appMenu addItem:quitMenuItem];
  [appMenuItem setSubmenu:appMenu];
  id glWindow = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, kWindowWidth, kWindowHeight) styleMask:NSTitledWindowMask | NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO] autorelease];
  [glWindow cascadeTopLeftFromPoint:NSMakePoint(0,0)];
  [glWindow setTitle:appName];

  id webWindow = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, kWindowWidth, kWindowHeight) styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO] autorelease];

  [webWindow setOpaque:NO];
  [webWindow setBackgroundColor:[NSColor clearColor]];

  WebView *webView = [[WebView alloc] initWithFrame:NSMakeRect(0, 0, 320, 480)];
  [webView setPolicyDelegate:self];
  [webView setWantsLayer:YES];
  [webView setDrawsBackground:NO];

  WebFrame *mainFrame = [webView mainFrame];
  NSURL *url = [[NSBundle mainBundle] URLForResource:@"index" withExtension:@"html" subdirectory:@"../../../assets"];
  [[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
  
  NSOpenGLPixelFormatAttribute attributes[] = { NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize, 32, 0 };

  NSOpenGLPixelFormat *format; format = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
  GLView *glView = [[GLView alloc] initWithFrame:NSMakeRect(0, 0, 320, 480) pixelFormat:format]; 

  ///////////////////
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
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    NSBitmapImageRep *image = [NSBitmapImageRep imageRepWithData:texData];

    if (image == nil) {
      throw 1;
    }

	  textures.push_back([self loadTexture:image]);
    [image release];
    [texData release];
  }
  ////////////////////

  ///////////////////////
  game = new RunAndJump(kWindowWidth, kWindowHeight, textures, models);
  [NSTimer scheduledTimerWithTimeInterval:1.0/60.0 target:self selector:@selector(draw:) userInfo:nil repeats:YES];
  //////////////////////


  [glWindow setContentView:glView];
  [webWindow setContentView:webView];
  [glWindow addChildWindow:webWindow ordered:NSWindowAbove];

  
  [glWindow makeKeyAndOrderFront:nil];
  [NSApp activateIgnoringOtherApps:YES];
  [NSApp run];
  [pool release];
}


-(void)draw:(id)userInfo {
  game->draw(0);
}


- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id)listener {
  NSLog(@"the fuck: %@", [request URL]);
  NSString *fragment = [[[request mainDocumentURL] fragment] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
	//NSLog(@"the fuck: %@", [[request mainDocumentURL] scheme]);
	//NSLog(@"the fuck: %@", [[request mainDocumentURL] path]);
	NSLog(@"the fuck: %@", fragment);
	if ([fragment length] == 0 && [[[request mainDocumentURL] scheme] isEqualToString:@"file"]) {
    [listener use];
	} else {
    game->parse([fragment cStringUsingEncoding:NSUTF8StringEncoding], [fragment length]);
    [listener ignore];
	}
}

-(GLuint)loadTexture:(NSBitmapImageRep *)image {
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


@end


int main(int argc, char** argv) {

  //glutInit(&argc, argv);
  //glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  //glutGameModeString("1440x900:32@65");

  //glutInitWindowSize(kWindowWidth, kWindowHeight);
  //glutInitWindowPosition(1000, 500);
  //glutCreateWindow(argv[0]);

  /*
  */













//  Audio *foo = new Audio();

  //gameController = new RunAndJump(kWindowWidth, kWindowHeight, textures, models, js);
  //gameController->go();

//  glutKeyboardFunc(processNormalKeys);
//	glutMouseFunc(processMouse);
//	glutMotionFunc(processMouseMotion);
//  glutDisplayFunc(draw);
//	glutIdleFunc(draw);
//  glutReshapeFunc(resize);
//  glutMainLoop();

  LOGV("the fuck\n");

  [[Skeleton alloc] init];
  return 0;
}
