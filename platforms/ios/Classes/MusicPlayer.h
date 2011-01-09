//
//  MusicPlayer.h
//  modizer4
//
//  Created by Yohann Magnien on 12/06/10.
//  Copyright 2010 __YoyoFR / Yohann Magnien__. All rights reserved.
//

#import "ModizerConstants.h"

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>

//#import "AppDelegate_Phone.h"
//MODPLUG
#import "modplug.h"

//GME
//#import "gme.h"
//SIDPLAY
//SID2
//#import "sidplay2.h"
//#import "resid.h"
//SID1
//#import "emucfg.h"
//ADPLUG
//#import "adplug.h"
//#import "emuopl.h"
//#import "opl.h"
//STSOUND
//#import "YmMusic.h"
//HVL
//#import "hvl_replay.h"

//SC68
//#import "api68.h"

//extern "C" {
//	//AOSDK
//#import "ao.h"
//#import "eng_protos.h"
//#import <dirent.h>
//#import "driver.h"
// MDX
//#import "mdx.h"
//	
//}

@interface MusicPlayer : NSObject {
	//General infos
	int mod_message_updated,mod_subsongs;
	int mod_currentsub,mod_minsub,mod_maxsub;
	int mPlayType; //1:GME, 2:libmodplug, 3:Adplug, 4:AO, 5:SexyPSF, 6:UADE, 7:HVL
	int mp_datasize,numChannels;
	int mLoopMode; //0:off, 1:infinite
	float mPanningFactor;

	//Player status
	int bGlobalAudioPause;
	int iCurrentTime,iModuleLength;
	float mVolume;
	//
	//for spectrum analyzer
	short int **buffer_ana_cpy;
	//
	//Option
	//Global
	int optForceMono;
	//HVL
	struct hvl_tune *hvl_song;
	//UADE
	int mUADE_OptChange;
	int mUADE_OptLED,mUADE_OptNORM,mUADE_OptPOSTFX,mUADE_OptPAN,mUADE_OptHEAD,mUADE_OptGAIN;
	float mUADE_OptGAINValue,mUADE_OptPANValue;
	//GME
	int optAccurateGME;
	int optGMEFadeOut;
	int optGMEDefaultLength;
	//Modplug
	ModPlug_Settings mp_settings;
	//AO
	//SexyPSF
	//adplug
	//SID
	int optSIDoptim;
	int mSidEngineType,mAskedSidEngineType; //Current / next
	
	
	//Adplug stuff
	//CPlayer	*adPlugPlayer;
	//CEmuopl *opl;
	int opl_towrite;
	//
	//GME stuff
	//Music_Emu* gme_emu;
	//
	//AO stuff
	//unsigned char *ao_buffer;
	//ao_display_info ao_info;
	//
	
	//Modplug stuff
	ModPlugFile *mp_file;
	int *genRow,*genPattern,*playRow,*playPattern;	
	char *mp_data;
	int numPatterns,numSamples,numInstr;
	
	//
	AudioQueueRef mAudioQueue;
	AudioQueueBufferRef *mBuffers;
	int mQueueIsBeingStopped;
};
@property int mod_message_updated,mod_subsongs,mod_currentsub,mod_minsub,mod_maxsub,mLoopMode;
@property int optForceMono;
@property int mPlayType; //1:GME, 2:libmodplug, 3:Adplug
@property int mp_datasize;
//Adplug stuff
//@property CPlayer	*adPlugPlayer;
//@property CEmuopl *opl;
//@property int opl_towrite;
//GME stuff
//@property Music_Emu* gme_emu;
//@property int optAccurateGME,optGMEDefaultLength;
//SID
//@property int optSIDoptim,mSidEngineType,mAskedSidEngineType;
//AO stuff
//@property unsigned char *ao_buffer;
//@property ao_display_info ao_info;
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
//for spectrum analyzer
@property short int **buffer_ana_cpy;
@property AudioQueueRef mAudioQueue;
@property AudioQueueBufferRef *mBuffers;
@property int mQueueIsBeingStopped;

-(id) initMusicPlayer;

-(void) generateSoundThread;

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

-(int) getSongLengthfromMD5:(int)track_nb;
-(void) setSongLengthfromMD5:(int)track_nb songlength:(int)slength;

-(ModPlug_Settings*) getMPSettings;
-(void) updateMPSettings;

-(BOOL) isEndReached;
-(void) Stop;
-(void) Pause:(BOOL)paused;
-(void) Play;
-(void) PlaySeek:(int)startPos subsong:(int)subsong;
//-(int) LoadModule:(NSString*)_filePath defaultUADE:(int)defaultUADE slowDevice:(int)slowDevice;
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

-(void) optUADE_Led:(int)isOn;
-(void) optUADE_Norm:(int)isOn;
-(void) optUADE_PostFX:(int)isOn;
-(void) optUADE_Pan:(int)isOn;
-(void) optUADE_PanValue:(float_t)val;
-(void) optUADE_Head:(int)isOn;
-(void) optUADE_Gain:(int)isOn;
-(void) optUADE_PanValue:(float_t)val;
-(void) optUADE_GainValue:(float_t)val;
-(void) optGME_DefaultLength:(float_t)val;

-(void) optSEXYPSF:(int)reverb interpol:(int)interpol;
-(void) optAOSDK:(int)reverb interpol:(int)interpol;
-(void) setLoopInf:(int)val;

@end
