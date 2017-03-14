package com.efurture.tools.patch;

import org.objectweb.asm.Label;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
/**
 * Created by ½£°×(jianbai.gbj) on 2017/3/14.
 */
public class PatchMethodVisitor extends MethodVisitor implements  Opcodes {

    public PatchMethodVisitor(int i, MethodVisitor methodVisitor) {
        super(i, methodVisitor);
    }


    @Override
    public void visitCode() {
        mv.visitFieldInsn(GETSTATIC, "com/efurture/tools/patch/Service", "change", "Lcom/efurture/tools/patch/IncrementChange;");
        Label l0 = new Label();
        mv.visitJumpInsn(IFNULL, l0);
        mv.visitFieldInsn(GETSTATIC, "com/efurture/tools/patch/Service", "change", "Lcom/efurture/tools/patch/IncrementChange;");
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
    }


}
