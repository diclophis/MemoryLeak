//
//  EAGLView.h
//  WangCrapTemplate
//
//  Created by Jon Bardin on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

//#import <Foundation/Foundation.h>
//#import <UIKit/UIKit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <QuartzCore/QuartzCore.h>

#include "importgl.h"


void iPhoneDrv_AudioCallback(void *data, AudioQueueRef mQueue, AudioQueueBufferRef mBuffer);
void interruptionListenerCallback (void *inUserData,UInt32 interruptionState );
void propertyListenerCallback (void                   *inUserData,                             
							   AudioSessionPropertyID inPropertyID,                                
							   UInt32                 inPropertyValueSize,                         
							   const void             *inPropertyValue );

GLuint loadTexture(UIImage *image);

struct Engine;
typedef struct Engine Engine;

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView
{    
@private

    BOOL animating;
    BOOL displayLinkSupported;
    NSInteger animationFrameInterval;
    // Use of the CADisplayLink class is the preferred method for controlling your animation timing.
    // CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
    // The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
    // isn't available.
    id displayLink;
    NSTimer *animationTimer;
	EAGLContext *context;
    // The pixel dimensions of the CAEAGLLayer
    GLint backingWidth;
    GLint backingHeight;
    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view
    GLuint defaultFramebuffer, colorRenderbuffer, depthRenderbuffer;
	
	Engine *game;
	int gameState;
	
	OSStatus status;
	AudioComponentInstance audioUnit;

	//General infos
	int mod_message_updated,mod_subsongs;
	int mod_currentsub,mod_minsub,mod_maxsub;
	int mp_datasize,numChannels;
	int iCurrentTime,iModuleLength;

	//for spectrum analyzer
	short int **buffer_ana_cpy;
		
	//Modplug stuff
	int *genRow,*genPattern,*playRow,*playPattern;	
	char *mp_data;
	int numPatterns,numSamples,numInstr;
	
	//AVFoundation
	AudioQueueRef mAudioQueue;
	AudioQueueBufferRef *mBuffers;
	
}


@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;


//Modplug stuff

@property char *mp_data;
@property int *genRow,*genPattern,*playRow,*playPattern;
@property int numChannels,numPatterns,numSamples,numInstr;


//for spectrum analyzer
@property short int **buffer_ana_cpy;
@property AudioQueueRef mAudioQueue;
@property AudioQueueBufferRef *mBuffers;



-(void)build;
-(void)startAnimation;
-(void)stopAnimation;
-(void)drawView:(id)sender;
-(void)startGame;
-(void)parse:(const char*)json withLength:(size_t)length;
-(void)pump_audio;










-(void)initAudio;
-(BOOL)iPhoneDrv_FillAudioBuffer:(AudioQueueBufferRef) mBuffer;
-(BOOL)iPhoneDrv_Init;
-(void)iPhoneDrv_Exit;
-(BOOL)iPhoneDrv_PlayStart;
-(void)iPhoneDrv_PlayStop;
-(void)iPhoneDrv_PlayWaitStop;
-(void)iPhoneDrv_Update:(AudioQueueBufferRef) mBuffer;
-(int)getCurrentPlayedBufferIdx;










@end
