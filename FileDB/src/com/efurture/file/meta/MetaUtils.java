package com.efurture.file.meta;

import com.efurture.file.io.FormatInputStream;

import java.io.*;
import java.util.HashMap;
import java.util.Map;
import java.util.WeakHashMap;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/23.
 */
public class MetaUtils {
    /**
     * 索引的文件的缓存
     * */
    private static final WeakHashMap<String, Map<String, Meta>> metaFileCache = new WeakHashMap<>();

    /**
     * 读取索引文件
     * */
    public  static Map<String, Meta> readMeta(String metaFile) throws IOException{
        Map<String, Meta> fileMeta = metaFileCache.get(metaFile);
        if(fileMeta != null){
            return  fileMeta;
        }
        fileMeta = new HashMap<>();
        File file = new File(metaFile);
        if(!file.exists()){
            return  fileMeta;
        }
        FileInputStream fileInputStream = null;
        FormatInputStream formatInputStream = null;
        try {
            fileInputStream = new FileInputStream(metaFile);
            formatInputStream = new FormatInputStream(new BufferedInputStream(fileInputStream));
            Meta meta = new Meta();
            boolean isValidMeta = meta.read(formatInputStream);
            while (isValidMeta){
                fileMeta.put(meta.fileName, meta);
                isValidMeta = meta.read(formatInputStream);
            }
            metaFileCache.put(metaFile, fileMeta);
            return fileMeta;
        }finally {
            if (formatInputStream != null) {
                formatInputStream.close();
            }
            if (fileInputStream != null) {
                fileInputStream.close();
            }
        }
    }
}
