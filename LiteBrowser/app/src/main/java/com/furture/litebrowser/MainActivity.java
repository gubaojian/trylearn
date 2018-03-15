package com.furture.litebrowser;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.FrameLayout;

/**
 * 1、图片加载支持
 * 2、滑动支持
 * 3、点击事件支持
 * 4、研究和NetSurf的区别
 * 5、迁移到skia库，研究flutter
 * 6、包大小裁剪
 *
 *
 *  317
 *  698
 *  1277   262kb
 * */
public class MainActivity extends AppCompatActivity {


    LiteBrowser browser;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        findViewById(R.id.button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                FrameLayout frameLayout = findViewById(R.id.browser_container);
                frameLayout.removeAllViews();
                if(browser != null){
                    browser.destory();
                }
                browser = new LiteBrowser(getBaseContext());
                browser.loadHtml(AssetUtils.read(getBaseContext(), "hello.html"));
               // browser.loadUrl("https://en.m.wikipedia.org/wiki/Display_resolution");
                frameLayout.addView(browser, new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));
            }
        });

    }
}
