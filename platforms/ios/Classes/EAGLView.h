//
//  EAGLView.h
//  WangCrapTemplate
//
//  Created by Jon Bardin on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import "Audio.h"


// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView {    


@private

  BOOL animating;
  BOOL displayLinkSupported;
  NSInteger animationFrameInterval;
  
  CADisplayLink *displayLink;
	EAGLContext *context;
  EAGLSharegroup *glShareGroup;
  EAGLContext *glWorkingContext;
    
  // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view
  GLuint defaultFramebuffer, colorRenderbuffer, depthRenderbuffer;

}


@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;


-(void)startAnimation;
-(void)stopAnimation;
-(void)drawView:(id)sender;
-(void)startGame:(id)i;
-(BOOL)wasActive;
-(GLuint)loadTexture:(UIImage *)image;


@end