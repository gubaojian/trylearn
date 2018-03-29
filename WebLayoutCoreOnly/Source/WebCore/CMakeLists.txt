cmake_minimum_required(VERSION 2.8.12)

include(WebKitCommon)
set_property(DIRECTORY . PROPERTY FOLDER "WebCore")

find_library(WTF REQUIRED)


set(WebCore_HEADERS
        config.h
        RenderStyle.h
        RoundedRect.h
        FloatRect.h
        FloatSize.h
        FloatQuad.h
        FloatPoint.h
        FloatPoint3D.h
        FloatRoundedRect.h
        GeometryUtilities.h
        Length.h
        LengthSize.h
        LayoutPoint.h
        LayoutRect.h
        LayoutSize.h
        LengthFunctions.h
        IntPoint.h
        IntRect.h
        IntSize.h
        LayoutUnit.h
        CalculationValue.h
        StyleBoxData.h
        StyleVisualData.h
        StyleSurroundData.h
        StyleInheritedData.h
        BorderData.h
        BorderValue.h
        StyleRareInheritedData.h
        StyleRareNonInheritedData.h
        StyleBackgroundData.h
        StyleGridData.h
        StyleGridItemData.h
        StyleMultiColData.h
        StyleFlexibleBoxData.h
        StyleDeprecatedFlexibleBoxData.h
        StyleSelfAlignmentData.h
        GridArea.h
        GridLength.h
        GridPosition.h
        GridPositionsResolver.h
        GridTrackSize.h
        QuotesData.h
        Color.h
        ExtendedColor.h
        ColorSpace.h
        CounterDirectives.h
        TransformationMatrix.h
        AffineTransform.h
        AnimationUtilities.h
        RenderStyleConstants.h
        BasicShapes.h
        BasicShapeFunctions.h
        CSSPrimitiveValue.h
        CSSValueKeywords.h
        DataRef.h
        Exception.h
        ExceptionCode.h
        ExceptionOr.h
        FloatConversion.h
        HashTools.h
        RenderObjectEnums.h
        RenderObject.h
)

set(WebCore_SOURCES
        RenderStyle.cpp
        RoundedRect.cpp
        FloatRect.cpp
        FloatSize.cpp
        FloatQuad.cpp
        FloatPoint.cpp
        FloatPoint3D.cpp
        FloatRoundedRect.cpp
        GeometryUtilities.cpp
        Length.cpp
        LengthSize.cpp
        LengthFunctions.cpp
        LayoutPoint.cpp
        LayoutRect.cpp
        LayoutSize.cpp
        IntPoint.cpp
        IntRect.cpp
        IntSize.cpp
        LayoutUnit.cpp
        CalculationValue.cpp
        StyleBoxData.cpp
        StyleVisualData.cpp
        StyleSurroundData.cpp
        StyleInheritedData.cpp
        StyleRareInheritedData.cpp
        StyleRareNonInheritedData.cpp
        StyleBackgroundData.cpp
        StyleGridData.cpp
        StyleGridItemData.cpp
        StyleMultiColData.cpp
        StyleFlexibleBoxData.cpp
        StyleDeprecatedFlexibleBoxData.cpp
        GridPosition.cpp
        QuotesData.cpp
        Color.cpp
        ExtendedColor.cpp
        ColorData.cpp
        CounterDirectives.cpp
        TransformationMatrix.cpp
        AffineTransform.cpp
        RenderStyleConstants.cpp
        BasicShapeFunctions.cpp
        BasicShapes.cpp
        CSSPrimitiveValue.cpp
        CSSValueKeywords.cpp
        RenderObject.cpp
)


## GridPositionsResolver.cpp

set(WebCore_INCLUDE_DIRECTORIES
        "${CMAKE_BINARY_DIR}"
        ${C_INCLUDE_DIRS}
        ${WTF_INCLUDE_DIRS}
        "${DERIVED_SOURCES_DIR}"
        "${THIRDPARTY_DIR}"
        )

set(WebCore_LIBRARIES
        WTF
        icucore
        ${CMAKE_DL_LIBS}
)


WEBKIT_INCLUDE_CONFIG_FILES_IF_EXISTS()
WEBKIT_WRAP_SOURCELIST(${WebCore_SOURCES})
WEBKIT_FRAMEWORK(WebCore)

target_link_libraries(WebCore WTF)

message("linkd library ${WebCore_LIBRARIES}")


if (APPLE)
    set_target_properties(WebCore PROPERTIES LINK_FLAGS "-sub_library libobjc -umbrella WebKit -allowable_client WebCoreTestSupport -allowable_client WebKit2 -allowable_client WebKitLegacy")
endif ()


# The -ftree-sra optimization (implicit with -O2) causes crashes when
# allocating large chunks of memory using bmalloc on Intel 32bit.
# See https://bugs.webkit.org/show_bug.cgi?id=146440
string(TOLOWER ${CMAKE_HOST_SYSTEM_PROCESSOR} LOWERCASE_CMAKE_HOST_SYSTEM_PROCESSOR)
if (CMAKE_COMPILER_IS_GNUCXX AND "${LOWERCASE_CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "(i[3-6]86|x86)$")
    WEBKIT_ADD_TARGET_CXX_FLAGS(WebCore -fno-tree-sra)
endif ()

