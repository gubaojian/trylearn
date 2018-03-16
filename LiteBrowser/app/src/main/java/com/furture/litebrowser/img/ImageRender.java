package com.furture.litebrowser.img;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;

import com.squareup.picasso.Picasso;
import com.squareup.picasso.Target;

/**
 * Created by furture on 2018/3/15.
 */

public class ImageRender implements Target {


    public static final int STATE_INIT  = 0;

    public static final int STATE_SUCCESS  = 1;

    public static final int STATE_FAILED  = -1;

    private String url;
    private int state;
    private Bitmap bitmap;

    public ImageRender(String url) {
        this.url = url;
        this.state = STATE_INIT;
    }

    @Override
    public void onBitmapLoaded(Bitmap bitmap, Picasso.LoadedFrom from) {
        this.state = STATE_SUCCESS;
        this.bitmap = bitmap;
    }

    @Override
    public void onBitmapFailed(Exception e, Drawable errorDrawable) {
        this.state = STATE_FAILED;
    }

    @Override
    public void onPrepareLoad(Drawable placeHolderDrawable) {

    }

    public Bitmap getBitmap() {
        return bitmap;
    }
}
