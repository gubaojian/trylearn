package com.efurture.tools.patch;

/**
 * Created by ½£°×(jianbai.gbj) on 2017/3/14.
 */
import java.util.*;
import org.objectweb.asm.*;

public class ServiceDump implements Opcodes {

    public static byte[] dump () throws Exception {
        ClassWriter cw = new ClassWriter(0);
        FieldVisitor fv;
        MethodVisitor mv;
        AnnotationVisitor av0;

        cw.visit(52, ACC_PUBLIC + ACC_SUPER, "com/efurture/tools/patch/ServiceDemo", null, "java/lang/Object", null);

        {
            fv = cw.visitField(ACC_PUBLIC + ACC_STATIC, "change", "Lcom/efurture/tools/patch/IncrementChange;", null, null);
            fv.visitEnd();
        }
        {
            mv = cw.visitMethod(ACC_PUBLIC, "<init>", "()V", null, null);
            mv.visitCode();
            mv.visitVarInsn(ALOAD, 0);
            mv.visitMethodInsn(INVOKESPECIAL, "java/lang/Object", "<init>", "()V", false);
            mv.visitInsn(RETURN);
            mv.visitMaxs(1, 1);
            mv.visitEnd();
        }
        {
            mv = cw.visitMethod(ACC_PUBLIC, "getName", "()Ljava/lang/Object;", null, null);
            mv.visitCode();
            mv.visitFieldInsn(GETSTATIC, "com/efurture/tools/patch/ServiceDemo", "change", "Lcom/efurture/tools/patch/IncrementChange;");
            Label l0 = new Label();
            mv.visitJumpInsn(IFNULL, l0);
            mv.visitFieldInsn(GETSTATIC, "com/efurture/tools/patch/ServiceDemo", "change", "Lcom/efurture/tools/patch/IncrementChange;");
            mv.visitInsn(ICONST_1);
            mv.visitTypeInsn(ANEWARRAY, "java/lang/Object");
            mv.visitInsn(DUP);
            mv.visitInsn(ICONST_0);
            mv.visitVarInsn(ALOAD, 0);
            mv.visitInsn(AASTORE);
            mv.visitMethodInsn(INVOKEINTERFACE, "com/efurture/tools/patch/IncrementChange", "dispatch", "([Ljava/lang/Object;)Ljava/lang/Object;", true);
            mv.visitInsn(ARETURN);
            mv.visitLabel(l0);
            mv.visitFrame(Opcodes.F_SAME, 0, null, 0, null);
            mv.visitLdcInsn("ServiceName");
            mv.visitInsn(ARETURN);
            mv.visitMaxs(5, 1);
            mv.visitEnd();
        }
        {
            mv = cw.visitMethod(ACC_PUBLIC, "getName", "(Ljava/lang/Object;)Ljava/lang/Object;", null, null);
            mv.visitCode();
            mv.visitFieldInsn(GETSTATIC, "com/efurture/tools/patch/ServiceDemo", "change", "Lcom/efurture/tools/patch/IncrementChange;");
            Label l0 = new Label();
            mv.visitJumpInsn(IFNULL, l0);
            mv.visitFieldInsn(GETSTATIC, "com/efurture/tools/patch/ServiceDemo", "change", "Lcom/efurture/tools/patch/IncrementChange;");
            mv.visitInsn(ICONST_2);
            mv.visitTypeInsn(ANEWARRAY, "java/lang/Object");
            mv.visitInsn(DUP);
            mv.visitInsn(ICONST_0);
            mv.visitVarInsn(ALOAD, 0);
            mv.visitInsn(AASTORE);
            mv.visitInsn(DUP);
            mv.visitInsn(ICONST_1);
            mv.visitVarInsn(ALOAD, 1);
            mv.visitInsn(AASTORE);
            mv.visitMethodInsn(INVOKEINTERFACE, "com/efurture/tools/patch/IncrementChange", "dispatch", "([Ljava/lang/Object;)Ljava/lang/Object;", true);
            mv.visitInsn(ARETURN);
            mv.visitLabel(l0);
            mv.visitFrame(Opcodes.F_SAME, 0, null, 0, null);
            mv.visitLdcInsn("ServiceName2");
            mv.visitInsn(ARETURN);
            mv.visitMaxs(5, 2);
            mv.visitEnd();
        }
        cw.visitEnd();

        return cw.toByteArray();
    }

}
