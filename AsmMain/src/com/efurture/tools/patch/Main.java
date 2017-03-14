package com.efurture.tools.patch;

import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.Opcodes;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;

public class Main  extends ClassLoader{

    public static void main(String[] args) throws IOException, IllegalAccessException, InstantiationException, ClassNotFoundException, NoSuchFieldException, NoSuchMethodException, InvocationTargetException {
	    // write your code here

        Main loader = new Main();
        Service service = new Service();

        System.out.println(service.getName());


        ClassReader reader = new ClassReader("com/efurture/tools/patch/Service");
        ClassWriter cw = new ClassWriter(ClassWriter.COMPUTE_MAXS);
        PatchClassVisitor patchVisitor = new PatchClassVisitor(Opcodes.ASM5, cw);
        reader.accept(patchVisitor, Opcodes.ASM5);

        byte[] bts = cw.toByteArray();
        Object patchService = loader.defineClass(Service.class.getName(), bts, 0, bts.length).newInstance();

        //newClass(Service.class.getName(), cw.toByteArray());



        patchService.getClass().getField("change").set(patchService, new IncrementChange() {
            @Override
            public Object dispatch(Object[] args) {
                return "patchMethod " + args[0];
            }
        });

        saveClass(patchService.getClass().getName(), cw.toByteArray());

        System.out.println(patchService.getClass().getMethod("getName").invoke(patchService));

    }


    public static Object  newClass(String name, byte[] bts) throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        ClassLoader classLoader = new ClassLoader() {

            @Override
            protected Class<?> findClass(String name)
                    throws ClassNotFoundException {
                return defineClass(name, bts, 0,bts.length);
            }
        };
        return classLoader.loadClass(name).newInstance();
    }

    public static void  saveClass(String name, byte[] bts) throws IOException {
        String fileName = name.replace('.', '/') + ".class";
        File file = new File(fileName);
        if(file.getParentFile() != null){
            if(!file.getParentFile().exists()){
                file.getParentFile().mkdirs();
            }
        }
        FileOutputStream fout = new FileOutputStream(file);
        fout.write(bts);
        fout.close();
    }




}
