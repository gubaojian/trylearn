package com.efurture.file.io;

import java.io.IOException;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/27.
 * 刷新到磁盘的事件回调
 */
public interface FlushCallback {
    public void onDisk(boolean close) throws IOException;
}
