package com.furture.yoga;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.facebook.soloader.SoLoader;
import com.facebook.yoga.YogaAlign;
import com.facebook.yoga.YogaConfig;
import com.facebook.yoga.YogaFlexDirection;
import com.facebook.yoga.YogaLogLevel;
import com.facebook.yoga.YogaLogger;
import com.facebook.yoga.YogaNode;

public class MainActivity extends AppCompatActivity {

    private StringBuilder stringBuilder = new StringBuilder();


    public  YogaConfig YOGA_CONFIG;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        findViewById(R.id.button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                testYoga();
            }
        });
        SoLoader.init(this, false);
        YOGA_CONFIG = new YogaConfig();
        YOGA_CONFIG.setLogger(new YogaLogger() {
            @Override
            public void log(YogaNode yogaNode, YogaLogLevel yogaLogLevel, String s) {
                stringBuilder.append(s);
                stringBuilder.append("\n");
            }
        });
    }


    public void testYoga(){
        long start = System.currentTimeMillis();
        YogaNode node = new YogaNode(YOGA_CONFIG);
        node.setWidth(100);
        node.setFlexDirection(YogaFlexDirection.COLUMN);
        node.setAlignItems(YogaAlign.CENTER);
        for(int i=0; i<5000; i++){
            YogaNode child = new YogaNode(YOGA_CONFIG);
            child.setWidth(20*i);
            child.setHeight(i*10);
            node.addChildAt(child, i);
        }
        node.calculateLayout(200, 200);
        Log.e("yoga", "layout used " + (System.currentTimeMillis() - start));
        stringBuilder.setLength(0);
        Log.e("yoga", stringBuilder.toString());
    }

}
