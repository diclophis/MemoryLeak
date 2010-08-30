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
import android.util.Log;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.content.res.AssetManager;
import android.opengl.GLUtils;
import android.opengl.GLES10;
import java.io.InputStream;
import java.io.IOException;


public class DemoActivity extends Activity {

    private GLSurfaceView mGLView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mGLView = new DemoGLSurfaceView(this);
        setContentView(mGLView);

        android.content.res.AssetFileDescriptor afd;
        try {
          afd = getAssets().openFd("models/vincent.wav");
        } catch(java.io.IOException e) {
          System.out.println(e);
          afd = null;
        }

        int res = 0;
        if (afd != null) {
            java.io.FileDescriptor fd = afd.getFileDescriptor();
                int off = (int)afd.getStartOffset();
                int len = (int)afd.getLength();
                res = initNative(fd, off, len);
        }

    }

    private static native int initNative(java.io.FileDescriptor fd, int off, int len); 

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
        String[] texture_file_names = {
          "textures/vincent_texture.png",
          "textures/ground_texture.png",
          "textures/skyboxes/noonclouds_up.jpg",

          //#1
          "textures/skyboxes/noonclouds_east.jpg",

          "textures/skyboxes/noonclouds_down.jpg",

          //#3
          "textures/skyboxes/noonclouds_west.jpg",

          //#2
          "textures/skyboxes/noonclouds_north.jpg",

          //#4
          "textures/skyboxes/noonclouds_south.jpg",

          "textures/font_texture.png"
        };
        int[] textures = new int[texture_file_names.length];
        int[] tmp_tex = new int[texture_file_names.length];
        gl.glGenTextures(texture_file_names.length, tmp_tex, 0);
        textures = tmp_tex; 
        for (int i=0; i<texture_file_names.length; i++) {
          InputStream stream = am.open(texture_file_names[i]);
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
