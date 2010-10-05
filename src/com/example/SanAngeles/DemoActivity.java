/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This is a small port of the "San Angeles Observation" demo
 * program for OpenGL ES 1.x. For more details, see:
 *
 *    http://jet.ro/visuals/san-angeles-observation/
 *
 * This program demonstrates how to use a GLSurfaceView from Java
 * along with native OpenGL calls to perform frame rendering.
 *
 * Touching the screen will start/stop the animation.
 *
 * Note that the demo runs much faster on the emulator than on
 * real devices, this is mainly due to the following facts:
 *
 * - the demo sends bazillions of polygons to OpenGL without
 *   even trying to do culling. Most of them are clearly out
 *   of view.
 *
 * - on a real device, the GPU bus is the real bottleneck
 *   that prevent the demo from getting acceptable performance.
 *
 * - the software OpenGL engine used in the emulator uses
 *   the system bus instead, and its code rocks :-)
 *
 * Fixing the program to send less polygons to the GPU is left
 * as an exercise to the reader. As always, patches welcomed :-)
 */

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

    //requestWindowFeature(Window.FEATURE_NO_TITLE);
    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);   


		mGLView = new DemoGLSurfaceView(this);
		setContentView(mGLView);


    /*
		android.content.res.AssetFileDescriptor afd2;
		android.content.res.AssetFileDescriptor afd3;
		android.content.res.AssetFileDescriptor afd4;
    */
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
        //afd2 = getAssets().openFd("models/barrel.wav");
        //afd3 = getAssets().openFd("models/vincent.wav");
        //afd4 = getAssets().openFd("models/crate.wav");

        if (afd1 != null) { //  && afd2 != null && afd3 != null && afd4 != null) {
            fd1[i] = afd1.getFileDescriptor();
            off1[i] = (int)afd1.getStartOffset();
            len1[i] = (int)afd1.getLength();

            /*
            java.io.FileDescriptor fd2 = afd2.getFileDescriptor();
            int off2 = (int)afd2.getStartOffset();
            int len2 = (int)afd2.getLength();
            java.io.FileDescriptor fd3 = afd3.getFileDescriptor();
            int off3 = (int)afd3.getStartOffset();
            int len3 = (int)afd3.getLength();
            java.io.FileDescriptor fd4 = afd4.getFileDescriptor();
            int off4 = (int)afd4.getStartOffset();
            int len4 = (int)afd4.getLength();
            */

        }
      }
		  int res = initNative(texture_file_names.length, fd1, off1, len1); //, fd2, off2, len2, fd3, off3, len3, fd4, off4, len4);
    } catch(java.io.IOException e) {
      System.out.println(e);
    }

	}

  public void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
    //setContentView(R.layout.myLayout);
  }


	private static native int initNative(int count, java.io.FileDescriptor[] fd1, int[] off1, int[] len1); //, java.io.FileDescriptor fd2, int off2, int len2, java.io.FileDescriptor fd3, int off3, int len3, java.io.FileDescriptor fd4, int off4, int len4); 

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
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            nativeTouch();
        }
        return true;
    }

    DemoRenderer mRenderer;

    private static native void nativePause();
    private static native void nativeTouch();
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
    private static native void nativeDone();
}
