//
//  EAGLView.h
//  WangCrapTemplate
//
//  Created by Jon Bardin on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include "importgl.h"

GLuint loadTexture(UIImage *image);

struct Engine;
typedef struct Engine Engine;

struct RaptorIsland;
typedef struct RaptorIsland RaptorIsland;

struct RunAndJump;
typedef struct RunAndJump RunAndJump;

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
	Engine *gameController;
	BOOL myFrameRequested;
	int gameState;
	
	//std::vector<GLuint> textures;
	//std::vector<foo*> models;

}


@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;


-(void)build;
-(void)startAnimation;
-(void)stopAnimation;
-(void)drawView:(id)sender;
-(void)startGame;


@end