package com.example.SanAngeles;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.util.Log;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.content.res.AssetManager;
import android.opengl.GLUtils;
import android.opengl.GLES10;
import android.content.res.Configuration;
import java.io.InputStream;
import java.io.IOException;
import android.view.ViewGroup.LayoutParams;
import android.graphics.Color;

public class DemoActivity extends Activity {

	// default song to start with
	public static final int DEFAULT_SONG = 0;

	private PlayerThread player = null;

	private InputStream modfileInStream;
	private byte[] modData;
	private int modsize;    // holds the size in bytes of the mod file

	private GLSurfaceView mGLView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);   

		mGLView = new DemoGLSurfaceView(this);
		setContentView(mGLView);

    AssetManager am = getAssets();
    String path;
    String[] files;

    int model_count;
    java.io.FileDescriptor[] fd1;
    int[] off1;
    int[] len1;

    int level_count;
    java.io.FileDescriptor[] fd2;
    int[] off2;
    int[] len2;

    /*
    // allocate a buffer for the modfile data
    modData = new byte[PlayerThread.MAXMODSIZE];
    modfileInStream =  getResources().openRawResource(R.raw.song);
    try {
        modsize = modfileInStream.read(modData, 0, PlayerThread.MAXMODSIZE);
    } catch (IOException e) {
        e.printStackTrace();
    }

    player = new PlayerThread(modData, 0);


    */

        int rate = 44100;
        int minbuffer = AudioTrack.getMinBufferSize(rate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT);
        Log.i("PLAYERTHREAD", "minbuffer="+minbuffer);
        mytrack = new AudioTrack(AudioManager.STREAM_MUSIC, rate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT, minbuffer, AudioTrack.MODE_STREAM);
        mytrack.play();
        mytrack.setStereoVolume(1.0f, 1.0f);
        at1 = mytrack;
        ModPlug_Init(try_rates[rateindex]);

    try {
      path = "models";
      files = am.list(path);
      model_count = files.length;
      android.content.res.AssetFileDescriptor afd1;
      fd1 = new java.io.FileDescriptor[model_count];
      off1 = new int[model_count];
      len1 = new int[model_count];
      for (int i=0; i<model_count; i++) {
        afd1 = getAssets().openFd(path + "/" + files[i]);
        if (afd1 != null) {
            fd1[i] = afd1.getFileDescriptor();
            off1[i] = (int)afd1.getStartOffset();
            len1[i] = (int)afd1.getLength();
        }
      }
      
      path = "levels";
      files = am.list(path);
      level_count = files.length;
      android.content.res.AssetFileDescriptor afd2;
      fd2 = new java.io.FileDescriptor[level_count];
      off2 = new int[level_count];
      len2 = new int[level_count];
      for (int i=0; i<level_count; i++) {
        afd2 = getAssets().openFd(path + "/" + files[i]);
        if (afd2 != null) {
            fd2[i] = afd2.getFileDescriptor();
            off2[i] = (int)afd2.getStartOffset();
            len2[i] = (int)afd2.getLength();
        }
      }

		  int res = initNative(model_count, fd1, off1, len1, level_count, fd2, off2, len2);
    } catch(java.io.IOException e) {
      System.out.println(e);
    }
	}


  public void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
  }


	private static native int initNative(int model_count, java.io.FileDescriptor[] fd1, int[] off1, int[] len1, int level_count, java.io.FileDescriptor[] fd2, int[] off2, int[] len2);

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLView.onResume();
    }

    static {
        System.loadLibrary("sanangeles");
    }
}

class DemoGLSurfaceView extends GLSurfaceView {
    public DemoGLSurfaceView(Context context) {
        super(context);
        mRenderer = new DemoRenderer(context);
        setRenderer(mRenderer);
    }

    public boolean onTouchEvent(final MotionEvent event) {
      float x = event.getRawX();
      float y = event.getRawY();
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            nativeTouch(x, y, 0);
        }
        if (event.getAction() == MotionEvent.ACTION_MOVE) {
            nativeTouch(x, y, 1);
        }
        if (event.getAction() == MotionEvent.ACTION_UP) {
            nativeTouch(x, y, 2);
        }
        event.recycle();
        return true;
    }

    @Override
    public void onPause() {
      super.onPause();
      nativePause();
    }

    DemoRenderer mRenderer;

    private static native void nativePause();
    private static native void nativeTouch(float x, float y, int hitState);
}

class DemoRenderer implements GLSurfaceView.Renderer {

    Context mContext;

    public DemoRenderer(Context context) {
      mContext = context;
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
      try {
        AssetManager am = mContext.getAssets();
        String path = "textures";
        String[] texture_file_names = am.list(path);
        int[] textures = new int[texture_file_names.length];
        int[] tmp_tex = new int[texture_file_names.length];
        gl.glGenTextures(texture_file_names.length, tmp_tex, 0);

int glError;
if ((glError = gl.glGetError()) != 0) {
System.out.println("1");
}

        textures = tmp_tex; 
        for (int i=0; i<texture_file_names.length; i++) {
          InputStream stream = am.open(path + "/" + texture_file_names[i]);
          Bitmap bitmap = BitmapFactory.decodeStream(stream);
          int t = textures[i];
          gl.glBindTexture(GL10.GL_TEXTURE_2D, t);
          gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR);
          gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_LINEAR);
          GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, bitmap, 0);
if ((glError = gl.glGetError()) != 0) {
System.out.println("2");
}
          gl.glBindTexture(GL10.GL_TEXTURE_2D, 0);
        }
        nativeOnSurfaceCreated(texture_file_names.length, textures);
      } catch(IOException e) {
      }
    }

    public void onSurfaceChanged(GL10 gl, int w, int h) {
        nativeResize(w, h);
    }

    public void onDrawFrame(GL10 gl) {
        nativeRender();
    }

    private native void nativeOnSurfaceCreated(int count, int[] textures);
    private static native void nativeResize(int w, int h);
    private static native void nativeRender();
}
