// this class is for a separate thread to control the
// modplug player, getting commands from the Activity

// to do:  needs more error checking (I ignore the minbuffer size when getting the audio track, LoadMODData()
//         may fail, etc. etc.
//

//    Typical call order:
//
//    PlayerThread() - to get player instance (in top most activity that will use music)
//	  LoadMODData() - to call libmodplug's Load() function with the MOD data
// or
//    PlayerThread(moddatabuffer) - to get player instance and load data in one call
//
// then
//    start()
//
// 	then when changing songs (i.e. new game level or transition to another sub-activity, etc.) 
// 	  PausePlay()
//	  UnLoadMod()
//
//    LoadMODData(newmodfiledata)
//    UnPausePlay()
//    repeat...

// *NOTE*
// This class sort of assumes there's only one player thread for a whole application (all activities)
// thus the static lock objects (mPVlock, mRDlock) below, and lots of other probably bad coding
// practice below... :-(  
// For a multi-Activity application, you can try the TakeOwnership() and GiveUpOwnership() calls...
// e.g. TakeOwnership(this) in an Activity's OnCreate(), and then GiveUpOwnership(this) in onPause()
//      YMMV


package com.example.SanAngeles;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.SystemClock;
import android.util.Log;


public class PlayerThread extends Thread {

  protected static AudioTrack at1;

  public static void writeAudio(short[] b, int sz) {
    at1.write(b, 0, sz);
  }

	//
	// kind of an arbitrary maximum mod file size...  (stub code in libmodplug/modplug.cpp apparently
	//                                                 works without knowing this limit, but using a
	//                                                 GetByteArrayElements() call to access the Java buffer)
	// 
	public final static int MAXMODSIZE = 50000;    // maximum size for a mod file
	
	// limit volume volume steps to 8 steps (just an arbitrary decision...) 
	public final static float[] vol_floats = {0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.75f, 1.0f};
	
	// object for lock on PlayerValid check (mostly necessary for passing a single PlayerThread instance among
	// Activities in an Android multi-activity application)
	public static Object mPVlock;

	// object for lock on ReadData call (to prevent UI thread messing with player thread's GetSoundData() calls)
	public static Object mRDlock;

	// mark the player as invalid (for when an Activity shuts it down, but Android allows a reference to
	// the player to persist -- better solution is probably to just null out the reference to the PlayerThread
	// object in whichever Activity shuts it down)
	public boolean mPlayerValid = false;

	public static boolean mInitOK = false;

	private int rcnt;

	private  boolean playing = true;
	private  boolean running = true;

	private int minbuffer;
	private int modsize;    // holds the size in bytes of the mod file

	private final static int BUFFERSIZE = 3000; // the full sound sample buffer size
	private final static int NUMPACKETS = 2; // we'll use 2 distinct sound data buffers to make up the full BUFFERSIZE
	private final static int PACKETSIZE = BUFFERSIZE/NUMPACKETS;  // size of sound sample packet we write to the AudioTrack each time
	private int process_index=0;   // current packet we'll have libmodplug fill
	private int sound_index=0;  // next packet we'll write to the audio track
	private AudioTrack mytrack;
	
	private static short[][] packets;  // (there are NUMPACKETS in this array -- allocated in static block at bottom)
	
	private boolean load_ok;
	
	// for storing info about the MOD file currently loaded
	private String modname;
	private int numchannels;
	private int rate;

	// could probably get rid of this, unneeded when using libmodplug as single-entrant library?!?
	private static byte[] mdunused;

	// start the player in a paused state?
	private boolean start_paused;

	// play once through (one packet of sample data) then pause
	private boolean play_once;

	
	//private static final int NUM_RATES = 5;
	//private final int[] try_rates = {44100, 32000, 22000, 16000, 8000};

	private static final int NUM_RATES = 1;
	private final int[] try_rates = {44100};
	
	//
	// ownership code -- for when several activities try to share a single mod player instance...
	//
	// probably needs to be synchronized...
	//
	private Object mOwner;
	
	public boolean TakeOwnership(Object newowner) {
		if (mOwner == null || mOwner == newowner) {
			mOwner = newowner;
			return true;
		}
		else
			return false;
	}
	
	public boolean GiveUpOwnership(Object currowner) {
		if (mOwner == null || mOwner == currowner) {
			mOwner = null;
			return true;
		}
		else
			return false;
	}

	public Object GetOwner() {
		return mOwner;
	}
	
	
	//
	//  here's (one of) the constructor(s) -- grabs an audio track and loads a mod file
	//
	//  mod file data has already been read in (using a FileStream) by the caller -- that
	//  functionality could probably be included here, but for now we'll do it this way.
	//
	//  you could use this constructor in the top parent activity (like a game menu) to
	//  create a PlayerThread and load the mod data in one call
	//
	public PlayerThread(byte[] modData, int desiredrate)  {

		int rateindex = 0;
		
		boolean init_ok = false;

		// no Activity owns this player yet
		mOwner = null;

		start_paused = false;
		play_once = false;

		// reference to the mod data buffer we'll pass to libmodplug in Unload(), but it's not really needed
		// with the current JNI stub code in modplug.cpp
        mdunused = modData;
        
		// get a stereo audio track from android 
        // PACKETSIZE is the amount of data we request from libmodplug, minbuffer is the size
        // Android tells us is necessary to play smoothly for the rate, configuration we want and
        // is a separate buffer the OS handles

        // init the track and player for the desired rate (or if none specified, highest possible)
        if (desiredrate == 0) {
            boolean success = false;
            while (!success && (rateindex < NUM_RATES)) {
                try {
                    minbuffer = AudioTrack.getMinBufferSize(try_rates[rateindex], AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT);
                    Log.i("PLAYERTHREAD", "minbuffer="+minbuffer+" our PACKETSIZE="+PACKETSIZE);
                    mytrack = new AudioTrack(AudioManager.STREAM_MUSIC, try_rates[rateindex], AudioFormat.CHANNEL_CONFIGURATION_STEREO,
                    		AudioFormat.ENCODING_PCM_16BIT, minbuffer, AudioTrack.MODE_STREAM);
                    // init the Modplug player for this sample rate
                    ModPlug_Init(try_rates[rateindex]);
                    success = true;
                }
                catch (IllegalArgumentException e) {
                	Log.i("PLAYERTHREAD", "couldn't get an AUDIOTRACK at rate "+try_rates[rateindex]+"Hz!");
                	rateindex++;
                }
            }
        }
        else {
            minbuffer = AudioTrack.getMinBufferSize(desiredrate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT);
            Log.i("PLAYERTHREAD", "minbuffer="+minbuffer+" our PACKETSIZE="+PACKETSIZE);
            mytrack = new AudioTrack(AudioManager.STREAM_MUSIC, desiredrate, AudioFormat.CHANNEL_CONFIGURATION_STEREO,
            		AudioFormat.ENCODING_PCM_16BIT, minbuffer, AudioTrack.MODE_STREAM);
            // init the Modplug player for this sample rate
            ModPlug_Init(desiredrate);
        }

        if (desiredrate == 0)
        	rate = try_rates[rateindex];
        else
        	rate = desiredrate;

        if (mytrack == null) {
            Log.i("PLAYERTHREAD", "COULDN'T GET AN AUDIOTRACK");
            mPlayerValid = false;
            return;
        }
        else {
            switch(mytrack.getState()) {
            case AudioTrack.STATE_INITIALIZED:
                at1 = mytrack;
                Log.i("PLAYERTHREAD", "GOT THE INITIALIZED AUDIOTRACK!"); break;
            default:
            	Log.i("PLAYERTHREAD", "GOT THE AUDIOTRACK, BUT IT'S UNINITIALIZED?!?");
                Log.v("PLAYERTHREAD", "trying minbuffer*2 sized audiotrack instantiation...");
                mytrack = new AudioTrack(AudioManager.STREAM_MUSIC, rate, AudioFormat.CHANNEL_CONFIGURATION_STEREO,
            		AudioFormat.ENCODING_PCM_16BIT, minbuffer*2, AudioTrack.MODE_STREAM);
                switch(mytrack.getState()) {
                case AudioTrack.STATE_INITIALIZED:
                    Log.v("--------", "STATE_INITIALIZED"); break;
                default:
                    Log.v("--------", "STATE_UNINITIALIZED or NO STATIC DATA?"); 
                    break;
                }
                break;
            }

        }


        
        
        // load the mod file (data) into libmodplug
        load_ok = ModPlug_JLoad(modData, MAXMODSIZE);
        
        if (load_ok) {
        	// get info (name and number of tracks) for the loaded MOD file
        	modname = ModPlug_JGetName();
        	numchannels = ModPlug_JNumChannels();
        	
        	// init both sound and process indices to first sound buffer packet
        	sound_index = 0;
        	process_index = 0;
        }
        
        mPlayerValid = true;
	}

	//
	//  this one just gets an audio track. the mod file will be loaded later with 
	//  a call to LoadMODData()
	//
	public PlayerThread(int desiredrate)  {
		int rateindex = 0;

		boolean init_ok = false;
		
		start_paused = false;
		play_once = false;

        // init the track and player for the desired rate (or if none specified, highest possible)
        if (desiredrate == 0) {
            boolean success = false;
            while (!success && (rateindex < NUM_RATES)) {
                try {
                    minbuffer = AudioTrack.getMinBufferSize(try_rates[rateindex], AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT);
                    Log.i("PLAYERTHREAD", "minbuffer="+minbuffer+" our PACKETSIZE="+PACKETSIZE);
                    mytrack = new AudioTrack(AudioManager.STREAM_MUSIC, try_rates[rateindex], AudioFormat.CHANNEL_CONFIGURATION_STEREO,
                    		AudioFormat.ENCODING_PCM_16BIT, minbuffer, AudioTrack.MODE_STREAM);
                    // init the Modplug player for this sample rate
                    //at1 = mytrack;
                    ModPlug_Init(try_rates[rateindex]);
                    success = true;
                }
                catch (IllegalArgumentException e) {
                	Log.i("PLAYERTHREAD", "couldn't get an AUDIOTRACK at rate "+try_rates[rateindex]+"Hz!");
                	rateindex++;
                }
            }
        }
        else {
            minbuffer = AudioTrack.getMinBufferSize(desiredrate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT);
            Log.i("PLAYERTHREAD", "minbuffer="+minbuffer+" our PACKETSIZE="+PACKETSIZE);
            mytrack = new AudioTrack(AudioManager.STREAM_MUSIC, desiredrate, AudioFormat.CHANNEL_CONFIGURATION_STEREO,
            		AudioFormat.ENCODING_PCM_16BIT, minbuffer, AudioTrack.MODE_STREAM);
            // init the Modplug player for this sample rate
            ModPlug_Init(desiredrate);
        }


        if (desiredrate == 0)
        	rate = try_rates[rateindex];
        else
        	rate = desiredrate;

        if (mytrack == null) {
            Log.i("PLAYERTHREAD", "COULDN'T GET AN AUDIOTRACK");
            mPlayerValid = false;
            return;
        }
        else {
            switch(mytrack.getState()) {
            case AudioTrack.STATE_INITIALIZED:
                at1 = mytrack;
                Log.i("PLAYERTHREAD", "GOT THE INITIALIZED AUDIOTRACK!"); break;
            default:
            	Log.i("PLAYERTHREAD", "GOT THE AUDIOTRACK, BUT IT'S UNINITIALIZED?!?");
                Log.v("PLAYERTHREAD", "trying minbuffer*2 sized audiotrack instantiation...");
                mytrack = new AudioTrack(AudioManager.STREAM_MUSIC, rate, AudioFormat.CHANNEL_CONFIGURATION_STEREO,
            		AudioFormat.ENCODING_PCM_16BIT, minbuffer*2, AudioTrack.MODE_STREAM);
                switch(mytrack.getState()) {
                case AudioTrack.STATE_INITIALIZED:
                    Log.v("--------", "STATE_INITIALIZED"); break;
                default:
                    Log.v("--------", "STATE_UNINITIALIZED or NO STATIC DATA?"); 
                    break;
                }
                break;
            }

        }



        mPlayerValid = true;
 	}


	//
	// load new mod file data   (kind of assumes that PausePlay() has been called first?!)
	public void LoadMODData(byte[] modData) {

		Log.i("PLAYERTHREAD", "unloading mod data");
		
		UnLoadMod();

        mdunused = modData;
    	
        Log.i("PLAYERTHREAD", "calling ModPLug_JLoad()");

        load_ok = ModPlug_JLoad(modData, MAXMODSIZE);
        
        if (load_ok) {
        	modname = ModPlug_JGetName();
        	numchannels = ModPlug_JNumChannels();
        	
        	// 	init both sound and process indices to first sound buffer packet
        	sound_index = 0;
        	process_index = 0;
        }
    
	}
	
	
	
	//
	// This PlayerValid stuff is for multi-activity use, or also Android's Pause - Resume 
	//
	// better way to deal with it is probably to always stop and join() the PlayerThread
	// in onPause() and allocate a new PlayerThread in onResume()  (or onCreate() ?? )
	//
	
	// check if the player thread is still valid
	public boolean PlayerValid() {
		// return whether this player is valid
		synchronized(mPVlock) {
			return mPlayerValid;
		}
	}

	// mark this playerthread as invalid (typically when we're closing down the main Activity)
	public void InvalidatePlayer() {
        synchronized(mPVlock) {
        	mPlayerValid = false;
        }
	}
	
	
	//
	// the thread's run() call, where the actual sounds get played
	//
    public void run() {

    	long pause_start_time=0;
    	
    	if (start_paused)
    		playing = false;
    	else
    		playing = true;
		
    	// main play loop
        mytrack.play();

        /*
        while (running) {
        	while (playing) {
        	
        		pause_start_time=0;
        		
        		// pre-load another packet
            	synchronized(mRDlock) {
            		// for non-looping mod files, may need to check this read cnt (rcnt) returned...
            		// HACK rcnt = ModPlug_JGetSoundData(packets[process_index++], PACKETSIZE);
        	      SystemClock.sleep(500);
            	}
        		if (process_index >= NUMPACKETS) process_index = 0;
        	        	
        		// pass a packet of sound sample data to the audio track (blocks until audio track
        		// can accept the new data)
        		mytrack.write(packets[sound_index++], 0, PACKETSIZE);
        		if (sound_index >= NUMPACKETS) sound_index = 0;
        		
        		if (play_once) {
        			play_once = false;
        			playing = false;
        		}
        	} 

        	// outside the main loop now (Paused)
        	// *** THIS IS SORT OF STRANGE CODE to shut down the thread after a while ***
        	// allows the same mod player thread to be paused, load new data and unpaused
        	// or passed between two Activities
        	// allows the thread to hang around for a while (20 seconds) before shutting down
        	if (pause_start_time == 0) {
        		// record the time now (when pause has started)
        		pause_start_time = System.currentTimeMillis();
        	}
        	// sleep a little while to not hog the CPU
        	SystemClock.sleep(500);
        	
        	// now check how long we've been paused
        	synchronized (mPVlock) {
        		if ((System.currentTimeMillis()-pause_start_time) > (1000*20)  && mOwner == null) { // 20 seconds
        			Log.i("PLAYERTHREAD", "TIMEOUT REACHED!!");

        			// shut down
        			running = false;
        			
        			// just in case for some reason the audio track is uninitialized?!?
        			if (mytrack.getState() == AudioTrack.STATE_INITIALIZED)
        				mytrack.stop();
        			
        	        ModPlug_JUnload(mdunused, MAXMODSIZE);

        			mPlayerValid = false;
        		}
        		else {
        			//Log.i("PLAYERTHREAD", "timeout not reached, sleeping...");
        		}
        	}

        	
        	
        }
        */
        
        //**********************
        // experimental
        //**********************
        //mytrack.release();
    }
    
    //
    // MOD file info getters
	public String getModName() {
		return modname;
	}
	
	public int getNumChannels() {
		return numchannels;
	}
	
	public int getModSize() {
		return modsize;
	}

	public int getRate() {
		return rate;
	}

	
	
	//
	// Pause/UnPause code
    public void PausePlay() {
    	playing = false;

    	// this check is usually not needed before stop()ing the audio track, but seem
    	// to get an uninitialized audio track here occasionally, generating an IllegalStateException
    	if (mytrack.getState() == AudioTrack.STATE_INITIALIZED)
    		mytrack.stop();
    	
    }
    
    public void UnPausePlay() {
    	mytrack.play();

    	playing = true;
    }
    
    public void Flush() {
    	if (!playing) {
    		mytrack.flush();
    	}
    }

    
    //
    // sets volume with an integer value from 0-255 in 8 increments
    //
    // probably easier to just pass in a float!
    //
    public void setVolume(int vol) {
    	vol = vol>>5;
    	if (vol>7) vol = 7;
    	if (vol<0) vol = 0;
    	mytrack.setStereoVolume(vol_floats[vol], vol_floats[vol]);
    }
    
    
    //
    // startup options
    public void startPaused(boolean flag) {
    	// set before calling the thread's start() method, will cause it
    	// to start in paused mode
    	start_paused = flag;
    }

    public void playthroughOnce(boolean flag) {
    	// to wake up the audio pcm playback track
    	play_once = flag;
    }

    
    //
    // closing down code
    public void StopThread() {
    	// stops the thread playing  (see run() above)
    	playing = false;
    	running = false;
    	// this check is usually not needed before stop()ing the audio track, but seem
    	// to get an uninitialized audio track here occasionally, generating an IllegalStateException
    	if (mytrack.getState() == AudioTrack.STATE_INITIALIZED)
    		mytrack.stop();
    }
    
    public static void CloseLIBMODPLUG() {
        ModPlug_JUnload(mdunused, MAXMODSIZE);
        //Log.i("CloseLIBMODPLUG()", "JUnload() returned!");
        ModPlug_CloseDown();
        //Log.i("CloseLIBMODPLUG()", "CloseDown() returned!");
    }
    

    //
    // unload the current mod from libmodplug, but make sure to wait until any GetSoundData()
    // call in the player thread has finished.
    //
    public void UnLoadMod() {
    	// since this can/will be called from the UI thread, need to synch and not
    	// have a call into libmodplug unloading the file, while a call to GetModData() is
    	// also executing in the player thread (see run() above)
    	synchronized(mRDlock) {
    		ModPlug_JUnload(mdunused, MAXMODSIZE);
    	}
    }

    //
    // native methods in libmodplug
    //
    
    //
    // some of these don't do anything (CloseDown(), since 
    // I haven't tried to make the libmodplug JNI stub code truly re-entrant...

    // Init() now takes a sample rate in case the Android device doesn't support higher rates?!? 
    
    public static native boolean  ModPlug_Init(int rate);    // init libmodplug
    public native boolean  ModPlug_JLoad(byte[] buffer, int size);    // load a mod file (in the buffer)
    public native String ModPlug_JGetName();      // for info only, gets the mod's name
    public native int ModPlug_JNumChannels();     // info only, how many channels are used
    public native int ModPlug_JGetSoundData(short[] sndbuffer, int datasize);  // get another packet of sample data
    public static native boolean  ModPlug_JUnload(byte[] buffer, int size);  // unload a mod file
    public static native boolean ModPlug_CloseDown();   // close down libmodplug
 
  

    
    static {
    	   try {
    	        //Log.i("JNI", "Trying to load libmodplug.so");
    	        System.loadLibrary("sanangeles");
    	    }
    	    catch (UnsatisfiedLinkError ule) {
    	        //Log.e("JNI", "WARNING: Could not load libmodplug.so");
    	    }
    	    
    	    // get lock objects for synchronizing access to playervalid flag and
    	    // GetSoundData() call
    	    mPVlock = new Object();
    	    mRDlock = new Object();

    	    // set up our sample packets (libmodplug processes the mod file and fills these
    	    // with sample data)
    	    //
    	    // for proper error checking, this should check that PACKETSIZE is greater than the
    	    // minbuffer size the audio system reports in the contructors...
        	packets = new short[NUMPACKETS][PACKETSIZE];

    }
}

