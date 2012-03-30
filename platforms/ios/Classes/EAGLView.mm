//  Jon Bardin on 7/12/10 GPL


#import "EAGLView.h"
#import "MemoryLeak.h"


static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> levels;
static std::vector<foo*> sounds;
static GLuint g_LastFrameBuffer = -1;
static GLuint g_LastRenderBuffer = -1;


@implementation EAGLView


@synthesize animating;
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

    eaglLayer.opaque = TRUE;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGB565, kEAGLDrawablePropertyColorFormat, nil];
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1 sharegroup:glShareGroup];
    
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
    
    glWorkingContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1 sharegroup:glShareGroup];
    
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
      textures.push_back([self loadTexture2:image]);
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
  }
  return self;
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
	
    if (g_LastRenderBuffer != colorRenderbuffer) {
      g_LastRenderBuffer = colorRenderbuffer;
      glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    }

    const GLenum discards[]  = {GL_DEPTH_ATTACHMENT_OES};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER_OES, 1, discards);
    
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
  }
}


- (GLuint)loadTexture2:(UIImage *)image {
  unsigned int* inPixel32;
  unsigned short* outPixel16;
	GLuint text = 0;
  CGImageRef textureImage = image.CGImage;
  CGColorSpaceRef colorSpace;
  GLuint textureWidth = CGImageGetWidth(textureImage);
  GLuint textureHeight = CGImageGetHeight(textureImage);
  colorSpace = CGColorSpaceCreateDeviceRGB();
  
  void *textureData = malloc(textureWidth * textureHeight * sizeof(unsigned int));
  void *tempData = malloc(textureHeight * textureWidth * sizeof(unsigned short));

  inPixel32 = (unsigned int *)textureData;
  outPixel16 = (unsigned short *)tempData;
  
  //swizzle in all the bits for the to-be-created bitmap, because CGContextDrawImage/CGBitmapContextCreate doesnt like rgba
  for(int i=0; i < textureWidth*textureHeight; ++i) {
    inPixel32[i] = 0;
  }

  CGContextRef textureContext = CGBitmapContextCreate(textureData, textureWidth, textureHeight, 8, sizeof(int) * textureWidth, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big); //
  CGContextDrawImage(textureContext, CGRectMake(0, 0, textureWidth, textureHeight), textureImage);

  //Convert "RRRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA" to "RRRRGGGGBBBBAAAA"
  for (int i=0; i<(textureHeight * textureWidth); i++) {
    unsigned int inP = ((unsigned int *)textureData)[i];
    outPixel16[i] = ((((inP >> 0) & 0xFF) >> 4) << 12) | ((((inP >> 8) & 0xFF) >> 4) << 8) | ((((inP >> 16) & 0xFF) >> 4) << 4) | ((((inP >> 24) & 0xFF) >> 4) << 0);
  }
  
  free(textureData);
  CGContextRelease(textureContext);
  CGColorSpaceRelease(colorSpace);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &text);
	glBindTexture(GL_TEXTURE_2D, text);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, tempData);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
  free(tempData);
	return text;
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
  Engine::Start([i intValue], self.layer.frame.size.width, self.layer.frame.size.height, textures, models, levels, sounds, NULL);
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
