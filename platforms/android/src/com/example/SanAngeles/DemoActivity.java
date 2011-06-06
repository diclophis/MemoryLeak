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
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebChromeClient;
import android.webkit.WebViewClient;
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
    queueEvent(new Runnable() {
      public void run() {
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
      }
    });
    return true;
  }

  @Override
  public void onPause() {
    super.onPause();
    nativePause();
  }

  private static native void nativePause();
  private static native void nativeTouch(float x, float y, int hitState);
}


class JavascriptBridge {
  private DemoActivity mActivity;
  public JavascriptBridge(DemoActivity theActivity) {
    mActivity = theActivity;
  }

  public void pushToJava(String messageFromJavascript) {
    mActivity.mLastMessagePopped = messageFromJavascript;
  }
}


public class DemoActivity extends Activity {

  protected static AudioTrack at1;
	private GLSurfaceView mGLView;
  private WebView mWebView;
  private JavascriptBridge mJavascriptBridge;
  public String mLastMessagePopped;

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

  public String getStringContent(String uri) throws Exception {
    try {
      HttpClient client = new DefaultHttpClient();
      HttpGet request = new HttpGet();
      request.setURI(new URI(uri));
      HttpResponse response = client.execute(request);
      InputStream ips  = response.getEntity().getContent();
      BufferedReader buf = new BufferedReader(new InputStreamReader(ips,"UTF-8"));
      StringBuilder sb = new StringBuilder();
      String s;
      while(true) {
        s = buf.readLine();
        if(s==null || s.length()==0)
          break;
        sb.append(s);
      }
      buf.close();
      ips.close();
      Log.v(this.toString(), sb.toString());
      Log.v(this.toString(), "foo " + sb.length());
      return sb.toString();
    } finally {
    }
  } 

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);   

		mGLView = new DemoGLSurfaceView(this);
		setContentView(mGLView);

    mWebView = new WebView(this);
    mJavascriptBridge = new JavascriptBridge(this);
    mWebView.addJavascriptInterface(mJavascriptBridge, "javascriptBridge");
    mWebView.setBackgroundColor(Color.argb(0,0,0,0));

    final Context myApp = this;

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
    
    WebSettings webSettings = mWebView.getSettings();
    webSettings.setLoadsImagesAutomatically(true);
    webSettings.setJavaScriptEnabled(true);
    webSettings.setSupportZoom(false);
    webSettings.setJavaScriptCanOpenWindowsAutomatically(true);
    webSettings.setRenderPriority(WebSettings.RenderPriority.LOW);
    webSettings.setBuiltInZoomControls(false);

    try {
      String url = "http://192.168.1.144:3000/OFConnectJavascript/index.html";
      String base_url = "https://api.openfeint.com/";
      BufferedReader rd = new BufferedReader(new InputStreamReader(getInputStreamFromUrl(url)), 4096);
      String line;
      StringBuilder sb =  new StringBuilder();
      while ((line = rd.readLine()) != null) {
        sb.append(line);
      }
      rd.close();
      String contentOfMyInputStream = sb.toString();
      mWebView.loadDataWithBaseURL(base_url, contentOfMyInputStream, "text/html", "utf-8", "about:blank");
    } catch (java.lang.Exception e) {
      Log.v(this.toString(), "WTF!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      Log.v(this.toString(), e.toString());
    }
 
    addContentView(mWebView, new LayoutParams(LayoutParams.FILL_PARENT, 144));

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

  public void pushMessageToWebView(String messageToPush) {
    // pushed messages are direct JS that is eval'ed in the context of the webview
    mWebView.loadUrl(messageToPush);
  }

  public String popMessageFromWebView() {
    // Popped messages are JSON structures that indicate status of operations, etc
    String messagePopBridge = "javascript:(function() { javascriptBridge.pushToJava('some message from queue.pop'); })()";
    // After invoking this, mLastMessagePumped, should contain 'some message from queue.pop'
    mWebView.loadUrl(message);
    return mLastMessagePopped;
  }

  public void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
  }

	private native int initNative(int model_count, java.io.FileDescriptor[] fd1, int[] off1, int[] len1, int level_count, java.io.FileDescriptor[] fd2, int[] off2, int[] len2, int sound_count, java.io.FileDescriptor[] fd3, int[] off3, int[] len3);
  private static native void setMinBuffer(int size);

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
        nativeRender();
    }

    private native void nativeOnSurfaceCreated(int count, int[] textures);
    private static native void nativeResize(int w, int h);
    private static native void nativeRender();
}
