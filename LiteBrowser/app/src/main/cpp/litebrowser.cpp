#include <jni.h>
#include <android/log.h>
#include <string>
#include "litehtml.h"
#include "mobile_container.h"
#include "litebrowsercommon.h"
#include "helper/JniHelper.h"


#define LOGE(TAG,...) __android_log_print(ANDROID_LOG_ERROR, TAG,__VA_ARGS__)

typedef struct LiteBrowser{
    mobile_container* container;
    litehtml::context* context;
    std::shared_ptr<litehtml::document>  document;
} LiteBrowser;


static JavaVM*  jvm;
static LiteBrowserJava* liteBrowserJava;


#define convert_to_context(ptr)    ((LiteBrowser *)((intptr_t)ptr))


extern "C"
JNIEXPORT jlong JNICALL
Java_com_furture_litebrowser_LiteBrowser_init(JNIEnv *env, jobject instance, jstring masterCssRef,
jint screenWidth, jint screenHeight, jfloat density, jint densityDpi, jint defaultFontSize) {
    liteBrowserJava->screen.height = screenHeight;
    liteBrowserJava->screen.width = screenWidth;
    liteBrowserJava->screen.defaultFontSize = defaultFontSize;
    liteBrowserJava->screen.density = density;
    liteBrowserJava->screen.densityDpi = densityDpi;
    const char *masterCss = env->GetStringUTFChars(masterCssRef, 0);
    mobile_container* container = new mobile_container(liteBrowserJava, instance);
    litehtml::context* context = new litehtml::context();
    context->load_master_stylesheet(masterCss);
    LiteBrowser* browser = new LiteBrowser();
    browser->container = container;
    browser->context = context;
    env->ReleaseStringUTFChars(masterCssRef, masterCss);
    return convert_to_ptr(browser);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_furture_litebrowser_LiteBrowser_layout(JNIEnv *env, jobject instance, jlong ptr,
                                                jint width, jint height) {
    //
    LiteBrowser* browser = convert_to_context(ptr);
    if(browser->container){
        browser->container->clientFrame->height = height;
        browser->container->clientFrame->width = width;
    }
    if(browser->document){
        browser->document->render(width, render_all);
        LOGE("LiteBrowser", "LiteBrowser render success width %d  height %d", browser->document->width(), browser->document->height());
        return browser->document->height();
    }else{
        LOGE("LiteBrowser", "LiteBrowser has none document");
    }
    return 0;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_furture_litebrowser_LiteBrowser_draw(JNIEnv *env, jobject instance, jlong ptr,
                                              jobject canvas, jint left, jint right, jint width,
                                              jint height) {

    LiteBrowser* browser = convert_to_context(ptr);
    if(browser->document){
        position drawPosition(0,0, width, height);
        browser->document->draw(canvas, 0, 0, &drawPosition);
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_furture_litebrowser_LiteBrowser_loadHtml__JLjava_lang_String_2(JNIEnv *env,
                                                                        jobject instance, jlong ptr,
                                                                        jstring html_) {
    const char *html = env->GetStringUTFChars(html_, 0);
    LiteBrowser* browser = convert_to_context(ptr);
    litehtml::css* css = nullptr;
    browser->document = litehtml::document::createFromUTF8(html, browser->container, browser->context, css);
    env->ReleaseStringUTFChars(html_, html);
}




extern "C"
JNIEXPORT void JNICALL
Java_com_furture_litebrowser_LiteBrowser_destory__J(JNIEnv *env, jobject instance, jlong ptr) {
    LiteBrowser *browser = convert_to_context(ptr);
    if(browser){
        if(browser->context){
            delete browser->context;
        }
        if(browser->container){
            delete browser->container;
        }
    }
    delete browser;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_furture_litebrowser_jni_NativeUtils_setSize(JNIEnv *env, jclass type, jlong ptr,
                                                     jint width, jint height) {
    litehtml::size* sz = (litehtml::size *)ptr;
    sz->width = width;
    sz->height = height;
}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved){
    LOGE("litebrowser", "litebrowser jni onload");
    jvm  = vm;
    JNIEnv* env;
    if(vm->GetEnv((void**)&env,JNI_VERSION_1_4) != JNI_OK){
        LOGE("ipc", "litebrowser jni onload get env error");
    }
    init_jni_helper(vm, env);
    liteBrowserJava = new LiteBrowserJava();
    liteBrowserJava->jvm = jvm;
    jclass  browserClass  = env->FindClass("com/furture/litebrowser/LiteBrowser");
    liteBrowserJava->browserClass = (jclass) env->NewGlobalRef(browserClass);
    liteBrowserJava->createFontFaceMethodId = env->GetMethodID(liteBrowserJava->browserClass,  "createPaint", "(Ljava/lang/String;IIII)Landroid/graphics/Paint;");



    env->DeleteLocalRef(browserClass);

    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_furture_litebrowser_jni_NativeUtils_getStringBytes(JNIEnv *env, jclass type, jlong ptr) {
    const litehtml::tchar_t *text = (const litehtml::tchar_t *) ptr;
    int size = strlen(text);
    jbyteArray  array = env->NewByteArray(size);
    env->SetByteArrayRegion(array, 0, size, (const jbyte *) text);
    return  array;
}