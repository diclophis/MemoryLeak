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

#import "MusicPlayer.h"

/* file types */
static char modtype[64];
//static uint32 ao_type;
static volatile int mSlowDevice;
static volatile int moveToPrevSubSong,moveToNextSubSong,mod_wantedcurrentsub,mChangeOfSong,mNewModuleLength;
static int sampleVolume,mInterruptShoudlRestart;
static char str_name[1024];
static char stil_info[MAX_STIL_DATA_LENGTH];
static char mod_message[4096+MAX_STIL_DATA_LENGTH];
static char mod_name[256];

static char song_md5[33];

//MDX
//static MDX_DATA *mdx;
//static PDX_DATA *pdx;

//HVL
//static int hvl_sample_to_write,mHVLinit;
//STSOUND
//static YMMUSIC *ymMusic;
//SC68
//static api68_t *sc68;
//SID
//static sidplay2 *mSidEmuEngine;
//static ReSIDBuilder *mBuilder;
//static SidTune *mSidTune;
//static emuEngine *mSid1EmuEngine;
//static sidTune *mSid1Tune;


/*
//GME
static gme_info_t* gme_info;
//AOSDK
static struct { 
	uint32 sig; 
	const char *name; 
	int32 (*start)(uint8 *, uint32, int); 
	int32 (*gen)(int16 *, uint32); 
	int32 (*stop)(void); 
	int32 (*command)(int32, int32); 
	uint32 rate; 
	int32 (*fillinfo)(ao_display_info *); 
} ao_types[] = {
	//{ 0x50534601, "Sony PlayStation (.psf)", psf_start, psf_gen, psf_stop, psf_command, 60, psf_fill_info },
	{ 0x53505500, "Sony PlayStation (.spu)", spu_start, spu_gen, spu_stop, spu_command, 60, spu_fill_info },
	{ 0x50534602, "Sony PlayStation 2 (.psf2)", psf2_start, psf2_gen, psf2_stop, psf2_command, 60, psf2_fill_info },
	//	{ 0x50534641, "Capcom QSound (.qsf)", qsf_start, qsf_gen, qsf_stop, qsf_command, 60, qsf_fill_info },
	//	{ 0x50534611, "Sega Saturn (.ssf)", ssf_start, ssf_gen, ssf_stop, ssf_command, 60, ssf_fill_info },
	//{ 0x50534612, "Sega Dreamcast (.dsf)", dsf_start, dsf_gen, dsf_stop, dsf_command, 60, dsf_fill_info },
	{ 0xffffffff, "", NULL, NULL, NULL, NULL, 0, NULL }
};
 */

int uade_song_end_trigger;

extern "C" {
	extern int sexypsf_missing_psflib;
	extern char sexypsf_psflib_str[256];
	extern int aopsf2_missing_psflib;
	extern char aopsf2_psflib_str[256];

	void mdx_update(unsigned char *data,int len,int end_reached);
	
	// redirect stubs to interface the Z80 core to the QSF engine
	/*	uint8 memory_read(uint16 addr)	{
	 return qsf_memory_read(addr);
	 }
	 uint8 memory_readop(uint16 addr) {
	 return memory_read(addr);
	 }
	 uint8 memory_readport(uint16 addr) {
	 return qsf_memory_readport(addr);
	 }
	 void memory_write(uint16 addr, uint8 byte) {
	 qsf_memory_write(addr, byte);
	 }
	 void memory_writeport(uint16 addr, uint8 byte) {
	 qsf_memory_writeport(addr, byte);
	 }
	 // stub for MAME stuff
	 int change_pc(int foo){
	 return 0;
	 }
	 */
	
	
	//UADE
/*
#include "uadecontrol.h"
#include "ossupport.h"
#include "uadeconfig.h"
#include "uadeconf.h"
#include "sysincludes.h"
#include "songdb.h"
#include "amigafilter.h"
#include "uadeipc.h"
#include "uadeconstants.h"
#include "md5.h"
*/	
	
	//void uade_dummy_wait() {
	//	[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_UADE_MS]; 
	//}
	
	//int uade_main (int argc, char **argv);
	
	//struct uade_state UADEstate,UADEstatebase;	
	char UADEconfigname[PATH_MAX];
	char UADEplayername[PATH_MAX];
	char UADEscorename[PATH_MAX];
	
}

static char my_data [] = "Our cleanup function was called";

/* Example cleanup function automatically called when emulator is deleted. */
static void my_cleanup( void* my_data ) {
	//NSLog(@"\n%s\n", (char*) my_data );
}

static 	int buffer_ana_subofs;
static short int **buffer_ana;
static volatile int uadeThread_running;
static volatile int buffer_ana_gen_ofs,buffer_ana_play_ofs;
static volatile int *buffer_ana_flag;
static volatile int bGlobalIsPlaying,bGlobalShouldEnd,bGlobalSeekProgress,bGlobalEndReached,bGlobalSoundGenInProgress,bGlobalSoundHasStarted;
static volatile int mNeedSeek,mNeedSeekTime;

// String holding the relative path to the source directory
static const char *pathdir;

static void md5_from_buffer(char *dest, size_t destlen,char * buf, size_t bufsize)
{
	uint8_t md5[16];
	int ret;
	//MD5_CTX ctx;
	//MD5Init(&ctx);
	//MD5Update(&ctx, (const unsigned char*)buf, bufsize);
	//MD5Final(md5, &ctx);
	ret =
	snprintf(dest, destlen,
		     "%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
		     md5[0], md5[1], md5[2], md5[3], md5[4], md5[5], md5[6],
		     md5[7], md5[8], md5[9], md5[10], md5[11], md5[12], md5[13],
		     md5[14], md5[15]);
	if (ret >= destlen || ret != 32) {
		fprintf(stderr, "md5 buffer error (%d/%zd)\n", ret, destlen);
		exit(1);
	}
}

static void writeLEword(unsigned char ptr[2], int someWord)
{
	ptr[0] = (someWord & 0xFF);
	ptr[1] = (someWord >> 8);
}

/*
int ao_get_lib(char *filename, uint8 **buffer, uint64 *length) {
	uint8 *filebuf;
	uint32 size;
	FILE *auxfile;
	struct dirent **filelist = {0};
	const char *directory = ".";
	int fcount = -1;
	int i = 0;
	int found=0;
	
	//some aux files come from case insensitive OS, so parsing of dir is needed...
	fcount = scandir(pathdir, &filelist, 0, alphasort);
	
	if(fcount < 0) {
		perror(directory);
		return AO_FAIL;
	}
	
	for(i = 0; i < fcount; i++)  {
		if (!strcasecmp(filelist[i]->d_name,filename)) {
			found=1;
			strcpy(filename,filelist[i]->d_name);
		}
		free(filelist[i]);
	}
	free(filelist);
	
	auxfile = fopen([[NSString stringWithFormat:@"%s/%s",pathdir,filename] UTF8String] , "rb");
	if (!auxfile) {
		printf("Unable to find auxiliary file %s\n", filename);
		return AO_FAIL;
	}
	fseek(auxfile, 0, SEEK_END);
	size = ftell(auxfile);
	fseek(auxfile, 0, SEEK_SET);
	filebuf = (uint8*)malloc(size);
	if (!filebuf) {
		fclose(auxfile);
		printf("ERROR: could not allocate %d bytes of memory\n", size);
		return AO_FAIL;
	}
	fread(filebuf, size, 1, auxfile);
	fclose(auxfile);
	*buffer = filebuf;
	*length = (uint64)size;
	return AO_SUCCESS;
}
*/

void iPhoneDrv_AudioCallback(void *data, AudioQueueRef mQueue, AudioQueueBufferRef mBuffer) {
	MusicPlayer *mplayer=(MusicPlayer*)data;
    if ( !  mplayer.mQueueIsBeingStopped ) {
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
@synthesize mPlayType; //1:GME, 2:libmodplug, 3:Adplug, 4:AO, 5:SexyPSF, 6:UADE
@synthesize mp_datasize;
@synthesize optForceMono;
//Adplug stuff
//@synthesize adPlugPlayer;
//@synthesize opl;
//@synthesize opl_towrite;
//GME stuff
//@synthesize gme_emu;
//@synthesize optAccurateGME,optGMEDefaultLength;
//SID
//@synthesize optSIDoptim,mSidEngineType,mAskedSidEngineType;
//AO stuff
//@synthesize ao_buffer;
//@synthesize ao_info;

//Modplug stuff
//@synthesize mp_settings;
//@synthesize mp_file;
//@synthesize mp_data;
//@synthesize mVolume;
//@synthesize numChannels,numPatterns,numSamples,numInstr;
@synthesize genRow,genPattern,playRow,playPattern;
//Player status
@synthesize bGlobalAudioPause;
@synthesize iCurrentTime,iModuleLength;
//for spectrum analyzer
@synthesize buffer_ana_cpy;
@synthesize mAudioQueue;
@synthesize mBuffers;
@synthesize mQueueIsBeingStopped;

/*
-(api68_t*)setupSc68 {
	api68_init_t init68;
	
	NSBundle *bundle = [NSBundle mainBundle];
	NSString *path = [bundle bundlePath];
	NSMutableString *argData = [NSMutableString stringWithString:@"--sc68_data="];
	[argData appendString:path];
	[argData appendString:@"/Contents/Resources"];
	
	char *t = (char *)[argData UTF8String];
	char *args[] = { (char*)"sc68", t, NULL };
	
	memset(&init68, 0, sizeof(init68));
	init68.alloc = (void *(*)(unsigned int))malloc;
	init68.free = free;
	init68.argc = 2;
	init68.argv = args;
	
	api68_t* api = api68_init(&init68);
	
	return api;
}
*/

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
		AudioSessionSetActive (true);	
		
		
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
		mLoopMode=0;
		mPanningFactor=0.7f;
		
		//
		//MODPLUG specific
		//mp_file=NULL;
		//genPattern=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
		//genRow=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
		//playPattern=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));
		//playRow=(int*)malloc(SOUND_BUFFER_NB*sizeof(int));		
		//
		//GME specific
		optAccurateGME=1;
		optGMEFadeOut=2000;
		optGMEDefaultLength=GME_DEFAULT_LENGTH;
		//
		//UADE specific
		mUADE_OptLED=0;
		mUADE_OptNORM=0;
		mUADE_OptPOSTFX=1;
		mUADE_OptPAN=0;
		mUADE_OptPANValue=0.7f;
		mUADE_OptHEAD=0;
		mUADE_OptGAIN=0;
		mUADE_OptGAINValue=0.5f;
		mUADE_OptChange=0;
		uadeThread_running=0;
		
		/*
		// init  UADE stuff
		char uadeconfname[PATH_MAX];
		memset(&UADEstate, 0, sizeof UADEstate);
		memset(&UADEstatebase, 0, sizeof UADEstatebase);
		//load conf
		NSString *uade_path = [[NSBundle mainBundle] resourcePath];
		sprintf(UADEstatebase.config.basedir.name,"%s",[uade_path UTF8String]);
		UADEstatebase.config.basedir_set=1;
		if (uade_load_initial_config(uadeconfname,sizeof(uadeconfname),&UADEstate.config, &UADEstatebase.config)==0) {
			NSLog(@"Not able to load uade.conf from ~/.uade2/ or %s/.\n",UADEstate.config.basedir.name);
			//TODO : decide if continue or stop
		} else {
			//NSLog(@"Loaded configuration: %s\n", uadeconfname);
		}
		sprintf(UADEstate.config.basedir.name,"%s",[uade_path UTF8String]);
		sprintf(UADEconfigname, "%s/uaerc",UADEstate.config.basedir.name);
		sprintf(UADEscorename, "%s/score",UADEstate.config.basedir.name);
		*/
		
		/*
		//HVL specific
		//mHVLinit=0;
		//
		// SIDPLAY
		// Init SID emu engine
		mSid1EmuEngine=NULL;
		
		mSidEngineType=2;
		mSidEmuEngine=NULL;
		mBuilder=NULL;
		mSidTune=NULL;
		
		mSid1Tune=NULL;
		optSIDoptim=1;
		//
		// SC68
		sc68 = [self setupSc68];
		//
		*/
		[self iPhoneDrv_Init];
		
		[NSThread detachNewThreadSelector:@selector(generateSoundThread) toTarget:self withObject:NULL];
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
	//free(playRow);
	//free(playPattern);
	//free(genRow);
	//free(genPattern);
	
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
    
	
	
    return 1;//VC_Init();
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
	//	NSLog(@"stopping queue");
	bGlobalAudioPause=2;
	AudioQueueStop( mAudioQueue, FALSE );
	while ([self isEndReached]==NO) {
		[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
		counter++;
		if (counter*DEFAULT_WAIT_TIME_MS>2) break;
	}
	AudioQueueReset( mAudioQueue );	
    mQueueIsBeingStopped = FALSE;
}



-(void) iPhoneDrv_PlayStop {
    mQueueIsBeingStopped = TRUE;
	//	NSLog(@"stopping queue");
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
			
			if (mPlayType==2) {//Modplug
				playPattern[buffer_ana_play_ofs]=genPattern[buffer_ana_play_ofs];
				playRow[buffer_ana_play_ofs]=genRow[buffer_ana_play_ofs];
			}
			
			iCurrentTime+=1000*SOUND_BUFFER_SIZE_SAMPLE/PLAYBACK_FREQ;
			
			//if (mPlayType==10) {//SC68
			//	iCurrentTime=api68_seek(sc68, -1,NULL);
			//}
			
			/*	if ((iModuleLength>0)&&(iCurrentTime>iModuleLength)) {
			 if (mPlayType==8) {//SID does not track end, so use length
			 //iCurrentTime=0;
			 bGlobalAudioPause=2;
			 }
			 }*/
			
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
			}
			else {
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

void mdx_update(unsigned char *data,int len,int end_reached) {
	static int count_upd=0;
	count_upd+=len;
	
    if (bGlobalShouldEnd||(!bGlobalIsPlaying)) {
		//mdx_stop();
		return;
	}
	
	while (buffer_ana_flag[buffer_ana_gen_ofs]) {
		[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
		if (bGlobalShouldEnd||(!bGlobalIsPlaying)) {
			//mdx_stop();
			return;
		}
	}
	
	int to_fill=SOUND_BUFFER_SIZE_SAMPLE*2*2-buffer_ana_subofs;
	
	if (mSlowDevice) len=len*2;
	
	if (len<to_fill) {
		
		if (mSlowDevice) {
			signed int *dst=(signed int *)((char*)(buffer_ana[buffer_ana_gen_ofs])+buffer_ana_subofs);
			signed int *src=(signed int *)data;
			for (int i=len/8-1;i>=0;i--) {
				dst[i*2]=src[i];
				dst[i*2+1]=src[i];
			}
		} else memcpy( (char*)(buffer_ana[buffer_ana_gen_ofs])+buffer_ana_subofs,(char*)data,len);
		buffer_ana_subofs+=len;
	} else {
		
		if (mSlowDevice) {
			signed int *dst=(signed int *)((char*)(buffer_ana[buffer_ana_gen_ofs])+buffer_ana_subofs);
			signed int *src=(signed int *)data;
			for (int i=to_fill/8-1;i>=0;i--) {
				dst[i*2]=src[i];
				dst[i*2+1]=src[i];
			}
		} else memcpy((char*)(buffer_ana[buffer_ana_gen_ofs])+buffer_ana_subofs,(char*)data,to_fill);
		
		len-=to_fill;
		buffer_ana_subofs=0;
		
		
		if (mNeedSeek==2) {
			mNeedSeek=3;
			buffer_ana_flag[buffer_ana_gen_ofs]=3;
 		} else buffer_ana_flag[buffer_ana_gen_ofs]=1;
		
		if (end_reached) {
			buffer_ana_flag[buffer_ana_gen_ofs]|=4;
		}
		
		if (mNeedSeek==1) { //ask for seeking
			mNeedSeek=2;  //taken into account
			//NSLog(@"sexy seek : %d",mNeedSeekTime);
			//sexy_seek(mNeedSeekTime);
			len=0;
		}
		
		buffer_ana_gen_ofs++;
		if (buffer_ana_gen_ofs==SOUND_BUFFER_NB) buffer_ana_gen_ofs=0;
		
		if (len>=SOUND_BUFFER_SIZE_SAMPLE*2*2) {
			NSLog(@"*****************\n*****************\n***************");
		} else if (len) {
			
			while (buffer_ana_flag[buffer_ana_gen_ofs]) {
				[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
				if (bGlobalShouldEnd||(!bGlobalIsPlaying)) {
					//mdx_stop();
					return;
				}
			}
			if (mSlowDevice) {				
				signed int *dst=(signed int *)buffer_ana[buffer_ana_gen_ofs];
				signed int *src=(signed int *)((char*)data+to_fill);
				for (int i=len/8-1;i>=0;i--) {
					dst[i*2]=src[i];
					dst[i*2+1]=src[i];
				}
			} else memcpy((char*)(buffer_ana[buffer_ana_gen_ofs]),((char*)data)+to_fill,len);
			buffer_ana_subofs=len;
		}
	}
}

int sexyd_updateseek(int progress) {  //called during seek. return 1 to stop play
	if (bGlobalShouldEnd||(!bGlobalIsPlaying)) return 1;
	bGlobalSeekProgress=progress;
	return 0;
}

void sexyd_update(unsigned char* pSound,long lBytes) {
	static int count_upd=0;
	count_upd+=lBytes;
	
    if (bGlobalShouldEnd||(!bGlobalIsPlaying)) {
		//sexy_stop();
		return;
	}
	
	while (buffer_ana_flag[buffer_ana_gen_ofs]) {
		[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
		if (bGlobalShouldEnd||(!bGlobalIsPlaying)) {
			//sexy_stop();
			return;
		}
	}
	
	int to_fill=SOUND_BUFFER_SIZE_SAMPLE*2*2-buffer_ana_subofs;
	
	if (lBytes<to_fill) {
		memcpy( (char*)(buffer_ana[buffer_ana_gen_ofs])+buffer_ana_subofs,(char*)pSound,lBytes);
		buffer_ana_subofs+=lBytes;
	} else {
		memcpy((char*)(buffer_ana[buffer_ana_gen_ofs])+buffer_ana_subofs,(char*)pSound,to_fill);
		lBytes-=to_fill;
		buffer_ana_subofs=0;
		
		
		if (mNeedSeek==2) {
			mNeedSeek=3;
			buffer_ana_flag[buffer_ana_gen_ofs]=3;
 		} else buffer_ana_flag[buffer_ana_gen_ofs]=1;
		
		if (mNeedSeek==1) { //ask for seeking
			mNeedSeek=2;  //taken into account
			//NSLog(@"sexy seek : %d",mNeedSeekTime);
			//sexy_seek(mNeedSeekTime);
			lBytes=0;
		}
		
		buffer_ana_gen_ofs++;
		if (buffer_ana_gen_ofs==SOUND_BUFFER_NB) buffer_ana_gen_ofs=0;
		
		if (lBytes>=SOUND_BUFFER_SIZE_SAMPLE*2*2) {
			//NSLog(@"*****************\n*****************\n***************");
		} else if (lBytes) {
			
			while (buffer_ana_flag[buffer_ana_gen_ofs]) {
				[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
				if (bGlobalShouldEnd||(!bGlobalIsPlaying)) {
					//sexy_stop();
					return;
				}
			}
			memcpy((char*)(buffer_ana[buffer_ana_gen_ofs]),((char*)pSound)+to_fill,lBytes);
			buffer_ana_subofs=lBytes;
		}
	}
}

// Handle main UADE thread (amiga emu)
-(void) uadeThread {
	//	printf("start thread\n");
	
	uadeThread_running=1;
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	int argc;
	char *argv[5];
	char *argv_buffer;
	
	if ([[NSThread currentThread] respondsToSelector:@selector(setThreadPriority)])	[[NSThread currentThread] setThreadPriority:0.9f];
	
	argc=5;
	argv_buffer=(char*)malloc(argc*32);
	for (int i=0;i<argc;i++) argv[i]=argv_buffer+32*i;
	strcpy(argv[0],"uadecore");
	strcpy(argv[1],"-i");
	strcpy(argv[2],"fd://0");
	strcpy(argv[3],"-o");
	strcpy(argv[4],"fd://0");
	//uade_main(argc,(char**)argv);
	free(argv_buffer);
	
	//remove our pool and free the memory collected by it
    [pool release];
	
	uadeThread_running=0;
	//	printf("stop thread\n");
}

//invoked by secondary UADE thread, in charge of receiving sound data 
int uade_audio_play(char *pSound,int lBytes,int song_end) {
	do {
		while (buffer_ana_flag[buffer_ana_gen_ofs]) {
			[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
			if (bGlobalShouldEnd||(!bGlobalIsPlaying)) {
				return 0;
			}
		}
		int to_fill=SOUND_BUFFER_SIZE_SAMPLE*2*2-buffer_ana_subofs;
		
		if (lBytes<to_fill) {
			memcpy( (char*)(buffer_ana[buffer_ana_gen_ofs])+buffer_ana_subofs,(char*)pSound,lBytes);
			buffer_ana_subofs+=lBytes;
			lBytes=0;
			if (song_end) {
				memset( (char*)(buffer_ana[buffer_ana_gen_ofs])+buffer_ana_subofs,0,SOUND_BUFFER_SIZE_SAMPLE*2*2-buffer_ana_subofs);
				buffer_ana_flag[buffer_ana_gen_ofs]=1|4;
				buffer_ana_gen_ofs++;
				if (buffer_ana_gen_ofs==SOUND_BUFFER_NB) buffer_ana_gen_ofs=0;
			}
		} else {
			memcpy((char*)(buffer_ana[buffer_ana_gen_ofs])+buffer_ana_subofs,(char*)pSound,to_fill);
			lBytes-=to_fill;
			pSound+=to_fill;
			buffer_ana_subofs=0;
			
			buffer_ana_flag[buffer_ana_gen_ofs]=1;
			if (song_end) {
				buffer_ana_flag[buffer_ana_gen_ofs]=1|4;
			}
			
			buffer_ana_gen_ofs++;
			if (buffer_ana_gen_ofs==SOUND_BUFFER_NB) buffer_ana_gen_ofs=0;
		}
	} while (lBytes);
	
	
	return 0;
}

/*
-(void) optSEXYPSF:(int)reverb interpol:(int)interpol {
	sexyPSF_setReverb(reverb);
	sexyPSF_setInterpolation(interpol);
}
-(void) optAOSDK:(int)reverb interpol:(int)interpol {
	AOSDK_setReverb(reverb);
	AOSDK_setInterpolation(interpol);
}
*/

-(void) optGME_DefaultLength:(float_t)val {
	optGMEDefaultLength=(int)(val*1000);
}

-(void) optUADE_Led:(int)isOn {
	if (isOn!=mUADE_OptLED)	{
		mUADE_OptChange=1;
		mUADE_OptLED=isOn;
	}
}
/*
-(void) optUADE_Norm:(int)isOn {
	mUADE_OptNORM=isOn;
	if (isOn) uade_effect_enable(&(UADEstate.effects), UADE_EFFECT_NORMALISE);
	else uade_effect_disable(&(UADEstate.effects), UADE_EFFECT_NORMALISE);
}
-(void) optUADE_PostFX:(int)isOn {
	mUADE_OptPOSTFX=isOn;
	if (isOn) uade_effect_enable(&(UADEstate.effects), UADE_EFFECT_ALLOW);
	else uade_effect_disable(&(UADEstate.effects), UADE_EFFECT_ALLOW);
}
-(void) optUADE_Pan:(int)isOn {
	mUADE_OptPAN=isOn;
	if (isOn) uade_effect_enable(&(UADEstate.effects), UADE_EFFECT_PAN);
	else uade_effect_disable(&(UADEstate.effects), UADE_EFFECT_PAN);
}
-(void) optUADE_Head:(int)isOn {
	mUADE_OptHEAD=isOn;
	if (isOn) uade_effect_enable(&(UADEstate.effects), UADE_EFFECT_HEADPHONES);
	else uade_effect_disable(&(UADEstate.effects), UADE_EFFECT_HEADPHONES);
}
-(void) optUADE_Gain:(int)isOn {
	mUADE_OptGAIN=isOn;
	if (isOn) uade_effect_enable(&(UADEstate.effects), UADE_EFFECT_GAIN);
	else uade_effect_disable(&(UADEstate.effects), UADE_EFFECT_GAIN);
}
-(void) optUADE_GainValue:(float)val {
	mUADE_OptGAINValue=val;
	uade_effect_gain_set_amount(&(UADEstate.effects), val);
}
-(void) optUADE_PanValue:(float)val {
	mUADE_OptPANValue=val;
	uade_effect_pan_set_amount(&(UADEstate.effects), val);
}
-(int) uade_playloop {	
	struct uade_state *state=&UADEstate;
	uint16_t *sm;
	int i;
	int skip_first;
	uint32_t *u32ptr;
	char playerName[128];
	char formatName[128];
	char moduleName[128];
	
	uint8_t space[UADE_MAX_MESSAGE_SIZE];
	struct uade_msg *um = (struct uade_msg *) space;
	
	uint8_t sampledata[UADE_MAX_MESSAGE_SIZE];
	int left = 0;
	int what_was_left = 0;
	
	int subsong_end = 0;
	int next_song = 0;
	//	int ret;
	int tailbytes = 0;
	int playbytes;
	char *reason;
	
	int64_t subsong_bytes = 0;
	
	const int framesize = UADE_BYTES_PER_SAMPLE * UADE_CHANNELS;
	const int bytes_per_second = UADE_BYTES_PER_FRAME * state->config.frequency;
	
	enum uade_control_state controlstate = UADE_S_STATE;
	
	struct uade_ipc *ipc = &state->ipc;
	struct uade_song *us = state->song;
	struct uade_effect *ue = &state->effects;
	struct uade_config *uc = &state->config;
	
	uade_effect_reset_internals();
	
	uade_song_end_trigger=0;
	playerName[0]=0;
	formatName[0]=0;
	strcpy(moduleName,mod_name);
	skip_first=0;
	
	while (next_song == 0) {
		
		if (controlstate == UADE_S_STATE) {
			//handle option
			if (mUADE_OptChange) {
				switch (mUADE_OptChange) {
					case 1:
						uade_set_config_option(uc, UC_FORCE_LED, uc->led_state ? "off" : "on");
						//tprintf("\nForcing LED %s\n", (uc->led_state & 1) ? "ON" : "OFF");
						uade_send_filter_command(state);
						break;
					case 2:
						uade_effect_toggle(ue, UADE_EFFECT_NORMALISE);
						//tprintf("\nNormalise effect %s\n", uade_effect_is_enabled(ue, UADE_EFFECT_NORMALISE) ? "ON" : "OFF");
						break;
					case 3:
						uade_effect_toggle(ue, UADE_EFFECT_ALLOW);
						//tprintf("\nPostprocessing effects %s\n", uade_effect_is_enabled(ue, UADE_EFFECT_ALLOW) ? "ON" : "OFF");
						break;
					case 4:
						uade_effect_toggle(ue, UADE_EFFECT_PAN);
						//tprintf("\nPanning effect %s %s\n", uade_effect_is_enabled(ue, UADE_EFFECT_PAN) ? "ON" : "OFF", (uade_effect_is_enabled(ue, UADE_EFFECT_ALLOW) == 0 && uade_effect_is_enabled(ue, UADE_EFFECT_PAN) == 1) ? "(Remember to turn ON postprocessing!)" : "");
						break;
					case 5:
						uade_effect_toggle(ue, UADE_EFFECT_HEADPHONES);
						//tprintf("\nHeadphones effect %s %s\n", uade_effect_is_enabled(ue, UADE_EFFECT_HEADPHONES) ? "ON" : "OFF", (uade_effect_is_enabled(ue, UADE_EFFECT_ALLOW) == 0 && uade_effect_is_enabled(ue, UADE_EFFECT_HEADPHONES) == 1) ? "(Remember to turn ON postprocessing!)" : "");
						break;
					case 6:
						uade_effect_toggle(ue, UADE_EFFECT_GAIN);
						//tprintf("\nGain effect %s %s\n", uade_effect_is_enabled(ue, UADE_EFFECT_GAIN) ? "ON" : "OFF", (uade_effect_is_enabled(ue, UADE_EFFECT_ALLOW) == 0 && uade_effect_is_enabled(ue, UADE_EFFECT_GAIN)) ? "(Remember to turn ON postprocessing!)" : "");
						break;
				}
				mUADE_OptChange=0;
			}
			
			
			
			if (subsong_end && uade_song_end_trigger == 0) {
				if (uc->one_subsong == 0 && us->cur_subsong != -1 && us->max_subsong != -1) {
					if (moveToPrevSubSong) {
						if (us->cur_subsong>us->min_subsong) us->cur_subsong--;
					} else us->cur_subsong++;
					
					if (us->cur_subsong > us->max_subsong) uade_song_end_trigger = 1;
					else {
						subsong_end = 0;
						subsong_bytes = 0;
						uade_change_subsong(state);
						mod_currentsub=us->cur_subsong;
						iCurrentTime=0;
						iModuleLength=[self getSongLengthfromMD5:mod_currentsub-mod_minsub+1];
						//Loop
						if (mLoopMode==1) iModuleLength=-1;
						mod_message_updated=1;
						
						if(moveToPrevSubSong||moveToNextSubSong) {
							mod_wantedcurrentsub=-1;
							what_was_left=0;
							tailbytes=0;
							if (moveToNextSubSong==2) {
								//[self iPhoneDrv_PlayWaitStop];
								//[self iPhoneDrv_PlayStart];
							} else {
								[self iPhoneDrv_PlayStop];
								[self iPhoneDrv_PlayStart];
							}
							skip_first=1;
						}
					}
					
					
					moveToPrevSubSong=0;
					moveToNextSubSong=0;
				} else {
					uade_song_end_trigger = 1;
				}
			}
			
			if ((us->cur_subsong != -1)&&(mod_wantedcurrentsub!=-1)&&(mod_wantedcurrentsub!=us->cur_subsong)) {
				subsong_end = 0;
				subsong_bytes = 0;
				us->cur_subsong=mod_wantedcurrentsub;
				mod_wantedcurrentsub=-1;
				if (us->cur_subsong>us->max_subsong) us->cur_subsong=us->max_subsong;
				if (us->cur_subsong<us->min_subsong) us->cur_subsong=us->min_subsong;
				mod_currentsub=us->cur_subsong;				
				iModuleLength=[self getSongLengthfromMD5:mod_currentsub-mod_minsub+1];
				//Loop
				if (mLoopMode==1) iModuleLength=-1;
				mod_message_updated=1;
				uade_change_subsong(state);
				
			}
			
			if (uade_song_end_trigger) {
				next_song=1;
				//				printf("now exiting\n");
				if (uade_send_short_message(UADE_EXIT, ipc)) {
					printf("\nCan not send exit\n");
				}
				goto sendtoken;
			}
			
			left = uade_read_request(ipc);
			
		sendtoken:
			if (uade_send_short_message(UADE_COMMAND_TOKEN, ipc)) {
				printf("\nCan not send token\n");
				return 0;
			}
			
			controlstate = UADE_R_STATE;
			
			if (what_was_left||(subsong_end&&tailbytes)) {
				if (subsong_end) {
				    //We can only rely on 'tailbytes' amount which was determined
					//earlier when UADE_REPLY_SONG_END happened
					playbytes = tailbytes;
					tailbytes = 0;
				} else {
					playbytes = what_was_left;
				}
				
				if (playbytes) {
					
					us->out_bytes += playbytes;
					subsong_bytes += playbytes;
					
					uade_effect_run(ue, (int16_t *) sampledata, playbytes / framesize);
					
					if (skip_first) skip_first=0;
					else {
						if (uade_audio_play((char*)sampledata, playbytes,subsong_end)) uade_song_end_trigger=2;
					}
				}	
				
			    //FIX ME
				if (uc->timeout != -1 && uc->use_timeouts) {
					if (uade_song_end_trigger == 0) {
						if (us->out_bytes / bytes_per_second >= uc->timeout) {
							printf("\nSong end (timeout %ds)\n", uc->timeout);
							uade_song_end_trigger = 1;
						}
					}
				}
				
				if (uc->subsong_timeout != -1 && uc->use_timeouts) {
					if (subsong_end == 0 && uade_song_end_trigger == 0) {
						if (subsong_bytes / bytes_per_second >= uc->subsong_timeout) {
							printf("\nSong end (subsong timeout %ds)\n", uc->subsong_timeout);
							subsong_end = 1;
						}
					}
				}
			}
			
		} else {
			if (moveToNextSubSong) {
				subsong_end=1;
			}
			if (moveToPrevSubSong) {
				subsong_end=1;
			}
			
			if (bGlobalShouldEnd||(!bGlobalIsPlaying)) {
				subsong_end=1;
			}
			
		//receive state
			if (uade_receive_message(um, sizeof(space), ipc) <= 0) {
				printf("\nCan not receive events from uade\n");
				return 0;
			}
			
			switch (um->msgtype) {
					
				case UADE_COMMAND_TOKEN:
					controlstate = UADE_S_STATE;
					break;
					
				case UADE_REPLY_DATA:
					sm = (uint16_t *) um->data;

					
					assert (left == um->size);
					assert (sizeof sampledata >= um->size);
					
					memcpy(sampledata, um->data, um->size);
					
					what_was_left = left;
					
					left = 0;
					break;
					
				case UADE_REPLY_FORMATNAME:
					uade_check_fix_string(um, 128);
					strcpy(formatName,(const char*)(um->data));
					if (1 + us->max_subsong - us->min_subsong==1) {
						sprintf(mod_message,"%s\n%s\n%s\nSubsong: 1",moduleName,formatName,playerName);
					}
					else {
						mod_minsub=us->min_subsong;
						mod_maxsub=us->max_subsong;
						mod_currentsub=us->cur_subsong;
						sprintf(mod_message,"%s\n%s\n%s\nSubsongs: %d",moduleName,formatName,playerName,1 + us->max_subsong - us->min_subsong);
						mod_subsongs=1 + us->max_subsong - us->min_subsong;
					}
					mod_message_updated=2;
					//printf("\nFormat name: %s\n", (uint8_t *) um->data);
					break;
					
				case UADE_REPLY_MODULENAME:
					uade_check_fix_string(um, 128);
					strcpy(moduleName,(const char*)(um->data));
					if (1 + us->max_subsong - us->min_subsong==1) {
						sprintf(mod_message,"%s\n%s\n%s\nSubsong: 1",moduleName,formatName,playerName);
					}
					else {
						mod_minsub=us->min_subsong;
						mod_maxsub=us->max_subsong;
						mod_currentsub=us->cur_subsong;
						sprintf(mod_message,"%s\n%s\n%s\nSubsongs: %d",moduleName,formatName,playerName,1 + us->max_subsong - us->min_subsong);
						mod_subsongs=1 + us->max_subsong - us->min_subsong;
					}
					mod_message_updated=2;
					//printf("\nModule name: %s\n", (uint8_t *) um->data);
					break;
					
				case UADE_REPLY_MSG:
					uade_check_fix_string(um, 512);
					//printf("UADE msg : %s\n",(const char*)(um->data));
					break;
					
				case UADE_REPLY_PLAYERNAME:					
					uade_check_fix_string(um, 128);
					strcpy(playerName,(const char*)(um->data));
					if (1 + us->max_subsong - us->min_subsong==1) {
						sprintf(mod_message,"%s\n%s\n%s\nSubsong: 1",moduleName,formatName,playerName);
					}
					else {
						mod_minsub=us->min_subsong;
						mod_maxsub=us->max_subsong;
						mod_currentsub=us->cur_subsong;
						sprintf(mod_message,"%s\n%s\n%s\nSubsongs: %d",moduleName,formatName,playerName,1 + us->max_subsong - us->min_subsong);
						mod_subsongs=1 + us->max_subsong - us->min_subsong;
					}
					mod_message_updated=2;
					//printf("\nPlayer name: %s\n", (uint8_t *) um->data);
					break;
					
				case UADE_REPLY_SONG_END:
					if (um->size < 9) {
						printf("\nInvalid song end reply\n");
						exit(-1);
					}
					tailbytes = ntohl(((uint32_t *) um->data)[0]);
					if (!tailbytes) 
						if (!what_was_left) what_was_left=2;
					if (ntohl(((uint32_t *) um->data)[1]) == 0) {
						subsong_end = 1;
						moveToNextSubSong=2;
						
						//update song length
						[self setSongLengthfromMD5:mod_currentsub-mod_minsub+1 songlength:iCurrentTime];
						//printf("received happy song end %d\n",mod_wantedcurrentsub);
					} else {

						uade_song_end_trigger = 1;
						
						//subsong_end = 1;
						//moveToNextSubSong=2;
						
						[self setSongLengthfromMD5:mod_currentsub-mod_minsub+1 songlength:iCurrentTime];
						//printf("received unhappy song end %d\n",mod_wantedcurrentsub);
					}
					i = 0;
					reason = (char *) &um->data[8];
					while (reason[i] && i < (um->size - 8))
						i++;
					if (reason[i] != 0 || (i != (um->size - 9))) {
						printf("\nbroken reason string with song end notice\n");
						exit(-1);
					}
					//					printf("\nSong end (%s)\n", reason);
					break;
					
				case UADE_REPLY_SUBSONG_INFO:
					if (um->size != 12) {
						printf("\nsubsong info: too short a message\n");
						exit(-1);
					}
					
					u32ptr = (uint32_t *) um->data;
					us->min_subsong = ntohl(u32ptr[0]);
					us->max_subsong = ntohl(u32ptr[1]);
					us->cur_subsong = ntohl(u32ptr[2]);
					
					//printf("\nsubsong: %d from range [%d, %d]\n", us->cur_subsong, us->min_subsong, us->max_subsong);
					
					if (!(-1 <= us->min_subsong && us->min_subsong <= us->cur_subsong && us->cur_subsong <= us->max_subsong)) {
						int tempmin = us->min_subsong, tempmax = us->max_subsong;
						fprintf(stderr, "\nThe player is broken. Subsong info does not match.\n");
						us->min_subsong = tempmin <= tempmax ? tempmin : tempmax;
						us->max_subsong = tempmax >= tempmin ? tempmax : tempmin;
						if (us->cur_subsong > us->max_subsong)
							us->max_subsong = us->cur_subsong;
						else if (us->cur_subsong < us->min_subsong)
							us->min_subsong = us->cur_subsong;
					}
					
					if ((us->max_subsong - us->min_subsong) != 0) {
						//	printf("\nThere are %d subsongs in range [%d, %d].\n", 1 + us->max_subsong - us->min_subsong, us->min_subsong, us->max_subsong);
						if (1 + us->max_subsong - us->min_subsong==1) {
							sprintf(mod_message,"%s\n%s\n%s\nSubsong: 1",moduleName,formatName,playerName);
						}
						else {
							mod_minsub=us->min_subsong;
							mod_maxsub=us->max_subsong;
							mod_currentsub=us->cur_subsong;
							sprintf(mod_message,"%s\n%s\n%s\nSubsongs: %d",moduleName,formatName,playerName,1 + us->max_subsong - us->min_subsong);
							mod_subsongs=1 + us->max_subsong - us->min_subsong;
						}
						mod_message_updated=2;
					}
					
					//NSLog(@"playtime : %d",us->playtime);
					break;
					
				default:
					printf("\nExpected sound data. got %u.\n", (unsigned int) um->msgtype);
					return 0;
			}
		}
	}

	return 0;
}
 */

-(void) generateSoundThread {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	if ([[NSThread currentThread] respondsToSelector:@selector(setThreadPriority)]) [[NSThread currentThread] setThreadPriority:0.9f];
	while (1) {
		[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
		if (bGlobalIsPlaying) {
			bGlobalSoundGenInProgress=1;
			
			if ( !bGlobalEndReached && mPlayType) {
				int nbBytes=0;				
				
				if (mPlayType==5) {  //Special case : SexyPSF
					int counter=0;
					//sexy_execute();
					AudioQueueStop( mAudioQueue, FALSE );
					while ([self isEndReached]==NO) {
						[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
						counter++;
						if (counter*DEFAULT_WAIT_TIME_MS>2) break;
					}
					AudioQueueStop( mAudioQueue, TRUE );
					AudioQueueReset( mAudioQueue );	
					mQueueIsBeingStopped = FALSE;
					bGlobalEndReached=1;
					bGlobalAudioPause=2;
					
					
					//bGlobalEndReached=1;
					//bGlobalAudioPause=2;
					//[self iPhoneDrv_PlayWaitStop];
					//AudioQueueStop( mAudioQueue, TRUE );
					
				} else if (mPlayType==6) {  //Special case : UADE
					int counter=0;
					[self uade_playloop];
					
					//NSLog(@"Waiting for end");
					while ([self isEndReached]==NO) {
						[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
						counter++;
						if (counter*DEFAULT_WAIT_TIME_MS>2) break;
					}
					AudioQueueStop( mAudioQueue, TRUE );
					AudioQueueReset( mAudioQueue );	
					mQueueIsBeingStopped = FALSE;
					bGlobalEndReached=1;
					bGlobalAudioPause=2;		
					//[self iPhoneDrv_PlayWaitStop];
					//AudioQueueStop( mAudioQueue, TRUE );
				} else if (mPlayType==11) {  //Special case : MDX
					int counter=0;
					//mdx_play(mdx,pdx);
					
					//[self iPhoneDrv_PlayWaitStop];
					while ([self isEndReached]==NO) {
						[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
						counter++;
						if (counter*DEFAULT_WAIT_TIME_MS>2) break;
					}
					AudioQueueStop( mAudioQueue, TRUE );
					AudioQueueReset( mAudioQueue );	
					mQueueIsBeingStopped = FALSE;
					bGlobalAudioPause=2;
					bGlobalEndReached=1;
				} else if (buffer_ana_flag[buffer_ana_gen_ofs]==0) {
					if (mNeedSeek==1) { //SEEK
						mNeedSeek=2;  //taken into account
						if (mPlayType==1) {   //GME
							bGlobalSeekProgress=-1;
							//gme_seek(gme_emu,mNeedSeekTime);
							//gme_set_fade( gme_emu, iModuleLength-optGMEFadeOut ); //Fade 2s before end
						}
						if (mPlayType==2) { //MODPLUG
							bGlobalSeekProgress=-1;
							ModPlug_Seek(mp_file,mNeedSeekTime);
						}
						if (mPlayType==3) { //ADPLUG
							bGlobalSeekProgress=-1;
							//adPlugPlayer->seek(mNeedSeekTime);
						}
						if (mPlayType==4) { //AOSDK : not supported
							mNeedSeek=0;
						}
						if (mPlayType==7) { //HVL
							bGlobalSeekProgress=-1;
							//mNeedSeekTime=hvl_Seek(hvl_song,mNeedSeekTime);
						}
						if (mPlayType==8) { //SID : not supported
							mNeedSeek=0;
						}
						if (mPlayType==9) {//STSOUND
							//if (ymMusicIsSeekable(ymMusic)==YMTRUE) {
							//	bGlobalSeekProgress=-1;
							//	ymMusicSeek(ymMusic,mNeedSeekTime);
							//} else mNeedSeek=0;
						}
						if (mPlayType==10) {//SC68
							bGlobalSeekProgress=-1;
							//api68_seek(sc68, mNeedSeekTime, NULL);
						}
					}
					if (moveToNextSubSong) {
						if (mod_currentsub<mod_maxsub) {
							mod_currentsub++;
							mod_message_updated=1;
							if (mPlayType==1) {//GME
								/*
								gme_start_track(gme_emu,mod_currentsub);
								
								if (gme_track_info( gme_emu, &gme_info, mod_currentsub )==0) {
									strcpy(modtype,gme_info->system);
									iModuleLength=gme_info->play_length;
									if (iModuleLength<=0) iModuleLength=optGMEDefaultLength;
									
									sprintf(mod_message,"Song:%s\nGame:%s\nAuthor:%s\nDumper:%s\nTracks:%d\n%s",
											(gme_info->song?gme_info->song:" "),
											(gme_info->game?gme_info->game:" "),
											(gme_info->author?gme_info->author:" "),
											(gme_info->dumper?gme_info->dumper:" "),
											gme_track_count( gme_emu ),
											(gme_info->comment?gme_info->comment:" "));
									gme_free_info(gme_info);
								} else {
									strcpy(modtype,"N/A");
									strcpy(mod_message,"N/A\n");
									iModuleLength=optGMEDefaultLength;
								}
								//LOOP
								if (mLoopMode==1) iModuleLength=-1;
								
								mod_message_updated=2;
								
								if (iModuleLength>0) {
									if (iModuleLength>optGMEFadeOut) gme_set_fade( gme_emu, iModuleLength-optGMEFadeOut ); //Fade 1s before end
									else gme_set_fade( gme_emu, iModuleLength/2 ); 
								}
								if (moveToNextSubSong==2) {
									//[self iPhoneDrv_PlayWaitStop];
									//[self iPhoneDrv_PlayStart];
								} else {
									[self iPhoneDrv_PlayStop];
									[self iPhoneDrv_PlayStart];
								}
								iCurrentTime=0;
								*/
							}
							if (mPlayType==8) { //SID
								/*
								if (mSidEngineType==1) {
									sidEmuInitializeSong(*mSid1EmuEngine,*mSid1Tune, mod_currentsub);
								} else {
									mSidTune->selectSong(mod_currentsub);
									mSidEmuEngine->load(mSidTune);
								}
								
								if (moveToNextSubSong==2) {
									//[self iPhoneDrv_PlayWaitStop];
									//[self iPhoneDrv_PlayStart];
								} else {
									[self iPhoneDrv_PlayStop];
									[self iPhoneDrv_PlayStart];
								}
								iCurrentTime=0;
								iModuleLength=[self getSongLengthfromMD5:mod_currentsub-mod_minsub+1];
								if (iModuleLength<=0) iModuleLength=SID_DEFAULT_LENGTH;
								if (mLoopMode) iModuleLength=-1;
								mod_message_updated=1;
								*/
							}
							if (mPlayType==10) {//SC68		
								/*
								api68_music_info_t info;
								if (moveToNextSubSong==2) {
									//[self iPhoneDrv_PlayWaitStop];
									//[self iPhoneDrv_PlayStart];
								} else {
									[self iPhoneDrv_PlayStop];
									[self iPhoneDrv_PlayStart];
								}
								api68_play(sc68,mod_currentsub,1);
								api68_music_info( sc68, &info, mod_currentsub, NULL );
								iModuleLength=info.time_ms;
								if (iModuleLength<=0) iModuleLength=SC68_DEFAULT_LENGTH;
								if (mLoopMode) iModuleLength=-1;
								//NSLog(@"track : %d, time : %d, start : %d",mod_currentsub,info.time_ms,info.start_ms);
								iCurrentTime=0;
								mod_message_updated=1;
								 */
							}
						}
						moveToNextSubSong=0;
					}
					if (moveToPrevSubSong) {
						moveToPrevSubSong=0;
						if (mod_currentsub>mod_minsub) {
							mod_currentsub--;
							mod_message_updated=1;
							if (mPlayType==1) {//GME
								/*
								gme_start_track(gme_emu,mod_currentsub);
								
								if (gme_track_info( gme_emu, &gme_info, mod_currentsub )==0) {
									iModuleLength=gme_info->play_length;
									if (iModuleLength<=0) iModuleLength=optGMEDefaultLength;
									strcpy(modtype,gme_info->system);
									sprintf(mod_message,"Song:%s\nGame:%s\nAuthor:%s\nDumper:%s\nTracks:%d\n%s",
											(gme_info->song?gme_info->song:" "),
											(gme_info->game?gme_info->game:" "),
											(gme_info->author?gme_info->author:" "),
											(gme_info->dumper?gme_info->dumper:" "),
											gme_track_count( gme_emu ),
											(gme_info->comment?gme_info->comment:" "));
									gme_free_info(gme_info);
								
								} else {
									strcpy(modtype,"N/A");
									strcpy(mod_message,"N/A\n");
									iModuleLength=optGMEDefaultLength;
								}
								//LOOP
								if (mLoopMode==1) iModuleLength=-1;
								
								mod_message_updated=2;
								
								if (iModuleLength>0) {
									if (iModuleLength>optGMEFadeOut) gme_set_fade( gme_emu, iModuleLength-optGMEFadeOut ); //Fade 1s before end
									else gme_set_fade( gme_emu, iModuleLength/2 ); //Fade 1s before end
								}
								[self iPhoneDrv_PlayStop];
								[self iPhoneDrv_PlayStart];
								iCurrentTime=0;
								*/
							}
							if (mPlayType==8) { //SID
								/*
								if (mSidEngineType==1) {
									sidEmuInitializeSong(*mSid1EmuEngine,*mSid1Tune, mod_currentsub);
								} else {
									mSidTune->selectSong(mod_currentsub);
									mSidEmuEngine->load(mSidTune);
								}
								
								[self iPhoneDrv_PlayStop];
								[self iPhoneDrv_PlayStart];
								iCurrentTime=0;
								iModuleLength=[self getSongLengthfromMD5:mod_currentsub-mod_minsub+1];
								if (iModuleLength<=0) iModuleLength=SID_DEFAULT_LENGTH;
								if (mLoopMode) iModuleLength=-1;
								mod_message_updated=1;
								 */
							}
							if (mPlayType==10) {//SC68		
								/*
								api68_music_info_t info;
								[self iPhoneDrv_PlayStop];
								[self iPhoneDrv_PlayStart];
								api68_play(sc68,mod_currentsub,1);
								api68_music_info( sc68, &info, mod_currentsub, NULL );
								iModuleLength=info.time_ms;
								if (iModuleLength<=0) iModuleLength=SC68_DEFAULT_LENGTH;
								if (mLoopMode) iModuleLength=-1;
								//NSLog(@"track : %d, time : %d, start : %d",mod_currentsub,info.time_ms,info.start_ms);
								iCurrentTime=0;
								mod_message_updated=1;
								 */
							}
						}
					}
					
					if (mPlayType==1) {  //GME
						/*
						if (gme_track_ended(gme_emu)) {
							//NSLog(@"Track ended : %d",iCurrentTime);
							if (mLoopMode==1) {
								gme_start_track(gme_emu,mod_currentsub);
								
								
								if (mSlowDevice) {
									gme_play( gme_emu, SOUND_BUFFER_SIZE_SAMPLE, buffer_ana[buffer_ana_gen_ofs] );

									signed int *dst=(signed int *)buffer_ana[buffer_ana_gen_ofs];
									signed int *src=(signed int *)buffer_ana[buffer_ana_gen_ofs];
									for (int i=SOUND_BUFFER_SIZE_SAMPLE/2-1;i>=0;i--) {
										dst[i*2]=src[i];
										dst[i*2+1]=src[i];
									}
									
									
								} else {
									gme_play( gme_emu, SOUND_BUFFER_SIZE_SAMPLE*2, buffer_ana[buffer_ana_gen_ofs] );
								}
								
								nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
							} else if (mChangeOfSong==0) {
								if (mod_currentsub<mod_maxsub) {
									//NSLog(@"time : %d song:%d",iCurrentTime,mod_currentsub);
									mod_currentsub++;
									gme_start_track(gme_emu,mod_currentsub);
									
									if (gme_track_info( gme_emu, &gme_info, mod_currentsub )==0) {
										mChangeOfSong=1;
										mNewModuleLength=gme_info->play_length;
										if (mNewModuleLength<=0) mNewModuleLength=optGMEDefaultLength;
										strcpy(modtype,gme_info->system);
										
										sprintf(mod_message,"Song:%s\nGame:%s\nAuthor:%s\nDumper:%s\nTracks:%d\n%s",
												(gme_info->song?gme_info->song:" "),
												(gme_info->game?gme_info->game:" "),
												(gme_info->author?gme_info->author:" "),
												(gme_info->dumper?gme_info->dumper:" "),
												gme_track_count( gme_emu ),
												(gme_info->comment?gme_info->comment:" "));
										
										gme_free_info(gme_info);
									} else {
										strcpy(modtype,"N/A");
										strcpy(mod_message,"N/A\n");
										mNewModuleLength=optGMEDefaultLength;
									}
									//mod_message_updated=2;
									
									if (mNewModuleLength>0) {
										if (mNewModuleLength>optGMEFadeOut) gme_set_fade( gme_emu, mNewModuleLength-optGMEFadeOut ); //Fade 1s before end
										else gme_set_fade( gme_emu, mNewModuleLength/2 ); //Fade 1s before end
									}
									if (mSlowDevice) {
										gme_play( gme_emu, SOUND_BUFFER_SIZE_SAMPLE, buffer_ana[buffer_ana_gen_ofs] );

										signed int *dst=(signed int *)buffer_ana[buffer_ana_gen_ofs];
										signed int *src=(signed int *)buffer_ana[buffer_ana_gen_ofs];
										for (int i=SOUND_BUFFER_SIZE_SAMPLE/2-1;i>=0;i--) {
											dst[i*2]=src[i];
											dst[i*2+1]=src[i];
										}
										
									} else {
										gme_play( gme_emu, SOUND_BUFFER_SIZE_SAMPLE*2, buffer_ana[buffer_ana_gen_ofs] );
									}
									
									nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
									
									//iCurrentTime=0;
									
								} else nbBytes=0;
							}
						}
						else {
							if (mSlowDevice) {
								gme_play( gme_emu, SOUND_BUFFER_SIZE_SAMPLE, buffer_ana[buffer_ana_gen_ofs] );

								signed int *dst=(signed int *)buffer_ana[buffer_ana_gen_ofs];
								signed int *src=(signed int *)buffer_ana[buffer_ana_gen_ofs];
								for (int i=SOUND_BUFFER_SIZE_SAMPLE/2-1;i>=0;i--) {
									dst[i*2]=src[i];
									dst[i*2+1]=src[i];
								}
								
							} else {
								gme_play( gme_emu, SOUND_BUFFER_SIZE_SAMPLE*2, buffer_ana[buffer_ana_gen_ofs] );
							}
							nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
						}
						 */
					}
						 
					if (mPlayType==4) { //AOSDK
						/*
						if ((*ao_types[ao_type].gen)((int16*)(buffer_ana[buffer_ana_gen_ofs]), SOUND_BUFFER_SIZE_SAMPLE)==AO_FAIL) {
							nbBytes=0;

						} else {
							if ((iModuleLength>-1)&&(iCurrentTime<iModuleLength)) nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
							else nbBytes=0;
						}
						 */
					}
					if (mPlayType==2) {  //MODPLUG
						genPattern[buffer_ana_gen_ofs]=ModPlug_GetCurrentPattern(mp_file);
						genRow[buffer_ana_gen_ofs]=ModPlug_GetCurrentRow(mp_file);
						nbBytes = ModPlug_Read(mp_file,buffer_ana[buffer_ana_gen_ofs],SOUND_BUFFER_SIZE_SAMPLE*2*2);
					}
					if (mPlayType==3) {  //ADPLUG
						/*
						if (opl_towrite) {
							int written=0;
							nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
							if (opl_towrite>=SOUND_BUFFER_SIZE_SAMPLE) {
								written=SOUND_BUFFER_SIZE_SAMPLE;
								opl->update((short int *)(buffer_ana[buffer_ana_gen_ofs]),SOUND_BUFFER_SIZE_SAMPLE);
								opl_towrite-=(SOUND_BUFFER_SIZE_SAMPLE);
							} else {
								written=opl_towrite;
								opl->update((short int *)(buffer_ana[buffer_ana_gen_ofs]),opl_towrite);
								opl_towrite=0;
							}
							if (!opl_towrite) {
								if (adPlugPlayer->update()) opl_towrite=(int)(PLAYBACK_FREQ*1.0f/adPlugPlayer->getrefresh());
								else {
									if (mLoopMode==1) {
										opl_towrite=(int)(PLAYBACK_FREQ*1.0f/adPlugPlayer->getrefresh());
										adPlugPlayer->seek(0);
									} else opl_towrite=0;
								}
								if ((written<SOUND_BUFFER_SIZE_SAMPLE)&&opl_towrite) {
									short int *dest=(short int *)(buffer_ana[buffer_ana_gen_ofs]);
									
									while ((written<SOUND_BUFFER_SIZE_SAMPLE)&&opl_towrite) {
										if (opl_towrite>(SOUND_BUFFER_SIZE_SAMPLE-written)) {
											opl->update(&dest[written*2],SOUND_BUFFER_SIZE_SAMPLE-written);
											opl_towrite-=SOUND_BUFFER_SIZE_SAMPLE-written;
											written=SOUND_BUFFER_SIZE_SAMPLE;
										} else {
											opl->update(&dest[written*2],opl_towrite);
											written+=opl_towrite;
											if (adPlugPlayer->update()) opl_towrite=(int)(PLAYBACK_FREQ*1.0f/adPlugPlayer->getrefresh());
											else {
												if (mLoopMode==1) {
													opl_towrite=(int)(PLAYBACK_FREQ*1.0f/adPlugPlayer->getrefresh());
													adPlugPlayer->seek(0);
												} else opl_towrite=0;
											}
										}
									}
								}
							}
						} else nbBytes=0;
						 */
					}
					if (mPlayType==7) {  //HVL
						/*
						if (hvl_sample_to_write) {
							int written=0;
							nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
							if (hvl_sample_to_write>=SOUND_BUFFER_SIZE_SAMPLE) {
								written=SOUND_BUFFER_SIZE_SAMPLE;
								hvl_mixchunk(hvl_song,SOUND_BUFFER_SIZE_SAMPLE,(int8*)(buffer_ana[buffer_ana_gen_ofs]),(int8*)(buffer_ana[buffer_ana_gen_ofs])+2,4);
								
								hvl_sample_to_write-=(SOUND_BUFFER_SIZE_SAMPLE);
							} else {
								written=hvl_sample_to_write;
								hvl_mixchunk(hvl_song,hvl_sample_to_write,(int8*)(buffer_ana[buffer_ana_gen_ofs]),(int8*)(buffer_ana[buffer_ana_gen_ofs])+2,4);
								
								hvl_sample_to_write=0;
							}
							if (!hvl_sample_to_write) { 
								hvl_play_irq(hvl_song); 
								if (hvl_song->ht_SongEndReached) {//end reached
									nbBytes=0;
								} else {
									hvl_sample_to_write=hvl_song->ht_Frequency/50/hvl_song->ht_SpeedMultiplier;
									
									if ((written<SOUND_BUFFER_SIZE_SAMPLE)&&hvl_sample_to_write) {
										int8 *dest=(int8 *)(buffer_ana[buffer_ana_gen_ofs]);
										
										while ((written<SOUND_BUFFER_SIZE_SAMPLE)&&hvl_sample_to_write) {
											if (hvl_sample_to_write>(SOUND_BUFFER_SIZE_SAMPLE-written)) {
												hvl_mixchunk(hvl_song,SOUND_BUFFER_SIZE_SAMPLE-written,&dest[written*4],&dest[written*4+2],4);
												hvl_sample_to_write-=SOUND_BUFFER_SIZE_SAMPLE-written;
												written=SOUND_BUFFER_SIZE_SAMPLE;
											} else {
												hvl_mixchunk(hvl_song,hvl_sample_to_write,&dest[written*4],&dest[written*4+2],4);
												written+=hvl_sample_to_write;
												hvl_play_irq(hvl_song);  //Check end ?
												if (hvl_song->ht_SongEndReached) {//end reached
													nbBytes=0;
													break;
												}
												hvl_sample_to_write=hvl_song->ht_Frequency/50/hvl_song->ht_SpeedMultiplier;
											}
										}
									}
								}
							}
						} else nbBytes=0;
						*/
					}
					if (mPlayType==8) { //SID
						/*
						if (mSidEngineType==1) {
							sidEmuFillBuffer(*mSid1EmuEngine,*mSid1Tune,buffer_ana[buffer_ana_gen_ofs],SOUND_BUFFER_SIZE_SAMPLE*2*2);
							nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
						} else nbBytes=mSidEmuEngine->play(buffer_ana[buffer_ana_gen_ofs],SOUND_BUFFER_SIZE_SAMPLE*2*2);
						if (mChangeOfSong==0) {
							if ((nbBytes<SOUND_BUFFER_SIZE_SAMPLE*2*2)||( (mLoopMode==0)&&(iModuleLength>0)&&(iCurrentTime>iModuleLength)) ) {
								if (mod_currentsub<mod_maxsub) {
									nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
									mod_currentsub++;
									
									if (mSidEngineType==1) {
										sidEmuInitializeSong(*mSid1EmuEngine,*mSid1Tune, mod_currentsub);
									} else {
										mSidTune->selectSong(mod_currentsub);
										mSidEmuEngine->load(mSidTune);
									}
									
									mChangeOfSong=1;
									mNewModuleLength=[self getSongLengthfromMD5:mod_currentsub-mod_minsub+1];
									if (mNewModuleLength<=0) mNewModuleLength=SID_DEFAULT_LENGTH;
									if (mLoopMode) mNewModuleLength=-1;
								} else {
									nbBytes=0;
								}
							}
						}
						 */
					}
					if (mPlayType==9) { //STSOUND
						/*
						int nbSample = SOUND_BUFFER_SIZE_SAMPLE;
						if (ymMusicComputeStereo((void*)ymMusic,(ymsample*)buffer_ana[buffer_ana_gen_ofs],nbSample)==YMTRUE) nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
						else nbBytes=0;
						 */
					}
					if (mPlayType==10) {//SC68
						/*
						nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
						int code = api68_process( sc68, buffer_ana[buffer_ana_gen_ofs], SOUND_BUFFER_SIZE_SAMPLE );
						if (code & API68_END) nbBytes=0;
						//if (code & API68_LOOP) nbBytes=0;
						//if (code & API68_CHANGE) nbBytes=0;
						
						if (mChangeOfSong==0) {
							if ((nbBytes==0)||( (iModuleLength>0)&&(iCurrentTime>iModuleLength)) ) {
								if (mod_currentsub<mod_maxsub) {
									api68_music_info_t info;
									nbBytes=SOUND_BUFFER_SIZE_SAMPLE*2*2;
									mod_currentsub++;
									api68_play(sc68,mod_currentsub,1);
									api68_music_info( sc68, &info, mod_currentsub, NULL );
									mChangeOfSong=1;
									mNewModuleLength=info.time_ms;
									if (mNewModuleLength<=0) mNewModuleLength=SC68_DEFAULT_LENGTH;
									if (mLoopMode) mNewModuleLength=-1;
								} else nbBytes=0;
							}
						}
						if (code==API68_MIX_ERROR) nbBytes=0;
						 */
					}
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
		if (bGlobalShouldEnd) break;
		
	}
	//Tell our callback what we've done
	//    [self performSelectorOnMainThread:@selector(loadComplete) withObject:fileName waitUntilDone:NO];
	
    //remove our pool and free the memory collected by it
    [pool release];
}

-(void) playPrevSub{
	if (mod_subsongs<=1) return;
	moveToPrevSubSong=1;
}
-(void) playNextSub{
	if (mod_subsongs<=1) return;	
	moveToNextSubSong=1;
}

-(BOOL) isEndReached{
	UInt32 i,datasize;
	datasize=sizeof(UInt32);
	AudioQueueGetProperty(mAudioQueue,kAudioQueueProperty_IsRunning,&i,&datasize);
	if (i==0) {
		return YES;
	}
	return NO;
}

-(void) Play {
	int counter=0;
	pthread_mutex_lock(&play_mutex);	
	bGlobalSoundHasStarted=0;
	iCurrentTime=0;
	bGlobalAudioPause=0;
	[self iPhoneDrv_PlayStart];
	bGlobalEndReached=0;
	bGlobalIsPlaying=1;
	mChangeOfSong=0;
	//Ensure play has been taken into account
	//wait for sound generation thread to end
	if (mSlowDevice) {
		while (bGlobalSoundHasStarted<SOUND_BUFFER_NB/2) {
			[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_UADE_MS]; 
			counter++;
			if (counter*DEFAULT_WAIT_TIME_UADE_MS>2) break;
		}
	}
	pthread_mutex_unlock(&play_mutex);	
}

-(void) PlaySeek:(int)startPos subsong:(int)subsong {
	if (startPos>iModuleLength-SEEK_START_MARGIN_FROM_END) {
		startPos=iModuleLength-SEEK_START_MARGIN_FROM_END;
		if (startPos<0) startPos=0;
	}
	
	switch (mPlayType) {
		case 1:  //GME
			/*
			if ((subsong!=-1)&&(subsong>=mod_minsub)&&(subsong<=mod_maxsub)) {
				
				gme_start_track(gme_emu,subsong);
				mod_currentsub=subsong;
				
				if (gme_track_info( gme_emu, &gme_info, mod_currentsub )==0) {
					iModuleLength=gme_info->play_length;
					if (iModuleLength<=0) iModuleLength=optGMEDefaultLength;
					strcpy(modtype,gme_info->system);
					sprintf(mod_message,"Song:%s\nGame:%s\nAuthor:%s\nDumper:%s\nTracks:%d\n%s",
							(gme_info->song?gme_info->song:" "),
							(gme_info->game?gme_info->game:" "),
							(gme_info->author?gme_info->author:" "),
							(gme_info->dumper?gme_info->dumper:" "),
							gme_track_count( gme_emu ),
							(gme_info->comment?gme_info->comment:" "));
					gme_free_info(gme_info);
				} else {
					strcpy(modtype,"N/A");
					strcpy(mod_message,"N/A\n");
					iModuleLength=optGMEDefaultLength;
				}
				//Loop
				if (mLoopMode==1) iModuleLength=-1;
				gme_set_fade( gme_emu, iModuleLength-optGMEFadeOut ); //Fade 1s before end
				
				mod_message_updated=2;
			}
			gme_seek(gme_emu,startPos);
			if (startPos) [self Seek:startPos];
			[self Play];
			iCurrentTime=startPos;
			break;
			*/
		case 2:  //MODPLUG
			if (startPos) [self Seek:startPos];
			[self Play];
			iCurrentTime=startPos;
			break;
		case 3:  //ADPLUG
			if (startPos) [self Seek:startPos];
			[self Play];
			iCurrentTime=startPos;
			break;
		case 4:  //
			[self Play];
			if (startPos) [self Seek:startPos];
			break;
		case 5:
			if (startPos) [self Seek:startPos];
			[self Play];
			break;
		case 6:  //UADE
			mod_wantedcurrentsub=subsong;
			if (startPos) [self Seek:startPos];
			[self Play];
			break;
		case 7://HVL/AHX
			mod_wantedcurrentsub=subsong;
			if (startPos) [self Seek:startPos];
			[self Play];
			break;
		case 8: //SID
			/*
			if (mSidEngineType==1) {
				sidEmuInitializeSong(*mSid1EmuEngine,*mSid1Tune, mod_currentsub);
			} else {
				mSidTune->selectSong(mod_currentsub);
				mSidEmuEngine->load(mSidTune);
			}
			iModuleLength=[self getSongLengthfromMD5:mod_currentsub-mod_minsub+1];
			if (iModuleLength<=0) iModuleLength=SID_DEFAULT_LENGTH;
			if (mLoopMode) iModuleLength=-1;
			
			mod_message_updated=1;
			if (startPos) [self Seek:startPos];
			[self Play];
			*/
			break;
		case 9:  //YM
			if (startPos) [self Seek:startPos];
			[self Play];
			break;
		case 10: //SC68
			/*
			if (startPos) [self Seek:startPos];
			if ((subsong!=-1)&&(subsong>=mod_minsub)&&(subsong<=mod_maxsub)) {
				mod_currentsub=subsong;
			}
			api68_music_info_t info;
			api68_play( sc68, mod_currentsub, 1);
			api68_music_info( sc68, &info, mod_currentsub, NULL );
			iModuleLength=info.time_ms;
			if (iModuleLength<=0) iModuleLength=SC68_DEFAULT_LENGTH;
			
			//Loop
			if (mLoopMode==1) iModuleLength=-1;
			
			
			//NSLog(@"track : %d, time : %d, start : %d",mod_currentsub,info.time_ms,info.start_ms);
			[self Play];
			*/
			break;
		case 11: //MDX
			[self Play];
			break;
	}
}

-(void) Stop {
	bGlobalIsPlaying=0;
	[self iPhoneDrv_PlayStop];
	
	//wait for sound generation thread to end
	while (bGlobalSoundGenInProgress) {
		[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS]; 
		//NSLog(@"Wait for end of thread");
	}
	bGlobalSeekProgress=0;
	bGlobalAudioPause=0;
	/*
	if (mPlayType==1) {
		gme_delete( gme_emu );
	}
	*/
	if (mPlayType==2) { 
		if (mp_file) {
			ModPlug_Unload(mp_file);
		}
		if (mp_data) free(mp_data);
		mp_file=NULL;
	}
	/*
	if (mPlayType==3) {
		delete adPlugPlayer;
		adPlugPlayer=NULL;
		delete opl;
		opl=NULL;
	}
	if (mPlayType==4) {
		(*ao_types[ao_type].stop)();
		if (ao_buffer) free(ao_buffer);
	}
	if (mPlayType==5) { //SexyPSF
	}
	if (mPlayType==6) {  //UADE
		//		NSLog(@"Wait for end of UADE thread");
		while (uadeThread_running) {
			[NSThread sleepForTimeInterval:DEFAULT_WAIT_TIME_MS];
			
		}
		//		NSLog(@"ok");
		uade_unalloc_song(&UADEstate);
	}
	if (mPlayType==7) { //HVL
		hvl_FreeTune(hvl_song);
		hvl_song=NULL;
	}
	if (mPlayType==8) { //SID
		if (mSidTune) {
			delete mSidTune;
			mSidTune = NULL;
		}
		if (mBuilder) {
			delete mBuilder;
			mBuilder = NULL;
		}
		if (mSidEmuEngine) {
			delete mSidEmuEngine;
			mSidEmuEngine = NULL;
		}
		if (mSid1Tune) {
			delete mSid1Tune;
			mSid1Tune = NULL;
		}
		if (mSid1EmuEngine) {
			delete mSid1EmuEngine;
			mSid1EmuEngine = NULL;
		}
	}
	if (mPlayType==9) { //STSOUND
		ymMusicStop(ymMusic);
		ymMusicDestroy(ymMusic);
	}
	if (mPlayType==10) {//SC68
		api68_stop( sc68 );
		api68_close(sc68);
	}
	if (mPlayType==11) { //MDX
		mdx_close(mdx,pdx);
	}
	 */
}

-(void) Pause:(BOOL) paused {
	bGlobalAudioPause=(paused?1:0);
	if (paused) AudioQueuePause(mAudioQueue);// [self iPhoneDrv_PlayStop];
	else AudioQueueStart(mAudioQueue,NULL);//[self iPhoneDrv_PlayStart];
	mod_message_updated=1;
}

-(ModPlug_Settings*) getMPSettings {
	if (mPlayType==2) ModPlug_GetSettings(&mp_settings);
	return &mp_settings;
}
-(void) updateMPSettings {
	if (mPlayType==2) {
		ModPlug_SetSettings(&mp_settings);
	}
}


-(int) getSongLengthfromMD5:(int)track_nb {
	/*
	NSString *pathToDB=[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:DATABASENAME_MAIN];
	sqlite3 *db;
	int err;	
	int songlength=-1;
	
	pthread_mutex_lock(&db_mutex);
	
	if (sqlite3_open([pathToDB UTF8String], &db) == SQLITE_OK){
		char sqlStatement[1024];
		sqlite3_stmt *stmt;
		
		sprintf(sqlStatement,"SELECT song_length FROM songlength WHERE id_md5=\"%s\" AND track_nb=%d",song_md5,track_nb);
		err=sqlite3_prepare_v2(db, sqlStatement, -1, &stmt, NULL);
		if (err==SQLITE_OK){
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				songlength=sqlite3_column_int(stmt, 0)*1000;
			}
			sqlite3_finalize(stmt);
		} else NSLog(@"ErrSQL : %d getSongLengthfromMD51",err);
		
	};
	sqlite3_close(db);
	
	if (songlength==-1) {
		//Try in user DB
		pathToDB=[NSString stringWithFormat:@"%@/%@",[NSHomeDirectory() stringByAppendingPathComponent:  @"Documents"],DATABASENAME_USER];
		
		if (sqlite3_open([pathToDB UTF8String], &db) == SQLITE_OK){
			char sqlStatement[1024];
			sqlite3_stmt *stmt;
			
			sprintf(sqlStatement,"SELECT song_length FROM songlength_user WHERE id_md5=\"%s\" AND track_nb=%d",song_md5,track_nb);
			err=sqlite3_prepare_v2(db, sqlStatement, -1, &stmt, NULL);
			if (err==SQLITE_OK){
				while (sqlite3_step(stmt) == SQLITE_ROW) {
					songlength=sqlite3_column_int(stmt, 0)*1000;
				}
				sqlite3_finalize(stmt);
			} else NSLog(@"ErrSQL : %d getSongLengthfromMD52",err);
			
		};
		sqlite3_close(db);
	}
	
	pthread_mutex_unlock(&db_mutex);
	return songlength;
	 */
	return 0;
}

-(void) getStilInfo:(char*)fullPath {
	/*
	NSString *pathToDB=[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:DATABASENAME_MAIN];
	sqlite3 *db;
	int err;	
	
	strcpy(stil_info,"");
	pthread_mutex_lock(&db_mutex);
	
	if (sqlite3_open([pathToDB UTF8String], &db) == SQLITE_OK){
		char sqlStatement[1024];
		char tmppath[256];
		sqlite3_stmt *stmt;
		char *realPath=strstr(fullPath,"/HVSC");
		
		if (!realPath) {
			//try to find realPath with md5
			sprintf(sqlStatement,"SELECT filepath FROM hvsc_path WHERE id_md5=\"%s\"",song_md5);
			err=sqlite3_prepare_v2(db, sqlStatement, -1, &stmt, NULL);
			if (err==SQLITE_OK){
				while (sqlite3_step(stmt) == SQLITE_ROW) {					
					strcpy(tmppath,(const char*)sqlite3_column_text(stmt, 0));
					realPath=tmppath;
				}
				sqlite3_finalize(stmt);
			} else NSLog(@"ErrSQL : %d",err);
		} else realPath+=5;
		if (realPath) {
			
			sprintf(sqlStatement,"SELECT stil_info FROM stil WHERE fullpath=\"%s\"",realPath);
			err=sqlite3_prepare_v2(db, sqlStatement, -1, &stmt, NULL);
			if (err==SQLITE_OK){
				while (sqlite3_step(stmt) == SQLITE_ROW) {
					strcpy(stil_info,(const char*)sqlite3_column_text(stmt, 0));
					while (realPath=strstr(stil_info,"\\n")) {
						*realPath='\n';
						realPath++;
						memmove(realPath,realPath+1,strlen(realPath));
					}
				}
				sqlite3_finalize(stmt);
			} else NSLog(@"ErrSQL : %d",err);
		}
	};
	sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
	 */
}

-(void) setSongLengthfromMD5:(int)track_nb songlength:(int)slength {
	/*
	NSString *pathToDB=[NSString stringWithFormat:@"%@/%@",[NSHomeDirectory() stringByAppendingPathComponent:  @"Documents"],DATABASENAME_USER];
	sqlite3 *db;
	int err;	
	
	pthread_mutex_lock(&db_mutex);
	
	if (sqlite3_open([pathToDB UTF8String], &db) == SQLITE_OK){
		char sqlStatement[1024];
		
		sprintf(sqlStatement,"DELETE FROM songlength_user WHERE id_md5=\"%s\" AND track_nb=%d",song_md5,track_nb);
		err=sqlite3_exec(db, sqlStatement, NULL, NULL, NULL);
		if (err==SQLITE_OK){
		} else NSLog(@"ErrSQL : %d setSongLengthfromMD51",err);
		
		sprintf(sqlStatement,"INSERT INTO songlength_user (id_md5,track_nb,song_length) VALUES (\"%s\",%d,%d)",song_md5,track_nb,slength/1000);
		err=sqlite3_exec(db, sqlStatement, NULL, NULL, NULL);
		if (err==SQLITE_OK){
		} else NSLog(@"ErrSQL : %d setSongLengthfromMD52",err);
		
	};
	sqlite3_close(db);
	
	pthread_mutex_unlock(&db_mutex);
	 */
}

//-(int) LoadModule:(NSString*)_filePath defaultUADE:(int)defaultUADE slowDevice:(int)slowDevice {
-(int)LoadModule {


	
	NSString *filePath = [[NSBundle mainBundle] pathForResource:@"0" ofType:nil inDirectory:@"assets/sounds"];

	
	int i;
	int found=0;
	
	/*
	int i;
	NSArray *filetype_extARCHIVE=[SUPPORTED_FILETYPE_ARCHIVE componentsSeparatedByString:@","];
	NSArray *filetype_extMDX=[SUPPORTED_FILETYPE_MDX componentsSeparatedByString:@","];
	NSArray *filetype_extSID=[SUPPORTED_FILETYPE_SID componentsSeparatedByString:@","];
	NSArray *filetype_extSTSOUND=[SUPPORTED_FILETYPE_STSOUND componentsSeparatedByString:@","];
	NSArray *filetype_extSC68=[SUPPORTED_FILETYPE_SC68 componentsSeparatedByString:@","];
	NSArray *filetype_extUADE=[SUPPORTED_FILETYPE_UADE componentsSeparatedByString:@","];
	NSArray *filetype_extMODPLUG=[SUPPORTED_FILETYPE_MODPLUG componentsSeparatedByString:@","];
	NSArray *filetype_extGME=[SUPPORTED_FILETYPE_GME componentsSeparatedByString:@","];
	NSArray *filetype_extADPLUG=[SUPPORTED_FILETYPE_ADPLUG componentsSeparatedByString:@","];
	NSArray *filetype_extSEXYPSF=[SUPPORTED_FILETYPE_SEXYPSF componentsSeparatedByString:@","];
	NSArray *filetype_extAOSDK=[SUPPORTED_FILETYPE_AOSDK componentsSeparatedByString:@","];
	NSArray *filetype_extHVL=[SUPPORTED_FILETYPE_HVL componentsSeparatedByString:@","];
	
	NSString *extension = [_filePath pathExtension];
    NSString *file_no_ext = [[_filePath lastPathComponent] stringByDeletingPathExtension];
	NSString *filePath=[NSHomeDirectory() stringByAppendingPathComponent:_filePath];
	
	mNeedSeek=0;
	mod_message_updated=0;
	mod_subsongs=0;
	mod_currentsub=mod_minsub=mod_maxsub=0;
	mod_wantedcurrentsub=-1;
	moveToPrevSubSong=0;
	moveToNextSubSong=0;
	mSidEngineType=mAskedSidEngineType;
	
	//slowDevice=1;
	
	mSlowDevice=slowDevice;
	
	int found=0;
	for (int i=0;i<[filetype_extARCHIVE count];i++) {
		if ([extension caseInsensitiveCompare:[filetype_extARCHIVE objectAtIndex:i]]==NSOrderedSame) {found=1;break;}
		if ([file_no_ext caseInsensitiveCompare:[filetype_extARCHIVE objectAtIndex:i]]==NSOrderedSame) {found=1;break;}
	}
	if (found) { //archived file, need to be uncompressed first
		//TODO : uncompress in tmp dir & try to load
	}
	
	found=0;
	for (int i=0;i<[filetype_extGME count];i++) {
		if ([extension caseInsensitiveCompare:[filetype_extGME objectAtIndex:i]]==NSOrderedSame) {found=1;break;}
		if ([file_no_ext caseInsensitiveCompare:[filetype_extGME objectAtIndex:i]]==NSOrderedSame) {found=1;break;}
	}
	if (!found)
		for (int i=0;i<[filetype_extSID count];i++) {
			if ([extension caseInsensitiveCompare:[filetype_extSID objectAtIndex:i]]==NSOrderedSame) {found=8;break;}
			if ([file_no_ext caseInsensitiveCompare:[filetype_extSID objectAtIndex:i]]==NSOrderedSame) {found=8;break;}
		}
	if (!found)
		for (int i=0;i<[filetype_extMDX count];i++) {
			if ([extension caseInsensitiveCompare:[filetype_extMDX objectAtIndex:i]]==NSOrderedSame) {found=11;break;}
			if ([file_no_ext caseInsensitiveCompare:[filetype_extMDX objectAtIndex:i]]==NSOrderedSame) {found=11;break;}
		}
	if (!found)
		for (int i=0;i<[filetype_extADPLUG count];i++) {
			if ([extension caseInsensitiveCompare:[filetype_extADPLUG objectAtIndex:i]]==NSOrderedSame) {found=3;break;}
			if ([file_no_ext caseInsensitiveCompare:[filetype_extADPLUG objectAtIndex:i]]==NSOrderedSame) {found=3;break;}
		}
	
	if (!found)
		for (int i=0;i<[filetype_extSTSOUND count];i++) {
			if ([extension caseInsensitiveCompare:[filetype_extSTSOUND objectAtIndex:i]]==NSOrderedSame) {found=9;break;}
			if ([file_no_ext caseInsensitiveCompare:[filetype_extSTSOUND objectAtIndex:i]]==NSOrderedSame) {found=9;break;}
		}
	if (!found)
		for (int i=0;i<[filetype_extSC68 count];i++) {
			if ([extension caseInsensitiveCompare:[filetype_extSC68 objectAtIndex:i]]==NSOrderedSame) {found=10;break;}
			if ([file_no_ext caseInsensitiveCompare:[filetype_extSC68 objectAtIndex:i]]==NSOrderedSame) {found=10;break;}
		}
	if (!found)
		for (int i=0;i<[filetype_extSEXYPSF count];i++) {
			if ([extension caseInsensitiveCompare:[filetype_extSEXYPSF objectAtIndex:i]]==NSOrderedSame) {found=5;break;}
			if ([file_no_ext caseInsensitiveCompare:[filetype_extSEXYPSF objectAtIndex:i]]==NSOrderedSame) {found=5;break;}
		}
	if (!found)
		for (int i=0;i<[filetype_extAOSDK count];i++) {
			if ([extension caseInsensitiveCompare:[filetype_extAOSDK objectAtIndex:i]]==NSOrderedSame) {found=4;break;}
			if ([file_no_ext caseInsensitiveCompare:[filetype_extAOSDK objectAtIndex:i]]==NSOrderedSame) {found=4;break;}
		}
	if (!found)
		for (int i=0;i<[filetype_extUADE count];i++) {
			if ([extension caseInsensitiveCompare:[filetype_extUADE objectAtIndex:i]]==NSOrderedSame) {found=6;break;}
			if ([file_no_ext caseInsensitiveCompare:[filetype_extUADE objectAtIndex:i]]==NSOrderedSame) {found=6;break;}
		}
	if ((found==6)&&defaultUADE) {  //skip modplug in this case
	} else {
		for (int i=0;i<[filetype_extMODPLUG count];i++) {
			if ([extension caseInsensitiveCompare:[filetype_extMODPLUG objectAtIndex:i]]==NSOrderedSame) {found=2;break;}
			if ([file_no_ext caseInsensitiveCompare:[filetype_extMODPLUG objectAtIndex:i]]==NSOrderedSame) {found=2;break;}
		}
	}
	for (int i=0;i<[filetype_extHVL count];i++) {
		if ([extension caseInsensitiveCompare:[filetype_extHVL objectAtIndex:i]]==NSOrderedSame) {found=7;break;}
		if ([file_no_ext caseInsensitiveCompare:[filetype_extHVL objectAtIndex:i]]==NSOrderedSame) {found=7;break;}
	}
	 */
	
	found = 2;
	
	if (found==1) {  //GME
		/*
		long sample_rate = (slowDevice?PLAYBACK_FREQ/2:PLAYBACK_FREQ);//number of samples per second 
		int track = 0; //index of track to play (0 = first)
		gme_err_t err;
		mPlayType=1;
		
		FILE *f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		fclose(f);
		
		
		// Open music file in new emulator
		err=gme_open_file( [filePath UTF8String], &gme_emu, sample_rate );
		if (err) {
			NSLog(@"%s",err);
			return -1;
		} else {
			
			//Register cleanup function and confirmation string as data
			gme_set_user_data( gme_emu, my_data );
			gme_set_user_cleanup( gme_emu, my_cleanup );
			
			//Adjust equalizer for crisp, bassy sound
			{
				gme_equalizer_t eq;
				gme_equalizer( gme_emu, &eq );
				eq.treble = 0.0;
				eq.bass   = 20;
				gme_set_equalizer( gme_emu, &eq );
			}
			
			//Enable most accurate sound emulation
			gme_enable_accuracy( gme_emu, optAccurateGME );
			
			gme_ignore_silence(gme_emu,0);
			
			//Add some stereo enhancement
			gme_set_stereo_depth( gme_emu, 0.20 );
			
			track=0;
			mod_subsongs=gme_track_count( gme_emu );
			mod_minsub=0;
			mod_maxsub=mod_subsongs-1;
			mod_currentsub=track;
			
			// Start track
			err=gme_start_track( gme_emu, track );
			if (err) {
				NSLog(@"%s",err);
				if (gme_emu) gme_delete( gme_emu );
				gme_emu=NULL;
				mPlayType=0;
				return -4;
			}
			
			
			if (gme_track_info( gme_emu, &gme_info, mod_currentsub )==0) {
				iModuleLength=gme_info->play_length;
				if (iModuleLength<=0) iModuleLength=optGMEDefaultLength;
				strcpy(modtype,gme_info->system);
				
				sprintf(mod_message,"Song:%s\nGame:%s\nAuthor:%s\nDumper:%s\nTracks:%d\n%s",
						(gme_info->song?gme_info->song:" "),
						(gme_info->game?gme_info->game:" "),
						(gme_info->author?gme_info->author:" "),
						(gme_info->dumper?gme_info->dumper:" "),
						gme_track_count( gme_emu ),
						(gme_info->comment?gme_info->comment:" "));
				
				
				if (gme_info->song){
					if (gme_info->song[0]) sprintf(mod_name," %s",gme_info->song);
					else sprintf(mod_name," %s",[[[filePath lastPathComponent] stringByDeletingPathExtension] UTF8String]);
				} else sprintf(mod_name," %s",[[[filePath lastPathComponent] stringByDeletingPathExtension] UTF8String]);
				
				
				gme_free_info(gme_info);
			} else {
				strcpy(modtype,"N/A");
				strcpy(mod_message,"N/A\n");
				iModuleLength=optGMEDefaultLength;
				sprintf(mod_name," %s",[[[filePath lastPathComponent] stringByDeletingPathExtension] UTF8String]);
			}
			
			//Loop
			if (mLoopMode==1) iModuleLength=-1;
			if (iModuleLength>optGMEFadeOut) gme_set_fade( gme_emu, iModuleLength-optGMEFadeOut ); //Fade 1s before end
			
			iCurrentTime=0;
			numChannels=gme_voice_count( gme_emu );
			
			
			mod_message_updated=2;
			return 0;
		}
		 */
	}
	if (found==3) {   //ADPLUG
		/*
		mPlayType=3;
		FILE *f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}		
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		fclose(f);
		opl=new CEmuopl(PLAYBACK_FREQ,TRUE,TRUE);
		opl->settype(Copl::TYPE_OPL2);
		adPlugPlayer = CAdPlug::factory([filePath UTF8String], opl);
		
		if (!adPlugPlayer) {
			//could not open.
			//let try the other lib below...
			delete opl;
			mPlayType=0;
			for (int i=0;i<[filetype_extMODPLUG count];i++) { //TRy modplug if applicable
				if ([extension caseInsensitiveCompare:[filetype_extMODPLUG objectAtIndex:i]]==NSOrderedSame) {found=2;break;}
				if ([file_no_ext caseInsensitiveCompare:[filetype_extMODPLUG objectAtIndex:i]]==NSOrderedSame) {found=2;break;}
			}
			for (int i=0;i<[filetype_extUADE count];i++) { //TRy modplug if applicable
				if ([extension caseInsensitiveCompare:[filetype_extUADE objectAtIndex:i]]==NSOrderedSame) {found=6;break;}
				if ([file_no_ext caseInsensitiveCompare:[filetype_extUADE objectAtIndex:i]]==NSOrderedSame) {found=6;break;}
			}
		} else {
			if (adPlugPlayer->update()) {
				opl_towrite=PLAYBACK_FREQ/adPlugPlayer->getrefresh();
			}
			
			std::string title=adPlugPlayer->gettitle();
			
			if (title.length()) sprintf(mod_name," %s",title.c_str());
			else sprintf(mod_name," %s",[[[filePath lastPathComponent] stringByDeletingPathExtension] UTF8String]);
			sprintf(mod_message,"%s",mod_name);
			
			if ((adPlugPlayer->getauthor()).length()>0)	sprintf(mod_message,"%sAuthor: %s\n", mod_message,adPlugPlayer->getauthor().c_str());
			if ((adPlugPlayer->getdesc()).length()>0) sprintf(mod_message,"%sDescription: %s\n",mod_message, adPlugPlayer->getdesc().c_str());

			for (i=0;i<adPlugPlayer->getinstruments();i++) {
				sprintf(mod_message,"%s%s\n", mod_message, adPlugPlayer->getinstrument(i).c_str());
			};	
			
			iCurrentTime=0;
			iModuleLength=adPlugPlayer->songlength();
			
			//Loop
			if (mLoopMode==1) {
				iModuleLength=-1;
			}
			
			
			numChannels=9;
			
			return 0;
		}
		 */
	} 
	
	if (found==5) {  //SexyPSF
		/*
		mPlayType=5;
		PSFINFO *pi;
		FILE *f;
		
		f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		fclose(f);
		
		NSString *fileDir=[filePath stringByDeletingLastPathComponent];
		pathdir=[fileDir UTF8String];
		
		
		if(!(pi=sexy_load((char*)[filePath UTF8String],pathdir,(mLoopMode==1)))) {
			if (sexypsf_missing_psflib) {
				UIAlertView *alertMissingLib=[[[UIAlertView alloc] initWithTitle:@"Warning" message:[NSString stringWithFormat:@"Missing file required for playback: %s.",sexypsf_psflib_str] delegate:self cancelButtonTitle:@"Close" otherButtonTitles:nil] autorelease];
				if (alertMissingLib) [alertMissingLib show];
				mPlayType=0;
				return -99;
			} else {
				NSLog(@"Error loading PSF");
				mPlayType=0;
				return -1;
			}			
		}
		
		iModuleLength=pi->length;
		if (iModuleLength==0) {
			sexy_freepsfinfo(pi);
			sexy_stop();
			mPlayType=0;
			return -1;
		}
		
		iCurrentTime=0;
		numChannels=24;
		sprintf(mod_name,"");
		if (pi->title) 
			if (pi->title[0]) sprintf(mod_name," %s",pi->title);
		
		if (mod_name[0]==0) sprintf(mod_name," %s",[[[filePath lastPathComponent] stringByDeletingPathExtension] UTF8String]);
		
		sprintf(mod_message,"Game:\t%s\nTitle:\t%s\nArtist:\t%s\nYear:\t%s\nGenre:\t%s\nPSF By:\t%s\nCopyright:\t%s\n",
				(pi->game?pi->game:""),
				(pi->title?pi->title:""),
				(pi->artist?pi->artist:""),
				(pi->year?pi->year:""),
				(pi->genre?pi->genre:""),
				(pi->psfby?pi->psfby:""),
				(pi->copyright?pi->copyright:""));
		
		sexy_freepsfinfo(pi);
		
		//Loop
		if (mLoopMode==1) iModuleLength=-1;
		
		return 0;
		 */
	}
	if (found==4) {  //AOSDK
		/*
		mPlayType=4;
		FILE *f;
		uint32 filesig;
		
		f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		rewind(f);
		ao_buffer=(unsigned char*)malloc(mp_datasize);
		fread(ao_buffer,mp_datasize,sizeof(char),f);
		fclose(f);
		
		NSString *fileDir=[filePath stringByDeletingLastPathComponent];
		pathdir=[fileDir UTF8String];
		
		// now try to identify the file
		ao_type = 0;
		filesig = ao_buffer[0]<<24 | ao_buffer[1]<<16 | ao_buffer[2]<<8 | ao_buffer[3];
		while (ao_types[ao_type].sig != 0xffffffff)	{
			if (filesig == ao_types[ao_type].sig) break;
			else ao_type++;
		}
		
		// now did we identify it above or just fall through?
		if (ao_types[ao_type].sig != 0xffffffff) {
			//printf("File identified as %s\n", ao_types[ao_type].name);
		} else {
			printf("ERROR: File is unknown, signature bytes are %02x %02x %02x %02x\n", ao_buffer[0], ao_buffer[1], ao_buffer[2], ao_buffer[3]);
			free(ao_buffer);
			mPlayType=0;
			return -1;
		}
		
		if ((*ao_types[ao_type].start)(ao_buffer, mp_datasize, (mLoopMode==1)) != AO_SUCCESS) {
			free(ao_buffer);
			if (aopsf2_missing_psflib) {
				UIAlertView *alertMissingLib=[[[UIAlertView alloc] initWithTitle:@"Warning" message:[NSString stringWithFormat:@"Missing file required for playback: %s.",aopsf2_psflib_str] delegate:self cancelButtonTitle:@"Close" otherButtonTitles:nil] autorelease];
				if (alertMissingLib) [alertMissingLib show];
				mPlayType=0;
				return -99;
			} else  {
				printf("ERROR: Engine rejected file!\n");
				mPlayType=0;
				return -1;
			}
		}
		
		(*ao_types[ao_type].fillinfo)(&ao_info);
		
		iModuleLength=ao_info.length_ms+ao_info.fade_ms;
		iCurrentTime=0;
		numChannels=24;
		if (ao_info.info[1][0]) sprintf(mod_name," %s",ao_info.info[1]);
		else sprintf(mod_name," %s",[[[filePath lastPathComponent] stringByDeletingPathExtension] UTF8String]);
		sprintf(mod_message,"%s%s\n%s%s\n%s%s\n%s%s\n%s%s\n%s%s\n%s%s\n",
				ao_info.title[2],ao_info.info[2],
				ao_info.title[3],ao_info.info[3],
				ao_info.title[4],ao_info.info[4],
				ao_info.title[5],ao_info.info[5],
				ao_info.title[6],ao_info.info[6],
				ao_info.title[7],ao_info.info[7],
				ao_info.title[8],ao_info.info[8]);
		
		//Loop
		if (mLoopMode==1) iModuleLength=-1;
		
		
		return 0;
		 */
	}
	if (found==8) {  //SID
		/*
		mPlayType=8;
		
		//First check that the file is available and get size
		FILE *f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);		
		fclose(f);
		
		if (mSidEngineType==1) {
			
			mSid1EmuEngine = new emuEngine;
			// Set config
			struct emuConfig cfg;
			mSid1EmuEngine->getConfig(cfg);
			cfg.channels = SIDEMU_STEREO;
			cfg.volumeControl = SIDEMU_VOLCONTROL;
			mSid1EmuEngine->setConfig(cfg);
			
			
			// Load tune
			mSid1Tune=new sidTune([filePath UTF8String],true,0);
			
			if ((mSid1Tune==NULL)||(mSid1Tune->cachePtr==0)) {
				NSLog(@"SID SidTune init error");
				delete mSid1EmuEngine; mSid1EmuEngine=NULL;
				if (mSid1Tune) {delete mSid1Tune;mSid1Tune=NULL;}
				mPlayType=0;
				//try UADE	:sidmon1 or sidmon2
				found=6;		
			} else {
				struct sidTuneInfo sidtune_info;
				mSid1Tune->getInfo(sidtune_info);
				
				if (sidtune_info.infoString[0][0]) sprintf(mod_name," %s",sidtune_info.infoString[0]);
				else sprintf(mod_name," %s",[[filePath lastPathComponent] UTF8String]);
				mod_subsongs=sidtune_info.songs;
				mod_minsub=1;//sidtune_info.startSong;
				mod_maxsub=sidtune_info.songs;
				mod_currentsub=sidtune_info.startSong;
				
				int tmp_md5_data_size=sidtune_info.c64dataLen+2*3+sizeof(sidtune_info.songSpeed)*sidtune_info.songs;
				char *tmp_md5_data=(char*)malloc(tmp_md5_data_size);
				memset(tmp_md5_data,0,tmp_md5_data_size);
				int ofs_md5_data=0;
				unsigned char tmp[2];		
				memcpy(tmp_md5_data,mSid1Tune->cachePtr+mSid1Tune->fileOffset,sidtune_info.c64dataLen);
				ofs_md5_data+=sidtune_info.c64dataLen;
				// Include INIT and PLAY address.
				writeLEword(tmp,sidtune_info.initAddr);
				memcpy(tmp_md5_data+ofs_md5_data,tmp,2);
				ofs_md5_data+=2;
				writeLEword(tmp,sidtune_info.playAddr);
				memcpy(tmp_md5_data+ofs_md5_data,tmp,2);
				ofs_md5_data+=2;
				// Include number of songs.		
				writeLEword(tmp,sidtune_info.songs);
				memcpy(tmp_md5_data+ofs_md5_data,tmp,2);
				ofs_md5_data+=2;
				
				// Include song speed for each song.
				for (unsigned int s = 1; s <= sidtune_info.songs; s++) {
					sidEmuInitializeSong(*mSid1EmuEngine,*mSid1Tune, s);
					mSid1Tune->getInfo(sidtune_info);
					if (sidtune_info.songSpeed==50) sidtune_info.songSpeed=0;
					memcpy(tmp_md5_data+ofs_md5_data,&sidtune_info.songSpeed,sizeof(sidtune_info.songSpeed));
					//NSLog(@"sp : %d %d %d",s,sidtune_info.songSpeed,sizeof(sidtune_info.songSpeed));
					ofs_md5_data+=sizeof(sidtune_info.songSpeed);
				}
				// Deal with PSID v2NG clock speed flags: Let only NTSC
				// clock speed change the MD5 fingerprint. That way the
				// fingerprint of a PAL-speed sidtune in PSID v1, v2, and
				// PSID v2NG format is the same.
				if ( sidtune_info.clockSpeed == SIDTUNE_CLOCK_NTSC ) {
					memcpy(tmp_md5_data+ofs_md5_data,&sidtune_info.clockSpeed,sizeof(sidtune_info.clockSpeed));
					ofs_md5_data+=sizeof(sidtune_info.clockSpeed);
				}
				
				md5_from_buffer(song_md5,33,tmp_md5_data,tmp_md5_data_size);
				song_md5[32]=0;
				free(tmp_md5_data);		
				//NSLog(@"MD5: %s",song_md5);
				
				sidEmuInitializeSong(*mSid1EmuEngine,*mSid1Tune, mod_currentsub);
				mSid1Tune->getInfo(sidtune_info);		
				iModuleLength=[self getSongLengthfromMD5:mod_currentsub-mod_minsub+1];
				if (iModuleLength<=0) iModuleLength=SID_DEFAULT_LENGTH;
				
				if (sidtune_info.sidModel==SIDTUNE_SIDMODEL_6581) {
				}
				
				if (sidtune_info.sidModel == SIDTUNE_SIDMODEL_8580){
				} else {
					//mFilterSettings.distortion_enable = true;
					//mBuilder->filter(&mFilterSettings);
				}
				
				iCurrentTime=0;
				numChannels=4;
				
				
				mSid1EmuEngine->setVoiceVolume(1, 150, 255-150, 256);
				mSid1EmuEngine->setVoiceVolume(3, 150, 255-150, 256);
				mSid1EmuEngine->setVoiceVolume(2, 255-150, 150, 256);
				mSid1EmuEngine->setVoiceVolume(4, 255-150, 150, 256);
				
    			[self getStilInfo:(char*)[filePath UTF8String]];
				
				sprintf(mod_message,"");
				for (int i=0;i<sidtune_info.numberOfInfoStrings;i++)
					sprintf(mod_message,"%s%s\n",mod_message,sidtune_info.infoString[i]);
				
				sprintf(mod_message,"%s\n[STIL Information]\n%s\n",mod_message,stil_info);
				
				return 0;
			}
		} else {
			
			// Init SID emu engine
			mSidEmuEngine = new sidplay2;
			// Init ReSID
			mBuilder = new ReSIDBuilder("resid");
			// Set config
			sid2_config_t cfg = mSidEmuEngine->config();
			cfg.optimisation = optSIDoptim;
			cfg.sidEmulation  = mBuilder;
			cfg.frequency= PLAYBACK_FREQ;
			cfg.emulateStereo = false;
			cfg.playback = sid2_stereo;
			cfg.sidSamples	  = true;
			// setup resid
			if (mBuilder->devices(true) == 0) mBuilder->create(1);
			mBuilder->filter(false);
			mBuilder->filter((const sid_filter_t *)NULL);
			//		mBuilder->filter(&mFilterSettings);
			mBuilder->sampling(cfg.frequency);
			
			mSidEmuEngine->config(cfg);
			// Load tune
			mSidTune=new SidTune([filePath UTF8String],0,true);
			
			if ((mSidTune==NULL)||(mSidTune->cache.get()==0)) {
				NSLog(@"SID SidTune init error");
				delete mSidEmuEngine; mSidEmuEngine=NULL;
				delete mBuilder; mBuilder=NULL;
				if (mSidTune) {delete mSidTune;mSidTune=NULL;}
				mPlayType=0;
				//try UADE	:sidmon1 or sidmon2
				found=6;		
			} else {
				SidTuneInfo sidtune_info;
				sidtune_info=mSidTune->getInfo();
				
				if (sidtune_info.infoString[0][0]) sprintf(mod_name," %s",sidtune_info.infoString[0]);
				else sprintf(mod_name," %s",[[filePath lastPathComponent] UTF8String]);
				mod_subsongs=sidtune_info.songs;
				mod_minsub=1;//sidtune_info.startSong;
				mod_maxsub=sidtune_info.songs;
				mod_currentsub=sidtune_info.startSong;
				
				int tmp_md5_data_size=sidtune_info.c64dataLen+2*3+sizeof(sidtune_info.songSpeed)*sidtune_info.songs;
				char *tmp_md5_data=(char*)malloc(tmp_md5_data_size);
				memset(tmp_md5_data,0,tmp_md5_data_size);
				int ofs_md5_data=0;
				unsigned char tmp[2];		
				memcpy(tmp_md5_data,mSidTune->cache.get()+mSidTune->fileOffset,sidtune_info.c64dataLen);
				ofs_md5_data+=sidtune_info.c64dataLen;
				// Include INIT and PLAY address.
				writeLEword(tmp,sidtune_info.initAddr);
				memcpy(tmp_md5_data+ofs_md5_data,tmp,2);
				ofs_md5_data+=2;
				writeLEword(tmp,sidtune_info.playAddr);
				memcpy(tmp_md5_data+ofs_md5_data,tmp,2);
				ofs_md5_data+=2;
				// Include number of songs.		
				writeLEword(tmp,sidtune_info.songs);
				memcpy(tmp_md5_data+ofs_md5_data,tmp,2);
				ofs_md5_data+=2;
				
				// Include song speed for each song.
				for (unsigned int s = 1; s <= sidtune_info.songs; s++)
				{
					mSidTune->selectSong(s);
					memcpy(tmp_md5_data+ofs_md5_data,&mSidTune->info.songSpeed,sizeof(mSidTune->info.songSpeed));
					//NSLog(@"sp : %d %d %d",s,mSidTune->info.songSpeed,sizeof(mSidTune->info.songSpeed));
					ofs_md5_data+=sizeof(mSidTune->info.songSpeed);
				}
				// Deal with PSID v2NG clock speed flags: Let only NTSC
				// clock speed change the MD5 fingerprint. That way the
				// fingerprint of a PAL-speed sidtune in PSID v1, v2, and
				// PSID v2NG format is the same.
				if ( mSidTune->info.clockSpeed == SIDTUNE_CLOCK_NTSC ) {
					memcpy(tmp_md5_data+ofs_md5_data,&mSidTune->info.clockSpeed,sizeof(mSidTune->info.clockSpeed));
					ofs_md5_data+=sizeof(mSidTune->info.clockSpeed);
					//myMD5.append(&info.clockSpeed,sizeof(info.clockSpeed));
				}
				md5_from_buffer(song_md5,33,tmp_md5_data,tmp_md5_data_size);
				song_md5[32]=0;
				free(tmp_md5_data);
				//NSLog(@"MD5: %s",song_md5);
				
				
				mSidTune->selectSong(mod_currentsub);
				iModuleLength=[self getSongLengthfromMD5:mod_currentsub-mod_minsub+1];
				if (!iModuleLength) iModuleLength=SID_DEFAULT_LENGTH;
				
				
				
				if (sidtune_info.sidModel==SIDTUNE_SIDMODEL_6581) {
					mBuilder->filter((sid_filter_t*)NULL);
				}
				
				if (sidtune_info.sidModel == SIDTUNE_SIDMODEL_8580){
					mBuilder->filter((sid_filter_t*)NULL);
				} else {
					//mFilterSettings.distortion_enable = true;
					//mBuilder->filter(&mFilterSettings);
				}
				
				if (mSidEmuEngine->load(mSidTune)==0) {
					iCurrentTime=0;
					numChannels=mSidEmuEngine->info().channels;
					
					[self getStilInfo:(char*)[filePath UTF8String]];
					
					sprintf(mod_message,"");
					for (int i=0;i<sidtune_info.numberOfInfoStrings;i++)
						sprintf(mod_message,"%s%s\n",mod_message,sidtune_info.infoString[i]);
					sprintf(mod_message,"%s\n[STIL Information]\n%s\n",mod_message,stil_info);
					//Loop
					if (mLoopMode==1) iModuleLength=-1;
					return 0;
				}
			}
		}
		 */
	}
	if (found==6) {  //UADE
		/*
		int ret;
		
		mPlayType=6;
		// First check that the file is accessible and get the size
		FILE *f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		fseek(f,0,SEEK_SET);
		char *tmp_md5_data=(char*)malloc(mp_datasize);
		fread(tmp_md5_data, 1, mp_datasize, f);
		md5_from_buffer(song_md5,33,tmp_md5_data,mp_datasize);
		song_md5[32]=0;
		free(tmp_md5_data);
		fclose(f);	
		
		uadeThread_running=0;
		[NSThread detachNewThreadSelector:@selector(uadeThread) toTarget:self withObject:NULL];
		
		UADEstate.ipc.state = UADE_INITIAL_STATE;
		UADEstate.ipc.input= uade_ipc_set_input("fd://1");
		UADEstate.ipc.output = uade_ipc_set_output("fd://1");
		
		if (uade_send_string(UADE_COMMAND_CONFIG, UADEconfigname, &(UADEstate.ipc))) {
			printf("Can not send config name: %s\n",strerror(errno));
			mPlayType=0;
			return -4;
		}
		//try to determine player
		char modulename[PATH_MAX];
		char songname[PATH_MAX];
		strcpy(modulename, [filePath UTF8String]);
		UADEstate.song = NULL;
		UADEstate.ep = NULL;
		if (!uade_is_our_file(modulename, 0, &UADEstate)) {
			printf("Unknown format: %s\n", modulename);
			mPlayType=0;
			return -3;
		}
		
		//		printf("Player candidate: %s\n", UADEstate.ep->playername);
		
		if (strcmp(UADEstate.ep->playername, "custom") == 0) {
			strcpy(UADEplayername, modulename);
			modulename[0] = 0;
		} else {
			sprintf(UADEplayername, "%s/%s", UADEstate.config.basedir.name, UADEstate.ep->playername);
		}
		
		//		printf("Player name: %s\n", UADEplayername);
		
		if (strlen(UADEplayername) == 0) {
			printf("Error: an empty player name given\n");
			mPlayType=0;
			return -4;
		}
		
		strcpy(songname, modulename[0] ? modulename : UADEplayername);
		
		if (!uade_alloc_song(&UADEstate, songname)) {
			printf("Can not read %s: %s\n", songname,strerror(errno));
			mPlayType=0;
			return -5;
		}
		
		if (UADEstate.ep != NULL) uade_set_ep_attributes(&UADEstate);
		
	///Now we have the final configuration in "uc". 
		
		UADEstate.config.no_postprocessing=mUADE_OptPOSTFX^1;
		UADEstate.config.headphones=mUADE_OptHEAD;
		UADEstate.config.gain_enable=mUADE_OptGAIN;
		UADEstate.config.gain=mUADE_OptGAINValue;
		UADEstate.config.normalise=mUADE_OptNORM;
		UADEstate.config.panning_enable=mUADE_OptPAN;
		UADEstate.config.panning=mUADE_OptPANValue;
		
		uade_set_effects(&UADEstate);
		
		//		printf("Song: %s (%zd bytes)\n",UADEstate.song->module_filename, UADEstate.song->bufsize);
		
		ret = uade_song_initialization(UADEscorename, UADEplayername, modulename, &UADEstate);
		if (ret) {
			if (ret == UADECORE_INIT_ERROR) {
				uade_unalloc_song(&UADEstate);
				mPlayType=0;
				return -6;
				//				goto cleanup;
				
			} else if (ret == UADECORE_CANT_PLAY) {
				printf("Uadecore refuses to play the song.\n");
				uade_unalloc_song(&UADEstate);
				mPlayType=0;
				return -7;
				//				continue;
			}
			
			printf("Unknown error from uade_song_initialization()\n");
			exit(1);
		}
		
		
		// song info
		sprintf(mod_name," %s",[[filePath lastPathComponent] UTF8String]);
		sprintf(mod_message,"%s\n",mod_name);
		numChannels=0;
		iCurrentTime=0;
		iModuleLength=UADEstate.song->playtime;
		if (iModuleLength<0) iModuleLength=[self getSongLengthfromMD5:mod_currentsub-mod_minsub+1];
		
		//Loop
		if (mLoopMode==1) iModuleLength=-1;
		
		
		//		NSLog(@"playtime : %d",UADEstate.song->playtime);
		
		return 0;
		 */
	}
	
	if (found==2) {  //MODPLUG
		const char *modName;
		char *modMessage;
		mPlayType=2;
		
		FILE *f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		rewind(f);
		mp_data=(char*)malloc(mp_datasize);
		fread(mp_data,mp_datasize,sizeof(char),f);
		fclose(f);
		
		[self getMPSettings];
		if (mLoopMode==1) mp_settings.mLoopCount=1<<30; //Should be like "infinite"
		[self updateMPSettings];
		
		mp_file=ModPlug_Load(mp_data,mp_datasize);
		if (mp_file==NULL) {
			free(mp_data); /* ? */
			NSLog(@"ModPlug_load error");
			mPlayType=0;
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
	if (found==7) {  //HVL
		/*
		mPlayType=7;
		
		FILE *f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		fclose(f);
		
		if (mHVLinit==0) {
			hvl_InitReplayer();
			mHVLinit=1;
		}
		
		hvl_song=hvl_LoadTune((TEXT*)[filePath UTF8String],PLAYBACK_FREQ,1);
		
		if (hvl_song==NULL) {
			NSLog(@"HVL loadTune error");
			mPlayType=0;
			return -1;
		} else {
			
			
			
			if (hvl_song->ht_Name[0]) sprintf(mod_name," %s",hvl_song->ht_Name);
			else sprintf(mod_name," %s",[[filePath lastPathComponent] UTF8String]);
			
			mod_subsongs=hvl_song->ht_SubsongNr;
			mod_minsub=0;
			mod_maxsub=mod_subsongs-1;
			mod_currentsub=0;
			
			if (!hvl_InitSubsong( hvl_song,mod_currentsub )) {
				NSLog(@"HVL issue in initsubsong %d",mod_currentsub);
				hvl_FreeTune(hvl_song);
				mPlayType=0;	
				return -2;
			}
			iModuleLength=hvl_GetPlayTime(hvl_song);
			iCurrentTime=0;
			
			numChannels=hvl_song->ht_Channels;
			
			if (hvl_song->ht_InstrumentNr==0) sprintf(mod_message,"N/A\n");
			else {
				sprintf(mod_message,"");
				for (i=0;i<hvl_song->ht_InstrumentNr;i++) {
					sprintf(mod_message,"%s%s\n", mod_message,hvl_song->ht_Instruments[i].ins_Name);
				};
			}
			
			hvl_sample_to_write=hvl_song->ht_Frequency/50/hvl_song->ht_SpeedMultiplier;
			
			//Loop
			if (mLoopMode==1) iModuleLength=-1;
			
			
			return 0;
		}
		 */
	}
	if (found==9) {  //STSOUND
		/*
		mPlayType=9;
		
		FILE *f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		fclose(f);
		
		ymMusic = ymMusicCreate();
		
		if (!ymMusicLoad(ymMusic,[filePath UTF8String])) {
			NSLog(@"STSOUND ymMusicLoad error");
			mPlayType=0;
			ymMusicDestroy(ymMusic);
			return -1;
		} else {
			ymMusicInfo_t info;
			ymMusicGetInfo(ymMusic,&info);
			
			ymMusicSetLoopMode(ymMusic,YMFALSE);
			ymMusicSetLowpassFiler(ymMusic,YMTRUE);
			ymMusicPlay(ymMusic);
			
			if (info.pSongName[0]) sprintf(mod_name," %s",info.pSongName);
			else sprintf(mod_name," %s",[[filePath lastPathComponent] UTF8String]);
			mod_subsongs=1;
			mod_minsub=0;
			mod_maxsub=0;
			mod_currentsub=0;
			
			iModuleLength=info.musicTimeInMs;
			iCurrentTime=0;
			
			numChannels=0;
			
			sprintf(mod_message,"Name.....: %s\nAuthor...: %s\nType.....: %s\nPlayer...: %s\nComment..: %s\n",info.pSongName,info.pSongAuthor,info.pSongType,info.pSongPlayer,info.pSongComment);
			
			//Loop
			if (mLoopMode==1) iModuleLength=-1;
			
			
			return 0;
		}
		 */
	}
	if (found==10) {  //SC68
		/*
		mPlayType=10;
		
		FILE *f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		fclose(f);
		
		if (api68_load_file(sc68,[filePath UTF8String])) {
			NSLog(@"SC68 api68_load_file error");
			mPlayType=0;
			return -1;
		} else {
			api68_music_info_t info;
			api68_music_info( sc68, &info, 1, NULL );
			if (info.title[0]) sprintf(mod_name," %s",info.title);
			else sprintf(mod_name," %s",[[filePath lastPathComponent] UTF8String]);			
			
			mod_subsongs=info.tracks;
			mod_minsub=1;
			mod_maxsub=1+info.tracks-1;
			mod_currentsub=1;
			
			iModuleLength=info.time_ms;
			if (iModuleLength<=0) iModuleLength=SC68_DEFAULT_LENGTH;
			iCurrentTime=0;
			
			numChannels=0;
			
			sprintf(mod_message,"Title.....: %s\nAuthor...: %s\nComposer...: %s\nHardware...: %s\nConverter.....: %s\nRipper...: %s\n",
					info.title,info.author,info.composer,info.hwname,info.converter,info.ripper);
			
			return 0;
		}
		 */
	}
	if (found==11) {  //MDX
		/*
		mPlayType=11;
		
		FILE *f=fopen([filePath UTF8String],"rb");
		if (f==NULL) {
			NSLog(@"Cannot open file %@",filePath);
			mPlayType=0;
			return -1;
		}
		
		fseek(f,0L,SEEK_END);
		mp_datasize=ftell(f);
		fclose(f);
		
		if (mdx_load((char*)[filePath UTF8String],&mdx,&pdx,slowDevice,mLoopMode) ) {
			NSLog(@"MDX mdx_load error");
			mPlayType=0;
			return -1;
		} else {
			char *tmp_mod_name=(char*)mdx_get_title(mdx);
			if (tmp_mod_name) sprintf(mod_name," %s",tmp_mod_name);
			else sprintf(mod_name," %s",[[filePath lastPathComponent] UTF8String]);			
			
			mod_subsongs=1;
			mod_minsub=1;
			mod_maxsub=1;
			mod_currentsub=1;
			
			iModuleLength=mdx_get_length( mdx,pdx);
			if (iModuleLength<=0) iModuleLength=MDX_DEFAULT_LENGTH;
			iCurrentTime=0;
			
			numChannels=mdx->tracks;
			
			if (tmp_mod_name) sprintf(mod_message,"Title.....: %s\n",tmp_mod_name);
			else sprintf(mod_message,"Title.....: N/A\n");
			
			if (tmp_mod_name) free(tmp_mod_name);
			
			if (mdx->pdx_name) {
				if (strlen(mdx->pdx_name) && (mdx->haspdx==0)) {
					sprintf(mod_message,"%sMissing PDX file: %s\n",mod_message,mdx->pdx_name);
				}
			}
			
			//Loop
			if (mLoopMode==1) iModuleLength=-1;
			
			return 0;
		}
		 */
	}
	return 1;  //Could not find a lib to load module
}

-(NSString*) getModMessage {
	NSString *modMessage;
	if ((mPlayType==1)||(mPlayType==4)||(mPlayType==5)||(mPlayType==11)) modMessage=[NSString stringWithCString:mod_message encoding:NSShiftJISStringEncoding];
	else {
		modMessage=[NSString stringWithCString:mod_message encoding:NSUTF8StringEncoding];
		if (modMessage==nil) modMessage=[NSString stringWithFormat:@"%s",mod_message];
	}
	if (modMessage==nil) return @"";
	return modMessage;
}

-(NSString*) getModName {
	NSString *modName;
	if ((mPlayType==1)||(mPlayType==4)||(mPlayType==5)||(mPlayType==11)) modName=[NSString stringWithCString:mod_name encoding:NSShiftJISStringEncoding];
	else {
		modName=[NSString stringWithCString:mod_name encoding:NSUTF8StringEncoding];
		if (modName==nil) modName=[NSString stringWithFormat:@"%s",mod_name];
	}
	if (modName==nil) return @"";
	return modName;
}


-(void) setModPlugMasterVol:(float) mstVol {
	if (mPlayType==2) ModPlug_SetMasterVolume(mp_file,(int )(mstVol*512));
}


-(void) Seek:(int) seek_time {
	if ((mPlayType==4)||(mPlayType==6)||(mPlayType==8)||(mPlayType==11)||mNeedSeek) return;
	
	//if (mPlayType==9) {
	//	if (ymMusicIsSeekable(ymMusic)==YMFALSE) return;
	//}
	
	if (seek_time>iModuleLength-SEEK_START_MARGIN_FROM_END) {
		seek_time=iModuleLength-SEEK_START_MARGIN_FROM_END;
		if (seek_time<0) seek_time=0;
	}
	
	iCurrentTime=mNeedSeekTime=seek_time;
	mNeedSeek=1;
}

-(NSString*) getPlayerName {
	if (mPlayType==1) return @"Game Music Emulator";
	if (mPlayType==2) return @"Modplug";
	if (mPlayType==3) return @"Adplug";
	if (mPlayType==4) return @"Audio Overload";
	if (mPlayType==5) return @"SexyPSF";
	if (mPlayType==6) return @"UADE";
	if (mPlayType==7) return @"HVL";
	if (mPlayType==8) return (mSidEngineType==1?@"SIDPLAY1":@"SIDPLAY2/RESID");
	if (mPlayType==9) return @"STSOUND";
	if (mPlayType==10) return @"SC68";
	if (mPlayType==11) return @"MDX";
	return @"";	
}

-(int) isPlayingTrackedMusic {
	if ((mPlayType==2)/*&&(bGlobalIsPlaying)*/) return 1;
	return 0;
}

-(NSString*) getModType {
	if (mPlayType==1) {
		return [NSString stringWithFormat:@"%s",modtype];
	}
	if (mPlayType==2) {
		/*
		switch (ModPlug_GetModuleType(mp_file)) {
			case MOD_TYPE_MOD:return @"Amiga MODule";
			case MOD_TYPE_S3M:return @"Screamtracker 3";
			case MOD_TYPE_XM:return @"Fastracker 2";
			case MOD_TYPE_MED:return @"OctaMED";
			case MOD_TYPE_IT:return @"Impulse Tracker";
			case MOD_TYPE_669:return @"Composer/Unis 669";
			case MOD_TYPE_ULT:return @"UltraTracker";
			case MOD_TYPE_STM:return @"Screamtracker";
			case MOD_TYPE_FAR:return @"Farandole Composer";
			case MOD_TYPE_WAV:return @"wav";
			case MOD_TYPE_AMF:return @"amf";
			case MOD_TYPE_AMS:return @"ams";
			case MOD_TYPE_DSM:return @"dsm";
			case MOD_TYPE_MDL:return @"mdl";
			case MOD_TYPE_OKT:return @"OctaMED";
			case MOD_TYPE_MID:return @"mid";
			case MOD_TYPE_DMF:return @"dmf";
			case MOD_TYPE_DBM:return @"dbm";
			case MOD_TYPE_MT2:return @"mt2";
			case MOD_TYPE_AMF0:return @"amf0";
			case MOD_TYPE_PSM:return @"psm";
			case MOD_TYPE_J2B:return @"j2b";
			case MOD_TYPE_ABC:return @"abc";
			case MOD_TYPE_PAT:return @"pat";
			case MOD_TYPE_UMX:return @"umx";
			default:return @"???";
		}
		*/
		
	}
	//if (mPlayType==3) return [NSString stringWithFormat:@"%s",(adPlugPlayer->gettype()).c_str()];
	//if (mPlayType==4) return [NSString stringWithFormat:@"%s",ao_types[ao_type].name];
	if (mPlayType==5) return @"PSF";
	if (mPlayType==6) return @"UADE";
	//if (mPlayType==7) return (hvl_song->ht_ModType?@"HVL":@"AHX");
	if (mPlayType==8) return @"SID";
	if (mPlayType==9) return @"YM";
	if (mPlayType==10) {
		/*
		api68_music_info_t info;
		api68_music_info( sc68, &info, 1, NULL );
		
		return [NSString stringWithFormat:@"%s",info.replay];
		 */
	}
	if (mPlayType==11) {
		/*
		if (mdx->haspdx) return @"MDX/PDX";
		else return @"MDX";
		*/
	}
	return @" ";
}

-(BOOL) isPlaying {
	if (bGlobalIsPlaying) return TRUE;
	else return FALSE;
	//if ([self isEndReached]) return FALSE;
	//else return TRUE;
}

-(int) isSeeking {
	if (bGlobalAudioPause) return 0;
	return bGlobalSeekProgress;
}
-(void) setLoopInf:(int)val {
	mLoopMode=val;
}

@end
