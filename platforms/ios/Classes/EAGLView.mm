//
//  EAGLView.m
//  WangCrapTemplate
//
//  Created by Jon Bardin on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "MemoryLeakAppDelegate.h"
#include "MemoryLeak.h"
#include "Model.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Engine.h"
#include "MachineGun.h"
#include "octree.h"
#include "micropather.h"
#include "ModelOctree.h"
#include "PixelPusher.h"


#import "EAGLView.h"


EAGLView *g_View;

pthread_mutex_t play_mutex;

static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> levels;
static std::vector<foo*> sounds;

static 	int buffer_ana_subofs;
static short int **buffer_ana;
static volatile int buffer_ana_gen_ofs,buffer_ana_play_ofs;
static volatile int *buffer_ana_flag;

#define PLAYBACK_FREQ 8000
#define SOUND_BUFFER_SIZE_SAMPLE 1024
#define SOUND_BUFFER_NB 1

class Callbacks {
	static void *PumpAudio(void *b, int buffer_position, int d) {
		int div = d * sizeof(short);
		int len = (SOUND_BUFFER_SIZE_SAMPLE*2*2) / div;
		//g_Env->SetShortArrayRegion(ab, 0, len, (jshort *)((short *)buffer + off));
		memcpy(buffer_ana[buffer_ana_gen_ofs], b, len);
		buffer_ana_flag[buffer_ana_play_ofs] = 1;
	};
};





void iPhoneDrv_AudioCallback(void *data, AudioQueueRef mQueue, AudioQueueBufferRef mBuffer) {
	LOGV("iPhoneDrv_AudioCallback\n");
	EAGLView *view=(EAGLView *)data;
	[view iPhoneDrv_Update:mBuffer];
}

 
void interruptionListenerCallback (void *inUserData,UInt32 interruptionState ) {
	EAGLView *mplayer=(EAGLView *)inUserData;
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

/*************************************************/
/* Audio property listener                       */
/*************************************************/
void propertyListenerCallback (void                   *inUserData,                             
							   AudioSessionPropertyID inPropertyID,                                
							   UInt32                 inPropertyValueSize,                         
							   const void             *inPropertyValue ) {
	if (inPropertyID==kAudioSessionProperty_AudioRouteChange ) {
		EAGLView *mplayer = (EAGLView *) inUserData;		
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
		UITouch* touch;
		bounds = [self bounds];
		touch = [[allTouches allObjects] objectAtIndex:0];
		CGPoint location;
		location = [touch locationInView:self];
		location.y = location.y;		
		game->Hit(location.x, location.y, 0);
	}
}


-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		NSSet *allTouches = [event allTouches];
		CGRect bounds;
		UITouch* touch;
		bounds = [self bounds];
		touch = [[allTouches allObjects] objectAtIndex:0];
		CGPoint location;
		location = [touch locationInView:self];
		location.y = location.y;
		game->Hit(location.x, location.y, 1);
	}
}


-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {		
		NSSet *allTouches = [event allTouches];
		CGRect bounds;
		UITouch* touch;
		bounds = [self bounds];
		touch = [[allTouches allObjects] objectAtIndex:0];
		CGPoint location;
		location = [touch locationInView:self];
		location.y = location.y;
		game->Hit(location.x, location.y, 2);
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



-(void)initAudio {
	AudioSessionInitialize (
							NULL,
							NULL,
							interruptionListenerCallback,
							self
							);
	UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
	AudioSessionSetProperty (
							 kAudioSessionProperty_AudioCategory,
							 sizeof (sessionCategory),
							 &sessionCategory
							 );
	
	//TODO: Check if still required or not 
	//Float32 preferredBufferDuration = SOUND_BUFFER_SIZE_SAMPLE * 1.0f/PLAYBACK_FREQ;
	//AudioSessionSetProperty (                                     
	//						 kAudioSessionProperty_PreferredHardwareIOBufferDuration,
	//						 sizeof (preferredBufferDuration),
	//						 &preferredBufferDuration
	//						 );
	
	AudioSessionPropertyID routeChangeID = kAudioSessionProperty_AudioRouteChange;
	AudioSessionAddPropertyListener (                                 
									 routeChangeID,                                                
									 propertyListenerCallback,                                      
									 self                                                       
									 );
	AudioSessionSetActive(true);	
	
	
	buffer_ana_flag=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
	buffer_ana=(short int**)malloc(SOUND_BUFFER_NB*sizeof(unsigned short int *));
	buffer_ana_cpy=(short int**)malloc(SOUND_BUFFER_NB*sizeof(unsigned short int *));
	buffer_ana_subofs=0;
	for (int i=0;i<SOUND_BUFFER_NB;i++) {
		buffer_ana[i]=(short int *)malloc(SOUND_BUFFER_SIZE_SAMPLE*2*2);
		buffer_ana_cpy[i]=(short int *)malloc(SOUND_BUFFER_SIZE_SAMPLE*2*2);
		buffer_ana_flag[i]=0;
	}
	

	[self iPhoneDrv_Init];
	[self iPhoneDrv_PlayStart];

}




-(BOOL) iPhoneDrv_Init {
	LOGV("iPhoneDrv_Init\n");

    AudioStreamBasicDescription mDataFormat;
    UInt32 err;
    
    /* We force this format for iPhone */
    mDataFormat.mFormatID = kAudioFormatLinearPCM;
    mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger |
	kAudioFormatFlagIsPacked;
	
	mDataFormat.mSampleRate = PLAYBACK_FREQ;
    
	mDataFormat.mBitsPerChannel = 16;
    
	mDataFormat.mChannelsPerFrame = 2;
    
    mDataFormat.mBytesPerFrame = (mDataFormat.mBitsPerChannel>>3) * mDataFormat.mChannelsPerFrame;
	
    mDataFormat.mFramesPerPacket = 1; 
    mDataFormat.mBytesPerPacket = mDataFormat.mBytesPerFrame;
    
    /* Create an Audio Queue... */
    err = AudioQueueNewOutput( &mDataFormat, 
							  iPhoneDrv_AudioCallback, 
							  self, 
							  NULL, //CFRunLoopGetCurrent(),
							  kCFRunLoopCommonModes,
							  0, 
							  &mAudioQueue );
    
    /* ... and its associated buffers */
    mBuffers = (AudioQueueBufferRef*)malloc( sizeof(AudioQueueBufferRef) * SOUND_BUFFER_NB );
    for (int i=0; i<SOUND_BUFFER_NB; i++) {
		AudioQueueBufferRef mBuffer;
		err = AudioQueueAllocateBuffer( mAudioQueue, SOUND_BUFFER_SIZE_SAMPLE*2*2, &mBuffer );
		
		mBuffers[i]=mBuffer;
    }
    /* Set initial playback volume */
    err = AudioQueueSetParameter( mAudioQueue, kAudioQueueParam_Volume, 1.0);
	
	
    return 1;
}


-(void) iPhoneDrv_Exit {
	LOGV("iPhoneDrv_Exit\n");

    AudioQueueDispose( mAudioQueue, true );
    free( mBuffers );
}

-(BOOL) iPhoneDrv_PlayStart {
	LOGV("iPhoneDrv_PlayStart\n");

    UInt32 err;
    UInt32 i;
	
    /*
     * Enqueue all the allocated buffers before starting the playback.
     * The audio callback will be called as soon as one buffer becomes
     * available for filling.
     */
	
	//BOOL mQueueIsBeingStopped = FALSE;
	
	buffer_ana_gen_ofs=buffer_ana_play_ofs=0;
	buffer_ana_subofs=0;
	for (int i=0;i<SOUND_BUFFER_NB;i++) {
		buffer_ana_flag[i]=0;
		//playRow[i]=playPattern[i]=genRow[i]=genPattern[i]=0;
		memset(buffer_ana[i],0,SOUND_BUFFER_SIZE_SAMPLE*2*2);
		memset(buffer_ana_cpy[i],0,SOUND_BUFFER_SIZE_SAMPLE*2*2);		
	}
	//sampleVolume=256;
    for (i=0; i<SOUND_BUFFER_NB; i++) {
		memset(mBuffers[i]->mAudioData,0,SOUND_BUFFER_SIZE_SAMPLE*2*2);
		mBuffers[i]->mAudioDataByteSize = SOUND_BUFFER_SIZE_SAMPLE*2*2;
		AudioQueueEnqueueBuffer( mAudioQueue, mBuffers[i], 0, NULL);
		//		[self iPhoneDrv_FillAudioBuffer:mBuffers[i]];
		
    }
	//bGlobalAudioPause=0;	
    /* Start the Audio Queue! */
	//AudioQueuePrime( mAudioQueue, 0,NULL );
    err = AudioQueueStart( mAudioQueue, NULL );
	
    return 1;
}

-(void) iPhoneDrv_PlayWaitStop {
	LOGV("iPhoneDrv_PlayWaitStop\n");

	//int counter=0;
    //mQueueIsBeingStopped = TRUE;
	//bGlobalAudioPause=2;
	AudioQueueStop( mAudioQueue, FALSE );
	//while ([self isEndReached]==NO) {
	//	[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
	//	counter++;
	//	if (counter*DEFAULT_WAIT_TIME_MS>2) break;
	//}
	AudioQueueReset( mAudioQueue );	
    //mQueueIsBeingStopped = FALSE;
}



-(void) iPhoneDrv_PlayStop {
	LOGV("iPhoneDrv_PlayStop\n");

    //mQueueIsBeingStopped = TRUE;
	//bGlobalAudioPause=2;
    AudioQueueStop( mAudioQueue, TRUE );
	AudioQueueReset( mAudioQueue );	
    //mQueueIsBeingStopped = FALSE;
}


-(void) iPhoneDrv_Update:(AudioQueueBufferRef) mBuffer {   
	LOGV("iPhoneDrv_Update\n");

    /* the real processing takes place in FillAudioBuffer */
	[self iPhoneDrv_FillAudioBuffer:mBuffer];
}

-(BOOL) iPhoneDrv_FillAudioBuffer:(AudioQueueBufferRef) mBuffer {
	LOGV("iPhoneDrv_FillAudioBuffer\n");
	
	int skip_queue=0;
	mBuffer->mAudioDataByteSize = SOUND_BUFFER_SIZE_SAMPLE*2*2;
	BOOL bGlobalAudioPause = NO;
	
	
	if (bGlobalAudioPause) {
		memset(mBuffer->mAudioData, 0, SOUND_BUFFER_SIZE_SAMPLE*2*2);
		if (bGlobalAudioPause==2) skip_queue=1;//return 0;  //End of song
    } else {
		//consume another buffer
		if (buffer_ana_flag[buffer_ana_play_ofs]) {
			//bGlobalSoundHasStarted++;
			//if (buffer_ana_flag[buffer_ana_play_ofs]&2) { //changed currentTime
			//	iCurrentTime=mNeedSeekTime;
			//	mNeedSeek=0;
			//	bGlobalSeekProgress=0;
			//}
			
			//playPattern[buffer_ana_play_ofs]=genPattern[buffer_ana_play_ofs];
			//playRow[buffer_ana_play_ofs]=genRow[buffer_ana_play_ofs];
			
			iCurrentTime+=1000*SOUND_BUFFER_SIZE_SAMPLE/PLAYBACK_FREQ;
			

					
					
			memcpy((char*)mBuffer->mAudioData,buffer_ana[buffer_ana_play_ofs],SOUND_BUFFER_SIZE_SAMPLE*2*2);
			
			memcpy(buffer_ana_cpy[buffer_ana_play_ofs],buffer_ana[buffer_ana_play_ofs],SOUND_BUFFER_SIZE_SAMPLE*2*2);
			
			buffer_ana_flag[buffer_ana_play_ofs]=0;
			buffer_ana_play_ofs++;
			if (buffer_ana_play_ofs==SOUND_BUFFER_NB) buffer_ana_play_ofs=0;
		} else {
			memset((char*)mBuffer->mAudioData,0,SOUND_BUFFER_SIZE_SAMPLE*2*2);  //WARNING : not fast enough!!
		}
	}
	if (!skip_queue) {
		AudioQueueEnqueueBuffer( mAudioQueue, mBuffer, 0, NULL);
		if (bGlobalAudioPause==2) {
			AudioQueueStop( mAudioQueue, FALSE );
		}
	} else {
		buffer_ana_play_ofs++;
		if (buffer_ana_play_ofs==SOUND_BUFFER_NB) buffer_ana_play_ofs=0;
	}
    return 0;
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
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		[self initAudio];
		
		
		

		
		game = new PixelPusher(self.layer.frame.size.width, self.layer.frame.size.height, textures, models, levels, sounds, SOUND_BUFFER_SIZE_SAMPLE);
		game->CreateThread(Callbacks::PumpAudio);
		gameState = 1;
		
		
		UIDevice* device = [UIDevice currentDevice];
		BOOL backgroundSupported = NO;
		if ([device respondsToSelector:@selector(isMultitaskingSupported)])
			backgroundSupported = device.multitaskingSupported;
		

		//if (pthread_mutex_init(&db_mutex,NULL)) {
		//	printf("cannot create db mutex");
		//	return NO;
		//}
		//if (pthread_mutex_init(&download_mutex,NULL)) {
		//	printf("cannot create download mutex");
		//	return NO;
		//}
		//if (pthread_mutex_init(&play_mutex,NULL)) {
		//	printf("cannot create play mutex");
		//}
		
		//mplayer = [[MusicPlayer alloc] initMusicPlayer];
		//ModPlug_Settings *mpsettings=[mplayer getMPSettings];
		//mpsettings->mResamplingMode = MODPLUG_RESAMPLE_LINEAR;
		//mpsettings->mChannels = 2;
		//mpsettings->mBits = 32;
		//mpsettings->mFrequency = PLAYBACK_FREQ;
		//mpsettings->mLoopCount = -1;
		//[mplayer updateMPSettings];

		//int retcode;
		
		//if (retcode = [mplayer LoadModule]) {
		//	NSLog(@"Issue in LoadModule");
		//}
		
		//[mplayer Play];
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
