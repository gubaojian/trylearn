# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.


set(ANDROID_ABI, "armeabi")
set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_C_FLAGS "-std=c99 -O3")

if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
    include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
else()
    include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
endif()

set(SOURCE_LITEHTML src/main/cpp/litehtml/background.cpp
                    src/main/cpp/litehtml/box.cpp
                    src/main/cpp/litehtml/context.cpp
                    src/main/cpp/litehtml/css_length.cpp
                    src/main/cpp/litehtml/css_selector.cpp
                    src/main/cpp/litehtml/document.cpp
                    src/main/cpp/litehtml/el_anchor.cpp
                    src/main/cpp/litehtml/el_base.cpp
                    src/main/cpp/litehtml/el_before_after.cpp
                    src/main/cpp/litehtml/el_body.cpp
                    src/main/cpp/litehtml/el_break.cpp
                    src/main/cpp/litehtml/el_cdata.cpp
                    src/main/cpp/litehtml/el_comment.cpp
                    src/main/cpp/litehtml/el_div.cpp
                    src/main/cpp/litehtml/element.cpp
                    src/main/cpp/litehtml/el_font.cpp
                    src/main/cpp/litehtml/el_image.cpp
                    src/main/cpp/litehtml/el_link.cpp
                    src/main/cpp/litehtml/el_para.cpp
                    src/main/cpp/litehtml/el_script.cpp
                    src/main/cpp/litehtml/el_space.cpp
                    src/main/cpp/litehtml/el_style.cpp
                    src/main/cpp/litehtml/el_table.cpp
                    src/main/cpp/litehtml/el_td.cpp
                    src/main/cpp/litehtml/el_text.cpp
                    src/main/cpp/litehtml/el_title.cpp
                    src/main/cpp/litehtml/el_tr.cpp
                    src/main/cpp/litehtml/html.cpp
                    src/main/cpp/litehtml/html_tag.cpp
                    src/main/cpp/litehtml/iterators.cpp
                    src/main/cpp/litehtml/media_query.cpp
                    src/main/cpp/litehtml/style.cpp
                    src/main/cpp/litehtml/stylesheet.cpp
                    src/main/cpp/litehtml/table.cpp
                    src/main/cpp/litehtml/utf8_strings.cpp
                    src/main/cpp/litehtml/web_color.cpp
)


set(SOURCE_GUMBO    src/main/cpp/gumbo/attribute.c
                    src/main/cpp/gumbo/char_ref.c
                    src/main/cpp/gumbo/error.c
                    src/main/cpp/gumbo/parser.c
                    src/main/cpp/gumbo/string_buffer.c
                    src/main/cpp/gumbo/string_piece.c
                    src/main/cpp/gumbo/tag.c
                    src/main/cpp/gumbo/tokenizer.c
                    src/main/cpp/gumbo/utf8.c
                    src/main/cpp/gumbo/util.c
                    src/main/cpp/gumbo/vector.c
)


set(SOURCE_JNI_HELPERS    src/main/cpp/helper/JniHelper.cpp
                          src/main/cpp/helper/ScopedJObject.cpp
)


set(SOURCE_FILES src/main/cpp/litebrowser.cpp  src/main/cpp/mobile_container.cpp ${SOURCE_JNI_HELPERS} ${SOURCE_LITEHTML} ${SOURCE_GUMBO})

add_library( # Sets the name of the library.
             litebrowser-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ${SOURCE_FILES})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       litebrowser-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )