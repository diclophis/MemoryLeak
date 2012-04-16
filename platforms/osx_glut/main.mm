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
static int game_index = 3;
static short int *outData;


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


GLuint LoadTexture(const char *path) {
  png_t tex;
  unsigned char* data;
  GLuint textureHandle;

  png_init(0, 0);
  png_open_file_read(&tex, path);
  data = (unsigned char*) malloc(tex.width * tex.height * tex.bpp);
  png_get_data(&tex, data);

  glGenTextures(1, &textureHandle);
  glBindTexture(GL_TEXTURE_2D, textureHandle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  png_close_file(&tex);
  free(data);

  return textureHandle;
}



void draw(void) {
  Engine::CurrentGameDrawScreen(0);
  glutSwapBuffers();
  glutPostRedisplay();
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
    if (false) {
      NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
      NSBitmapImageRep *image = [NSBitmapImageRep imageRepWithData:texData];
      if (image == nil) {
        throw 1;
      }
      textures.push_back(loadTexture(image));
      [image release];
      [texData release];
    } else {
      textures.push_back(LoadTexture([path UTF8String]));
    }
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
