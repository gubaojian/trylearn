#include <jni.h>
#include <string>
#include <cstdlib>



static jbyte* share_buffer = new jbyte[1024*16];

extern "C"
JNIEXPORT jstring



JNICALL
Java_com_furture_jnitest_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_furture_jnitest_MainActivity_arrayCopy(JNIEnv *env, jobject instance, jbyteArray bts_) {
    jbyte *bts = env->GetByteArrayElements(bts_, NULL);
    share_buffer = (jbyte *) malloc(sizeof(jbyte) * 1024 * 8);

    memcpy(share_buffer, bts, 1024);
    free(share_buffer);
    env->ReleaseByteArrayElements(bts_, bts, 0);
}extern "C"
JNIEXPORT void JNICALL
Java_com_furture_jnitest_MainActivity_arrayCopyByteBuffer(JNIEnv *env, jobject instance,
                                                          jobject buffer) {
    void* src = env->GetDirectBufferAddress(buffer);
    share_buffer = (jbyte *) malloc(sizeof(jbyte) * 1024 * 8);

    memcpy(share_buffer, src, 1024);
    free(share_buffer);
}