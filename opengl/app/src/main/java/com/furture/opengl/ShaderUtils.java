package com.furture.opengl;

import android.opengl.GLES20;

/**
 * Created by furture on 2018/2/22.
 */

public class ShaderUtils {

    public static int loadShader(int type, String code){
        int shader = GLES20.glCreateShader(type);
        GLES20.glShaderSource(shader, code);
        GLES20.glCompileShader(shader);
        return shader;
    }
}
