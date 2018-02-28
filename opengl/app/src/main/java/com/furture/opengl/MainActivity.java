package com.furture.opengl;

import android.opengl.GLSurfaceView;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {


    GLSurfaceView mGLSurfaceView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mGLSurfaceView = findViewById(R.id.gl_sureface);
        mGLSurfaceView.setEGLContextClientVersion(2);
        mGLSurfaceView.setDebugFlags(GLSurfaceView.DEBUG_CHECK_GL_ERROR);
        mGLSurfaceView.setRenderer(new GLSurfaceViewRender());
        mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(mGLSurfaceView != null){
            mGLSurfaceView.onResume();
        }
    }


    @Override
    protected void onPause() {
        super.onPause();
        if(mGLSurfaceView != null){
            mGLSurfaceView.onPause();
        }
    }
}
