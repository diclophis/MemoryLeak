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


public class PlayerThread {

  protected static AudioTrack at1;

  public static void writeAudio(short[] b, int sz) {
    at1.write(b, 0, sz);
  }

	//
	// kind of an arbitrary maximum mod file size...  (stub code in libmodplug/modplug.cpp apparently
	//                                                 works without knowing this limit, but using a
	//                                                 GetByteArrayElements() call to access the Java buffer)
	// 
	public final static int MAXMODSIZE = 200000;    // maximum size for a mod file
	
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

	private final static int BUFFERSIZE = 6000; // the full sound sample buffer size
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
        

        rate = 44100;

        minbuffer = AudioTrack.getMinBufferSize(rate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT);
        Log.i("PLAYERTHREAD", "minbuffer="+minbuffer+" our PACKETSIZE="+PACKETSIZE);
        mytrack = new AudioTrack(AudioManager.STREAM_MUSIC, rate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT, minbuffer, AudioTrack.MODE_STREAM);
        mytrack.setStereoVolume(1.0f, 1.0f);
        at1 = mytrack;
        ModPlug_Init(try_rates[rateindex]);

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

    
    
    

    public static native boolean ModPlug_Init(int rate);    // init libmodplug
    public native boolean ModPlug_JLoad(byte[] buffer, int size);    // load a mod file (in the buffer)
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
    	        Log.e("JNI", "WARNING: Could not load libmodplug.so");
    	    }
    }
}
