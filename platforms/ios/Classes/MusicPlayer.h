//
//  MusicPlayer.h
//  modizer4
//
//  Created by Yohann Magnien on 12/06/10.
//  Copyright 2010 __YoyoFR / Yohann Magnien__. All rights reserved.
//


#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>

#import "ModizerConstants.h"
#import "modplug.h"


@interface MusicPlayer : NSObject {
	//General infos
	int mod_message_updated,mod_subsongs;
	int mod_currentsub,mod_minsub,mod_maxsub;
	int mp_datasize,numChannels;
	int mLoopMode; //0:off, 1:infinite
	float mPanningFactor;
	int bGlobalAudioPause;
	int iCurrentTime,iModuleLength;
	float mVolume;
	int optForceMono;


	//for spectrum analyzer
	short int **buffer_ana_cpy;
	

	//Modplug stuff
	ModPlug_Settings mp_settings;
	ModPlugFile *mp_file;
	int *genRow,*genPattern,*playRow,*playPattern;	
	char *mp_data;
	int numPatterns,numSamples,numInstr;

	
	//AVFoundation
	AudioQueueRef mAudioQueue;
	AudioQueueBufferRef *mBuffers;
	int mQueueIsBeingStopped;
};


//Modplug stuff
@property ModPlug_Settings mp_settings;
@property ModPlugFile *mp_file;
@property char *mp_data;
@property int *genRow,*genPattern,*playRow,*playPattern;
@property float mVolume;
@property int numChannels,numPatterns,numSamples,numInstr;

//Player status
@property int bGlobalAudioPause;
@property int iCurrentTime,iModuleLength;
@property int mod_message_updated,mod_subsongs,mod_currentsub,mod_minsub,mod_maxsub,mLoopMode;
@property int optForceMono;
@property int mPlayType; //1:GME, 2:libmodplug, 3:Adplug
@property int mp_datasize;

//for spectrum analyzer
@property short int **buffer_ana_cpy;
@property AudioQueueRef mAudioQueue;
@property AudioQueueBufferRef *mBuffers;
@property int mQueueIsBeingStopped;

-(id) initMusicPlayer;
-(BOOL) isPlaying;
-(int) isSeeking;
-(int) isPlayingTrackedMusic;
-(void) playPrevSub;
-(void) playNextSub;
-(NSString*) getModMessage;
-(NSString*) getModName;
-(NSString*) getModType;
-(NSString*) getPlayerName;
-(void) setModPlugMasterVol:(float)mstVol;
-(void) Seek:(int)seek_time;
-(ModPlug_Settings*) getMPSettings;
-(void) updateMPSettings;
-(BOOL) isEndReached;
-(void) Stop;
-(void) Pause:(BOOL)paused;
-(void) Play;
-(void) PlaySeek:(int)startPos subsong:(int)subsong;
-(int) LoadModule;
-(float) getIphoneVolume;
-(void) setIphoneVolume:(float) vol;
-(BOOL) iPhoneDrv_FillAudioBuffer:(AudioQueueBufferRef) mBuffer;
-(BOOL) iPhoneDrv_Init;
-(void) iPhoneDrv_Exit;
-(BOOL) iPhoneDrv_PlayStart;
-(void) iPhoneDrv_PlayStop;
-(void) iPhoneDrv_PlayWaitStop;
-(void) iPhoneDrv_Update:(AudioQueueBufferRef) mBuffer;
-(int) getCurrentPlayedBufferIdx;
-(void) setLoopInf:(int)val;
-(void)setTempo:(unsigned int) t;
-(void)Pump;


@end