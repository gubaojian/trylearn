package com.furture.litebrowser.http;

import android.content.Context;
import android.net.Uri;
import android.text.TextUtils;
import android.webkit.URLUtil;
import android.webkit.WebView;

import com.furture.litebrowser.AssetUtils;

import java.io.IOException;
import java.util.List;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

/**
 * Created by furture on 2018/3/13.
 */

public class HttpClient {
    private  static OkHttpClient client = new OkHttpClient();

    public static String get(Context context, String url, String baseUrl) throws IOException {
        if(TextUtils.isEmpty(url)){
            return null;
        }
        String requestUrl = UrlUtils.toUrl(url, baseUrl);
        if(URLUtil.isAssetUrl(requestUrl)){
            String fileName = requestUrl.substring(UrlUtils.ASSERT_PATH.length());
            return AssetUtils.read(context, fileName);
        }
        Request.Builder  builder = new Request.Builder().url(requestUrl);
        addHeader(builder);
        Request request =  builder.build();
        Response response = client.newCall(request).execute();
        return response.body().string();
    }

    private static void addHeader(Request.Builder builder){

        builder.addHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
        //builder.addHeader("Accept-Encoding", "gzip, deflate");
        builder.addHeader("User-Agent", "Mozilla/5.0 (iPhone; CPU iPhone OS 9_1 like Mac OS X) AppleWebKit/601.1.46 (KHTML, like Gecko) Version/9.0 Mobile/13B143 Safari/601.1");
    }
}
