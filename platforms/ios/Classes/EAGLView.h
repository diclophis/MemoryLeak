//
//  EAGLView.h
//  WangCrapTemplate
//
//  Created by Jon Bardin on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//


#import <AudioToolbox/AudioToolbox.h>
#import <QuartzCore/QuartzCore.h>


void iPhoneDrv_AudioCallback(void *data, AudioQueueRef mQueue, AudioQueueBufferRef mBuffer);
void interruptionListenerCallback (void *inUserData,UInt32 interruptionState );
void propertyListenerCallback (void *inUserData, AudioSessionPropertyID inPropertyID, UInt32 inPropertyValueSize, const void *inPropertyValue);
GLuint loadTexture(UIImage *image);
bool pushMessageToWebView(const char *);
const char *popMessageFromWebView();
struct Engine;
typedef struct Engine Engine;


// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView {    


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
  EAGLContext *WorkingContext;
  
  // The pixel dimensions of the CAEAGLLayer
  GLint backingWidth;
  GLint backingHeight;
  // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view
  GLuint defaultFramebuffer, colorRenderbuffer, depthRenderbuffer;

	OSStatus status;
	AudioComponentInstance audioUnit;
	AudioQueueRef mAudioQueue;
	AudioQueueBufferRef *mBuffers;
  UIWebView *webView;
	NSMutableArray *mPoppedMessages;
	BOOL mLastMessageReady;
}


@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
@property (retain) NSMutableArray *mPoppedMessages;


-(void)build:(UIWebView *)theWebView;
-(void)startAnimation;
-(void)stopAnimation;
-(void)drawView:(id)sender;
-(BOOL)pushMessageToWebView:(const char *)theMessage;
-(const char *)popMessageFromWebView;
-(void)startGame:(id)i;
-(BOOL)wasActive;
-(void)initAudio2;


@end
