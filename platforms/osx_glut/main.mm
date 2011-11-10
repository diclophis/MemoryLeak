// Vanilla MacOSX OpenGL App

#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#import <QuartzCore/QuartzCore.h>
#import <AudioToolbox/AudioToolbox.h>

#include "MemoryLeak.h"

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

OSStatus status;
AudioComponentInstance audioUnit;
AudioQueueRef mAudioQueue;
AudioQueueBufferRef *mBuffers;

static int kWindowWidth = 256;
static int kWindowHeight = 256;
static int win;
static bool left_down = false;
static bool right_down = false;
static bool reset_down = false;

static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> sounds;
static std::vector<foo*> levels;

static int game_index = 3;

AudioDeviceID device;
UInt32 deviceBufferSize;
AudioStreamBasicDescription deviceFormat;

OSStatus appIOProc (AudioDeviceID  inDevice, const AudioTimeStamp*  inNow, const AudioBufferList*   inInputData, const AudioTimeStamp*  inInputTime, AudioBufferList*  outOutputData, const AudioTimeStamp* inOutputTime, void* defptr) {    
  int numSamples = deviceBufferSize / deviceFormat.mBytesPerFrame;

	if (outOutputData->mNumberBuffers != 1) {
		LOGV("the fuck\n");
	}
  
	AudioBuffer *ioData = &outOutputData->mBuffers[0];
  memset(ioData->mData, 0, ioData->mDataByteSize);

LOGV("wtf %d\n", ioData->mDataByteSize);
  Engine::CurrentGameDoAudio((short int*)ioData->mData, ioData->mDataByteSize);

  return kAudioHardwareNoError;
}


bool startAudio() {
  OSStatus		err = kAudioHardwareNoError;
  void *def;

  AudioDeviceIOProcID theIOProcID = NULL;
  err = AudioDeviceCreateIOProcID(device, appIOProc, (void *)def, &theIOProcID);
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "AudioDeviceAddIOProc failed\n");
    return false;
  }

  err = AudioDeviceStart(device, theIOProcID);
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
  // get the default output device for the HAL
  count = sizeof(device);		// it is required to pass the size of the data to be returned
  err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &count, (void *) &device);
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

  count = sizeof(AudioStreamBasicDescription);
  AudioObjectPropertyAddress property = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
  property.mScope = kAudioDevicePropertyScopeOutput;

  property.mSelector = kAudioStreamPropertyPhysicalFormat;
  err = AudioObjectGetPropertyData(device, &property, 0, NULL,  &count, &deviceFormat);
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "get kAudioDevicePropertyStreamFormat error %ld\n", err);
    checkStatus(err);
  }
  fprintf(stderr, "mSampleRate = %g\n", deviceFormat.mSampleRate);
  fprintf(stderr, "mFormatFlags = %08lX\n", deviceFormat.mFormatFlags);
  fprintf(stderr, "mBytesPerPacket = %ld\n", deviceFormat.mBytesPerPacket);
  fprintf(stderr, "mFramesPerPacket = %ld\n", deviceFormat.mFramesPerPacket);
  fprintf(stderr, "mChannelsPerFrame = %ld\n", deviceFormat.mChannelsPerFrame);
  fprintf(stderr, "mBytesPerFrame = %ld\n", deviceFormat.mBytesPerFrame);
  fprintf(stderr, "mBitsPerChannel = %ld\n", deviceFormat.mBitsPerChannel);

  fprintf(stderr, "\n\n\n");

  deviceFormat.mSampleRate = 44100;
  deviceFormat.mFormatID = kAudioFormatLinearPCM;
  deviceFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
  deviceFormat.mFramesPerPacket = 1;
  deviceFormat.mChannelsPerFrame = 2;
  deviceFormat.mBitsPerChannel = 16;
  deviceFormat.mBytesPerFrame =  deviceFormat.mBitsPerChannel / 8 * deviceFormat.mChannelsPerFrame;
  deviceFormat.mBytesPerPacket = deviceFormat.mBytesPerFrame * deviceFormat.mFramesPerPacket;

  fprintf(stderr, "mSampleRate = %g\n", deviceFormat.mSampleRate);
  fprintf(stderr, "mFormatFlags = %08lX\n", deviceFormat.mFormatFlags);
  fprintf(stderr, "mBytesPerPacket = %ld\n", deviceFormat.mBytesPerPacket);
  fprintf(stderr, "mFramesPerPacket = %ld\n", deviceFormat.mFramesPerPacket);
  fprintf(stderr, "mChannelsPerFrame = %ld\n", deviceFormat.mChannelsPerFrame);
  fprintf(stderr, "mBytesPerFrame = %ld\n", deviceFormat.mBytesPerFrame);
  fprintf(stderr, "mBitsPerChannel = %ld\n", deviceFormat.mBitsPerChannel);

  property.mSelector = kAudioStreamPropertyPhysicalFormat;
  err = AudioObjectSetPropertyData(device, &property, 0, NULL, count, &deviceFormat);
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "set kAudioDevicePropertyStreamFormat error %ld\n", err);
    checkStatus(err);
    exit(1);
    return false;
  }


  /*
  float newVolume = 0.5;

  property.mSelector = kAudioHardwareServiceDeviceProperty_VirtualMasterVolume;

  UInt32 propertySize;
  propertySize = sizeof(Float32);
  err = AudioHardwareServiceSetPropertyData(device, &property, 0, NULL, propertySize, &newVolume); 
  if (err != kAudioHardwareNoError) {
    fprintf(stderr, "set volume error %ld\n", err);
    checkStatus(err);
    exit(1);
    return false;
  }
  */

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
  //printf("key: %d %c\n", key, key);
  if (key == 110) {
    if (left_down) {
      Engine::CurrentGameHit(0, 0, 2);
    } else {
      Engine::CurrentGameHit(0, 0, 0);
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
      game_index = key - 49;
      if (game_index > 3) {
        game_index = 0;
      }
      if (game_index < 0) {
        game_index = 0;
      }
      Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, pushMessageToWebView, popMessageFromWebView, SimulationThreadCleanup);
    }
    reset_down = !reset_down;
  }
}


int main(int argc, char** argv) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(kWindowWidth, kWindowHeight);
  glutInitWindowPosition(1000, 500);
  win = glutCreateWindow("main");
  glutDisplayFunc(draw);
  glutKeyboardFunc(processNormalKeys);
  glutKeyboardUpFunc(processNormalKeys);
  glutIgnoreKeyRepeat(true);
  glutMouseFunc(processMouse);
  glutMotionFunc(processMouseMotion);
  glutIdleFunc(draw);
  glutReshapeFunc(resize);
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

  //if (!setupAudio()) {
  //  printf("cant setup Audio\n");
  //}

  //if (!startAudio()) {
  //  printf("cant start Audio\n");
  //}

  Engine::Start(game_index, kWindowWidth, kWindowHeight, textures, models, levels, sounds, pushMessageToWebView, popMessageFromWebView, SimulationThreadCleanup);

  glutMainLoop();

  return 0;
}
