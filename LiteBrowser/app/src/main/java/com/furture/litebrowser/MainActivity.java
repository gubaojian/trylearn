package com.furture.litebrowser;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);



        findViewById(R.id.url_node_200).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                goUrl("node200.htm");
            }
        });


        findViewById(R.id.url_node_320).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                goUrl("node320.htm");
            }
        });

        findViewById(R.id.url_node_400).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                goUrl("node400.htm");
            }
        });



        findViewById(R.id.url_node_838).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                goUrl("node838.htm");
            }
        });

    }

    private void goUrl(String url){
        Intent intent = new Intent(getBaseContext(), LiteBrowserActivity.class);
        intent.putExtra("url", url);
        startActivity(intent);
    }
}
