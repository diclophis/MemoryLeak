//
//  Audio.h
//  MemoryLeak
//
//  Created by Jon Bardin on 2/8/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <AudioToolbox/AudioToolbox.h>

void checkStatus(OSStatus s);
void iPhoneDrv_AudioCallback(void *data, AudioQueueRef mQueue, AudioQueueBufferRef mBuffer);
void interruptionListenerCallback (void *inUserData,UInt32 interruptionState );
void propertyListenerCallback (void *inUserData, AudioSessionPropertyID inPropertyID, UInt32 inPropertyValueSize, const void *inPropertyValue);