package com.furture.litebrowser;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.widget.FrameLayout;
import android.widget.Toast;

/**
 * Created by furture on 2018/3/16.
 */

public class LiteBrowserActivity extends AppCompatActivity {

    LiteBrowser browser;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_litebrowser);
        FrameLayout frameLayout = findViewById(R.id.lite_browser_container);
        String url = getIntent().getExtras().getString("url");
        if(TextUtils.isEmpty(url)){
            Toast.makeText(getBaseContext(), "please pass url", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        if(browser != null){
            browser.destory();
        }
        browser = new LiteBrowser(getBaseContext());
        if(url.startsWith("http") || url.startsWith("file://")){
            browser.loadUrl(url);
        }else{
            browser.loadHtml(AssetUtils.read(getBaseContext(), url));
        }
        frameLayout.addView(browser, new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));
    }

    @Override
    protected void onDestroy() {
        if(browser != null){
            browser.destory();
        }
        super.onDestroy();
    }
}
