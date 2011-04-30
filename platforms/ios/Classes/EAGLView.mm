//
//  EAGLView.m
//  WangCrapTemplate
//
//  Created by Jon Bardin on 7/12/10.
//  GPL
//

#import "MemoryLeakAppDelegate.h"
#include "MemoryLeak.h"
#include "Model.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Engine.h"
#include "octree.h"
#include "micropather.h"
#include "ModelOctree.h"

#include "SuperBarrelBlast.h"


#import "EAGLView.h"

EAGLView *g_View;
pthread_mutex_t m_Mutex;
pthread_cond_t m_AudioSyncCond;
bool m_SyncAudio;

pthread_mutex_t play_mutex;

static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> levels;
static std::vector<foo*> sounds;

static volatile int buffer_ana_gen_ofs,buffer_ana_play_ofs;

#define PLAYBACK_FREQ 44100
 
void interruptionListenerCallback (void *inUserData,UInt32 interruptionState ) {
	LOGV("interupption\n");
	if (interruptionState == kAudioSessionBeginInterruption) {
		//TODO: phone call interuption, pause game?
	} else if (interruptionState == kAudioSessionEndInterruption) {
		// if the interruption was removed, and the app had been playing, resume playback
		UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
		AudioSessionSetProperty (
								 kAudioSessionProperty_AudioCategory,
								 sizeof (sessionCategory),
								 &sessionCategory
								 );
		AudioSessionSetActive (true);
	}
}

void propertyListenerCallback (void                   *inUserData,                             
							   AudioSessionPropertyID inPropertyID,                                
							   UInt32                 inPropertyValueSize,                         
							   const void             *inPropertyValue ) {
	LOGV("property\n");

	if (inPropertyID==kAudioSessionProperty_AudioRouteChange ) {
		CFDictionaryRef routeChangeDictionary = (CFDictionaryRef)inPropertyValue;
		NSString *oldroute = (NSString*)CFDictionaryGetValue (
															  routeChangeDictionary,
															  CFSTR (kAudioSession_AudioRouteChangeKey_OldRoute)
															  );
		NSLog(@"Audio route changed : %@",oldroute);
		if ([oldroute compare:@"Headphone"]==NSOrderedSame) {	
			//TODO: pause on headphone?
		}
	}	
}


@implementation EAGLView


@synthesize animating;
@dynamic animationFrameInterval;


// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
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
		return;
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
		return;
	}
	
	[self startGame];
	
	animating = FALSE;
	displayLinkSupported = FALSE;
	animationFrameInterval = 1;
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


-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		NSSet *allTouches = [event allTouches];
		CGRect bounds;
		CGPoint location;
		bounds = [self bounds];
		for (UITouch *touch in touches) {			
			//touch = [[allTouches allObjects] objectAtIndex:0];
			location = [touch locationInView:self];
			location.y = location.y;
			game->Hit(location.x, location.y, 0);
		}
	}
}


-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		NSSet *allTouches = [event allTouches];
		CGRect bounds;
		CGPoint location;
		bounds = [self bounds];
		for (UITouch *touch in touches) {			
			//touch = [[allTouches allObjects] objectAtIndex:0];
			location = [touch locationInView:self];
			location.y = location.y;
			game->Hit(location.x, location.y, 1);
		}
	}
}


-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		NSSet *allTouches = [event allTouches];
		CGRect bounds;
		CGPoint location;
		bounds = [self bounds];
		for (UITouch *touch in touches) {			
			//touch = [[allTouches allObjects] objectAtIndex:0];
			location = [touch locationInView:self];
			location.y = location.y;
			game->Hit(location.x, location.y, 2);
		}
	}
}


-(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		NSSet *allTouches = [event allTouches];
		CGRect bounds;
		UITouch* touch;
		bounds = [self bounds];
		touch = [[allTouches allObjects] objectAtIndex:0];
		CGPoint location;
		location = [touch locationInView:self];
		location.y = location.y;
		game->Hit(location.x, location.y, -1);
	}
}


-(void)drawView:(id)sender {
	if (animating) {
		m_SyncAudio = true;
		[EAGLContext setCurrentContext:context];
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
		game->DrawScreen(0);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
		[context presentRenderbuffer:GL_RENDERBUFFER_OES];
	}
}


GLuint loadTexture(UIImage *image) {
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


void checkStatus(OSStatus status) {
	if(status == 0)
		NSLog(@"success");
	else if(status == errSecNotAvailable)
		NSLog(@"no trust results available");
	else if(status == errSecItemNotFound)
		NSLog(@"the item cannot be found");
	else if(status == errSecParam)
		NSLog(@"parameter error");
	else if(status == errSecAllocate)
		NSLog(@"memory allocation error");
	else if(status == errSecInteractionNotAllowed)
		NSLog(@"user interaction not allowd");
	else if(status == errSecUnimplemented)
		NSLog(@"not implemented");
	else if(status == errSecDuplicateItem)
		NSLog(@"item already exists");
	else if(status == errSecDecode)
		NSLog(@"unable to decode data");
	else
		NSLog(@"unknown: %d", status);
	
}

static OSStatus playbackCallback(void *inRefCon, 
								 AudioUnitRenderActionFlags *ioActionFlags, 
								 const AudioTimeStamp *inTimeStamp, 
								 UInt32 inBusNumber, 
								 UInt32 inNumberFrames, 
								 AudioBufferList *ioDataList) {

	// Notes: ioData contains buffers (may be more than one!)
    // Fill them up as much as you can. Remember to set the size value in each buffer to match how
    // much data is in the buffer.
	
	if (ioDataList->mNumberBuffers != 1) {
		LOGV("the fuck\n");
	}
	
	AudioBuffer *ioData = &ioDataList->mBuffers[0];
	
	LOGV("wants: %d %d %d\n", inNumberFrames, ioDataList->mNumberBuffers, ioData->mDataByteSize);
	
	//Signal to get audio
	Engine *f = [g_View game];
	if (f) {
		memcpy(ioData->mData, f->DoAudio((int)ioData->mDataByteSize), ioData->mDataByteSize);
	}
		
    return noErr;
}

-(void)initAudio2 {
	AudioSessionInitialize (NULL, NULL, interruptionListenerCallback, NULL);
	status = AudioSessionSetActive (true);
	checkStatus(status);
	UInt32 sessionCategory = kAudioSessionCategory_AmbientSound;
	AudioSessionSetProperty (kAudioSessionProperty_AudioCategory, sizeof (sessionCategory), &sessionCategory);

	#define kOutputBus 0

	// Describe audio component
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	// Get component
	AudioComponent inputComponent = AudioComponentFindNext(NULL, &desc);
	
	// Get audio units
	status = AudioComponentInstanceNew(inputComponent, &audioUnit);
	checkStatus(status);
	
	UInt32 flag = 1;
	
	// Enable IO for playback
	status = AudioUnitSetProperty(audioUnit, 
								  kAudioOutputUnitProperty_EnableIO, 
								  kAudioUnitScope_Output, 
								  kOutputBus,
								  &flag, 
								  sizeof(flag));
	checkStatus(status);
	
	size_t bytesPerSample = sizeof(short);
	AudioStreamBasicDescription audioFormat = {0};

	
	audioFormat.mSampleRate			= PLAYBACK_FREQ;
	audioFormat.mFormatID			= kAudioFormatLinearPCM;
	audioFormat.mFormatFlags		= kAudioFormatFlagsCanonical;
	audioFormat.mFramesPerPacket	= 1;
	audioFormat.mChannelsPerFrame	= 1;
	audioFormat.mBitsPerChannel		= 16;
	audioFormat.mBytesPerPacket		= bytesPerSample;
	audioFormat.mBytesPerFrame		= bytesPerSample;
	
	status = AudioUnitSetProperty(audioUnit, 
								  kAudioUnitProperty_StreamFormat, 
								  kAudioUnitScope_Input, 
								  kOutputBus, 
								  &audioFormat, 
								  sizeof(audioFormat));
	checkStatus(status);
	
	AURenderCallbackStruct callbackStruct;

	// Set output callback
	callbackStruct.inputProc = playbackCallback;
	callbackStruct.inputProcRefCon = self;
	status = AudioUnitSetProperty(audioUnit, 
								  kAudioUnitProperty_SetRenderCallback, 
								  kAudioUnitScope_Global, 
								  kOutputBus,
								  &callbackStruct, 
								  sizeof(callbackStruct));
	checkStatus(status);
	
	Float32 aBufferLength; // In seconds
	UInt32 size = sizeof(aBufferLength);
	
    status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration,
							&size, &aBufferLength);
	checkStatus(status);

	LOGV("current duration: %f\n", aBufferLength);
	
	aBufferLength = 0.005;
	
	status = AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, 
							size, &aBufferLength);
	checkStatus(status);

	status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration,
							&size, &aBufferLength);
	
	LOGV("current duration: %f\n", aBufferLength);
	
	
	checkStatus(status);

	// Initialise
	status = AudioUnitInitialize(audioUnit);
	checkStatus(status);
}



-(void)startGame {
	if (game != NULL) {
		models.clear();
		textures.clear();
		levels.clear();
		delete game;

	}
	
	{
		NSArray *model_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"assets/models"];
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
		
		NSArray *level_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"assets/levels"];
		for (NSString *path in level_names) {
			FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
			fseek(fd, 0, SEEK_END);
			unsigned int len = ftell(fd);
			rewind(fd);
			foo *firstLevel = new foo;
			firstLevel->fp = fd;
			firstLevel->off = 0;
			firstLevel->len = len;
			levels.push_back(firstLevel);
		}

		NSArray *texture_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"assets/textures"];
		for (NSString *path in texture_names) {
			NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
			UIImage *image = [[UIImage alloc] initWithData:texData];		

			if (image == nil) {
				throw 1;
			}

			textures.push_back(loadTexture(image));
			[image release];
			[texData release];
		}
		
		
		NSArray *sound_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"assets/sounds"];
		for (NSString *path in sound_names) {
			FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
			fseek(fd, 0, SEEK_END);
			unsigned int len = ftell(fd);
			rewind(fd);
			foo *firstSound = new foo;
			firstSound->fp = fd;
			firstSound->off = 0;
			firstSound->len = len;
			sounds.push_back(firstSound);
		}
		
		
		pthread_cond_init(&m_AudioSyncCond, NULL);
		pthread_mutex_init(&m_Mutex, NULL);
		
		//ring = alloc();

		
		m_SyncAudio = false;
		
		
		[self initAudio2];
		status = AudioOutputUnitStart(audioUnit);
		checkStatus(status);
		
		g_View = self;

		game = new SuperBarrelBlast(self.layer.frame.size.width, self.layer.frame.size.height, textures, models, levels, sounds);
		game->CreateThread();
		
		gameState = 1;
	}
}

-(Engine *)game {
	return game;
}

-(void)reset {
	[self stopAnimation];	
}


-(void)parse:(const char*)json withLength:(size_t)length {
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
		[self build];
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
		game->PauseThread();
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
