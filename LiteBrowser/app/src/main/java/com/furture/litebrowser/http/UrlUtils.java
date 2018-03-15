package com.furture.litebrowser.http;

import android.net.Uri;
import android.webkit.URLUtil;

import java.io.IOException;
import java.util.List;

/**
 * Created by furture on 2018/3/15.
 */

public class UrlUtils {

    public static String toUrl(String url, String baseUrl) throws IOException {
        String requestUrl = null;
        if(URLUtil.isFileUrl(url)
                || URLUtil.isHttpsUrl(url)
                || URLUtil.isHttpUrl(url)
                || URLUtil.isAssetUrl(url)){
            requestUrl = url;
        }else{
            if(baseUrl != null){
                Uri uri = Uri.parse(url);
                requestUrl = buildRelativeURI(uri.buildUpon(), Uri.parse(baseUrl), uri).build().toString();
            }else{
                requestUrl = ASSERT_PATH + url;
            }
        }
        return requestUrl;
    }

    private static Uri.Builder buildRelativeURI(Uri.Builder resultBuilder, Uri base, Uri uri) {
        if (uri.getAuthority() != null) {
            return resultBuilder.scheme(base.getScheme());
        } else {
            resultBuilder
                    .encodedAuthority(base.getEncodedAuthority())
                    .scheme(base.getScheme())
                    .path(null);

            if (uri.getPath().startsWith("/")) {
                //relative to root
                resultBuilder.appendEncodedPath(uri.getEncodedPath().substring(1));
            } else {
                List<String> segments = base.getPathSegments();
                //ignore last segment if not end with /
                int ignoreLast = 1;
                if (base.getPath().endsWith("/")) {
                    ignoreLast = 0;
                }
                for (int i = 0, len = segments.size() - ignoreLast; i < len; i++) {
                    resultBuilder.appendEncodedPath(segments.get(i));
                }
                resultBuilder.appendEncodedPath(uri.getEncodedPath());
            }
            return resultBuilder;
        }
    }


    public static final  String ASSERT_PATH = "file:///android_asset/";

}