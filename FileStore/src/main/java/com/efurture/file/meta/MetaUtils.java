package com.efurture.file.meta;

import com.efurture.file.FileStore;
import com.efurture.file.io.FormatInputStream;

import java.io.*;
import java.util.*;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/23.
 */
public class MetaUtils {
    /**
     * 索引的文件的缓存
     * */
    private static final WeakHashMap<String, Map<String, Meta>> metaFileCache = new WeakHashMap<String, Map<String, Meta>>();

    /**
     * 读取索引文件
     * */
    public  static Map<String, Meta> readMeta(String metaFile) throws IOException{
        Map<String, Meta> fileMeta = metaFileCache.get(metaFile);
        if(fileMeta != null){
            return  fileMeta;
        }
        fileMeta = new HashMap<String, Meta>();
        File file = new File(metaFile);
        if(!file.exists()){
            throw  new FileNotFoundException(metaFile + " meta file not exist");
        }
        FileInputStream fileInputStream = null;
        FormatInputStream formatInputStream = null;
        try {
            fileInputStream = new FileInputStream(metaFile);
            formatInputStream = new FormatInputStream(new BufferedInputStream(fileInputStream));
            Meta meta = new Meta();
            boolean hasNextMeta = meta.read(formatInputStream);
            while (hasNextMeta){
                if(meta.flag == Meta.FLAG_DELETE){
                    continue;
                }
                fileMeta.put(meta.fileName, meta);
                hasNextMeta = meta.read(formatInputStream);
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


    /**
     * 获取当前的meta文件
     * */
    public static List<String> listSortMeta(File file){
        String[] stores = file.list();
        List<String> metas  = new ArrayList<String>(4);
        if(stores != null){
            for(String store : stores){
                if(store.endsWith(FileStore.META_SUFFIX)){
                    metas.add(store);
                }
            }
            Collections.sort(metas, new Comparator<String>() {
                @Override
                public int compare(String o1, String o2) {
                    int  num1 = Integer.parseInt(o1.split("\\.")[0]);
                    int  num2 = Integer.parseInt(o2.split("\\.")[0]);
                    return num1 - num2;
                }
            });
        }
        return  metas;
    }
}
