// Vanilla MacOSX OpenGL App


#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#import <QuartzCore/QuartzCore.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudio/CoreAudio.h>
#import <Accelerate/Accelerate.h>


#include "MemoryLeak.h"


#define kInputBus 1
#define kOutputBus 0


static int kWindowWidth = 500;
static int kWindowHeight = 500;
static bool left_down = false;
static bool right_down = false;
static bool reset_down = false;
static bool debug_down = false;
static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> sounds;
static std::vector<foo*> levels;
static int game_index = 1;
static short int *outData;

#ifdef USE_GLES2

static const char vertex_shader[] =
  "attribute vec3 position;\n"
  "attribute vec3 normal;\n"
  "\n"
  "uniform mat4 ModelViewProjectionMatrix;\n"
  "uniform mat4 NormalMatrix;\n"
  "uniform vec4 LightSourcePosition;\n"
  "uniform vec4 MaterialColor;\n"
  "\n"
  "varying vec4 Color;\n"
  "\n"
  "void main(void)\n"
  "{\n"
  " // Transform the normal to eye coordinates\n"
  " vec3 N = normalize(vec3(NormalMatrix * vec4(normal, 1.0)));\n"
  "\n"
  " // The LightSourcePosition is actually its direction for directional light\n"
  " vec3 L = normalize(LightSourcePosition.xyz);\n"
  "\n"
  " // Multiply the diffuse value by the vertex color (which is fixed in this case)\n"
  " // to get the actual color that we will use to draw this vertex with\n"
  " float diffuse = max(dot(N, L), 0.0);\n"
  " Color = diffuse * MaterialColor;\n"
  "\n"
  " // Transform the position to clip coordinates\n"
  " gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);\n"
  "}";

static const char fragment_shader[] =
  "#ifdef GL_ES\n"
  "precision mediump float;\n"
  "#endif\n"
  "varying vec4 Color;\n"
  "\n"
  "void main(void)\n"
  "{\n"
  " gl_FragColor = Color;\n"
  "}";

static GLuint ModelViewProjectionMatrix_location, NormalMatrix_location, LightSourcePosition_location, MaterialColor_location;
/** The projection matrix */
static GLfloat ProjectionMatrix[16];
/** The direction of the directional light for the scene */
static const GLfloat LightSourcePosition[4] = { 5.0, 5.0, 10.0, 1.0};

#ifndef HAVE_BUILTIN_SINCOS
#define sincos _sincos
static void sincos (double a, double *s, double *c) {
  *s = sin (a);
  *c = cos (a);
}
#endif

/**
* Creates an identity 4x4 matrix.
*
* @param m the matrix make an identity matrix
*/
static void identity(GLfloat *m) {
   GLfloat t[16] = {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0,
   };

   memcpy(m, t, sizeof(t));
}


static void ortho(GLfloat *m, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearZ, GLfloat farZ) {

  GLfloat deltaX = right - left;
  GLfloat deltaY = top - bottom;
  GLfloat deltaZ = farZ - nearZ;

  GLfloat tmp[16];
  identity(tmp);

  if ((deltaX == 0) || (deltaY == 0) || (deltaZ == 0)) {
    LOGV("Invalid ortho");
    return;
  }

  tmp[0] = 2 / deltaX;
  tmp[12] = -(right + left) / deltaX;
  tmp[5] = 2 / deltaY;
  tmp[13] = -(top + bottom) / deltaY;
  tmp[10] = -2 / deltaZ;
  tmp[14] = -(nearZ + farZ) / deltaZ;

  memcpy(m, tmp, sizeof(tmp));

}


/**
* Calculate a perspective projection transformation.
*
* @param m the matrix to save the transformation in
* @param fovy the field of view in the y direction
* @param aspect the view aspect ratio
* @param zNear the near clipping plane
* @param zFar the far clipping plane
*/
void perspective(GLfloat *m, GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
   GLfloat tmp[16];
   identity(tmp);

   double sine, cosine, cotangent, deltaZ;
   GLfloat radians = fovy / 2 * M_PI / 180;

   deltaZ = zFar - zNear;
   sincos(radians, &sine, &cosine);

   if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
      return;

   cotangent = cosine / sine;

   tmp[0] = cotangent / aspect;
   tmp[5] = cotangent;
   tmp[10] = -(zFar + zNear) / deltaZ;
   tmp[11] = -1;
   tmp[14] = -2 * zNear * zFar / deltaZ;
   tmp[15] = 0;

   memcpy(m, tmp, sizeof(tmp));
}


#endif


static void CheckError(OSStatus error, const char *operation) {
  if (error == noErr) return;
  char str[20];
  // see if it appears to be a 4-char-code
  *(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
  if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
    str[0] = str[5] = '\'';
    str[6] = '\0';
  } else {
    // no, format it as an integer
    sprintf(str, "%d", (int)error);
  }
      
  fprintf(stderr, "Error: %s (%s)\n", operation, str);
      
  exit(1);
}


GLuint loadTexture(NSBitmapImageRep *image) {
  GLuint text = 0;
  glEnable(GL_TEXTURE_2D);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glGenTextures(1, &text);
  glBindTexture(GL_TEXTURE_2D, text);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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


void draw(void) {

#ifdef USE_GLES2
  //GLfloat transform[16];
  //identity(transform);

  //perspective(ProjectionMatrix, 60.0, width / (float)height, 1.0, 1024.0);
  
  //glOrthof((-m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (-m_ScreenHalfHeight) * m_Zoom, m_ScreenHalfHeight * m_Zoom, 1.0f, -1.0f);

  float m_Zoom = 0.5;
  float m_ScreenHalfHeight = (float)kWindowHeight / 2.0;
  float m_ScreenAspect = (float)kWindowWidth / (float)kWindowHeight;

  float a = (-m_ScreenHalfHeight * m_ScreenAspect) * m_Zoom;
  float b = (m_ScreenHalfHeight * m_ScreenAspect) * m_Zoom;
  float c = (-m_ScreenHalfHeight) * m_Zoom;
  float d = m_ScreenHalfHeight * m_Zoom;
  float e = 1.0;
  float f = -1.0;

  ortho(ProjectionMatrix, a, b, c, d, e, f);

#endif

  Engine::CurrentGameDrawScreen(0);
  glutSwapBuffers();
  glutPostRedisplay();
}


void resize(int width, int height) {

#ifdef USE_GLES2
  /* Update the projection matrix */
  // maybe
#endif

  kWindowWidth = width;
  kWindowHeight = height;
  Engine::CurrentGameResizeScreen(width, height);

}


void processMouse(int button, int state, int x, int y) {
  switch (state) {
    case GLUT_DOWN:
      Engine::CurrentGameHit(x, y, 0);
      break;
    case GLUT_UP:
      Engine::CurrentGameHit(x, y, 2);
      break;
  }
}


void processMouseMotion(int x, int y) {
  Engine::CurrentGameHit(x, y, 1);
}


void processNormalKeys(unsigned char key, int x, int y) {
  //LOGV("key: %d %c\n", key, key);
  if (key == 49) {
    if (debug_down) {
      Engine::CurrentGameHit(0, 0, 2);
    } else {
      Engine::CurrentGameHit(0, 0, 0);
    }
    debug_down = !debug_down;
  } else if (key == 110) {
    if (left_down) {
      Engine::CurrentGameHit(0, 1024, 2);
    } else {
      Engine::CurrentGameHit(0, 1024, 0);
    }
    left_down = !left_down;
  } else if (key == 109) {
    if (right_down) {
      Engine::CurrentGameHit(1024, 1024, 2);
    } else {
      Engine::CurrentGameHit(1024, 1024, 0);
    }
    right_down = !right_down;
  } else {
    if (reset_down) {

    } else {
      if (key == 112) { // p
        Engine::CurrentGamePause();
      } else if (key == 114) { // r
        Engine::CurrentGameDestroyFoos();
        Engine::CurrentGameSetAssets(textures, models, levels, sounds);
        Engine::CurrentGameCreateFoos();
        Engine::CurrentGameStart();
      } else if (key == 115) { // s
        Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, NULL);
      }
    }
    reset_down = !reset_down;
  }
}


OSStatus renderCallback (void *inRefCon, AudioUnitRenderActionFlags * ioActionFlags, const AudioTimeStamp * inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames, AudioBufferList * ioDataList) {

  size_t size = inNumberFrames * sizeof(short) * 2;
  
  Engine::CurrentGameDoAudio(outData, size);

  for (unsigned int iBuffer=0; iBuffer < ioDataList->mNumberBuffers; ++iBuffer) {
    AudioBuffer *ioData = &ioDataList->mBuffers[iBuffer];
    float *buffer = (float *)ioData->mData;
    for (unsigned int j = 0; j < inNumberFrames; j++) {
      buffer[j] = (float)outData[(j * 2) + iBuffer] / (float)INT16_MAX;
    }

    ioData->mDataByteSize = size; // is this redundant?
  }

  return noErr;

}

void audioUnitSetup() {

  outData = (short int *)calloc(8192, sizeof(short int));

  AudioUnit outputUnit;

  AudioComponentDescription outputDescription = {0};
  outputDescription.componentType = kAudioUnitType_Output;
  outputDescription.componentSubType = kAudioUnitSubType_HALOutput;
  outputDescription.componentManufacturer = kAudioUnitManufacturer_Apple; 

  AudioComponent outputComponent = AudioComponentFindNext(NULL, &outputDescription);
  CheckError(AudioComponentInstanceNew(outputComponent, &outputUnit), "Couldn't create the output audio unit");

  // Enable output
  UInt32 one = 1;
  UInt32 zero = 0;

  CheckError(AudioUnitSetProperty(outputUnit,
                                   kAudioOutputUnitProperty_EnableIO,
                                   kAudioUnitScope_Output,
                                   kOutputBus,
                                   &one,
                                   sizeof(one)), "Couldn't enable IO on the input scope of output unit");
 
  // Disable input
  CheckError(AudioUnitSetProperty(outputUnit,
                                   kAudioOutputUnitProperty_EnableIO,
                                   kAudioUnitScope_Input,
                                   kInputBus,
                                   &zero,
                                   sizeof(UInt32)), "Couldn't disable output on the audio unit");

  AudioStreamBasicDescription outputFormat;

  AudioDeviceID defaultOutputDeviceID;

  AudioObjectPropertyAddress propertyAddress;
  UInt32 propertySize;

  propertyAddress.mSelector = kAudioHardwarePropertyDevices;
  propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
  propertyAddress.mElement = kAudioObjectPropertyElementMaster;
  AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize);

  propertySize = sizeof(defaultOutputDeviceID);
  propertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
  CheckError(AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, &defaultOutputDeviceID), "Couldn't set the current output audio device");

  CheckError( AudioUnitSetProperty(outputUnit,
                                   kAudioOutputUnitProperty_CurrentDevice,
                                   kAudioUnitScope_Global,
                                   kOutputBus,
                                   &defaultOutputDeviceID,
                                   sizeof(AudioDeviceID)), "Couldn't set the current output audio device");

	outputFormat.mSampleRate = 44100.0;
	outputFormat.mFormatID = kAudioFormatLinearPCM;
	outputFormat.mFormatFlags = kAudioFormatFlagsCanonical;
	outputFormat.mFramesPerPacket = 1;
	outputFormat.mChannelsPerFrame	= 2;
	outputFormat.mBitsPerChannel = 16;
  outputFormat.mBytesPerFrame = outputFormat.mBitsPerChannel / 8 * outputFormat.mChannelsPerFrame;
  outputFormat.mBytesPerPacket = outputFormat.mBytesPerFrame * outputFormat.mFramesPerPacket;

  propertySize = sizeof(AudioStreamBasicDescription);

  CheckError(AudioUnitSetProperty(outputUnit,
        kAudioUnitProperty_StreamFormat,
        kAudioUnitScope_Output,
        kInputBus,
        &outputFormat,
        propertySize),
        "Couldn't set the ASBD on the audio unit (after setting its sampling rate)");

  // Slap a render callback on the unit
  AURenderCallbackStruct callbackStruct;
  callbackStruct.inputProc = renderCallback;
  callbackStruct.inputProcRefCon = NULL;

  CheckError(AudioUnitSetProperty(outputUnit,
                                   kAudioUnitProperty_SetRenderCallback,
                                   kAudioUnitScope_Input,
                                   kOutputBus,
                                   &callbackStruct,
                                   sizeof(callbackStruct)),
             "Couldn't set the render callback on the input unit");

  CheckError(AudioUnitInitialize(outputUnit), "Couldn't initialize the output unit");
  CheckError(AudioOutputUnitStart(outputUnit), "Couldn't start the output unit");

}


int main(int argc, char** argv) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(kWindowWidth, kWindowHeight);
  glutInitWindowPosition(1000, 500);
  glutCreateWindow("main");
  glutDisplayFunc(draw);
  glutKeyboardFunc(processNormalKeys);
  glutKeyboardUpFunc(processNormalKeys);
  glutIgnoreKeyRepeat(true);
  glutMouseFunc(processMouse);
  glutMotionFunc(processMouseMotion);
  glutReshapeFunc(resize);

#ifdef USE_GLES2

  
  GLuint v, f, program;
  const char *p;
  char msg[512];

  /* Compile the vertex shader */
  p = vertex_shader;
  v = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(v, 1, &p, NULL);
  glCompileShader(v);
  glGetShaderInfoLog(v, sizeof msg, NULL, msg);
  LOGV("vertex shader info: %s\n", msg);

  /* Compile the fragment shader */
  p = fragment_shader;
  f = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(f, 1, &p, NULL);
  glCompileShader(f);
  glGetShaderInfoLog(f, sizeof msg, NULL, msg);
  LOGV("fragment shader info: %s\n", msg);

  /* Create and link the shader program */
  program = glCreateProgram();
  glAttachShader(program, v);
  glAttachShader(program, f);
  glBindAttribLocation(program, 0, "position");
  glBindAttribLocation(program, 1, "normal");

  glLinkProgram(program);
  glGetProgramInfoLog(program, sizeof msg, NULL, msg);
  LOGV("info: %s\n", msg);

  /* Enable the shaders */
  glUseProgram(program);

  /* Get the locations of the uniforms so we can access them */
  ModelViewProjectionMatrix_location = glGetUniformLocation(program, "ModelViewProjectionMatrix");
  NormalMatrix_location = glGetUniformLocation(program, "NormalMatrix");
  LightSourcePosition_location = glGetUniformLocation(program, "LightSourcePosition");
  MaterialColor_location = glGetUniformLocation(program, "MaterialColor");

  /* Set the LightSourcePosition uniform which is constant throught the program */
  glUniform4fv(LightSourcePosition_location, 1, LightSourcePosition);

#endif

  NSBundle *mainBundle = [NSBundle mainBundle];
  NSStringEncoding defaultCStringEncoding = [NSString defaultCStringEncoding];
  NSString *model_path = [NSString stringWithCString:"../../../assets/models" encoding:NSUTF8StringEncoding];
  NSString *textures_path = [NSString stringWithCString:"../../../assets/textures" encoding:NSUTF8StringEncoding];
  NSString *levels_path = [NSString stringWithCString:"../../../assets/levels" encoding:NSUTF8StringEncoding];
  NSString *sounds_path = [NSString stringWithCString:"../../../assets/sounds" encoding:NSUTF8StringEncoding];
	NSArray *model_names = [mainBundle pathsForResourcesOfType:nil inDirectory:model_path];
	for (NSString *path in model_names) {
		FILE *fd = fopen([path cStringUsingEncoding:defaultCStringEncoding], "rb");
		fseek(fd, 0, SEEK_END);
		unsigned int len = ftell(fd);
		rewind(fd);
		foo *firstModel = new foo;
		firstModel->fp = fd;
		firstModel->off = 0;
		firstModel->len = len;
		models.push_back(firstModel);
	}
  [model_names release];
  [model_path release];
	NSArray *texture_names = [mainBundle pathsForResourcesOfType:nil inDirectory:textures_path];
	for (NSString *path in texture_names) {
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    NSBitmapImageRep *image = [NSBitmapImageRep imageRepWithData:texData];
    if (image == nil) {
      throw 1;
    }
	  textures.push_back(loadTexture(image));
    [image release];
    [texData release];
  }
  [texture_names release];
  [textures_path release];
	NSArray *level_names = [mainBundle pathsForResourcesOfType:nil inDirectory:levels_path];
	for (NSString *path in level_names) {
		FILE *fd = fopen([path cStringUsingEncoding:defaultCStringEncoding], "rb");
		fseek(fd, 0, SEEK_END);
		unsigned int len = ftell(fd);
		rewind(fd);
		foo *firstModel = new foo;
		firstModel->fp = fd;
		firstModel->off = 0;
		firstModel->len = len;
		levels.push_back(firstModel);
	}
  [level_names release];
  [levels_path release];
	NSArray *sound_names = [mainBundle pathsForResourcesOfType:nil inDirectory:sounds_path];
	for (NSString *path in sound_names) {
		FILE *fd = fopen([path cStringUsingEncoding:defaultCStringEncoding], "rb");
		fseek(fd, 0, SEEK_END);
		unsigned int len = ftell(fd);
		rewind(fd);
		foo *firstModel = new foo;
		firstModel->fp = fd;
		firstModel->off = 0;
		firstModel->len = len;
		sounds.push_back(firstModel);
	}
  [sound_names release];
  [sounds_path release];
  [mainBundle release];

  Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, NULL);

  audioUnitSetup();

  glutMainLoop();

  [pool release];

  return 0;
}
