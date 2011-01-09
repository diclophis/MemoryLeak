/*
 *  ModizerConstants.h
 *  modizer
 *
 *  Created by yoyofr on 22/08/10.
 *  Copyright 2010 __YoyoFR / Yohann Magnien__. All rights reserved.
 *
 */

//#define LOAD_PROFILE

#define MODIZER_COMPIL 1

#define DEFAULT_WAIT_TIME_MS  0.0001   //in s
#define DEFAULT_WAIT_TIME_UADE_MS  0.001   //in s

#define PLAYBACK_FREQ 44100


#define SOUND_BUFFER_SIZE_SAMPLE PLAYBACK_FREQ/30 //256
#define SOUND_BUFFER_NB 30 //1second
#define SPECTRUM_BANDS 64
#define SND_BUFFER_CURRENTTIME_FIX 1000

#define SEEK_START_MARGIN_FROM_END 2000
//#define GME_END_TRACK_DETECT_LENGTH_LIMIT_LOW 500
//#define GME_END_TRACK_DETECT_LENGTH_LIMIT_HIGH 1500
//#define GME_END_TRACK_DETECT_LENGTH_LIMIT_NBTRACK 5
#define GME_DEFAULT_LENGTH 150000
#define SID_DEFAULT_LENGTH 150000
#define SC68_DEFAULT_LENGTH 150000
#define MDX_DEFAULT_LENGTH 150000

#define MAX_STIL_DATA_LENGTH 4400

#define VERSION_MAJOR 1
#define VERSION_MINOR 1

#define MAX_PL_ENTRIES 512

#define MAX_RANDFX_TIME 15   //max is in fact min + max (seconds)
#define MIN_RANDFX_TIME 15
#define ALLOW_CHANGE_ON_BEAT_TIME 10   //change on beat is allowed after this threashold (seconds)