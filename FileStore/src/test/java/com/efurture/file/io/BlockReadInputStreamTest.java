package com.efurture.file.io;

import com.efurture.file.FileStore;
import com.efurture.file.meta.Meta;
import junit.framework.TestCase;
import org.junit.Assert;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Map;

/**
 * Created by ����(jianbai.gbj) on 2017/6/26.
 */
public class BlockReadInputStreamTest extends TestCase {



    public void  testReadSmallBufferSize() throws IOException {
        String store = "target/testReadBuffer.store";
        String key = "testReadBuffer";
        String content = "����1997��7��1�գ�һ����������Ĵ��꣬ϴˢ�����150��������衣���Ǻ���������ۻ뼤���ġ����¾����������������𡣡�\n" +
                "\n" +
                "�����������꣬����ĥ�ѣ��ڵ������ͬ��ͬԴ�����ݵķ���ֲ���ŨŨ��Ѫ�����\n" +
                "\n" +
                "�������Ž�ͳһ���л������Ǻ������л���Ů��ͬ�ĸ������ڡ�һ�����ơ��Ļ��������£���ʮ�������ñ��ؾ۵��л���Ůʼ�ռ����������Ž�ͳһ֮�������������貵Ķ�������Ҳ���ӻ�����ҫ�۶�Ŀ�Ĺ�ʡ�\n" +
                "\n";
        FileStore fileStore = new FileStore(store);
        fileStore.put(key, content);

        String value = fileStore.getString(key);

        Assert.assertEquals("value == content", value, content);
        Map<String, Meta> indexMeta = fileStore.getFileMeta();
        fileStore.close();


        Meta fileMeta = indexMeta.get(key);

        System.out.println(fileMeta);


        BlockFileInputStream inputStream = new BlockFileInputStream(store, fileMeta.blocks);
        ByteArrayOutputStream data = new ByteArrayOutputStream(1024*4);
        byte[] buffer = new byte[8];
        int read = 0;
        while ((read = inputStream.read(buffer, 0)) > 0){
            data.write(buffer, 0, read);
        }
        inputStream.close();



    }
}
