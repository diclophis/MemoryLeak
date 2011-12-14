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


class DemoGLSurfaceView extends GLSurfaceView {


  private DemoRenderer mRenderer;


  public DemoGLSurfaceView(Context context) {
    super(context);
    mRenderer = new DemoRenderer(context);
    setRenderer(mRenderer);
  }


  @Override
  public boolean onTouchEvent(final MotionEvent event) {
    float x = 0;
    float y = 0;
    int type = -1;
    for (int i = 0; i < event.getPointerCount(); i++) {
      switch (event.getAction() & MotionEvent.ACTION_MASK) {
        case MotionEvent.ACTION_DOWN:
        case MotionEvent.ACTION_POINTER_DOWN:
          x = event.getX(i);
          y = event.getY(i);
          nativeTouch(x, y, 0);
          break;
        case MotionEvent.ACTION_MOVE:
          x = event.getX(i);
          y = event.getY(i);
          nativeTouch(x, y, 1);
          break;
        case MotionEvent.ACTION_UP:
        case MotionEvent.ACTION_CANCEL:
        case MotionEvent.ACTION_POINTER_UP:
          x = event.getX(i);
          y = event.getY(i);
          nativeTouch(x, y, 2);
          break;  
      }
    }
    return true;
  }


  @Override
  public void onPause() {
    super.onPause();
    nativePause();
  }


  @Override
  public void onResume() {
    super.onResume();
    nativeResume();
  }


  private static native void nativePause();
  private static native void nativeResume();
  private static native void nativeTouch(float x, float y, int hitState);
  private static native void nativeStartGame(int i);

}
