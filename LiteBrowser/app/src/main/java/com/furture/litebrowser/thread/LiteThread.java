package com.furture.litebrowser.thread;

import android.os.HandlerThread;

/**
 * Created by furture on 2018/3/13.
 */

public class LiteThread extends HandlerThread {

    public LiteThread(String name) {
        super(name);
    }

    public LiteThread(String name, int priority) {
        super(name, priority);

    }
}
