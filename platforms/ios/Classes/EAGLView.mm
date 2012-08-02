//  Jon Bardin on 7/12/10 GPL


#import "EAGLView.h"
#import "MemoryLeak.h"
#import "Bridge.h"

static GLuint g_LastFrameBuffer = -1;
static GLuint g_LastRenderBuffer = -1;
static EAGLView *gView;

void set_user_interaction_enabled(const char *s) {
  BOOL enabled = [[NSString stringWithUTF8String:s] boolValue];
  [[gView webView] setUserInteractionEnabled:enabled];
}

const char *push_and_pop(const char *s) {
  NSString *wrapper = @"(function() { try { %@ } catch(e) { return e; } })();";
  NSString *middle = [NSString stringWithFormat:@"%s; return bridge.dequeue();", s];
  NSString *executer = [NSString stringWithFormat:wrapper, middle];
  NSString *dequeued_message = [[gView webView] stringByEvaluatingJavaScriptFromString:executer];
  if ([dequeued_message length] > 0) {
    NSArray *parts = [dequeued_message componentsSeparatedByString:@"|"];
    if ([parts count] == 2) {
      unsigned int address_of_function = [[parts objectAtIndex:0] intValue];
      if (address_of_function > 0) {
        const char *(*pointer_to_function)(const char *) = (const char *(*)(const char *))address_of_function;
        return pointer_to_function([[parts objectAtIndex:1] cStringUsingEncoding:NSUTF8StringEncoding]);
      }
    } else {
      NSLog(@"Exception: %@", dequeued_message);
    }
  }
  
  return NULL;  
}

@implementation EAGLView


@synthesize animating;
@synthesize webView;
@dynamic animationFrameInterval;


// You must implement this method
+ (Class)layerClass {
  return [CAEAGLLayer class];
}



-(id)initWithCoder:(NSCoder *)aDecoder {
  if ((self = [super initWithCoder:aDecoder])) {

    animating = FALSE;
    animationFrameInterval = 1;
    displayLink = nil;
    
    // Get the layer
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

    eaglLayer.opaque = TRUE; //kEAGLColorFormatRGBA8
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGB565, kEAGLDrawablePropertyColorFormat, nil];
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:glShareGroup];
    
    if (!context || ![EAGLContext setCurrentContext:context]) {
      [self release];
      return nil;
    }
    
    glShareGroup = context.sharegroup;
    if (!glShareGroup)
    {
      NSLog(@"Could not get sharegroup from the main context");
      return nil;
    }
    
    glWorkingContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:glShareGroup];
    
    // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
    glGenFramebuffersOES(1, &defaultFramebuffer);
    glGenRenderbuffersOES(1, &colorRenderbuffer);
    g_LastFrameBuffer = defaultFramebuffer;
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);
    
    // The pixel dimensions of the CAEAGLLayer
    GLint backingWidth;
    GLint backingHeight;
    
    //Bind framebuffers to the context and this layer
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:eaglLayer];
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    glGenRenderbuffersOES(1, &depthRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    
    if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
      NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
      return nil;
    }

    NSString *reqSysVer = @"3.1";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending) {
    } else {
      NSLog(@"display link required");
      return nil;
    }

    NSArray *model_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"assets/models"];
    for (NSString *path in model_names) {
      FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
      fseek(fd, 0, SEEK_END);
      unsigned int len = ftell(fd);
      rewind(fd);
      Engine::PushBackFileHandle(MODELS, fd, 0, len);
    }
    
    NSArray *level_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"assets/levels"];
    for (NSString *path in level_names) {
      FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
      fseek(fd, 0, SEEK_END);
      unsigned int len = ftell(fd);
      rewind(fd);
      Engine::PushBackFileHandle(LEVELS, fd, 0, len);
    }

    
    NSArray *texture_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"assets/textures"];
    for (NSString *path in texture_names) {
      FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
      fseek(fd, 0, SEEK_END);
      unsigned int len = ftell(fd);
      rewind(fd);
      Engine::PushBackFileHandle(TEXTURES, fd, 0, len);
    }
    
    
    NSArray *sound_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"assets/sounds"];
    for (NSString *path in sound_names) {
      FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
      fseek(fd, 0, SEEK_END);
      unsigned int len = ftell(fd);
      rewind(fd);
      Engine::PushBackFileHandle(SOUNDS, fd, 0, len);
    }
    
    [self installWebView];
    
    gView = self;
  }
  return self;
}


-(void)installWebView {
  NSURL *urlToIndexHtml = [[NSBundle mainBundle] URLForResource:@"index" withExtension:@"html" subdirectory:@"assets/html5"];
  webView = [[UIWebView alloc] initWithFrame:[self frame]];
  [webView setDelegate:self];
  [webView setBackgroundColor:[UIColor clearColor]];
  [webView setOpaque:NO];
  [webView loadRequest:[NSURLRequest requestWithURL:urlToIndexHtml]];
  [[[webView subviews] objectAtIndex:0] setBounces:NO];
  //[webView setUserInteractionEnabled:NO];
  [self addSubview:webView];
}


- (void)webViewDidFinishLoad:(UIWebView *)theWebView {
  [webView stringByEvaluatingJavaScriptFromString:[NSString stringWithFormat:@"bridge.register('set_user_interaction_enabled', %d);", &set_user_interaction_enabled]];
  [webView stringByEvaluatingJavaScriptFromString:[NSString stringWithFormat:@"bridge.register('start_game', %d);", &start_game]];
  [webView stringByEvaluatingJavaScriptFromString:[NSString stringWithFormat:@"bridge.register('doo_thing_one', %d);", &doo_thing_one]];
  [webView stringByEvaluatingJavaScriptFromString:[NSString stringWithFormat:@"bridge.finalize();"]];
}


-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		CGRect bounds;
		CGPoint location;
		bounds = [self bounds];
		for (UITouch *touch in touches) {			
			location = [touch locationInView:self];
			location.y = location.y;
      Engine::CurrentGameHit(location.x, location.y, 0);
		}
	}
}


-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		CGRect bounds;
		CGPoint location;
		bounds = [self bounds];
		for (UITouch *touch in touches) {			
			location = [touch locationInView:self];
			location.y = location.y;
      Engine::CurrentGameHit(location.x, location.y, 1);
		}
	}
}


-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	if (animating) {
		CGRect bounds;
		CGPoint location;
		bounds = [self bounds];
		for (UITouch *touch in touches) {			
			location = [touch locationInView:self];
			location.y = location.y;
      Engine::CurrentGameHit(location.x, location.y, 2);
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
    Engine::CurrentGameHit(location.x, location.y, -1);
	}
}


-(void)drawView:(id)sender {
  if (animating) {
    [EAGLContext setCurrentContext:context];

    if (g_LastFrameBuffer != defaultFramebuffer) {
      g_LastFrameBuffer = defaultFramebuffer;
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
    }

    Engine::CurrentGameDrawScreen(0);		
    
    const GLenum discards[]  = {GL_DEPTH_ATTACHMENT_OES};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER_OES, 1, discards);
    
    if (g_LastRenderBuffer != colorRenderbuffer) {
      g_LastRenderBuffer = colorRenderbuffer;
      glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    }
    
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
  }
}


-(NSInteger)animationFrameInterval {
    return animationFrameInterval;
}


- (void)setAnimationFrameInterval:(NSInteger)frameInterval {
  // Frame interval defines how many display frames must pass between each time the
  // display link fires. The display link will only fire 30 times a second when the
  // frame internal is two on a display that refreshes 60 times a second. The default
  // frame interval setting of one will fire 60 times a second when the display refreshes
  // at 60 times a second. A frame interval setting of less than one results in undefined
  // behavior.
  if (frameInterval >= 1) {
    animationFrameInterval = frameInterval;
    if (animating) {
      [self stopAnimation];
      [self startAnimation];
    }
  }
}


-(void)startAnimation {
  if (!animating) {
    animating = TRUE;
    displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
    [displayLink setFrameInterval:animationFrameInterval];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  }
}


-(void)stopAnimation {
  if (animating) {
    Engine::CurrentGamePause(); 
    [displayLink invalidate];
    displayLink = nil;
    animating = FALSE;
  }
}


-(void)startGame:(id)i {
  Engine::Start([i intValue], self.layer.frame.size.width, self.layer.frame.size.height, &push_and_pop);
  initAudio2();
}


-(void)dealloc {	
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
	
  if ([EAGLContext currentContext] == context) {
    [EAGLContext setCurrentContext:nil];
  }	
	
  [context release];
  context = nil;
	
  [webView removeFromSuperview];
  [webView release];
  
  [super dealloc];
}


-(BOOL)wasActive {
	if (Engine::CurrentGame()) {
    Engine::CurrentGameStart();
    return YES;
  } else {
    return NO;
  }
}


@end
