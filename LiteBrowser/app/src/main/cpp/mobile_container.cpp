//
// Created by furture on 2017/9/22.
//

#include "mobile_container.h"
#include "helper/JniHelper.h"
#include "helper/ScopedJObject.h"


static inline int native_color(web_color web_color){
    jint  native_color = 0;
    if(web_color.alpha){
        int alpha = (((int)web_color.alpha) << 24); //&0XFF000000;
        int red = (((int)web_color.red) << 16); //&0X00FF0000;
        int green = (((int)web_color.green) << 8); //&0X0000FF00;
        int blue = (((int)web_color.blue)); //&0X000000FF;
        native_color =  alpha | red | green | blue;
    }
    return native_color;
}

mobile_container::mobile_container(LiteBrowserJava* liteBrowserJava, jobject thisObject):_liteBrowserJava(liteBrowserJava){
    JNIEnv* env = get_java_jni_env();
    _thisObject = env->NewGlobalRef(thisObject);
    clientFrame = new litehtml::position(0, 0, liteBrowserJava->screen.width, liteBrowserJava->screen.height);
}
mobile_container::~mobile_container() {
    JNIEnv* env = get_java_jni_env();
    env->DeleteGlobalRef(_thisObject);
    delete clientFrame;
}

litehtml::uint_ptr	mobile_container::create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm){
    JNIEnv* env = get_java_jni_env();
    jstring faceNameStr = nullptr;
    if(faceName){
        DEBUG_LOG("fontSize %s", faceName);
        faceNameStr = env->NewStringUTF(faceName);
    }

    static jmethodID fontMetricsIntMethodId;
    static jfieldID fontMetricsIntTopFieldID;
    static jfieldID fontMetricsIntAscentFieldID;
    static jfieldID fontMetricsIntDescentFieldID;
    static jfieldID fontMetricsIntBottomFieldID;
    static jfieldID fontMetricsIntLeadingFieldID;
    static jmethodID fontMetricsXHeightMethodId;
    DEBUG_LOG("fontSize %s %d  weight %d", faceName, size,  weight);
    jobject paint = env->CallObjectMethod(_thisObject, _liteBrowserJava->createFontFaceMethodId, faceNameStr, size, weight, italic, decoration);
    if(!fontMetricsIntMethodId){
        jclass  paintClass = env->GetObjectClass(paint);
        fontMetricsIntMethodId = env->GetMethodID(paintClass,  "getFontMetricsInt", "()Landroid/graphics/Paint$FontMetricsInt;");
        jclass fontMetricsIntClass = env->FindClass("android/graphics/Paint$FontMetricsInt");
        fontMetricsIntTopFieldID = env->GetFieldID(fontMetricsIntClass, "top", "I");
        fontMetricsIntAscentFieldID = env->GetFieldID(fontMetricsIntClass, "ascent", "I");
        fontMetricsIntDescentFieldID = env->GetFieldID(fontMetricsIntClass, "descent", "I");
        fontMetricsIntBottomFieldID = env->GetFieldID(fontMetricsIntClass, "bottom", "I");
        fontMetricsIntLeadingFieldID = env->GetFieldID(fontMetricsIntClass, "leading", "I");
        env->DeleteLocalRef(fontMetricsIntClass);
        env->DeleteLocalRef(paintClass);
        fontMetricsXHeightMethodId = env->GetMethodID(_liteBrowserJava->browserClass, "getXHeight", "(Landroid/graphics/Paint;)I");
    }
    jobject  fontMetricsInt = env->CallObjectMethod(paint, fontMetricsIntMethodId);
    fm->ascent = abs(env->GetIntField(fontMetricsInt, fontMetricsIntAscentFieldID));
    fm->descent = abs(env->GetIntField(fontMetricsInt, fontMetricsIntDescentFieldID));
    fm->height = abs(fm->ascent) + abs(fm->descent) ;//+ abs(env->GetIntField(fontMetricsInt, fontMetricsIntLeadingFieldID));
    fm->x_height = env->CallIntMethod(_thisObject, fontMetricsXHeightMethodId, paint);
    if(faceNameStr){
        env->DeleteLocalRef(faceNameStr);
    }
    ScopedJObject* scopedJObject = new ScopedJObject(paint);
    env->DeleteLocalRef(paint);
    return  scopedJObject;
}

void mobile_container::delete_font(litehtml::uint_ptr ptr){
    if(ptr){
        ScopedJObject* scopedJObject = (ScopedJObject *) ptr;
        delete scopedJObject;
    }
}


int mobile_container::text_width(const litehtml::tchar_t *text, litehtml::uint_ptr hFont) {

    if(!text){
        return 0;
    }
    JNIEnv* env = get_java_jni_env();
    static jmethodID  measureTextWidthMethodId;
    if(!measureTextWidthMethodId){
        measureTextWidthMethodId = env->GetMethodID(_liteBrowserJava->browserClass, "getTextWidth", "(Landroid/graphics/Paint;[B)I");
    }
    jobject *font = nullptr;
    if(hFont){
        ScopedJObject* scopedJObject = (ScopedJObject *) hFont;
        font = (jobject *) scopedJObject->getObject();
    }
    int size = strlen(text);
    jbyteArray  array = env->NewByteArray(size);
    env->SetByteArrayRegion(array, 0, size, (const jbyte *) text);
    int width = env->CallIntMethod(_thisObject, measureTextWidthMethodId, font, array);
    env->DeleteLocalRef(array);
    //DEBUG_LOG("text_width %s %d", text, width);
    return  width;
}

void mobile_container::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t *text, litehtml::uint_ptr hFont,
                                 litehtml::web_color color, const litehtml::position &pos) {
    if(!text){
        return;
    }
    JNIEnv* env = get_java_jni_env();
    static jmethodID drawTextMethodId;
    if(!drawTextMethodId){
        drawTextMethodId = env->GetMethodID(_liteBrowserJava->browserClass, "drawText", "(Landroid/graphics/Canvas;Landroid/graphics/Paint;[BIIII)V");
    }
    jobject  canvas = (jobject) hdc;
    jobject paint = ((ScopedJObject *) hFont)->getObject();
    int size = strlen(text);
    jbyteArray  array = env->NewByteArray(size);
    env->SetByteArrayRegion(array, 0, size, (const jbyte *) text);
    env->CallVoidMethod(_thisObject, drawTextMethodId,
                        canvas, paint, array, pos.x, pos.y, pos.width, pos.height);
    env->DeleteLocalRef(array);

    //DEBUG_LOG("draw_text %s  %d %d %d %d\n", text, pos.x, pos.y, pos.height, pos.width);
}


int	mobile_container::pt_to_px(int pt){
    return (int)(pt*_liteBrowserJava->screen.density);
}



const litehtml::tchar_t* mobile_container::get_default_font_name() const {
    return  "sans";
}

void mobile_container::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker &marker) {
    DEBUG_LOG("draw_list_marker \n");
}

void mobile_container::load_image(const litehtml::tchar_t *src, const litehtml::tchar_t *baseurl, bool redraw_on_ready) {
    DEBUG_LOG("load_image  %s   %s  %d\n", src, baseurl, redraw_on_ready);
}

void mobile_container::get_image_size(const litehtml::tchar_t *src, const litehtml::tchar_t *baseurl, litehtml::size &sz) {
    DEBUG_LOG("get_image_size  %s \n", src);
    sz.width = 100;
    sz.height = 100;
}



void mobile_container::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders &borders,
                                    const litehtml::position &draw_pos, bool root) {
    if(borders.left.style <= litehtml::border_style_hidden
            && borders.right.style <= litehtml::border_style_hidden
            && borders.top.style <= litehtml::border_style_hidden
            && borders.bottom.style <= litehtml::border_style_hidden){
        return;
    }
    if(borders.left.width <= 0
       && borders.right.style <= 0
       && borders.top.style <= 0
       && borders.bottom.style <= 0){
        return;
    }


    JNIEnv* env = get_java_jni_env();
    static jmethodID  drawBordersBackgroundId;
    if(!drawBordersBackgroundId){
        drawBordersBackgroundId = env->GetMethodID(_liteBrowserJava->browserClass,
                                                   "drawBorders", "(Landroid/graphics/Canvas;IIIIIIIIIIIIIIIIIIII)V");
    }
    jobject  canvas = (jobject) hdc;
    env->CallVoidMethod(_thisObject, drawBordersBackgroundId,canvas,
                        draw_pos.x, draw_pos.y, draw_pos.width, draw_pos.height,
                        native_color(borders.left.color), borders.left.style, borders.left.width,
                        native_color(borders.right.color), borders.right.style, borders.right.width,
                        native_color(borders.top.color), borders.top.style, borders.top.width,
                        native_color(borders.bottom.color), borders.bottom.style, borders.bottom.width,
                        borders.radius.top_left_x, borders.radius.top_right_x, borders.radius.bottom_left_x, borders.radius.bottom_right_x);


    DEBUG_LOG("draw_borders done\n");
}

void mobile_container::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint &bg) {
     JNIEnv* env = get_java_jni_env();
     static jmethodID  drawBackgroundMethodId;
     if(!drawBackgroundMethodId){
         drawBackgroundMethodId =  env->GetMethodID(_liteBrowserJava->browserClass, "drawBackground", "(Landroid/graphics/Canvas;I[Ljava/lang/String;IIIIIIIIIIII)V");
     }

    jobject  canvas = (jobject) hdc;
    jint  color = 0;
    DEBUG_LOG("bg.color.alpha2 %d  %d %d  %d", bg.color.alpha, bg.color.red,
              bg.color.green, bg.color.blue);
    if(bg.color.alpha){
        int alpha = (((int)bg.color.alpha) << 24); //&0XFF000000;
        int red = (((int)bg.color.red) << 16); //&0X00FF0000;
        int green = (((int)bg.color.green) << 8); //&0X0000FF00;
        int blue = (((int)bg.color.blue)); //&0X000000FF;
        color =  alpha | red | green | blue;
        DEBUG_LOG("bg.color.alpha value %d %d  %d %d %d", color, alpha, red, green, blue);
    }
    env->CallVoidMethod(_thisObject, drawBackgroundMethodId, canvas, color, nullptr, bg.clip_box.x,
                        bg.clip_box.y, bg.clip_box.width, bg.clip_box.height,
                        bg.border_radius.bottom_left_x, bg.border_radius.bottom_left_y,
                        bg.border_radius.bottom_right_x, bg.border_radius.bottom_right_y,
                        bg.border_radius.top_left_x, bg.border_radius.top_left_y, bg.border_radius.top_right_x, bg.border_radius.top_right_y);
    DEBUG_LOG("draw_background \n");
}

void mobile_container::set_caption(const litehtml::tchar_t *caption) {
    DEBUG_LOG("set_caption \n");
}

void mobile_container::set_base_url(const litehtml::tchar_t *base_url) {
    DEBUG_LOG("set_base_url  %s \n", base_url);
}

void mobile_container::link(const std::shared_ptr<litehtml::document> &doc, const litehtml::element::ptr &el) {
    const tchar_t* href		= el->get_attr(_t("href"));
    DEBUG_LOG("link  %s \n", href);
}

void mobile_container::on_anchor_click(const litehtml::tchar_t *url, const litehtml::element::ptr &el) {
    DEBUG_LOG("on_anchor_click \n");
}

void mobile_container::set_cursor(const litehtml::tchar_t *cursor) {
    DEBUG_LOG("set_cursor \n");
}

void mobile_container::transform_text(litehtml::tstring &text, litehtml::text_transform tt) {
    DEBUG_LOG("transform_text \n");
}

void mobile_container::import_css(litehtml::tstring &text, const litehtml::tstring &url, litehtml::tstring &baseurl) {
    if(url.length() == 0
            || !url.c_str()){
        return;
    }
    JNIEnv* env = get_java_jni_env();
    static jmethodID  importCssMethodId;
    if(!importCssMethodId){
        importCssMethodId = env->GetMethodID(_liteBrowserJava->browserClass,
        "importCss", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    }
    DEBUG_LOG("import_css %s ", url.c_str());
    //DEBUG_LOG("import_css  %s  url  base %d  %s \n", url.c_str(), baseurl.length(), baseurl.c_str());
    jstring  jUrl = env->NewStringUTF(url.c_str());
    //DEBUG_LOG("import_css  new StringUTF8");
    jstring  jBaseUrl = nullptr;
    if(baseurl.length() > 0){
        //DEBUG_LOG("import_css base url");
        jBaseUrl = env->NewStringUTF(baseurl.c_str());
    }
    jstring result = (jstring) env->CallObjectMethod(_thisObject, importCssMethodId, jUrl, jBaseUrl);
    if(result){
        jboolean isCopy = false;
        const char* utf8Data = env->GetStringUTFChars(result, &isCopy);
        text.append(utf8Data);
        env->ReleaseStringUTFChars(result, utf8Data);
        env->DeleteLocalRef(result);
    }
    env->DeleteLocalRef(jUrl);
    if(jBaseUrl){
        env->DeleteLocalRef(jBaseUrl);
    }
    //DEBUG_LOG("import_css  %s  url %s base %s \n", text.c_str(), url.c_str(), baseurl.c_str());
}

void
mobile_container::set_clip(const litehtml::position &pos, const litehtml::border_radiuses &bdr_radius, bool valid_x,
                           bool valid_y) {
    DEBUG_LOG("set_clip \n");
}

void mobile_container::del_clip() {
    DEBUG_LOG("del_clip \n");
}

void mobile_container::get_client_rect(litehtml::position &client) const {
    client.height = clientFrame->height;
    client.width = clientFrame->width;
    client.y = 0;
    client.x = 0;
}

std::shared_ptr<element>
mobile_container::create_element(const litehtml::tchar_t *tag_name, const litehtml::string_map &attributes,
                                 const std::shared_ptr<litehtml::document> &doc) {
    //LOGE("LiteBrowser", "create_element %s", tag_name);
    //create element
    return nullptr;
}

void mobile_container::get_media_features(litehtml::media_features &media) const {
    LOGE("LiteBrowser", "LiteBrowser get_media_features");
    media.width = _liteBrowserJava->screen.width/_liteBrowserJava->screen.density;
    media.height = _liteBrowserJava->screen.height/_liteBrowserJava->screen.density;
    media.device_width = _liteBrowserJava->screen.width/_liteBrowserJava->screen.density;
    media.device_height = _liteBrowserJava->screen.height/_liteBrowserJava->screen.density;
    media.resolution = media.device_width*media.device_height;
    media.type = media_type_screen;
    media.color			= 8;
    media.monochrome	= 0;
    media.color_index	= 256;
}

void mobile_container::get_language(litehtml::tstring &language, litehtml::tstring &culture) const {
    language.append("en");
    culture.append("am");
}

int  mobile_container::get_default_font_size() const {
    return _liteBrowserJava->screen.defaultFontSize;
}
