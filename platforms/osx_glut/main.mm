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
#define kDefaultDevice 999999


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
static float *outData;

static void CheckError(OSStatus error, const char *operation)
{
if (error == noErr) return;

char str[20];
// see if it appears to be a 4-char-code
*(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
str[0] = str[5] = '\'';
str[6] = '\0';
} else
// no, format it as an integer
sprintf(str, "%d", (int)error);
    
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


void somethingElse() {
  AudioObjectPropertyAddress  propertyAddress;
  AudioObjectID               *deviceIDs;
  UInt32                      propertySize;
  NSInteger                   numDevices;

  propertyAddress.mSelector = kAudioHardwarePropertyDevices;
  propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
  propertyAddress.mElement = kAudioObjectPropertyElementMaster;
  if (AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize) == noErr) {
      numDevices = propertySize / sizeof(AudioDeviceID);
      deviceIDs = (AudioDeviceID *)calloc(numDevices, sizeof(AudioDeviceID));

      if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, deviceIDs) == noErr) {
          AudioObjectPropertyAddress      deviceAddress;
          char                            deviceName[64];
          char                            manufacturerName[64];

          for (NSInteger idx=0; idx<numDevices; idx++) {
              propertySize = sizeof(deviceName);
              deviceAddress.mSelector = kAudioDevicePropertyDeviceName;
              deviceAddress.mScope = kAudioObjectPropertyScopeGlobal;
              deviceAddress.mElement = kAudioObjectPropertyElementMaster;
              if (AudioObjectGetPropertyData(deviceIDs[idx], &deviceAddress, 0, NULL, &propertySize, deviceName) == noErr) {
                  propertySize = sizeof(manufacturerName);
                  deviceAddress.mSelector = kAudioDevicePropertyDeviceManufacturer;
                  deviceAddress.mScope = kAudioObjectPropertyScopeGlobal;
                  deviceAddress.mElement = kAudioObjectPropertyElementMaster;
                  if (AudioObjectGetPropertyData(deviceIDs[idx], &deviceAddress, 0, NULL, &propertySize, manufacturerName) == noErr) {
                      CFStringRef     uidString;

                      propertySize = sizeof(uidString);
                      deviceAddress.mSelector = kAudioDevicePropertyDeviceUID;
                      deviceAddress.mScope = kAudioObjectPropertyScopeGlobal;
                      deviceAddress.mElement = kAudioObjectPropertyElementMaster;
                      if (AudioObjectGetPropertyData(deviceIDs[idx], &deviceAddress, 0, NULL, &propertySize, &uidString) == noErr) {
                          NSLog(@"device %s by %s id %@", deviceName, manufacturerName, uidString);

                          CFRelease(uidString);
                      }
                  }
              }
          }
      }

      free(deviceIDs);
  }
}


OSStatus inputCallback (void *inRefCon, AudioUnitRenderActionFlags * ioActionFlags, const AudioTimeStamp * inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames, AudioBufferList * ioDataList) {
LOGV("input\n");
}


OSStatus renderCallback (void *inRefCon, AudioUnitRenderActionFlags * ioActionFlags, const AudioTimeStamp * inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames, AudioBufferList * ioDataList) {

  LOGV("render\n");

  float zero = 0.0;

  for (int iBuffer=0; iBuffer < ioDataList->mNumberBuffers; ++iBuffer) {
    NSLog(@"  Buffer %d has %d channels and wants %d bytes of data.", iBuffer, ioDataList->mBuffers[iBuffer].mNumberChannels, ioDataList->mBuffers[iBuffer].mDataByteSize);
    //UInt16 *frameBuffer = (UInt16 *)ioDataList->mBuffers[iBuffer].mData;
    //for (int j = 0; j < inNumberFrames; j++) {
    //  frameBuffer[j] = rand();
    //}

    AudioBuffer *ioData = &ioDataList->mBuffers[iBuffer];
    size_t size = inNumberFrames * sizeof(short) * 2;

    Engine::CurrentGameDoAudio((short int *)ioData->mData, size);

    ioData->mDataByteSize = size;

    //memset(ioDataList->mBuffers[iBuffer].mData, 0, ioDataList->mBuffers[iBuffer].mDataByteSize);
  }

  // Collect data to render from the callbacks
  //sm.outputBlock(sm.outData, inNumberFrames, sm.numOutputChannels);

	//if (ioDataList->mNumberBuffers != 1) {
	//	LOGV("the fuck\n");
  //}	
	
	
  //Engine::CurrentGameDoAudio((short int *)outData, inNumberFrames * sizeof(short) * 2);

  /*
	AudioBuffer *ioData = &ioDataList->mBuffers[0];
  Engine::CurrentGameDoAudio((short int *)ioData->mData, (inNumberFrames / 2) * sizeof(short) * 2);

	ioData = &ioDataList->mBuffers[1];
  Engine::CurrentGameDoAudio((short int *)ioData->mData, (inNumberFrames / 2) * sizeof(short) * 2);
  */

  /*
  // Put the rendered data into the output buffer
  // TODO: convert SInt16 ranges to float ranges.
  if ( sm.numBytesPerSample == 4 ) // then we've already got floats
  {
    for (int iBuffer=0; iBuffer < ioData->mNumberBuffers; ++iBuffer) {
      int thisNumChannels = ioData->mBuffers[iBuffer].mNumberChannels;
      for (int iChannel = 0; iChannel < thisNumChannels; ++iChannel) {
        vDSP_vsadd(sm.outData+iChannel, sm.numOutputChannels, &zero, (float *)ioData->mBuffers[iBuffer].mData, thisNumChannels, inNumberFrames);
      }
    }
  }
  else if ( sm.numBytesPerSample == 2 ) // then we need to convert SInt16 -> Float (and also scale)
  {
  }
  */


    //float scale = (float)INT16_MAX;
    //vDSP_vsmul(outData, 1, &scale, outData, 1, inNumberFrames * 2);


    for (int iBuffer=0; iBuffer < ioDataList->mNumberBuffers; ++iBuffer) {
      int thisNumChannels = ioDataList->mBuffers[iBuffer].mNumberChannels;
      //LOGV("%d %d %d\n", ioDataList->mNumberBuffers, thisNumChannels, inNumberFrames);
      //for (int iChannel = 0; iChannel < thisNumChannels; ++iChannel) {
      //  vDSP_vfix16(outData + iChannel, 2, (SInt16 *)ioDataList->mBuffers[iBuffer].mData + iChannel, thisNumChannels, inNumberFrames);
      //}
      for (int i=0; i<inNumberFrames; i++) {
        //ioDataList->mBuffers[iBuffer].mData[i] = 0.0;
      }
    }
  

  return noErr;

}

void audioUnitSetup() {

  outData = (float *)calloc(8192, sizeof(float));

  AudioDeviceID *deviceIDs;
  NSMutableArray *deviceNames;
  AudioDeviceID defaultInputDeviceID;
  NSString *defaultDeviceName;
  UInt32 propSize;

  deviceNames = [[NSMutableArray alloc] initWithCapacity:100];

  UInt32 propsize = sizeof(AudioDeviceID);
  CheckError(AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &propsize, &defaultInputDeviceID), "Could not get the default device");

  AudioHardwareGetPropertyInfo( kAudioHardwarePropertyDevices, &propSize, NULL );
  uint32_t deviceCount = (propSize / sizeof(AudioDeviceID));

  // Allocate the device IDs
  deviceIDs = (AudioDeviceID *)calloc(deviceCount, sizeof(AudioDeviceID));
  [deviceNames removeAllObjects];

  // Get all the device IDs
  CheckError(AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &propSize, deviceIDs), "Could not get device IDs");

  // Get the names of all the device IDs
  for (int i = 0; i < deviceCount; i++) {
    UInt32 size = sizeof(AudioDeviceID);
    CheckError( AudioDeviceGetPropertyInfo(deviceIDs[i], 0, true, kAudioDevicePropertyDeviceName, &size, NULL ), "Could not get device name length");

    char cStringOfDeviceName[size];
    CheckError( AudioDeviceGetProperty(deviceIDs[i], 0, true, kAudioDevicePropertyDeviceName, &size, cStringOfDeviceName ), "Could not get device name");
    NSString *thisDeviceName = [NSString stringWithCString:cStringOfDeviceName encoding:NSUTF8StringEncoding];

    NSLog(@"Device: %@, ID: %d", thisDeviceName, deviceIDs[i]);
    [deviceNames addObject:thisDeviceName];
  }

  AudioUnit inputUnit;
  AudioUnit outputUnit;

  AudioComponentDescription inputDescription = {0};
  inputDescription.componentType = kAudioUnitType_Output;
  inputDescription.componentSubType = kAudioUnitSubType_HALOutput;
  inputDescription.componentManufacturer = kAudioUnitManufacturer_Apple;

  AudioComponentDescription outputDescription = {0};
  outputDescription.componentType = kAudioUnitType_Output;
  outputDescription.componentSubType = kAudioUnitSubType_HALOutput;
  outputDescription.componentManufacturer = kAudioUnitManufacturer_Apple; 

  // Get component
  AudioComponent inputComponent = AudioComponentFindNext(NULL, &inputDescription);
  CheckError( AudioComponentInstanceNew(inputComponent, &inputUnit), "Couldn't create the output audio unit");

  AudioComponent outputComponent = AudioComponentFindNext(NULL, &outputDescription);
  CheckError( AudioComponentInstanceNew(outputComponent, &outputUnit), "Couldn't create the output audio unit");


  // Enable input
  UInt32 one = 1;
  UInt32 zero = 0;

  /*
  CheckError( AudioUnitSetProperty(inputUnit,
                                   kAudioOutputUnitProperty_EnableIO,
                                   kAudioUnitScope_Input,
                                   kInputBus,
                                   &one,
                                   sizeof(one)), "Couldn't enable IO on the input scope of output unit");


  // Disable output on the input unit
  // (only on Mac, since on the iPhone, the input unit is also the output unit)
  CheckError( AudioUnitSetProperty(inputUnit,
                                   kAudioOutputUnitProperty_EnableIO,
                                   kAudioUnitScope_Output,
                                   kOutputBus,
                                   &zero,
                                   sizeof(UInt32)), "Couldn't disable output on the audio unit");
 
  */

  // Enable output
  CheckError( AudioUnitSetProperty(outputUnit,
                                   kAudioOutputUnitProperty_EnableIO,
                                   kAudioUnitScope_Output,
                                   kOutputBus,
                                   &one,
                                   sizeof(one)), "Couldn't enable IO on the input scope of output unit");
 
  /*
  // Disable input
  CheckError( AudioUnitSetProperty(outputUnit,
                                   kAudioOutputUnitProperty_EnableIO,
                                   kAudioUnitScope_Input,
                                   kInputBus,
                                   &zero,
                                   sizeof(UInt32)), "Couldn't disable output on the audio unit");
  */

  // TODO: first query the hardware for desired stream descriptions
  // Check the input stream format
  AudioStreamBasicDescription inputFormat;
  AudioStreamBasicDescription outputFormat;

  UInt32 size = sizeof(AudioDeviceID);
  if(defaultInputDeviceID == kAudioDeviceUnknown)
  {
  LOGV("set default in\n");
      AudioDeviceID thisDeviceID;
      propsize = sizeof(AudioDeviceID);
      CheckError(AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &propsize, &thisDeviceID), "Could not get the default device");
      defaultInputDeviceID = thisDeviceID;
  }

  AudioDeviceID defaultOutputDeviceID;
  //if (defaultOutputDeviceID == kAudioDeviceUnknown)
  //{
  LOGV("set default out\n");
      AudioDeviceID thisDeviceID;
      propsize = sizeof(AudioDeviceID);
      CheckError(AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propsize, &thisDeviceID), "Could not get the default device");
      defaultOutputDeviceID = thisDeviceID;
  //}
    
  /* 
  // Set the current device to the default input unit.
  CheckError( AudioUnitSetProperty( inputUnit,
                                   kAudioOutputUnitProperty_CurrentDevice,
                                   kAudioUnitScope_Global,
                                   kOutputBus,
                                   &defaultInputDeviceID,
                                   sizeof(AudioDeviceID) ), "Couldn't set the current input audio device");
  */

  CheckError( AudioUnitSetProperty( outputUnit,
                                   kAudioOutputUnitProperty_CurrentDevice,
                                   kAudioUnitScope_Global,
                                   kOutputBus,
                                   &defaultOutputDeviceID,
                                   sizeof(AudioDeviceID) ), "Couldn't set the current output audio device");
    
    
  UInt32 propertySize = sizeof(AudioStreamBasicDescription);
  //CheckError(AudioUnitGetProperty(inputUnit,
  //      kAudioUnitProperty_StreamFormat,
  //      kAudioUnitScope_Output,
  //      kInputBus,
  //      &outputFormat,
  //      &propertySize),
  //      "Couldn't get ASBD from input unit");
    
    
  // 9/6/10 - check the input device's stream format
  AudioStreamBasicDescription deviceFormat;
  CheckError(AudioUnitGetProperty(inputUnit,
        kAudioUnitProperty_StreamFormat,
        kAudioUnitScope_Input,
        kInputBus,
        &inputFormat,
        &propertySize),
        "Couldn't get ASBD from input unit");
    
  /*
  outputFormat.mSampleRate = inputFormat.mSampleRate;
  // outputFormat.mFormatFlags = kAudioFormatFlagsCanonical;
  Float64 samplingRate = inputFormat.mSampleRate;
  UInt32 numBytesPerSample = inputFormat.mBitsPerChannel / 8;

  UInt32 numInputChannels = inputFormat.mChannelsPerFrame;
  UInt32 numOutputChannels = outputFormat.mChannelsPerFrame;

  */

	outputFormat.mSampleRate = 44100;
	outputFormat.mFormatID = kAudioFormatLinearPCM;
	outputFormat.mFormatFlags = kAudioFormatFlagsCanonical | kAudioFormatFlagIsNonInterleaved;
	outputFormat.mFramesPerPacket = 1;
	outputFormat.mChannelsPerFrame	= 2;
	outputFormat.mBitsPerChannel = 16;
  outputFormat.mBytesPerFrame = outputFormat.mBitsPerChannel / 8 * outputFormat.mChannelsPerFrame;
  outputFormat.mBytesPerPacket = outputFormat.mBytesPerFrame * outputFormat.mFramesPerPacket;

  UInt32 numBytesPerSample = outputFormat.mBitsPerChannel / 8;

  LOGV("foo %d\n", numBytesPerSample);

  propertySize = sizeof(AudioStreamBasicDescription);

  CheckError(AudioUnitSetProperty(outputUnit,
        kAudioUnitProperty_StreamFormat,
        kAudioUnitScope_Input,
        kOutputBus,
        &outputFormat,
        propertySize),
        "Couldn't set the ASBD on the audio unit (after setting its sampling rate)");


  // Get the size of the IO buffer(s)
  //UInt32 bufferSizeFrames = 0;
  //size = sizeof(UInt32);
  //CheckError (AudioUnitGetProperty(inputUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &bufferSizeFrames, &size), "Couldn't get buffer frame size from input unit");
  //UInt32 bufferSizeBytes = bufferSizeFrames * sizeof(Float32);

  BOOL isInterleaved = false;

  AudioBufferList *inputBuffer;

  if (outputFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved) {
    // The audio is non-interleaved
    printf("Not interleaved!\n");
    isInterleaved = NO;
    
    /*
    // allocate an AudioBufferList plus enough space for array of AudioBuffers
    UInt32 propsize = offsetof(AudioBufferList, mBuffers[0]) + (sizeof(AudioBuffer) * outputFormat.mChannelsPerFrame);

    //malloc buffer lists
    inputBuffer = (AudioBufferList *)malloc(propsize);
    inputBuffer->mNumberBuffers = outputFormat.mChannelsPerFrame;

    //pre-malloc buffers for AudioBufferLists
    for(UInt32 i =0; i< inputBuffer->mNumberBuffers ; i++) {
      inputBuffer->mBuffers[i].mNumberChannels = 1;
      inputBuffer->mBuffers[i].mDataByteSize = bufferSizeBytes;
      inputBuffer->mBuffers[i].mData = malloc(bufferSizeBytes);
    }
    */
  } else {
    printf ("Format is interleaved\n");
    isInterleaved = YES;
    
    /*
    // allocate an AudioBufferList plus enough space for array of AudioBuffers
    UInt32 propsize = offsetof(AudioBufferList, mBuffers[0]) + (sizeof(AudioBuffer) * 1);

    //malloc buffer lists
    inputBuffer = (AudioBufferList *)malloc(propsize);
    inputBuffer->mNumberBuffers = 1;

    //pre-malloc buffers for AudioBufferLists
    inputBuffer->mBuffers[0].mNumberChannels = outputFormat.mChannelsPerFrame;
    inputBuffer->mBuffers[0].mDataByteSize = bufferSizeBytes;
    inputBuffer->mBuffers[0].mData = malloc(bufferSizeBytes);
    */
  }

  // Slap a render callback on the unit
  AURenderCallbackStruct callbackStruct;

  /*
  callbackStruct.inputProc = inputCallback;
  callbackStruct.inputProcRefCon = NULL;
  
  CheckError( AudioUnitSetProperty(inputUnit,
                                   kAudioOutputUnitProperty_SetInputCallback,
                                   kAudioUnitScope_Global,
                                   0,
                                   &callbackStruct,
                                   sizeof(callbackStruct)), "Couldn't set the callback on the input unit");
  
  */

  callbackStruct.inputProc = renderCallback;
  callbackStruct.inputProcRefCon = NULL;

  CheckError( AudioUnitSetProperty(outputUnit,
                                   kAudioUnitProperty_SetRenderCallback,
                                   kAudioUnitScope_Input,
                                   0,
                                   &callbackStruct,
                                   sizeof(callbackStruct)),
             "Couldn't set the render callback on the input unit");

  //CheckError(AudioUnitInitialize(inputUnit), "Couldn't initialize the output unit");

  CheckError(AudioUnitInitialize(outputUnit), "Couldn't initialize the output unit");

  UInt32 isInputAvailable = 0;
  size = sizeof(isInputAvailable);

  isInputAvailable = 1;

  //CheckError( AudioOutputUnitStart(inputUnit), "Couldn't start the output unit");
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
