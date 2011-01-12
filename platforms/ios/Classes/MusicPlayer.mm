//
//  MusicPlayer.mm
//  modizer4
//
//  Created by Yohann Magnien on 12/06/10.
//  Copyright 2010 __YoyoFR / Yohann Magnien__. All rights reserved.
//


#include <pthread.h>

#include <sqlite3.h>

extern pthread_mutex_t db_mutex;
extern pthread_mutex_t play_mutex;

#import "modplug.h"

#import "MusicPlayer.h"

/* file types */
static volatile int mSlowDevice;
static volatile int moveToPrevSubSong,moveToNextSubSong,mod_wantedcurrentsub,mChangeOfSong,mNewModuleLength;
static int sampleVolume,mInterruptShoudlRestart;
static char str_name[1024];
static char mod_message[4096+MAX_STIL_DATA_LENGTH];
static char mod_name[256];

static 	int buffer_ana_subofs;
static short int **buffer_ana;
static volatile int buffer_ana_gen_ofs,buffer_ana_play_ofs;
static volatile int *buffer_ana_flag;
static volatile int bGlobalIsPlaying,bGlobalShouldEnd,bGlobalSeekProgress,bGlobalEndReached,bGlobalSoundGenInProgress,bGlobalSoundHasStarted;
static volatile int mNeedSeek,mNeedSeekTime;


void iPhoneDrv_AudioCallback(void *data, AudioQueueRef mQueue, AudioQueueBufferRef mBuffer) {
	MusicPlayer *mplayer=(MusicPlayer*)data;
    if (!mplayer.mQueueIsBeingStopped) {
		[mplayer iPhoneDrv_Update:mBuffer];
    }
}


/************************************************/
/* Handle phone calls interruptions             */
/************************************************/ 
void interruptionListenerCallback (void *inUserData,UInt32 interruptionState ) {
	MusicPlayer *mplayer=(MusicPlayer*)inUserData;
	if (interruptionState == kAudioSessionBeginInterruption) {
		mInterruptShoudlRestart=0;
		if ([mplayer isPlaying] && (mplayer.bGlobalAudioPause==0)) {
			[mplayer Pause:YES];
			mInterruptShoudlRestart=1;
		}
	}
    else if (interruptionState == kAudioSessionEndInterruption) {
		// if the interruption was removed, and the app had been playing, resume playback
		if (mInterruptShoudlRestart) {
			[mplayer Pause:NO];
			mInterruptShoudlRestart=0;
		}
		
		UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
		AudioSessionSetProperty (
								 kAudioSessionProperty_AudioCategory,
								 sizeof (sessionCategory),
								 &sessionCategory
								 );
		AudioSessionSetActive (true);
	}
}
/*************************************************/
/* Audio property listener                       */
/*************************************************/
void propertyListenerCallback (void                   *inUserData,                                 // 1
							   AudioSessionPropertyID inPropertyID,                                // 2
							   UInt32                 inPropertyValueSize,                         // 3
							   const void             *inPropertyValue ) {
	if (inPropertyID==kAudioSessionProperty_AudioRouteChange ) {
		MusicPlayer *mplayer = (MusicPlayer *) inUserData; // 6
		if ([mplayer isPlaying]) {
			CFDictionaryRef routeChangeDictionary = (CFDictionaryRef)inPropertyValue;        // 8
			//CFStringRef 
			NSString *oldroute = (NSString*)CFDictionaryGetValue (
																  routeChangeDictionary,
																  CFSTR (kAudioSession_AudioRouteChangeKey_OldRoute)
																  );
			//NSLog(@"Audio route changed : %@",oldroute);
			if ([oldroute compare:@"Headphone"]==NSOrderedSame) {  // 9				
				[mplayer Pause:YES];
			}
		}
	}
}

@implementation MusicPlayer


@synthesize mod_message_updated,mod_subsongs,mod_currentsub,mod_minsub,mod_maxsub,mLoopMode;
@synthesize mp_datasize;
@synthesize optForceMono;
@synthesize mp_settings;
@synthesize mp_file;
@synthesize mp_data;
@synthesize mVolume;
@synthesize numChannels,numPatterns,numSamples,numInstr;
@synthesize genRow,genPattern,playRow,playPattern;
@synthesize bGlobalAudioPause;
@synthesize iCurrentTime,iModuleLength;
@synthesize mAudioQueue;
@synthesize mBuffers;
@synthesize mQueueIsBeingStopped;
@synthesize buffer_ana_cpy;


-(id) initMusicPlayer {
	
	
	if ((self = [super init])) {
		AudioSessionInitialize (
								NULL,
								NULL,
								interruptionListenerCallback,
								self
								);
		UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
		AudioSessionSetProperty (
								 kAudioSessionProperty_AudioCategory,
								 sizeof (sessionCategory),
								 &sessionCategory
								 );
		
		//Check if still required or not 
		Float32 preferredBufferDuration = SOUND_BUFFER_SIZE_SAMPLE*1.0f/PLAYBACK_FREQ;                      // 1
		AudioSessionSetProperty (                                     // 2
								 kAudioSessionProperty_PreferredHardwareIOBufferDuration,
								 sizeof (preferredBufferDuration),
								 &preferredBufferDuration
								 );
		AudioSessionPropertyID routeChangeID = kAudioSessionProperty_AudioRouteChange;    // 1
		AudioSessionAddPropertyListener (                                 // 2
										 routeChangeID,                                                 // 3
										 propertyListenerCallback,                                      // 4
										 self                                                       // 5
										 );
		AudioSessionSetActive(true);	
		
		
		buffer_ana_flag=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
		buffer_ana=(short int**)malloc(SOUND_BUFFER_NB*sizeof(unsigned short int *));
		buffer_ana_cpy=(short int**)malloc(SOUND_BUFFER_NB*sizeof(unsigned short int *));
		buffer_ana_subofs=0;
		for (int i=0;i<SOUND_BUFFER_NB;i++) {
			buffer_ana[i]=(short int *)malloc(SOUND_BUFFER_SIZE_SAMPLE*2*2);
			buffer_ana_cpy[i]=(short int *)malloc(SOUND_BUFFER_SIZE_SAMPLE*2*2);
			buffer_ana_flag[i]=0;
		}
		
		//Global
		bGlobalShouldEnd=0;
		bGlobalSoundGenInProgress=0;
		optForceMono=0;
		mod_subsongs=0;
		mod_message_updated=0;
		bGlobalAudioPause=0;
		bGlobalIsPlaying=0;
		mVolume=1.0f;
		mLoopMode=1;
		mPanningFactor=0.7f;
		
		//MODPLUG specific
		mp_file=NULL;
		genPattern=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
		genRow=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
		playPattern=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
		playRow=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
		
		[self iPhoneDrv_Init];		
	}
	return self;
}


-(void) dealloc {
	bGlobalShouldEnd=1;
	if (bGlobalIsPlaying) [self Stop];
	[self iPhoneDrv_Exit];
	for (int i=0;i<SOUND_BUFFER_NB;i++) {
		free(buffer_ana_cpy[i]);
		free(buffer_ana[i]);
	}
	free(buffer_ana_cpy);
	free(buffer_ana);
	free((void*)buffer_ana_flag);
	free(playRow);
	free(playPattern);
	free(genRow);
	free(genPattern);
	[super dealloc];
}



-(BOOL) iPhoneDrv_Init {
    AudioStreamBasicDescription mDataFormat;
    UInt32 err;
    
    /* We force this format for iPhone */
    mDataFormat.mFormatID = kAudioFormatLinearPCM;
    mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger |
	kAudioFormatFlagIsPacked;
	
	mDataFormat.mSampleRate = PLAYBACK_FREQ;
    
	mDataFormat.mBitsPerChannel = 16;
    
	mDataFormat.mChannelsPerFrame = 2;
    
    mDataFormat.mBytesPerFrame = (mDataFormat.mBitsPerChannel>>3) * mDataFormat.mChannelsPerFrame;
	
    mDataFormat.mFramesPerPacket = 1; 
    mDataFormat.mBytesPerPacket = mDataFormat.mBytesPerFrame;
    
    /* Create an Audio Queue... */
    err = AudioQueueNewOutput( &mDataFormat, 
							  iPhoneDrv_AudioCallback, 
							  self, 
							  NULL, //CFRunLoopGetCurrent(),
							  kCFRunLoopCommonModes,
							  0, 
							  &mAudioQueue );
    
    /* ... and its associated buffers */
    mBuffers = (AudioQueueBufferRef*)malloc( sizeof(AudioQueueBufferRef) * SOUND_BUFFER_NB );
    for (int i=0; i<SOUND_BUFFER_NB; i++) {
		AudioQueueBufferRef mBuffer;
		err = AudioQueueAllocateBuffer( mAudioQueue, SOUND_BUFFER_SIZE_SAMPLE*2*2, &mBuffer );
		
		mBuffers[i]=mBuffer;
    }
    /* Set initial playback volume */
    err = AudioQueueSetParameter( mAudioQueue, kAudioQueueParam_Volume, mVolume );


    return 1;
}


-(void) iPhoneDrv_Exit {
    AudioQueueDispose( mAudioQueue, true );
    free( mBuffers );
}

-(void) setIphoneVolume:(float) vol {
	UInt32 err;
	mVolume=vol;
    err = AudioQueueSetParameter( mAudioQueue, kAudioQueueParam_Volume, mVolume );
}

-(float) getIphoneVolume {
	UInt32 err;
    err = AudioQueueGetParameter( mAudioQueue, kAudioQueueParam_Volume, &mVolume );
	return mVolume;
}

-(BOOL) iPhoneDrv_PlayStart {
    UInt32 err;
    UInt32 i;
	
    /*
     * Enqueue all the allocated buffers before starting the playback.
     * The audio callback will be called as soon as one buffer becomes
     * available for filling.
     */
	
	mQueueIsBeingStopped=FALSE;
	
	buffer_ana_gen_ofs=buffer_ana_play_ofs=0;
	buffer_ana_subofs=0;
	for (int i=0;i<SOUND_BUFFER_NB;i++) {
		buffer_ana_flag[i]=0;
		playRow[i]=playPattern[i]=genRow[i]=genPattern[i]=0;
		memset(buffer_ana[i],0,SOUND_BUFFER_SIZE_SAMPLE*2*2);
		memset(buffer_ana_cpy[i],0,SOUND_BUFFER_SIZE_SAMPLE*2*2);		
	}
	sampleVolume=256;
    for (i=0; i<SOUND_BUFFER_NB; i++) {
		memset(mBuffers[i]->mAudioData,0,SOUND_BUFFER_SIZE_SAMPLE*2*2);
		mBuffers[i]->mAudioDataByteSize = SOUND_BUFFER_SIZE_SAMPLE*2*2;
		AudioQueueEnqueueBuffer( mAudioQueue, mBuffers[i], 0, NULL);
		//		[self iPhoneDrv_FillAudioBuffer:mBuffers[i]];
		
    }
	bGlobalAudioPause=0;	
    /* Start the Audio Queue! */
	//AudioQueuePrime( mAudioQueue, 0,NULL );
    err = AudioQueueStart( mAudioQueue, NULL );
	
    return 1;
}

-(void) iPhoneDrv_PlayWaitStop {
	int counter=0;
    mQueueIsBeingStopped = TRUE;
	bGlobalAudioPause=2;
	AudioQueueStop( mAudioQueue, FALSE );
	//while ([self isEndReached]==NO) {
	//	[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
	//	counter++;
	//	if (counter*DEFAULT_WAIT_TIME_MS>2) break;
	//}
	AudioQueueReset( mAudioQueue );	
    mQueueIsBeingStopped = FALSE;
}



-(void) iPhoneDrv_PlayStop {
    mQueueIsBeingStopped = TRUE;
	bGlobalAudioPause=2;
    AudioQueueStop( mAudioQueue, TRUE );
	AudioQueueReset( mAudioQueue );	
    mQueueIsBeingStopped = FALSE;
}


-(void) iPhoneDrv_Update:(AudioQueueBufferRef) mBuffer {   
    /* the real processing takes place in FillAudioBuffer */
	[self iPhoneDrv_FillAudioBuffer:mBuffer];
}

-(BOOL) iPhoneDrv_FillAudioBuffer:(AudioQueueBufferRef) mBuffer {       
	int skip_queue=0;
	mBuffer->mAudioDataByteSize = SOUND_BUFFER_SIZE_SAMPLE*2*2;
	if (bGlobalAudioPause) {
		memset(mBuffer->mAudioData, 0, SOUND_BUFFER_SIZE_SAMPLE*2*2);
		if (bGlobalAudioPause==2) skip_queue=1;//return 0;  //End of song
    } else {
		//consume another buffer
		if (buffer_ana_flag[buffer_ana_play_ofs]) {
			bGlobalSoundHasStarted++;
			if (buffer_ana_flag[buffer_ana_play_ofs]&2) { //changed currentTime
				iCurrentTime=mNeedSeekTime;
				mNeedSeek=0;
				bGlobalSeekProgress=0;
			}
			
			playPattern[buffer_ana_play_ofs]=genPattern[buffer_ana_play_ofs];
			playRow[buffer_ana_play_ofs]=genRow[buffer_ana_play_ofs];
						
			iCurrentTime+=1000*SOUND_BUFFER_SIZE_SAMPLE/PLAYBACK_FREQ;
			
			if (sampleVolume<256) {
				if (optForceMono) {
					for (int i=0;i<SOUND_BUFFER_SIZE_SAMPLE;i++) {
						int val=((short int *)mBuffer->mAudioData)[i*2+1]=((buffer_ana[buffer_ana_play_ofs][i*2]+buffer_ana[buffer_ana_play_ofs][i*2+1])/2)*sampleVolume/256;
						((short int *)mBuffer->mAudioData)[i*2]=val;
						((short int *)mBuffer->mAudioData)[i*2+1]=val;
					}
				} else for (int i=0;i<SOUND_BUFFER_SIZE_SAMPLE;i++) {
					((short int *)mBuffer->mAudioData)[i*2]=buffer_ana[buffer_ana_play_ofs][i*2]*sampleVolume/256;
					((short int *)mBuffer->mAudioData)[i*2+1]=buffer_ana[buffer_ana_play_ofs][i*2+1]*sampleVolume/256;
				}
			} else {
				if (optForceMono) {
					for (int i=0;i<SOUND_BUFFER_SIZE_SAMPLE;i++) {
						int val=((short int *)mBuffer->mAudioData)[i*2+1]=((buffer_ana[buffer_ana_play_ofs][i*2]+buffer_ana[buffer_ana_play_ofs][i*2+1])/2)*sampleVolume/256;
						((short int *)mBuffer->mAudioData)[i*2]=val;
						((short int *)mBuffer->mAudioData)[i*2+1]=val;
					}
				} else memcpy((char*)mBuffer->mAudioData,buffer_ana[buffer_ana_play_ofs],SOUND_BUFFER_SIZE_SAMPLE*2*2);
			}
			memcpy(buffer_ana_cpy[buffer_ana_play_ofs],buffer_ana[buffer_ana_play_ofs],SOUND_BUFFER_SIZE_SAMPLE*2*2);
			
			if (sampleVolume<256) sampleVolume+=32;
			
			if (buffer_ana_flag[buffer_ana_play_ofs]&4) { //end reached
				//iCurrentTime=0;
				bGlobalAudioPause=2;
			}
			
			if (buffer_ana_flag[buffer_ana_play_ofs]&8) { //end reached but continue to play
				iCurrentTime=0;
				iModuleLength=mNewModuleLength;
				mChangeOfSong=0;
				mod_message_updated=1;
			}
			
			
			buffer_ana_flag[buffer_ana_play_ofs]=0;
			buffer_ana_play_ofs++;
			if (buffer_ana_play_ofs==SOUND_BUFFER_NB) buffer_ana_play_ofs=0;
		} else {
			memset((char*)mBuffer->mAudioData,0,SOUND_BUFFER_SIZE_SAMPLE*2*2);  //WARNING : not fast enough!!
		}
	}
	if (!skip_queue) {
		AudioQueueEnqueueBuffer( mAudioQueue, mBuffer, 0, NULL);
		if (bGlobalAudioPause==2) {
			AudioQueueStop( mAudioQueue, FALSE );
		}
	} else {
		buffer_ana_play_ofs++;
		if (buffer_ana_play_ofs==SOUND_BUFFER_NB) buffer_ana_play_ofs=0;
	}
    return 0;
}


-(int) getCurrentPlayedBufferIdx {
	return buffer_ana_play_ofs;
}


-(void)Pump {
	if (bGlobalIsPlaying) {
		bGlobalSoundGenInProgress=1;
		if ( !bGlobalEndReached) {
			int nbBytes=0;
			if (buffer_ana_flag[buffer_ana_gen_ofs]==0) {
				if (mNeedSeek==1) { //SEEK
					mNeedSeek=2;  //taken into account
					bGlobalSeekProgress=-1;
					ModPlug_Seek(mp_file,mNeedSeekTime);
				}
				if (moveToNextSubSong) {
					if (mod_currentsub<mod_maxsub) {
						mod_currentsub++;
						mod_message_updated=1;
					}
					moveToNextSubSong=0;
				}
				
				if (moveToPrevSubSong) {
					moveToPrevSubSong=0;
					if (mod_currentsub>mod_minsub) {
						mod_currentsub--;
						mod_message_updated=1;
					}
				}
				
				genPattern[buffer_ana_gen_ofs]=ModPlug_GetCurrentPattern(mp_file);
				genRow[buffer_ana_gen_ofs]=ModPlug_GetCurrentRow(mp_file);
				nbBytes = ModPlug_Read(mp_file,buffer_ana[buffer_ana_gen_ofs],SOUND_BUFFER_SIZE_SAMPLE*2*2);
				printf("the fuck: %d\n", nbBytes);
				buffer_ana_flag[buffer_ana_gen_ofs]=1;
				if (mNeedSeek==2) {  //ask for a currentime update when this buffer will be played
					buffer_ana_flag[buffer_ana_gen_ofs]=buffer_ana_flag[buffer_ana_gen_ofs]|2;
					mNeedSeek=3;  //to avoid taking into account another time
				}
				
				if (nbBytes<SOUND_BUFFER_SIZE_SAMPLE*2*2) {
					buffer_ana_flag[buffer_ana_gen_ofs]=buffer_ana_flag[buffer_ana_gen_ofs]|4; //end reached
					bGlobalEndReached=1;
				}
				
				if (mChangeOfSong==1) {
					buffer_ana_flag[buffer_ana_gen_ofs]=buffer_ana_flag[buffer_ana_gen_ofs]|8; //end reached but continue
					mChangeOfSong=2;
				}
				
				buffer_ana_gen_ofs++;
				if (buffer_ana_gen_ofs==SOUND_BUFFER_NB) buffer_ana_gen_ofs=0;
			}
		}
		bGlobalSoundGenInProgress=0;
	}
}


-(void)playPrevSub {
	if (mod_subsongs<=1) return;
	moveToPrevSubSong=1;
}


-(void)playNextSub {
	if (mod_subsongs<=1) return;	
	moveToNextSubSong=1;
}


-(BOOL)isEndReached {
	UInt32 i,datasize;
	datasize=sizeof(UInt32);
	AudioQueueGetProperty(mAudioQueue,kAudioQueueProperty_IsRunning,&i,&datasize);
	if (i==0) {
		return YES;
	}
	return NO;
}


-(void)Play {
	int counter=0;
	pthread_mutex_lock(&play_mutex);	
	bGlobalSoundHasStarted=0;
	iCurrentTime=0;
	bGlobalAudioPause=0;
	[self iPhoneDrv_PlayStart];
	bGlobalEndReached=0;
	bGlobalIsPlaying=1;
	mChangeOfSong=0;
	pthread_mutex_unlock(&play_mutex);	
}


-(void) PlaySeek:(int)startPos subsong:(int)subsong {
	if (startPos>iModuleLength-SEEK_START_MARGIN_FROM_END) {
		startPos=iModuleLength-SEEK_START_MARGIN_FROM_END;
		if (startPos<0) startPos=0;
	}
	
	if (startPos) [self Seek:startPos];
	[self Play];
	iCurrentTime=startPos;
}


-(void)Stop {
	bGlobalIsPlaying=0;
	[self iPhoneDrv_PlayStop];
	
	bGlobalSeekProgress=0;
	bGlobalAudioPause=0;

	if (mp_file) {
		ModPlug_Unload(mp_file);
	}
	if (mp_data) free(mp_data);
	mp_file=NULL;
	
}


-(void)Pause:(BOOL)paused {
	bGlobalAudioPause=(paused?1:0);
	if (paused) AudioQueuePause(mAudioQueue);
	else AudioQueueStart(mAudioQueue,NULL);
	mod_message_updated=1;
}


-(ModPlug_Settings*)getMPSettings {
	ModPlug_GetSettings(&mp_settings);
	return &mp_settings;
}


-(void)updateMPSettings {
	ModPlug_SetSettings(&mp_settings);
}


-(int)LoadModule {

	
	NSString *filePath = [[NSBundle mainBundle] pathForResource:@"0" ofType:nil inDirectory:@"assets/sounds"];

	int i;
	mSlowDevice=0;
	

	const char *modName;
	char *modMessage;
	
	FILE *f=fopen([filePath UTF8String],"rb");
	if (f==NULL) {
		NSLog(@"Cannot open file %@",filePath);
		return -1;
	}
	
	fseek(f,0L,SEEK_END);
	mp_datasize=ftell(f);
	rewind(f);
	mp_data=(char*)malloc(mp_datasize);
	fread(mp_data,mp_datasize,sizeof(char),f);
	fclose(f);
	
	[self getMPSettings];
	//if (mLoopMode==1) mp_settings.mLoopCount=1<<30; //Should be like "infinite"
	[self updateMPSettings];
	
	mp_file=ModPlug_Load(mp_data,mp_datasize);
	if (mp_file==NULL) {
		free(mp_data); /* ? */
		NSLog(@"ModPlug_load error");
		return 1;  //Could not find a lib to load module
	} else {
		
		iModuleLength=ModPlug_GetLength(mp_file);
		iCurrentTime=0;
		
		numChannels=ModPlug_NumChannels(mp_file);
		numPatterns=ModPlug_NumPatterns(mp_file);
		
		modName=ModPlug_GetName(mp_file);
		if (!modName) {
			sprintf(mod_name," %s",[[[filePath lastPathComponent] stringByDeletingPathExtension] UTF8String]);
		} else if (modName[0]==0) {
			sprintf(mod_name," %s",[[[filePath lastPathComponent] stringByDeletingPathExtension] UTF8String]);
		} else {
			sprintf(mod_name," %s", modName);
		}
		
		
		numSamples=ModPlug_NumSamples(mp_file);
		numInstr=ModPlug_NumInstruments(mp_file);
		
		modMessage=ModPlug_GetMessage(mp_file);			
		if (modMessage) sprintf(mod_message,"%s\n",modMessage);
		else {
			if ((numInstr==0)&&(numSamples==0)) sprintf(mod_message,"N/A\n");
			else sprintf(mod_message,"");
		}
		
		
		if (numInstr>0) {
			for (i=1;i<=numInstr;i++) {
				ModPlug_InstrumentName(mp_file,i,str_name);
				sprintf(mod_message,"%s%s\n", mod_message,str_name);
			};
		}
		if (numSamples>0) {
			for (i=1;i<=numSamples;i++) {
				ModPlug_SampleName(mp_file,i,str_name);
				sprintf(mod_message,"%s%s\n", mod_message,str_name);
			};
		}
		
		//Loop
		if (mLoopMode==1) iModuleLength=-1;
		
		return 0;
	}
}

-(NSString*)getModMessage {
	NSString *modMessage;
	if (modMessage==nil) return @"";
	return modMessage;
}

-(NSString*)getModName {
	NSString *modName;
	if (modName==nil) return @"";
	return modName;
}


-(void) setModPlugMasterVol:(float) mstVol {
	ModPlug_SetMasterVolume(mp_file,(int )(mstVol*512));
}


-(void) Seek:(int) seek_time {
	if (mNeedSeek) return;
	
	if (seek_time>iModuleLength-SEEK_START_MARGIN_FROM_END) {
		seek_time=iModuleLength-SEEK_START_MARGIN_FROM_END;
		if (seek_time<0) seek_time=0;
	}
	
	iCurrentTime=mNeedSeekTime=seek_time;
	mNeedSeek=1;
}


-(NSString*) getPlayerName {
	return @"";	
}


-(int) isPlayingTrackedMusic {
	return 1;
}


-(NSString*) getModType {
	return @" ";
}

-(BOOL)isPlaying {
	if (bGlobalIsPlaying) return TRUE;
	else return FALSE;
}

-(int)isSeeking {
	if (bGlobalAudioPause) return 0;
	return bGlobalSeekProgress;
}
-(void)setLoopInf:(int)val {
	mLoopMode=val;
}

-(void)setTempo:(unsigned int) t {
	ModPlug_SetTempo(mp_file, t);
}

@end
