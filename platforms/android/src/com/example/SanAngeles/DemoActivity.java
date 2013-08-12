// Jon Bardin GPL

package com.example.SanAngeles;


import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.app.Activity;
import android.content.DialogInterface;
import android.app.AlertDialog;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.View;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebChromeClient;
import android.webkit.WebViewClient;
import android.util.Log;
import java.util.Queue;
import java.util.LinkedList;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
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
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.URI;
import java.net.URL;
import java.net.URLEncoder;
import java.io.UnsupportedEncodingException;

import com.example.SanAngeles.DemoRenderer;
import com.example.SanAngeles.DemoGLSurfaceView;


public class DemoActivity extends Activity {

  protected static AudioTrack at1;
	private DemoGLSurfaceView mGLView;

	private native int initNative(
    int model_count, java.io.FileDescriptor[] fd1, int[] off1, int[] len1,
    int level_count, java.io.FileDescriptor[] fd2, int[] off2, int[] len2,
    int sound_count, java.io.FileDescriptor[] fd3, int[] off3, int[] len3,
    int texture_count, java.io.FileDescriptor[] fd4, int[] off4, int[] len4
  );
  private static native void setMinBuffer(int size);

  static {
    System.loadLibrary("sanangeles");
  }


  public static void writeAudio(short[] bytes, int offset, int size) {
    if (at1.getPlayState() != AudioTrack.PLAYSTATE_PLAYING) {
      at1.play();
    }
    int written = at1.write(bytes, offset, size);
  }


  @Override
  protected void onDestroy() {
    Log.v("ClearActivity", ":::::: instance" + this + " onDestroy: is called");
    super.onDestroy();
  }


	@Override
	protected void onCreate(Bundle savedInstanceState) {
    Log.v(this.toString(), "DemoActivity::onCreate!!!!!!");
		super.onCreate(savedInstanceState);
    setRequestedOrientation(getResources().getConfiguration().orientation);
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

    int sound_count;
    int sound_count_actual;
    java.io.FileDescriptor[] fd3;
    int[] off3;
    int[] len3;

    int texture_count;
    java.io.FileDescriptor[] fd4;
    int[] off4;
    int[] len4;

    int rate = 44100;
    int min = AudioTrack.getMinBufferSize(rate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT);
    setMinBuffer(min / 16);
    at1 = new AudioTrack(AudioManager.STREAM_MUSIC, rate, AudioFormat.CHANNEL_CONFIGURATION_STEREO, AudioFormat.ENCODING_PCM_16BIT, min * 16, AudioTrack.MODE_STREAM);
    at1.setStereoVolume(1.0f, 1.0f);

    try {
      path = "models";
      files = am.list(path);
      model_count = files.length;
      android.content.res.AssetFileDescriptor afd1;
      fd1 = new java.io.FileDescriptor[model_count];
      off1 = new int[model_count];
      len1 = new int[model_count];
      for (int i=0; i<model_count; i++) {
        Log.v(this.toString(), path + "/" + files[i]);
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
        Log.v(this.toString(), path + "/" + files[i]);
        afd2 = getAssets().openFd(path + "/" + files[i]);
        if (afd2 != null) {
          fd2[i] = afd2.getFileDescriptor();
          off2[i] = (int)afd2.getStartOffset();
          len2[i] = (int)afd2.getLength();
        }
      }

      path = "sounds";
      files = am.list(path);
      sound_count = files.length;
      sound_count_actual = 0;
      android.content.res.AssetFileDescriptor afd3;
      fd3 = new java.io.FileDescriptor[sound_count];
      off3 = new int[sound_count];
      len3 = new int[sound_count];
      for (int i=0; i<sound_count; i++) {
        if (!files[i].contains("raw")) {
        Log.v(this.toString(), path + "/" + files[i]);
          afd3 = getAssets().openFd(path + "/" + files[i]);
          if (afd3 != null) {
              fd3[i] = afd3.getFileDescriptor();
              off3[i] = (int)afd3.getStartOffset();
              len3[i] = (int)afd3.getLength();
              sound_count_actual++;
          }
        }
      }

      path = "textures";
      files = am.list(path);
      texture_count = files.length;
      android.content.res.AssetFileDescriptor afd4;
      fd4 = new java.io.FileDescriptor[texture_count];
      off4 = new int[texture_count];
      len4 = new int[texture_count];
      for (int i=0; i<texture_count; i++) {
        Log.v(this.toString(), path + "/" + files[i]);
        afd4 = getAssets().openFd(path + "/" + files[i]);
        if (afd4 != null) {
            fd4[i] = afd4.getFileDescriptor();
            off4[i] = (int)afd4.getStartOffset();
            len4[i] = (int)afd4.getLength();
        }
      }

      int res = initNative(model_count, fd1, off1, len1, level_count, fd2, off2, len2, sound_count_actual, fd3, off3, len3, texture_count, fd4, off4, len4);

    } catch (java.io.IOException e) {
      Log.v(this.toString(), e.toString());
    }
	}


  public boolean pushMessageToWebView(String messageToPush) {
    return true;
  }


  public String popMessageFromWebView() {
    return "";
  }


  public void onConfigurationChanged(Configuration newConfig) {
    Log.v(this.toString(), "DemoActivity::onConfigurationChange!!!!!!");
    super.onConfigurationChanged(newConfig);
  }


  @Override
  protected void onPause() {
    Log.v(this.toString(), "DemoActivity::onPause!!!!!!" + mGLView);
    super.onPause();
    mGLView.onPause();
  }


  @Override
  protected void onResume() {
    Log.v(this.toString(), "DemoActivity::onResume!!!!!!" + mGLView);
    super.onResume();
    mGLView.onResume();
  }
}
