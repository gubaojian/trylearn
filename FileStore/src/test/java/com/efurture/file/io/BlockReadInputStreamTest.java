package com.efurture.file.io;

import com.efurture.file.FileStore;
import com.efurture.file.meta.Meta;
import junit.framework.TestCase;
import org.junit.Assert;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.util.Map;

/**
 * Created by 剑白(jianbai.gbj) on 2017/6/26.
 */
public class BlockReadInputStreamTest extends TestCase {



    public void  testReadSmallBufferSize() throws IOException {
        String store = "target/testReadBuffer.store";
        String key = "testReadBuffer";
        String content = "　　1997年7月1日，一场酣畅淋漓的大雨，洗刷掉香港150多年的屈辱。五星红旗伴随着雄浑激昂的《义勇军进行曲》徐徐升起。　\n" +
                "\n" +
                "　　几番风雨，几多磨难，内地与香港同根同源，短暂的分离抵不过浓浓的血脉情深。\n" +
                "\n" +
                "　　“团结统一的中华民族是海内外中华儿女共同的根。”在“一国两制”的基本国策下，二十年来，久别重聚的中华儿女始终坚守着民族团结统一之根本，香港这颗璀璨的东方明珠也愈加焕发出耀眼夺目的光彩。\n" +
                "\n";
        FileStore fileStore = new FileStore(store);
        fileStore.put(key, content);

        String value = fileStore.getString(key);

        Assert.assertEquals("value == content", value, content);
        Map<String, Meta> indexMeta = fileStore.getIndexMeta();
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
