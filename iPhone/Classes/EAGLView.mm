//
//  EAGLView.m
//  WangCrapTemplate
//
//  Created by Jon Bardin on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "EAGLView.h"

#include "OpenSteer/Vec3.h"
#include "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/Color.h"
#include "CaptureTheFlag.h"
#include "RaptorIsland.h"

#import "MemoryLeakAppDelegate.h"

#import <mach/mach_time.h>

#include <unistd.h>

static std::vector<GLuint> textures;
static std::vector<foo*> models;

@implementation EAGLView


@synthesize animating;
@dynamic animationFrameInterval;


// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}


-(id)initWithFrame:(CGRect)theFrame {
	if ((self = [super initWithFrame:theFrame])) {
		[self build];
	}
	return self;
}


-(void)build {
	[self setClearsContextBeforeDrawing:NO];
	[self setBackgroundColor:[UIColor blackColor]];
	
	// Get the layer
	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
	
	eaglLayer.opaque = TRUE;
	eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
									[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
	
	
	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	
	if (!context || ![EAGLContext setCurrentContext:context]) {
		[self release];
		//return nil;
	}
	
	// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
	glGenFramebuffersOES(1, &defaultFramebuffer);
	glGenRenderbuffersOES(1, &colorRenderbuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);
	
	//Bind framebuffers to the context and this layer
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:eaglLayer];
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
	
	glGenRenderbuffersOES(1, &depthRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
	
	if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		//return NO;
	}
	
	[self startGame];
	
	animating = FALSE;
	displayLinkSupported = FALSE;
	animationFrameInterval =  1;
	displayLink = nil;
	animationTimer = nil;
	
	// A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
	// class is used as fallback when it isn't available.
	NSString *reqSysVer = @"3.1";
	NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
	if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending) {
		displayLinkSupported = TRUE;
	}
}


-(id)initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
		[self build];
	}

    return self;
}


-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		//gameController->playerStartedJumping();
	}
}


-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
}


-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		//gameController->playerStoppedJumping();
	}
}


-(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		//gameController->playerStoppedJumping();
	}
}


-(void)drawView:(id)sender {
	if (animating) {
		[EAGLContext setCurrentContext:context];
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
		if (gameState) {
			//gameController->tick();
			gameController->draw(90);
		} else {
			[self startGame];
		}
		
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
		[context presentRenderbuffer:GL_RENDERBUFFER_OES];
	}
}


-(GLuint)loadTexture:(NSString *)filename ofType:(NSString *)type {
	GLuint text = 0;
	
	//Texture loading is done here
	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glGenTextures(1, &text);
	glBindTexture(GL_TEXTURE_2D, text);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	
	NSString *path = [[NSBundle mainBundle] pathForResource:filename ofType:type inDirectory:@"assets/textures"];
	//NSString *extension = [path pathExtension];
	NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
	//float inWidth = 512.0;
	//float inHeight = 512.0;
	// Assumes pvr4 is RGB not RGBA, which is how texturetool generates them
	//if ([extension isEqualToString:@"pvr4"]) {
	//	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, inWidth, inHeight, 0, (inWidth * inHeight) / 2, [texData bytes]);
	//} else if ([extension isEqualToString:@"pvr2"]) {
	//	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, inWidth, inHeight, 0, (inWidth * inHeight) / 2, [texData bytes]);
	//} else {
		UIImage *image = [[UIImage alloc] initWithData:texData];
		if (image == nil) {
			throw 1;
		}
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
		[image release];
	//}
	[texData release];
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	
	return text;
}


-(void)startGame {
	
	if (gameController != NULL) {
		delete gameController;
	}
	
	NSArray *model_names = [NSArray arrayWithObjects:@"raptor", @"barrel", @"crate", nil];
	
	for (NSString *model_name in model_names) {
		
		FILE *fd = fopen([[[NSBundle mainBundle] pathForResource:model_name ofType:@"wav" inDirectory:@"assets/models"] cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
		fseek(fd, 0, SEEK_END);
		unsigned int len = ftell(fd);
		rewind(fd);

		foo *firstModel = new foo;
		firstModel->fp = fd;
		firstModel->off = 0;
		firstModel->len = len;
		
		models.push_back(firstModel);
		
	}

	//raptor
	textures.push_back([self loadTexture:@"raptor" ofType:@"png"]);

	// font
	textures.push_back([self loadTexture:@"font_01" ofType:@"png"]);

	//tree
	//textures.push_back([self loadTexture:@"vincent" ofType:@"png"]);

	//barrel
	textures.push_back([self loadTexture:@"barrel_03" ofType:@"jpg"]);

	//crate
	textures.push_back([self loadTexture:@"crate_01" ofType:@"jpg"]);

	//skybox
	textures.push_back([self loadTexture:@"skybox_01" ofType:@"png"]);

	gameController = new RaptorIsland();
	

	gameController->build(self.layer.frame.size.width, self.layer.frame.size.height, textures, models);
	gameState = 1;
	

	models.clear();
	textures.clear();
}


-(void)reset {
	[self stopAnimation];	
}


-(NSInteger)animationFrameInterval {
    return animationFrameInterval;
}


- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;

        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}


-(void)startAnimation {
    if (!animating) {
		animating = TRUE;
        if (displayLinkSupported) {
            displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
            [displayLink setFrameInterval:animationFrameInterval];
            [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        }		
    }
}


-(void)stopAnimation {
    if (animating) {
        if (displayLinkSupported) {
            [displayLink invalidate];
            displayLink = nil;
        } else {
            [animationTimer invalidate];
            animationTimer = nil;
        }
        animating = FALSE;
    }
}

-(void)dealloc {	
	// Tear down GL
    if (defaultFramebuffer) {
        glDeleteFramebuffersOES(1, &defaultFramebuffer);
        defaultFramebuffer = 0;
    }
	
    if (colorRenderbuffer) {
        glDeleteRenderbuffersOES(1, &colorRenderbuffer);
        colorRenderbuffer = 0;
    }
	
	if(depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
	
    // Tear down context
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
	}
	

	
    [context release];
    context = nil;
	
    [super dealloc];
}


@end