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
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.URI;
import java.net.URL;
import java.net.URLEncoder;
import java.io.UnsupportedEncodingException;
import android.view.animation.ScaleAnimation;
import android.view.animation.TranslateAnimation;
import android.view.animation.AnimationSet;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;


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
        Log.v(this.toString(), "unable to glGenTextures");
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
          Log.v(this.toString(), "unable to GLUtils.texImage2D");
        }
        gl.glBindTexture(GL10.GL_TEXTURE_2D, 0);
      }
      nativeOnSurfaceCreated(texture_file_names.length, textures);
    } catch(IOException e) {
      Log.v(this.toString(), e.toString());
    }
  }


  public void onSurfaceChanged(GL10 gl, int w, int h) {
    nativeResize(w, h);
  }


  public void onDrawFrame(GL10 gl) {
    if (Global.wtf == -1) {
      nativeRender();
    } else {
      Global.mFooWtf.onFoo(Global.wtf);
      Global.wtf = -1;
    }
  }


  private native void nativeOnSurfaceCreated(int count, int[] textures);
  private static native void nativeResize(int w, int h);
  private static native void nativeRender();


}


class DemoGLSurfaceView extends GLSurfaceView {


  private DemoRenderer mRenderer;


  public DemoGLSurfaceView(Context context) {
    super(context);
    mRenderer = new DemoRenderer(context);
    setRenderer(mRenderer);
  }


  @Override
  public boolean onTouchEvent(final MotionEvent event) {
    float x = event.getX();
    float y = event.getY();
    if (event.getAction() == MotionEvent.ACTION_DOWN) {
      nativeTouch(x, y, 0);
    }
    if (event.getAction() == MotionEvent.ACTION_MOVE) {
      nativeTouch(x, y, 1);
    }
    if (event.getAction() == MotionEvent.ACTION_UP) {
      nativeTouch(x, y, 2);
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


  public void onFoo(int i) {
    nativeStartGame(i);
  }


  private static native void nativePause();
  private static native void nativeResume();
  private static native void nativeTouch(float x, float y, int hitState);
  private static native void nativeStartGame(int i);


}


class Global {
  public static DemoGLSurfaceView mFooWtf;
  public static int wtf = -1;
}


public class DemoActivity extends Activity {


  protected static AudioTrack at1;
	private DemoGLSurfaceView mGLView;
  private WebView mWebView;
  private JavascriptBridge mJavascriptBridge;
  public static BlockingQueue<String> mWebViewMessages;


  public static void writeAudio(short[] bytes, int offset, int size) {
    int written = at1.write(bytes, offset, size);
  }


  public InputStream getInputStreamFromUrl(String url) {
    InputStream content = null;
    try {
      HttpGet httpGet = new HttpGet(url);
      HttpClient httpclient = new DefaultHttpClient();
      HttpResponse response = httpclient.execute(httpGet);
      content = response.getEntity().getContent();
    } catch (Exception e) {
      Log.v(this.toString(), e.toString());
    }
    return content;
  }


	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
    DemoActivity.mWebViewMessages = new LinkedBlockingQueue<String>();
    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);   
    final Context myApp = this;
    final Activity MyActivity = this;
		mGLView = new DemoGLSurfaceView(this);
    Global.mFooWtf = mGLView;
		setContentView(mGLView);
    mWebView = new WebView(this);
    mJavascriptBridge = new JavascriptBridge(this);
    mWebView.addJavascriptInterface(mJavascriptBridge, "javascriptBridge");
    mWebView.setBackgroundColor(Color.argb(0,0,0,0));
    mWebView.setWebChromeClient(new WebChromeClient() {  
      @Override  
      public boolean onJsAlert(WebView view, String url, String message, final android.webkit.JsResult result) {
        new AlertDialog.Builder(myApp).setTitle("javaScript dialog").setMessage(message).setPositiveButton(android.R.string.ok, new AlertDialog.OnClickListener() {
          public void onClick(DialogInterface dialog, int which) {
            result.confirm();
          }
        }).setCancelable(false).create().show();
        return true;
      };
    });
    mWebView.setWebViewClient(new WebViewClient() {
      public void onReceivedError(WebView view, int errorCode, String description, String failingUrl) {
        Log.v(this.toString(), "WTF!@#!@#" + description);
      }
    });
    AssetManager am = getAssets();
    String path;
    String[] files;
    WebSettings webSettings = mWebView.getSettings();
    webSettings.setLoadsImagesAutomatically(true);
    webSettings.setJavaScriptEnabled(true);
    webSettings.setSupportZoom(false);
    webSettings.setJavaScriptCanOpenWindowsAutomatically(true);
    webSettings.setRenderPriority(WebSettings.RenderPriority.LOW);
    webSettings.setBuiltInZoomControls(false);
    webSettings.setPluginsEnabled(true);
    try {
      String base_url = "https://api.openfeint.com/?key=lxJAPbgkzhW91LqMeXEIg&secret=anQAUrXZTMfJxP8bLOMzmhfBlpuZMH9UPw45wCkGsQ";
      InputStream inputStream = am.open("offline/index.html");
      BufferedReader rd = new BufferedReader(new InputStreamReader(inputStream), 4096);
      String line;
      StringBuilder sb =  new StringBuilder();
      while ((line = rd.readLine()) != null) {
        Log.v(this.toString(), line);
        sb.append(line);
      }
      rd.close();
      String contentOfMyInputStream = sb.toString();
      mWebView.loadDataWithBaseURL(base_url, contentOfMyInputStream, "text/html", "utf-8", "about:blank");
    } catch (java.lang.Exception e) {
      Log.v(this.toString(), "WTF!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      Log.v(this.toString(), e.toString());
    }
    addContentView(mWebView, new LayoutParams(LayoutParams.FILL_PARENT, 120));

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

    int rate = 44100;
    int min = AudioTrack.getMinBufferSize(rate, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT);
    setMinBuffer(min);
    at1 = new AudioTrack(AudioManager.STREAM_MUSIC, rate, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT, min, AudioTrack.MODE_STREAM);
    at1.play();
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
          System.out.println(path + "/" + files[i]);
          afd3 = getAssets().openFd(path + "/" + files[i]);
          if (afd3 != null) {
              fd3[i] = afd3.getFileDescriptor();
              off3[i] = (int)afd3.getStartOffset();
              len3[i] = (int)afd3.getLength();
              sound_count_actual++;
          }
        }
      }

      int res = initNative(model_count, fd1, off1, len1, level_count, fd2, off2, len2, sound_count_actual, fd3, off3, len3);

    } catch (java.io.IOException e) {
      Log.v(this.toString(), e.toString());
    }
	}


  public boolean pushMessageToWebView(String messageToPush) {
    boolean r = false;
    int p = mWebView.getProgress();
    if (p == 100) {
      final String f = new String(messageToPush);
      mWebView.loadUrl(f);
      r = true;
    }

    messageToPush = null;
    return r;
  }


  public String popMessageFromWebView() {
    // Popped messages are JSON structures that indicate status of operations, etc
    if (mWebView.getProgress() < 100) {
      return "wtfc";
    }

    if (DemoActivity.mWebViewMessages == null) {
      return "wtf3";
    }

    final String mLastMessagePopped;

    try {
      if (DemoActivity.mWebViewMessages.isEmpty()) {
        final String messagePopBridge = "javascript:(function() { var foo = dequeue(); if (foo) { window.javascriptBridge.pushToJava(foo); } })()";
        mWebView.loadUrl(messagePopBridge);
        return "empty_in_java";
      } else {
        mLastMessagePopped = DemoActivity.mWebViewMessages.take();
        try {
          URI action = new URI(mLastMessagePopped);
          String scheme = action.getScheme();
          String path = action.getPath();
          String query = action.getQuery();
          if ("memoryleak".equals(scheme)) {
            if ("/start".equals(path)) {
              Global.wtf = Integer.parseInt(query);
            } else if ("/show".equals(path)) {
            } else if ("/hide".equals(path)) {
            }
          }
        } catch(java.net.URISyntaxException wtf) {
          Log.v(this.toString(), wtf.toString());
        }
        return mLastMessagePopped;
      }
    } catch (java.lang.InterruptedException wtf) {
      Log.v(this.toString(), wtf.toString());
      return "wtfa";
    }
  }


  public void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
  }



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


	private native int initNative(int model_count, java.io.FileDescriptor[] fd1, int[] off1, int[] len1, int level_count, java.io.FileDescriptor[] fd2, int[] off2, int[] len2, int sound_count, java.io.FileDescriptor[] fd3, int[] off3, int[] len3);
  private static native void setMinBuffer(int size);


  static {
    System.loadLibrary("sanangeles");
  }


}


class JavascriptBridge {


  private DemoActivity mActivity;


  public JavascriptBridge(DemoActivity theActivity) {
    mActivity = theActivity;
  }


  public void pushToJava(String messageFromJavascript) {
    DemoActivity.mWebViewMessages.offer(messageFromJavascript);
  }


}
