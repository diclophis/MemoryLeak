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
import android.util.Log;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.content.res.AssetManager;
import android.opengl.GLUtils;
import android.opengl.GLES10;
import android.content.res.Configuration;
import java.io.InputStream;
import java.io.IOException;


public class DemoActivity extends Activity {

	private GLSurfaceView mGLView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);   

		mGLView = new DemoGLSurfaceView(this);
		setContentView(mGLView);

    AssetManager am = getAssets();
    String path = "models";
    String[] texture_file_names;

    java.io.FileDescriptor[] fd1;
    int[] off1;
    int[] len1;

    try {
      texture_file_names = am.list(path);
      android.content.res.AssetFileDescriptor afd1;
      fd1 = new java.io.FileDescriptor[texture_file_names.length];
      off1 = new int[texture_file_names.length];
      len1 = new int[texture_file_names.length];
      for (int i=0; i<texture_file_names.length; i++) {
        afd1 = getAssets().openFd(path + "/" + texture_file_names[i]);
        if (afd1 != null) {
            fd1[i] = afd1.getFileDescriptor();
            off1[i] = (int)afd1.getStartOffset();
            len1[i] = (int)afd1.getLength();
        }
      }
		  int res = initNative(texture_file_names.length, fd1, off1, len1);
    } catch(java.io.IOException e) {
      System.out.println(e);
    }
	}


  public void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
  }


	private static native int initNative(int count, java.io.FileDescriptor[] fd1, int[] off1, int[] len1);

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
        textures = tmp_tex; 
        for (int i=0; i<texture_file_names.length; i++) {
          InputStream stream = am.open(path + "/" + texture_file_names[i]);
          Bitmap bitmap = BitmapFactory.decodeStream(stream);
          int t = textures[i];
          gl.glBindTexture(GL10.GL_TEXTURE_2D, t);
          gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR);
          gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_LINEAR);
          GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, bitmap, 0);
          gl.glBindTexture(GL10.GL_TEXTURE_2D, 0);
        }
        nativeOnSurfaceCreated(textures);
      } catch(IOException e) {
      }
    }

    public void onSurfaceChanged(GL10 gl, int w, int h) {
        nativeResize(w, h);
    }

    public void onDrawFrame(GL10 gl) {
        nativeRender();
    }

    private native void nativeOnSurfaceCreated(int[] textures);
    private static native void nativeResize(int w, int h);
    private static native void nativeRender();

    //private static native void nativeDone();
}
