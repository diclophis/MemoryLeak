//
//  Audio.cpp
//  MemoryLeak
//
//  Created by Jon Bardin on 2/8/12.
//  Copyright GPL
//

#import <QuartzCore/QuartzCore.h>
#include "Audio.h"
#include "MemoryLeak.h"

#define kOutputBus 0

static OSStatus status;
static AudioComponentInstance audioUnit;

void checkStatus(OSStatus status) {
	if(status == 0)
		printf("success");
	else if(status == errSecNotAvailable)
		printf("no trust results available");
	else if(status == errSecItemNotFound)
		printf("the item cannot be found");
	else if(status == errSecParam)
		printf("parameter error");
	else if(status == errSecAllocate)
		printf("memory allocation error");
	else if(status == errSecInteractionNotAllowed)
		printf("user interaction not allowd");
	else if(status == errSecUnimplemented)
		printf("not implemented");
	else if(status == errSecDuplicateItem)
		printf("item already exists");
	else if(status == errSecDecode)
		printf("unable to decode data");
	else
		NSLog(@"unknown: %ld", status);
}


void interruptionListenerCallback (void *inUserData,UInt32 interruptionState ) {
	if (interruptionState == kAudioSessionBeginInterruption) {
		//TODO: phone call interuption, pause game?
	} else if (interruptionState == kAudioSessionEndInterruption) {
		// if the interruption was removed, and the app had been playing, resume playback
		UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
		AudioSessionSetProperty (
                             kAudioSessionProperty_AudioCategory,
                             sizeof (sessionCategory),
                             &sessionCategory
                             );
		AudioSessionSetActive (true);
	}
}


void propertyListenerCallback (void *inUserData, AudioSessionPropertyID inPropertyID, UInt32 inPropertyValueSize, const void *inPropertyValue) {
	if (inPropertyID==kAudioSessionProperty_AudioRouteChange ) {
    /*
		CFDictionaryRef routeChangeDictionary = (CFDictionaryRef)inPropertyValue;
		NSString *oldroute = (NSString*)CFDictionaryGetValue (
                                                          routeChangeDictionary,
                                                          CFSTR (kAudioSession_AudioRouteChangeKey_OldRoute)
                                                          );
		NSLog(@"Audio route changed : %@",oldroute);
		if ([oldroute compare:@"Headphone"]==NSOrderedSame) {	
			//TODO: pause on headphone?
		}
    */
	}	
}


static OSStatus playbackCallback(void *inRefCon, 
                                 AudioUnitRenderActionFlags *ioActionFlags, 
                                 const AudioTimeStamp *inTimeStamp, 
                                 UInt32 inBusNumber, 
                                 UInt32 inNumberFrames, 
                                 AudioBufferList *ioDataList) {
  
  
	
	if (ioDataList->mNumberBuffers != 1) {
		LOGV("the fuck\n");
	}
	
	AudioBuffer *ioData = &ioDataList->mBuffers[0];
	
  Engine::CurrentGameDoAudio((short int *)ioData->mData, inNumberFrames * sizeof(short) * 2);
	
  return noErr;
}

void initAudio2() {
	AudioSessionInitialize (NULL, NULL, interruptionListenerCallback, NULL);
	status = AudioSessionSetActive (true);
	checkStatus(status);
	UInt32 sessionCategory = kAudioSessionCategory_AmbientSound;
	AudioSessionSetProperty (kAudioSessionProperty_AudioCategory, sizeof (sessionCategory), &sessionCategory);
  
  
	// Describe audio component
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	// Get component
	AudioComponent inputComponent = AudioComponentFindNext(NULL, &desc);
	
	// Get audio units
	status = AudioComponentInstanceNew(inputComponent, &audioUnit);
	checkStatus(status);
	
	UInt32 flag = 1;
	
	// Enable IO for playback
	status = AudioUnitSetProperty(audioUnit, 
                                kAudioOutputUnitProperty_EnableIO, 
                                kAudioUnitScope_Output, 
                                kOutputBus,
                                &flag, 
                                sizeof(flag));
	checkStatus(status);
	
	AudioStreamBasicDescription audioFormat = {0};
	
	audioFormat.mSampleRate = 44100;
	audioFormat.mFormatID = kAudioFormatLinearPCM;
	audioFormat.mFormatFlags = kAudioFormatFlagsCanonical;
	audioFormat.mFramesPerPacket = 1;
	audioFormat.mChannelsPerFrame	= 2;
	audioFormat.mBitsPerChannel = 16;
  audioFormat.mBytesPerFrame = audioFormat.mBitsPerChannel / 8 * audioFormat.mChannelsPerFrame;
  audioFormat.mBytesPerPacket = audioFormat.mBytesPerFrame * audioFormat.mFramesPerPacket;
	
	status = AudioUnitSetProperty(audioUnit, 
                                kAudioUnitProperty_StreamFormat, 
                                kAudioUnitScope_Input, 
                                kOutputBus, 
                                &audioFormat, 
                                sizeof(audioFormat));
	checkStatus(status);
	
	AURenderCallbackStruct callbackStruct;
  
	// Set output callback
	callbackStruct.inputProc = playbackCallback;
	callbackStruct.inputProcRefCon = NULL; //was self
	status = AudioUnitSetProperty(audioUnit, 
                                kAudioUnitProperty_SetRenderCallback, 
                                kAudioUnitScope_Global, 
                                kOutputBus,
                                &callbackStruct, 
                                sizeof(callbackStruct));
	checkStatus(status);
	
	Float32 aBufferLength; // In seconds
	UInt32 size = sizeof(aBufferLength);
	
  status = AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration, &size, &aBufferLength);
	checkStatus(status);
  
	// Initialise
	status = AudioUnitInitialize(audioUnit);
	checkStatus(status);
  
	status = AudioOutputUnitStart(audioUnit);
	checkStatus(status);
}
