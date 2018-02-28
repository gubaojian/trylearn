package com.furture.opengl;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by furture on 2018/2/22.
 */

public class GLSurfaceViewRender implements GLSurfaceView.Renderer {


    static final  int  COORDS_PER_VECTEX = 3;

    public static final int VERTEX_STRIPE = 3 * Float.BYTES;

    static float triangleCoords[] = {
            0.0f, 0.622008459f, 0.0f,
            -0.5f, -0.311004243f, 0.0f,
            0.5f, -0.311004243f, 0.0f
    };


    float triangleColor[] = {0.63671875f, 0.76953125f, 0.22265625f, 1.0f};

    private String  triangleVectorShaderCode  = "" +
            "uniform mat4 uMVPMatrix;" +
            "attribute vec4 vPosition;" +
            "void main(){ "  +
            "   gl_Position = uMVPMatrix * vPosition;" +
            "}";

    private int triangleProgram;

    private int triangleVertexShader;
    private int triangleFragmentShader;

    private ByteBuffer triangleVertexByteBuffer = null;


    private final  float[] mMVPMatrix = new float[16];

    private final  float[] mProjectionMatrix = new float[16];

    private final  float[] mViewMatrix = new float[16];

    private final  float[] result = new float[32];


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        triangleProgram  = GLES20.glCreateProgram();
        triangleVertexShader = ShaderUtils.loadShader(GLES20.GL_VERTEX_SHADER, triangleVectorShaderCode);
        triangleFragmentShader = ShaderUtils.loadShader(GLES20.GL_FRAGMENT_SHADER,
                "precision mediump float; " +
                        "uniform vec4 vColor; " +
                        "void main(){ "  +
                        " gl_FragColor = vColor;" +
                        "}"
        );
        GLES20.glAttachShader(triangleProgram, triangleVertexShader);
        GLES20.glAttachShader(triangleProgram, triangleFragmentShader);
        GLES20.glLinkProgram(triangleProgram);

        int error = GLES20.glGetError();

        Log.e("weex", "WEEX gl link error " + error);

        triangleVertexByteBuffer = ByteBuffer.allocateDirect(
                triangleCoords.length * Float.BYTES
        );
        triangleVertexByteBuffer.order(ByteOrder.nativeOrder());
        triangleVertexByteBuffer.asFloatBuffer().put(triangleCoords);
        triangleVertexByteBuffer.position(0);


    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        gl.glViewport(0,0, width, height);

        float ratio = (float)width/height;


        // this projection matrix is applied to object coordinates
        // in the onDrawFrame() method
        Matrix.frustumM(mProjectionMatrix, 0, -ratio, ratio, -1, 1, 3, 7);



    }

    @Override
    public void onDrawFrame(GL10 gl) {

        Matrix.setLookAtM(mViewMatrix,  0, 0, 0, 3, 0f, 0f, 0f, 0f, 1.0f, 0.0f);
        Matrix.multiplyMM(mMVPMatrix, 0, mProjectionMatrix, 0, mViewMatrix, 0);

        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);


        // Set the camera position (View matrix)
        //Matrix.setLookAtM(mViewMatrix, 0, 0, 0, -3, 0f, 0f, 0f, 0f, 1.0f, 0.0f);

        // Calculate the projection and view transformation
        //Matrix.multiplyMM(mMVPMatrix, 0, mProjectionMatrix, 0, mViewMatrix, 0);




        GLES20.glUseProgram(triangleProgram);



        //



        //Matrix.multiplyMM(result, 0, mMVPMatrix, 0, triangleCoords, 0);



        for(int i=0; i<mMVPMatrix.length; i++){
           // Log.e("weex", "weex result " + mMVPMatrix[i]);
        }


        int triangleAttributeLocation = GLES20.glGetAttribLocation(triangleProgram, "vPosition");
        GLES20.glEnableVertexAttribArray(triangleAttributeLocation);
        GLES20.glVertexAttribPointer(triangleAttributeLocation, COORDS_PER_VECTEX,
                GLES20.GL_FLOAT, false,  VERTEX_STRIPE, triangleVertexByteBuffer);


        int triangleUniformLocation =  GLES20.glGetUniformLocation(triangleProgram, "vColor");

        GLES20.glUniform4fv(triangleUniformLocation, 1, triangleColor, 0);

        checkGlError("rrr color");

        int mvpMatixHandle = GLES20.glGetUniformLocation(triangleProgram, "uMVPMatrix");

        checkGlError("rrr uMVPMatrix");

        GLES20.glUniformMatrix4fv(mvpMatixHandle, 1, false, mMVPMatrix, 0);

        checkGlError("rrr0");

        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0 ,  triangleCoords.length/COORDS_PER_VECTEX);

        checkGlError("rrr1");


        GLES20.glDisableVertexAttribArray(triangleAttributeLocation);

        checkGlError("rrr1");
    }

    public static void checkGlError(String op) {
        int error;
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            throw new RuntimeException(op + ": glError " + error);
        }
    }
}
