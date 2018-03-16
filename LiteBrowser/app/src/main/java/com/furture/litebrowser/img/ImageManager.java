package com.furture.litebrowser.img;

import android.graphics.Bitmap;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;

import com.furture.litebrowser.LiteBrowser;
import com.furture.litebrowser.http.UrlUtils;
import com.furture.litebrowser.jni.NativeUtils;
import com.squareup.picasso.Picasso;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by furture on 2018/3/15.
 */

public class ImageManager {

    private Map<String, ImageRender> imageRenderMap;

    private LiteBrowser liteBrowser;

    public ImageManager(LiteBrowser liteBrowser){
        this.liteBrowser = liteBrowser;
        this.imageRenderMap = new ConcurrentHashMap<>();
    }


    public ImageRender getBitmap(String url, String baseUrl){
        String requestUrl = UrlUtils.toUrl(url, baseUrl);
        return imageRenderMap.get(requestUrl);
    }


    public void loadImg(String url, String baseUrl, boolean redraw_on_ready){
        try{
            if(TextUtils.isEmpty(url)){
                return;
            }
            String requestUrl = UrlUtils.toUrl(url, baseUrl);
            ImageRender imageRender = imageRenderMap.get(requestUrl);
            if(imageRender == null){
                imageRender = new ImageRender(requestUrl);
                imageRenderMap.put(requestUrl, imageRender);
            }
            Bitmap bitmap = Picasso.get().load(UrlUtils.toUrl(url, baseUrl)).get();
            imageRender.onBitmapLoaded(bitmap, Picasso.LoadedFrom.MEMORY);
        }catch (Exception e){
            //fixme
            Log.e("weex","", e);
        }
    }

    public void getImageSize(String url, String baseUrl, long ptr){
        ImageRender imageRender = getBitmap(url, baseUrl);
        if(imageRender == null || imageRender.getBitmap() == null){
            return;
        }
        DisplayMetrics metrics = liteBrowser.getResources().getDisplayMetrics();
        NativeUtils.setSize(ptr, (int) (imageRender.getBitmap().getWidth()*metrics.density), (int) (imageRender.getBitmap().getHeight()*metrics.density));
    }


}
