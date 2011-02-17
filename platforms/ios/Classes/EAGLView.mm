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
#include "MainMenu.h"


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

static AudioUnitSampleType **buffer_ana;
static volatile int buffer_ana_gen_ofs,buffer_ana_play_ofs;
static volatile int *buffer_ana_flag;

#define PLAYBACK_FREQ 8000
#define AUDIO_BUFFER_SIZE (186)
#define BUF_SIZE 4

struct buf_t {
	volatile int writepos;
	volatile void **buffer[BUF_SIZE][AUDIO_BUFFER_SIZE]; 
    volatile int readpos;
};

void *produce (buf_t *b) {
    int next = (b->writepos+1) % BUF_SIZE;
    if (b->readpos == next); // queue is full. wait
	b->writepos = next;
	return b->buffer[b->writepos];
}

volatile void *consume (buf_t *b) {
    //while (b->readpos == b->writepos); // nothing to consume. wait
	if (b->readpos == b->writepos) {
		return NULL;
	}
	
    int next = (b->readpos+1) % BUF_SIZE;
    volatile void *res = b->buffer[b->readpos]; b->readpos = next;
    return res;
}

buf_t *ring;

buf_t *alloc () {
    buf_t *b = (buf_t *)malloc(sizeof(buf_t));
    b->writepos = 0; b->readpos = 0; return b;
}


class Callbacks {
	static void *PumpAudio(void *b, int buffer_position, int d) {

			//void *f = (void *)malloc(sizeof(short) * 1024);
		memcpy(produce(ring), b, AUDIO_BUFFER_SIZE);

		//memcpy(buffer_ana[buffer_ana_gen_ofs], (short *)b + (i * len), len);

		/*
		if (m_SyncAudio) {
			pthread_mutex_lock(&m_Mutex);
			pthread_cond_wait(&m_AudioSyncCond, &m_Mutex);
			pthread_mutex_unlock(&m_Mutex);
		}
		
		bool r = true;
		
		int t = (AUDIO_BUFFER_SIZE / AUDIO_SEG);
		
		int len = AUDIO_SEG;
			
		for (unsigned int i=0; i<t; i++) {
			//LOGV("!!!! %d\n", buffer_ana_flag[buffer_ana_gen_ofs]);
			//if (buffer_ana_flag[buffer_ana_gen_ofs]) {
			//	LOGV("skip\n");
			//	return (void *)false;
			//}
			
			memcpy(buffer_ana[buffer_ana_gen_ofs], (short *)b + (i * len), len);
			buffer_ana_flag[buffer_ana_gen_ofs] = 1;
			buffer_ana_gen_ofs++;
			if ((buffer_ana_gen_ofs) == SOUND_BUFFER_NB) {
				buffer_ana_gen_ofs = 0;
			}
			r = true;
		}
		
		//LOGV(" end %d \n", buffer_ana_gen_ofs);
		
		//pthread_cond_signal(&m_AudioSyncCond);
		
		
		
		return (void *)r;
		*/
		
		return (void *)true;
	};
};

 
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
			game->Hit(location.x, location.y, 1);
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
			game->Hit(location.x, location.y, 1);
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
		game->Hit(location.x, location.y, 0);
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
	
	//LOGV("wants: %d %d %d\n", inNumberFrames, ioDataList->mNumberBuffers, ioData->mDataByteSize);
	
	if (m_SyncAudio) {
		void *b;
		if ((b = (void *)consume(ring))) {
			memcpy(ioData->mData, b, ioData->mDataByteSize);
		} else {
			*ioActionFlags = kAudioUnitRenderAction_OutputIsSilence;
			memset(ioData->mData, 0, ioData->mDataByteSize);

			//for(unsigned int currentBuffer = 0; currentBuffer < ioData->mNumberBuffers; ++currentBuffer) {
			//	memset(ioData->mBuffers[currentBuffer].mData, 0, ioData->mBuffers[currentBuffer].mDataByteSize);
			//}
		}			
	}
	
	//LOGV("got: %d\n", ioData->mDataByteSize);

	/*
	int len = AUDIO_SEG;

	int desired = ioData->mDataByteSize;
	
	ioData->mDataByteSize = 0;

	int got = 0;
	int tried = 0;
	
	//buffer_ana_play_ofs = 0;	
	//while (buffer_ana_play_ofs < SOUND_BUFFER_NB) {
	
	while (tried++ < SOUND_BUFFER_NB && ioData->mDataByteSize < desired) {
	
		if (buffer_ana_play_ofs >= SOUND_BUFFER_NB) {
			buffer_ana_play_ofs = 0;
		}
		
		if (buffer_ana_flag[buffer_ana_play_ofs]) {
			
			//LOGV("#### gen: %d=>%d play: %d=>%d\n", buffer_ana_gen_ofs, buffer_ana_flag[buffer_ana_gen_ofs], buffer_ana_play_ofs, buffer_ana_flag[buffer_ana_play_ofs]);

			memcpy(ioData->mData, buffer_ana[buffer_ana_play_ofs], len);
			ioData->mDataByteSize += len;
			buffer_ana_flag[buffer_ana_play_ofs] = 0;
			got++;
		}
		
		buffer_ana_play_ofs++;
	}
	
	if (got == 0) {
		LOGV("MISSED %d %d\n", buffer_ana_play_ofs, desired);
	} else {
		LOGV("PUMPED: %d\n", ioData->mDataByteSize);
	}
	
	pthread_cond_signal(&m_AudioSyncCond);

	*/
	
    return noErr;
}

-(void)initAudio2 {

#define kOutputBus 0
//#define kInputBus 1

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
	/*
	audioFormat.mFormatID          = kAudioFormatLinearPCM;
	
	//audioFormat.mFormatFlags       = kAudioFormatFlagsAudioUnitCanonical;
	
	audioFormat.mBytesPerPacket    = bytesPerSample;
	
	audioFormat.mBytesPerFrame     = bytesPerSample;
	
	audioFormat.mFramesPerPacket   = 1;
	
	audioFormat.mBitsPerChannel    = 8 * bytesPerSample;
	
	audioFormat.mChannelsPerFrame  = 1;           // 2 indicates stereo
	
	audioFormat.mSampleRate        = 44100.0;
	 */
	
	audioFormat.mSampleRate			= PLAYBACK_FREQ;
	audioFormat.mFormatID			= kAudioFormatLinearPCM;
	audioFormat.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	audioFormat.mFramesPerPacket	= 1;
	audioFormat.mChannelsPerFrame	= 1;
	audioFormat.mBitsPerChannel		= 16;
	audioFormat.mBytesPerPacket		= 2;
	audioFormat.mBytesPerFrame		= 2;
	
	
	/*
	AudioStreamBasicDescription audioFormat;

	// Describe format
	audioFormat.mSampleRate			= PLAYBACK_FREQ;
	audioFormat.mFormatID			= kAudioFormatLinearPCM;
	audioFormat.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	audioFormat.mFramesPerPacket	= 1;
	audioFormat.mChannelsPerFrame	= 1;
	audioFormat.mBitsPerChannel		= 16;
	audioFormat.mBytesPerPacket		= sizeof(short);
	audioFormat.mBytesPerFrame		= sizeof(short);
	
	
	
	
	*/
	/*
	// Apply format
	status = AudioUnitSetProperty(audioUnit, 
								  kAudioUnitProperty_StreamFormat, 
								  kAudioUnitScope_Output, 
								  kInputBus, 
								  &audioFormat, 
								  sizeof(audioFormat));
	checkStatus(status);
	*/
	
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
	
	/*
	buffer_ana_flag=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
	buffer_ana=(AudioUnitSampleType**)malloc(SOUND_BUFFER_NB*sizeof(AudioUnitSampleType *));
	for (int i=0;i<SOUND_BUFFER_NB;i++) {
		buffer_ana[i]= (AudioUnitSampleType *)malloc(AUDIO_BUFFER_SIZE);
		buffer_ana_flag[i] = 0;
	}
	
	buffer_ana_gen_ofs = 0;
	buffer_ana_play_ofs = 0;
	*/
	
	Float32 aBufferLength; // In seconds
	UInt32 size = sizeof(aBufferLength);
	
    status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration,
							&size, &aBufferLength);
	checkStatus(status);

	LOGV("current duration: %f\n", aBufferLength);
	
	
	aBufferLength = 1.0;
	
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

		m_SyncAudio = false;
		
		ring = alloc();
		
		[self initAudio2];
		
		game = new MainMenu(self.layer.frame.size.width, self.layer.frame.size.height, textures, models, levels, sounds, AUDIO_BUFFER_SIZE, 1);
		game->CreateThread(Callbacks::PumpAudio);
		
		gameState = 1;
	
		//status = AudioOutputUnitStart(audioUnit);
		//checkStatus(status);
	}
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
		g_View = self;
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
