package com.furture.litebrowser;

import android.content.Context;
import android.net.Uri;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.InputStream;
import java.net.URL;

/**
 * Created by furture on 2018/3/12.
 */

public class AssetUtils {


    public static String read(Context context, String fileName){
        try {
            InputStream is = context.getAssets().open(fileName);
            ByteArrayOutputStream output = new ByteArrayOutputStream();

            BufferedInputStream bufferIn = new BufferedInputStream(is);
            byte[] buffer = new byte[2048];
            int length = 0;
            while ((length = bufferIn.read(buffer)) >= 0){
                output.write(buffer, 0, length);
            }
            String script = output.toString();
            bufferIn.close();
            return script;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
}
