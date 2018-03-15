package com.furture.opengl;

import android.opengl.GLSurfaceView;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.FrameLayout;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {


    GLSurfaceView mGLSurfaceView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        mGLSurfaceView = new GLSurfaceView(getBaseContext());
        mGLSurfaceView.setEGLContextClientVersion(2);
        mGLSurfaceView.setDebugFlags(GLSurfaceView.DEBUG_CHECK_GL_ERROR);
        mGLSurfaceView.setRenderer(new GLSurfaceViewRender());
        mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        final FrameLayout layout = findViewById(R.id.gl_sureface_container);
        layout.postDelayed(new Runnable() {
            @Override
            public void run() {

                layout.addView(mGLSurfaceView, new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));

            }
        }, 1000);
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
