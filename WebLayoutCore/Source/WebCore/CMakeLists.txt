cmake_minimum_required(VERSION 2.8.12)

include(WebKitCommon)
set_property(DIRECTORY . PROPERTY FOLDER "WebCore")

add_subdirectory(PAL/pal)

if (PAL_LIBRARY_TYPE MATCHES STATIC)
    add_definitions(-DSTATICALLY_LINKED_WITH_PAL=1)
endif ()


#css selector headers
set(WebCore_HEADERS
        ${WEBCORE_DIR}/platform/LayoutUnit.h
        ${WEBCORE_DIR}/css/parser/CSSAtRuleID.h
        ${WEBCORE_DIR}/css/parser/CSSDeferredParser.h
        ${WEBCORE_DIR}/css/parser/CSSParser.h
        ${WEBCORE_DIR}/css/parser/CSSParserFastPaths.h
        ${WEBCORE_DIR}/css/parser/CSSParserIdioms.h
        ${WEBCORE_DIR}/css/parser/CSSParserImpl.h
        ${WEBCORE_DIR}/css/parser/CSSParserObserverWrapper.h
        ${WEBCORE_DIR}/css/parser/CSSParserToken.h
        ${WEBCORE_DIR}/css/parser/CSSParserTokenRange.h
        ${WEBCORE_DIR}/css/parser/CSSParserSelector.h
        ${WEBCORE_DIR}/css/parser/CSSPropertyParser.h
        ${WEBCORE_DIR}/css/parser/CSSPropertyParserHelpers.h
        ${WEBCORE_DIR}/css/parser/CSSSelectorParser.h
        ${WEBCORE_DIR}/css/parser/CSSSupportsParser.h
        ${WEBCORE_DIR}/css/parser/CSSTokenizer.h
        ${WEBCORE_DIR}/css/parser/CSSTokenizerInputStream.h
        ${WEBCORE_DIR}/css/parser/CSSVariableParser.h
        ${WEBCORE_DIR}/css/parser/MediaQueryBlockWatcher.h
        ${WEBCORE_DIR}/css/parser/MediaQueryParser.h
        ${WEBCORE_DIR}/css/parser/SizesAttributeParser.h
        ${WEBCORE_DIR}/css/parser/SizesCalcParser.h
        ${WEBCORE_DIR}/css/parser/CSSParserToken.h
        ${WEBCORE_DIR}/css/CSSPropertyNames.h
        ${WEBCORE_DIR}/platform/graphics/LayoutSize.h)


## ${WEBCORE_DIR}/dom/AbortController.cpp
## ${WEBCORE_DIR}/dom/AbortSignal.cpp



INCLUDE_DIRECTORIES(${WEBCORE_DIR}/platform/ ${WEBCORE_DIR}/platform/text/ ${WEBCORE_DIR}/platform/graphics/  ${WEBCORE_DIR}/platform/animation/ ${WEBCORE_DIR}/rendering/style/
        ${WEBCORE_DIR}/loader/cache/ ${WEBCORE_DIR}/dom/ ${WEBCORE_DIR}/css/ ${WEBCORE_DIR}/page/ ${WEBCORE_DIR}/rendering/ ${WEBCORE_DIR}/css/parser/ ${WEBCORE_DIR}/rendering/line/
        ${WEBCORE_DIR}/rendering/shapes/)

set(WebCore_SOURCES
        ${WEBCORE_DIR}/rendering/shapes/ShapeOutsideInfo.cpp
        ${WEBCORE_DIR}/platform/graphics/LayoutRect.cpp
        ${WEBCORE_DIR}/platform/graphics/IntRect.cpp
        ${WEBCORE_DIR}/platform/graphics/LayoutSize.cpp
        ${WEBCORE_DIR}/platform/LayoutUnit.cpp
        ${WEBCORE_DIR}/rendering/line/LineWidth.cpp
        ${WEBCORE_DIR}/rendering/AutoTableLayout.cpp
        ${WEBCORE_DIR}/rendering/BidiRun.cpp
        ${WEBCORE_DIR}/rendering/BorderEdge.cpp
        ${WEBCORE_DIR}/rendering/BreakLines.cpp
        ${WEBCORE_DIR}/rendering/ClipRect.cpp
        ${WEBCORE_DIR}/rendering/CounterNode.cpp
        ${WEBCORE_DIR}/rendering/EllipsisBox.cpp
        ${WEBCORE_DIR}/rendering/FlexibleBoxAlgorithm.cpp
        ${WEBCORE_DIR}/rendering/RenderFlexibleBox.cpp
        ${WEBCORE_DIR}/rendering/FixedTableLayout.cpp
        ${WEBCORE_DIR}/rendering/FloatingObjects.cpp
        ${WEBCORE_DIR}/rendering/Grid.cpp
        ${WEBCORE_DIR}/rendering/GridLayoutFunctions.cpp
        ${WEBCORE_DIR}/rendering/GridTrackSizingAlgorithm.cpp
        ${WEBCORE_DIR}/rendering/RenderGrid.cpp
        ${WEBCORE_DIR}/rendering/RenderBlock.cpp
        ${WEBCORE_DIR}/rendering/RenderBlockFlow.cpp
        ${WEBCORE_DIR}/rendering/RenderBox.cpp
        ${WEBCORE_DIR}/rendering/RenderBoxModelObject.cpp
        ${WEBCORE_DIR}/rendering/RenderButton.cpp
        ${WEBCORE_DIR}/rendering/RenderCombineText.cpp
        ${WEBCORE_DIR}/rendering/RenderCounter.cpp
        ${WEBCORE_DIR}/rendering/RenderDeprecatedFlexibleBox.cpp
        ${WEBCORE_DIR}/rendering/RenderDetailsMarker.cpp
        ${WEBCORE_DIR}/rendering/RenderElement.cpp
        ${WEBCORE_DIR}/rendering/LayoutDisallowedScope.cpp
        ${WEBCORE_DIR}/rendering/LayoutRepainter.cpp
        ${WEBCORE_DIR}/rendering/LayoutState.cpp
        ${WEBCORE_DIR}/rendering/MarkerSubrange.cpp
        ${WEBCORE_DIR}/rendering/OrderIterator.cpp
        ${WEBCORE_DIR}/rendering/RenderInline.cpp
        ${WEBCORE_DIR}/rendering/InlineBox.cpp
        ${WEBCORE_DIR}/rendering/InlineElementBox.cpp
        ${WEBCORE_DIR}/rendering/InlineFlowBox.cpp
        ${WEBCORE_DIR}/rendering/InlineIterator.cpp
        ${WEBCORE_DIR}/rendering/InlineTextBox.cpp
        ${WEBCORE_DIR}/rendering/PointerEventsHitRules.cpp
        ${WEBCORE_DIR}/rendering/RenderEmbeddedObject.cpp
        ${WEBCORE_DIR}/rendering/RenderFileUploadControl.cpp
        ${WEBCORE_DIR}/rendering/RenderFragmentedFlow.cpp
        ${WEBCORE_DIR}/rendering/RenderFragmentContainer.cpp
        ${WEBCORE_DIR}/rendering/RenderFragmentContainerSet.cpp
        ${WEBCORE_DIR}/rendering/RenderFrame.cpp
        ${WEBCORE_DIR}/rendering/RenderFrameBase.cpp
        ${WEBCORE_DIR}/rendering/RenderFrameSet.cpp
        ${WEBCORE_DIR}/rendering/RenderFullScreen.cpp
        ${WEBCORE_DIR}/rendering/RenderGeometryMap.cpp
        ${WEBCORE_DIR}/rendering/RenderHTMLCanvas.cpp
        ${WEBCORE_DIR}/rendering/RenderIFrame.cpp
        ${WEBCORE_DIR}/rendering/RenderImage.cpp
        ${WEBCORE_DIR}/rendering/RenderBlockLineLayout.cpp
        ${WEBCORE_DIR}/rendering/RenderImageResource.cpp
        ${WEBCORE_DIR}/rendering/RenderImageResourceStyleImage.cpp
        ${WEBCORE_DIR}/rendering/RenderLayer.cpp
        ${WEBCORE_DIR}/rendering/RenderLayerBacking.cpp
        ${WEBCORE_DIR}/rendering/RenderLayerCompositor.cpp
        ${WEBCORE_DIR}/rendering/RenderLayerFilterInfo.cpp
        ${WEBCORE_DIR}/rendering/RenderLayerModelObject.cpp
        ${WEBCORE_DIR}/rendering/RenderLineBoxList.cpp
        ${WEBCORE_DIR}/rendering/RenderLineBreak.cpp
        ${WEBCORE_DIR}/rendering/RenderListBox.cpp
        ${WEBCORE_DIR}/rendering/RenderListItem.cpp
        ${WEBCORE_DIR}/rendering/RenderListMarker.cpp
        ${WEBCORE_DIR}/rendering/RenderMarquee.cpp
        ${WEBCORE_DIR}/rendering/RenderMedia.cpp
        ${WEBCORE_DIR}/rendering/RenderMediaControlElements.cpp
        ${WEBCORE_DIR}/rendering/RenderMediaControls.cpp
        ${WEBCORE_DIR}/rendering/RenderMenuList.cpp
        ${WEBCORE_DIR}/rendering/RenderMeter.cpp
        ${WEBCORE_DIR}/rendering/RenderMultiColumnFlow.cpp
        ${WEBCORE_DIR}/rendering/RenderMultiColumnSet.cpp
        ${WEBCORE_DIR}/rendering/RenderMultiColumnSpannerPlaceholder.cpp
        ${WEBCORE_DIR}/rendering/RenderObject.cpp
        ${WEBCORE_DIR}/rendering/RenderProgress.cpp
        ${WEBCORE_DIR}/rendering/RenderQuote.cpp
        ${WEBCORE_DIR}/rendering/RenderReplaced.cpp
        ${WEBCORE_DIR}/rendering/RenderReplica.cpp
        ${WEBCORE_DIR}/rendering/RenderRuby.cpp
        ${WEBCORE_DIR}/rendering/RenderRubyBase.cpp
        ${WEBCORE_DIR}/rendering/RenderRubyRun.cpp
        ${WEBCORE_DIR}/rendering/RenderRubyText.cpp
        ${WEBCORE_DIR}/rendering/RenderScrollbar.cpp
        ${WEBCORE_DIR}/rendering/RenderScrollbarPart.cpp
        ${WEBCORE_DIR}/rendering/RenderScrollbarTheme.cpp
        ${WEBCORE_DIR}/rendering/RenderSearchField.cpp
        ${WEBCORE_DIR}/rendering/RenderSelectionInfo.cpp
        ${WEBCORE_DIR}/rendering/RenderSlider.cpp
        ${WEBCORE_DIR}/rendering/RenderSnapshottedPlugIn.cpp
        ${WEBCORE_DIR}/rendering/RenderTable.cpp
        ${WEBCORE_DIR}/rendering/RenderTableCaption.cpp
        ${WEBCORE_DIR}/rendering/RenderTableCell.cpp
        ${WEBCORE_DIR}/rendering/RenderTableCol.cpp
        ${WEBCORE_DIR}/rendering/RenderTableRow.cpp
        ${WEBCORE_DIR}/rendering/RenderTableSection.cpp
        ${WEBCORE_DIR}/rendering/RenderText.cpp
        ${WEBCORE_DIR}/rendering/RenderTextControl.cpp
        ${WEBCORE_DIR}/rendering/RenderTextControlMultiLine.cpp
        ${WEBCORE_DIR}/rendering/RenderTextControlSingleLine.cpp
        ${WEBCORE_DIR}/rendering/RenderTextFragment.cpp
        ${WEBCORE_DIR}/rendering/RenderTextLineBoxes.cpp
        ${WEBCORE_DIR}/rendering/RenderTheme.cpp
        ${WEBCORE_DIR}/rendering/RenderTreeAsText.cpp
        ${WEBCORE_DIR}/rendering/RenderVTTCue.cpp
        ${WEBCORE_DIR}/rendering/RenderVideo.cpp
        ${WEBCORE_DIR}/rendering/RenderView.cpp
        ${WEBCORE_DIR}/rendering/RenderWidget.cpp
        ${WEBCORE_DIR}/rendering/RootInlineBox.cpp
        ${WEBCORE_DIR}/rendering/ScrollAlignment.cpp
        ${WEBCORE_DIR}/rendering/SelectionRangeData.cpp
        ${WEBCORE_DIR}/rendering/SimpleLineLayout.cpp
        ${WEBCORE_DIR}/rendering/SimpleLineLayoutCoverage.cpp
        ${WEBCORE_DIR}/rendering/SimpleLineLayoutFlowContents.cpp
        ${WEBCORE_DIR}/rendering/SimpleLineLayoutFunctions.cpp
        ${WEBCORE_DIR}/rendering/SimpleLineLayoutPagination.cpp
        ${WEBCORE_DIR}/rendering/SimpleLineLayoutResolver.cpp
        ${WEBCORE_DIR}/rendering/SimpleLineLayoutTextFragmentIterator.cpp
        ${WEBCORE_DIR}/rendering/TextDecorationPainter.cpp
        ${WEBCORE_DIR}/rendering/TextPaintStyle.cpp
        ${WEBCORE_DIR}/rendering/TextPainter.cpp
        ${WEBCORE_DIR}/rendering/line/LineBreaker.cpp
        ${WEBCORE_DIR}/rendering/line/LineInfo.cpp
        ${WEBCORE_DIR}/rendering/line/LineWidth.cpp
        ${WEBCORE_DIR}/rendering/line/TrailingObjects.cpp
        ${WEBCORE_DIR}/rendering/style/RenderStyleConstants.cpp
        ${WEBCORE_DIR}/rendering/HitTestLocation.cpp
        ${WEBCORE_DIR}/rendering/HitTestResult.cpp
)


message("webcore dir rect ${WEBCORE_DIR}  ${WebCore_SOURCES}")




set(WebCore_SYSTEM_INCLUDE_DIRECTORIES
    ${ICU_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
    ${LIBXSLT_INCLUDE_DIR}
    ${SQLITE_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

set(WebCore_LIBRARIES
    ${WebCore_SOURCES}
    ${ICU_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${LIBXSLT_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${ZLIB_LIBRARIES}
    JavaScriptCore${DEBUG_SUFFIX}
    PAL${DEBUG_SUFFIX}
)



WEBKIT_INCLUDE_CONFIG_FILES_IF_EXISTS()
WEBKIT_WRAP_SOURCELIST(${WebCore_SOURCES})
WEBKIT_FRAMEWORK(WebCore)

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


target_link_libraries(WebCore WTF icucore)
target_link_libraries(WebCore ${WebCore_LIBRARIES})

if (SHARED_CORE)
    set_target_properties(WebCore PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
    install(TARGETS WebCore DESTINATION "${LIB_INSTALL_DIR}")
endif ()

if (MSVC)
    set_target_properties(WebCore PROPERTIES
        COMPILE_PDB_NAME ${WebCore_OUTPUT_NAME}
    )
endif ()
