package com.efurture.tools.patch;


import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

/**
 * Created by ½£°×(jianbai.gbj) on 2017/3/14.
 * http://www.cnblogs.com/liuling/archive/2013/05/25/asm.html
 * https://github.com/scouter-project/scouter/blob/master/scouter.agent.java/src/scouter/agent/asm/AddFieldASM.java
 */
public class PatchClassVisitor extends ClassVisitor implements  Opcodes {


    public PatchClassVisitor(int i, ClassVisitor classVisitor) {
        super(i, classVisitor);
    }


    @Override
   public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {
        MethodVisitor methodVisitor = super.visitMethod(access, name, desc, signature, exceptions);
        if(methodVisitor == null || name.equals("<init>")){
            return  methodVisitor;
        }
        return  new PatchMethodVisitor(Opcodes.ASM5, methodVisitor);
    }



    @Override
    public FieldVisitor visitField(int access, String name, String desc, String signature, Object value) {
        return  super.visitField(access, name, desc, signature, value);
    }


        @Override
    public void visitEnd() {
        FieldVisitor fv =  cv.visitField(ACC_PUBLIC + ACC_STATIC, "change", "Lcom/efurture/tools/patch/IncrementChange;", null, null);
        if(fv!=null){
             fv.visitEnd();
        }
        cv.visitEnd();
    }
}
