// Vanilla MacOSX OpenGL App

#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#import <QuartzCore/QuartzCore.h>
#include <CoreAudio/AudioHardware.h>

#include "MemoryLeak.h"

static int kWindowWidth = 320;
static int kWindowHeight = 480;

static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> sounds;
static std::vector<foo*> levels;

static int game_index = 0;
static bool not_printed = true;

//typedef struct {
  // the default device
  // bufferSize returned by kAudioDevicePropertyBufferSize
  // info about the default device
  AudioDeviceID device;
  UInt32 deviceBufferSize;
  AudioStreamBasicDescription deviceFormat;
//} sinewavedef;

OSStatus appIOProc (AudioDeviceID  inDevice, const AudioTimeStamp*  inNow, const AudioBufferList*   inInputData, 
                             const AudioTimeStamp*  inInputTime, AudioBufferList*  outOutputData, const AudioTimeStamp* inOutputTime, 
                             void* defptr)
{    
    //sinewavedef* def = defptr;

/*
    sinewavedef*	def = defptr; // get access to Sinewave's data
    int i;
    
    // load instance vars into registers
    double phase = def->phase;
    double amp = def->amp;
    double pan = def->pan;
    double freq = def->freq;

    double ampz = def->ampz;
    double panz = def->panz;
    double freqz = def->freqz;
    
    int numSamples = def->deviceBufferSize / def->deviceFormat.mBytesPerFrame;
    
    // assume floats for now....
    float *out = outOutputData->mBuffers[0].mData;
    
    for (i=0; i<numSamples; ++i) {
    
        float wave = sin(phase) * ampz;		// generate sine wave
        phase = phase + freqz;			// increment phase
        
        // write output
        *out++ = wave * (1.0-panz);		// left channel
        *out++ = wave * panz;			// right channel
        
        // de-zipper controls
        panz  = 0.001 * pan  + 0.999 * panz;
        ampz  = 0.001 * amp  + 0.999 * ampz;
        freqz = 0.001 * freq + 0.999 * freqz;
    }
    
    // save registers back to object
    def->phase = phase;
    def->freqz = freqz;
    def->ampz = ampz;
    def->panz = panz;
*/

  int numSamples = deviceBufferSize / deviceFormat.mBytesPerFrame;


	if (outOutputData->mNumberBuffers != 1) {
		LOGV("the fuck\n");
	}
	
	AudioBuffer *ioData = &outOutputData->mBuffers[0];

  //4096 8 512
  if (not_printed) {
    not_printed = false;
    printf("%d %d %d %d\n", deviceBufferSize, deviceFormat.mBytesPerFrame, numSamples, ioData->mDataByteSize);
  }


/*
deviceBufferSize = 4096
mSampleRate = 44100
mFormatFlags = 00000009
mBytesPerPacket = 8
mFramesPerPacket = 1
mChannelsPerFrame = 2
mBytesPerFrame = 8
mBitsPerChannel = 32
*/
	
  memset(ioData->mData, 0, ioData->mDataByteSize);
  //Engine::CurrentGameDoAudio(ioData->mData, numSamples);
  
  return kAudioHardwareNoError;     
}

bool startAudio()
{
  OSStatus		err = kAudioHardwareNoError;
  //sinewavedef *def;
  void *def;

  err = AudioDeviceAddIOProc(device, appIOProc, (void *) def);	// setup our device with an IO proc
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "AudioDeviceAddIOProc failed\n");
    return false;
  }

  err = AudioDeviceStart(device, appIOProc);				// start playing sound through the device
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "AudioDeviceStart failed\n");
    return false;
  }

  return true;

}

bool setupAudio() {
  OSStatus				err = kAudioHardwareNoError;
  UInt32				count;    
  device = kAudioDeviceUnknown;

  //initialized = NO;
 
  // get the default output device for the HAL
  count = sizeof(device);		// it is required to pass the size of the data to be returned
  err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,  &count, (void *) &device);
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "get kAudioHardwarePropertyDefaultOutputDevice error %ld\n", err);
      return false;
  }


  
  // get the buffersize that the default device uses for IO
  count = sizeof(deviceBufferSize);	// it is required to pass the size of the data to be returned
  err = AudioDeviceGetProperty(device, 0, false, kAudioDevicePropertyBufferSize, &count, &deviceBufferSize);
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "get kAudioDevicePropertyBufferSize error %ld\n", err);
      return false;
  }
  fprintf(stderr, "deviceBufferSize = %ld\n", deviceBufferSize);
 
  // get a description of the data format used by the default device
  count = sizeof(deviceFormat);	// it is required to pass the size of the data to be returned
  err = AudioDeviceGetProperty(device, 0, false, kAudioDevicePropertyStreamFormat, &count, &deviceFormat);
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "get kAudioDevicePropertyStreamFormat error %ld\n", err);
      return false;
  }
  if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
    fprintf(stderr, "mFormatID !=  kAudioFormatLinearPCM\n");
      return false;
  }
  if (!(deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat)) {
    fprintf(stderr, "Sorry, currently only works with float format....\n");
      return false;
  }

  /*
  UInt32 propsize = sizeof(AudioStreamBasicDescription);
	size_t bytesPerSample = sizeof(short);
	deviceFormat.mSampleRate = 44100;
	deviceFormat.mFormatID = kAudioFormatLinearPCM;
	deviceFormat.mFormatFlags = kLinearPCMFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
	deviceFormat.mFramesPerPacket = 1;
	deviceFormat.mChannelsPerFrame = 1;
	deviceFormat.mBitsPerChannel = 16;
	deviceFormat.mBytesPerPacket = bytesPerSample;
	deviceFormat.mBytesPerFrame	= bytesPerSample;

  err = AudioDeviceSetProperty(device, NULL, 0, false, kAudioDevicePropertyStreamFormat, propsize, &deviceFormat);
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "cant set kAudioDevicePropertyStreamFormat error %ld\n", err);
  }
  */
  
  //initialized = YES;

  fprintf(stderr, "mSampleRate = %g\n", deviceFormat.mSampleRate);
  fprintf(stderr, "mFormatFlags = %08lX\n", deviceFormat.mFormatFlags);
  fprintf(stderr, "mBytesPerPacket = %ld\n", deviceFormat.mBytesPerPacket);
  fprintf(stderr, "mFramesPerPacket = %ld\n", deviceFormat.mFramesPerPacket);
  fprintf(stderr, "mChannelsPerFrame = %ld\n", deviceFormat.mChannelsPerFrame);
  fprintf(stderr, "mBytesPerFrame = %ld\n", deviceFormat.mBytesPerFrame);
  fprintf(stderr, "mBitsPerChannel = %ld\n", deviceFormat.mBitsPerChannel);
  return true;
}

bool pushMessageToWebView(const char *theMessage) {
	return true;
}

const char *popMessageFromWebView() {
  return "";
}

void SimulationThreadCleanup() {
}

GLuint loadTexture(NSBitmapImageRep *image) {
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

void draw(void) {
  Engine::CurrentGameDrawScreen(0);
  glutSwapBuffers();
}

void resize(int width, int height) {
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
  printf("key: %d %c\n", key, key);

  game_index += 1;
  if (game_index > 2) {
    game_index = 0;
  }

  Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, pushMessageToWebView, popMessageFromWebView, SimulationThreadCleanup);
/*
  switch (key) {
    case 27:
    gameController->pause();
    break;

    case 13:
    gameController->hitTest(x, y, 0);
    gameController->hitTest(x, y, 2);
    break;

    case 113:
    glDepthFunc(GL_NEVER);
    break;

    case 119:
    glDepthFunc(GL_LESS);
    break;

    case 101:
    glDepthFunc(GL_EQUAL);
    break;

    case 114:
    glDepthFunc(GL_LEQUAL);
    break;

    case 116:
    glDepthFunc(GL_GREATER);
    break;

    case 121:
    glDepthFunc(GL_NOTEQUAL);
    break;

    case 117:
    glDepthFunc(GL_GEQUAL);
    break;

    case 105:
    glDepthFunc(GL_ALWAYS);
    break;

    case 111:
    glDisable(GL_BLEND);
    break;

    case 112:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    break;
  }

  LOGV("the fuck: %d\n", key);
*/
}

int main(int argc, char** argv) {

  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  glutInitWindowSize(kWindowWidth, kWindowHeight);
  glutInitWindowPosition(1000, 500);
  glutCreateWindow("main");

	NSArray *model_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../../assets/models"];
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

	NSArray *texture_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../../assets/textures"];
	for (NSString *path in texture_names) {
    NSLog(@"path: %@", path);
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    NSBitmapImageRep *image = [NSBitmapImageRep imageRepWithData:texData];
    
    if (image == nil) {
      throw 1;
    }

	  textures.push_back(loadTexture(image));
    [image release];
    [texData release];
  }

	NSArray *level_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../../assets/levels"];
	for (NSString *path in level_names) {
		FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
		fseek(fd, 0, SEEK_END);
		unsigned int len = ftell(fd);
		rewind(fd);

		foo *firstModel = new foo;
		firstModel->fp = fd;
		firstModel->off = 0;
		firstModel->len = len;
		
		levels.push_back(firstModel);
	}

	NSArray *sound_names = [[NSBundle mainBundle] pathsForResourcesOfType:nil inDirectory:@"../../../assets/sounds"];
	for (NSString *path in sound_names) {
		FILE *fd = fopen([path cStringUsingEncoding:[NSString defaultCStringEncoding]], "rb");
		fseek(fd, 0, SEEK_END);
		unsigned int len = ftell(fd);
		rewind(fd);

		foo *firstModel = new foo;
		firstModel->fp = fd;
		firstModel->off = 0;
		firstModel->len = len;
		
		sounds.push_back(firstModel);
	}

  if (!setupAudio()) {
    printf("cant setup Audio\n");
  }

  if (!startAudio()) {
    printf("cant start Audio\n");
  }

  Engine::Start(2, kWindowWidth, kWindowHeight, textures, models, levels, sounds, pushMessageToWebView, popMessageFromWebView, SimulationThreadCleanup);

  glutKeyboardFunc(processNormalKeys);
  glutMouseFunc(processMouse);
  glutMotionFunc(processMouseMotion);
  glutDisplayFunc(draw);
  glutIdleFunc(draw);
  glutReshapeFunc(resize);
  glutMainLoop();

  [pool release];

  return 0;
}
