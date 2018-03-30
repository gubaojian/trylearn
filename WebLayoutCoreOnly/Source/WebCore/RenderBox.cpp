/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2005 Allan Sandfeld Jensen (kde@carewolf.com)
 *           (C) 2005, 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2005-2010, 2015 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "RenderBox.h"
#include "LayoutContext.h"

//#include "CSSFontSelector.h"
//#include "ControlStates.h"
//#include "Document.h"
//#include "Editing.h"
//#include "EventHandler.h"
#include "FloatQuad.h"
#include "FloatRoundedRect.h"
//#include "Frame.h"
//#include "FrameView.h"
//#include "GraphicsContext.h"
//#include "HTMLBodyElement.h"
//#include "HTMLButtonElement.h"
//#include "HTMLFrameOwnerElement.h"
//#include "HTMLHtmlElement.h"
//#include "HTMLImageElement.h"
//#include "HTMLInputElement.h"
//#include "HTMLLegendElement.h"
//#include "HTMLNames.h"
//#include "HTMLSelectElement.h"
//#include "HTMLTextAreaElement.h"
#include "HitTestResult.h"
//#include "InlineElementBox.h"
#include "LayoutState.h"
//#include "MainFrame.h"
//#include "Page.h"
//#include "PaintInfo.h"
//#include "RenderBoxFragmentInfo.h"
#include "RenderChildIterator.h"
#include "RenderDeprecatedFlexibleBox.h"
#include "RenderFlexibleBox.h"
#include "RenderFragmentContainer.h"
//#include "RenderGeometryMap.h"
#include "RenderInline.h"
#include "RenderIterator.h"
//#include "RenderLayer.h"
//#include "RenderLayerCompositor.h"
//#include "RenderMultiColumnFlow.h"
#include "RenderTableCell.h"
//#include "RenderTheme.h"
//#include "RenderView.h"
//#include "RuntimeApplicationChecks.h"
//#include "ScrollAnimator.h"
//#include "ScrollbarTheme.h"
//#include "StyleScrollSnapPoints.h"
//#include "TransformState.h"
#include <algorithm>
#include <math.h>
#include <wtf/IsoMallocInlines.h>
#include <wtf/StackStats.h>

#include "RenderGrid.h"

#if PLATFORM(IOS)
#include "Settings.h"
#endif

namespace WebCore {

WTF_MAKE_ISO_ALLOCATED_IMPL(RenderBox);

struct SameSizeAsRenderBox : public RenderBoxModelObject {
    virtual ~SameSizeAsRenderBox() = default;
    LayoutRect frameRect;
    LayoutBoxExtent marginBox;
    LayoutUnit preferredLogicalWidths[2];
    void* pointers[2];
};

COMPILE_ASSERT(sizeof(RenderBox) == sizeof(SameSizeAsRenderBox), RenderBox_should_stay_small);

//using namespace HTMLNames;

// Used by flexible boxes when flexing this element and by table cells.
typedef WTF::HashMap<const RenderBox*, LayoutUnit> OverrideSizeMap;
static OverrideSizeMap* gOverrideHeightMap = nullptr;
static OverrideSizeMap* gOverrideWidthMap = nullptr;

// Used by grid elements to properly size their grid items.
typedef WTF::HashMap<const RenderBox*, std::optional<LayoutUnit>> OverrideOptionalSizeMap;
static OverrideOptionalSizeMap* gOverrideContainingBlockLogicalHeightMap = nullptr;
static OverrideOptionalSizeMap* gOverrideContainingBlockLogicalWidthMap = nullptr;

// Size of border belt for autoscroll. When mouse pointer in border belt,
// autoscroll is started.
static const int autoscrollBeltSize = 20;
static const unsigned backgroundObscurationTestMaxDepth = 4;




bool RenderBox::s_hadOverflowClip = false;

RenderBox::RenderBox(RenderStyle&& style, BaseTypeFlags baseTypeFlags)
    : RenderBoxModelObject(WTFMove(style), baseTypeFlags)
{
    setIsBox();
}


RenderBox::~RenderBox()
{
    // Do not add any code here. Add it to willBeDestroyed() instead.
}

void RenderBox::willBeDestroyed()
{
    /**
    if (frame().eventHandler().autoscrollRenderer() == this)
        frame().eventHandler().stopAutoscrollTimer(true);

    clearOverrideSize();
    clearContainingBlockOverrideSize();

    RenderBlock::removePercentHeightDescendantIfNeeded(*this);

    ShapeOutsideInfo::removeInfo(*this);

    view().unscheduleLazyRepaint(*this);
    removeControlStatesForRenderer(*this);

#if ENABLE(CSS_SCROLL_SNAP)
    if (hasInitializedStyle() && style().scrollSnapArea().hasSnapPosition())
        view().unregisterBoxWithScrollSnapPositions(*this);
#endif

    RenderBoxModelObject::willBeDestroyed();
     */
}

RenderFragmentContainer* RenderBox::clampToStartAndEndFragments(RenderFragmentContainer* fragment) const
{
    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();

    ASSERT(isRenderView() || (fragment && fragmentedFlow));
    if (isRenderView())
        return fragment;

    // We need to clamp to the block, since we want any lines or blocks that overflow out of the
    // logical top or logical bottom of the block to size as though the border box in the first and
    // last fragments extended infinitely. Otherwise the lines are going to size according to the fragments
    // they overflow into, which makes no sense when this block doesn't exist in |fragment| at all.
    RenderFragmentContainer* startFragment = nullptr;
    RenderFragmentContainer* endFragment = nullptr;
    /**
    if (!fragmentedFlow->getFragmentRangeForBox(this, startFragment, endFragment))
        return fragment;
   */
    if (fragment->logicalTopForFragmentedFlowContent() < startFragment->logicalTopForFragmentedFlowContent())
        return startFragment;
    if (fragment->logicalTopForFragmentedFlowContent() > endFragment->logicalTopForFragmentedFlowContent())
        return endFragment;

    return fragment;
}

bool RenderBox::hasFragmentRangeInFragmentedFlow() const
{
    /**
    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();
    if (!fragmentedFlow || !fragmentedFlow->hasValidFragmentInfo())
        return false;

    return fragmentedFlow->hasCachedFragmentRangeForBox(*this);
     */
return false;
}

LayoutRect RenderBox::clientBoxRectInFragment(RenderFragmentContainer* fragment) const
{
    if (!fragment)
        return clientBoxRect();

    LayoutRect clientBox = borderBoxRectInFragment(fragment);
    clientBox.setLocation(clientBox.location() + LayoutSize(borderLeft(), borderTop()));
    clientBox.setSize(clientBox.size() - LayoutSize(borderLeft() + borderRight() + verticalScrollbarWidth(), borderTop() + borderBottom() + horizontalScrollbarHeight()));

    return clientBox;
}

LayoutRect RenderBox::borderBoxRectInFragment(RenderFragmentContainer*, RenderBoxFragmentInfoFlags) const
{
    return borderBoxRect();
}

static RenderBlockFlow* outermostBlockContainingFloatingObject(RenderBox& box)
{
    ASSERT(box.isFloating());
    RenderBlockFlow* parentBlock = nullptr;
    for (auto& ancestor : ancestorsOfType<RenderBlockFlow>(box)) {
        if (ancestor.isRenderView())
            break;
        if (!parentBlock || ancestor.containsFloat(box))
            parentBlock = &ancestor;
    }
    return parentBlock;
}

void RenderBox::removeFloatingOrPositionedChildFromBlockLists()
{
    ASSERT(isFloatingOrOutOfFlowPositioned());

    if (renderTreeBeingDestroyed())
        return;

    if (isFloating()) {
        if (RenderBlockFlow* parentBlock = outermostBlockContainingFloatingObject(*this)) {
            parentBlock->markSiblingsWithFloatsForLayout(this);
            parentBlock->markAllDescendantsWithFloatsForLayout(this, false);
        }
    }

    if (isOutOfFlowPositioned())
        RenderBlock::removePositionedObject(*this);
}

void RenderBox::styleWillChange(StyleDifference diff, const RenderStyle& newStyle)
{
    s_hadOverflowClip = hasOverflowClip();

    /**
    const RenderStyle* oldStyle = hasInitializedStyle() ? &style() : nullptr;
    if (oldStyle) {
        // The background of the root element or the body element could propagate up to
        // the canvas. Issue full repaint, when our style changes substantially.
        if (diff >= StyleDifferenceRepaint && (isDocumentElementRenderer() || isBody())) {
            view().repaintRootContents();
            if (oldStyle->hasEntirelyFixedBackground() != newStyle.hasEntirelyFixedBackground())
                view().compositor().rootFixedBackgroundsChanged();
        }
        
        // When a layout hint happens and an object's position style changes, we have to do a layout
        // to dirty the render tree using the old position value now.
        if (diff == StyleDifferenceLayout && parent() && oldStyle->position() != newStyle.position()) {
            markContainingBlocksForLayout();
            if (oldStyle->position() == StaticPosition)
                repaint();
            else if (newStyle.hasOutOfFlowPosition())
                parent()->setChildNeedsLayout();
            if (isFloating() && !isOutOfFlowPositioned() && newStyle.hasOutOfFlowPosition())
                removeFloatingOrPositionedChildFromBlockLists();
        }
    } else if (isBody())
        view().repaintRootContents();

#if ENABLE(CSS_SCROLL_SNAP)
    bool boxContributesSnapPositions = newStyle.scrollSnapArea().hasSnapPosition();
    if (boxContributesSnapPositions || (oldStyle && oldStyle->scrollSnapArea().hasSnapPosition())) {
        if (boxContributesSnapPositions)
            view().registerBoxWithScrollSnapPositions(*this);
        else
            view().unregisterBoxWithScrollSnapPositions(*this);
    }
#endif

    RenderBoxModelObject::styleWillChange(diff, newStyle);
     */
}

void RenderBox::styleDidChange(StyleDifference diff, const RenderStyle* oldStyle)
{
    /***
    // Horizontal writing mode definition is updated in RenderBoxModelObject::updateFromStyle,
    // (as part of the RenderBoxModelObject::styleDidChange call below). So, we can safely cache the horizontal
    // writing mode value before style change here.
    bool oldHorizontalWritingMode = isHorizontalWritingMode();

    RenderBoxModelObject::styleDidChange(diff, oldStyle);

    const RenderStyle& newStyle = style();
    if (needsLayout() && oldStyle) {
        RenderBlock::removePercentHeightDescendantIfNeeded(*this);

        // Normally we can do optimized positioning layout for absolute/fixed positioned objects. There is one special case, however, which is
        // when the positioned object's margin-before is changed. In this case the parent has to get a layout in order to run margin collapsing
        // to determine the new static position.
        if (isOutOfFlowPositioned() && newStyle.hasStaticBlockPosition(isHorizontalWritingMode()) && oldStyle->marginBefore() != newStyle.marginBefore()
            && parent() && !parent()->normalChildNeedsLayout())
            parent()->setChildNeedsLayout();
    }

    if (RenderBlock::hasPercentHeightContainerMap() && firstChild()
        && oldHorizontalWritingMode != isHorizontalWritingMode())
        RenderBlock::clearPercentHeightDescendantsFrom(*this);

    // If our zoom factor changes and we have a defined scrollLeft/Top, we need to adjust that value into the
    // new zoomed coordinate space.
    if (hasOverflowClip() && layer() && oldStyle && oldStyle->effectiveZoom() != newStyle.effectiveZoom()) {
        ScrollPosition scrollPosition = layer()->scrollPosition();
        float zoomScaleFactor = newStyle.effectiveZoom() / oldStyle->effectiveZoom();
        scrollPosition.scale(zoomScaleFactor);
        layer()->setPostLayoutScrollPosition(scrollPosition);
    }

    // Our opaqueness might have changed without triggering layout.
    if (diff >= StyleDifferenceRepaint && diff <= StyleDifferenceRepaintLayer) {
        auto parentToInvalidate = parent();
        for (unsigned i = 0; i < backgroundObscurationTestMaxDepth && parentToInvalidate; ++i) {
            parentToInvalidate->invalidateBackgroundObscurationStatus();
            parentToInvalidate = parentToInvalidate->parent();
        }
    }

    bool isBodyRenderer = isBody();
    bool isDocElementRenderer = isDocumentElementRenderer();

    if (isDocElementRenderer || isBodyRenderer) {
        // Propagate the new writing mode and direction up to the RenderView.
        auto* documentElementRenderer = document().documentElement()->renderer();
        auto& viewStyle = view().mutableStyle();
        bool rootStyleChanged = false;
        bool viewDirectionOrWritingModeChanged = false;
        auto* rootRenderer = isBodyRenderer ? documentElementRenderer : nullptr;
        if (viewStyle.direction() != newStyle.direction() && (isDocElementRenderer || !documentElementRenderer->style().hasExplicitlySetDirection())) {
            viewStyle.setDirection(newStyle.direction());
            viewDirectionOrWritingModeChanged = true;
            if (isBodyRenderer) {
                rootRenderer->mutableStyle().setDirection(newStyle.direction());
                rootStyleChanged = true;
            }
            setNeedsLayoutAndPrefWidthsRecalc();

            view().frameView().topContentDirectionDidChange();
        }

        if (viewStyle.writingMode() != newStyle.writingMode() && (isDocElementRenderer || !documentElementRenderer->style().hasExplicitlySetWritingMode())) {
            viewStyle.setWritingMode(newStyle.writingMode());
            viewDirectionOrWritingModeChanged = true;
            view().setHorizontalWritingMode(newStyle.isHorizontalWritingMode());
            view().markAllDescendantsWithFloatsForLayout();
            if (isBodyRenderer) {
                rootStyleChanged = true;
                rootRenderer->mutableStyle().setWritingMode(newStyle.writingMode());
                rootRenderer->setHorizontalWritingMode(newStyle.isHorizontalWritingMode());
            }
            setNeedsLayoutAndPrefWidthsRecalc();
        }

        view().frameView().recalculateScrollbarOverlayStyle();
        
        const Pagination& pagination = view().frameView().pagination();
        if (viewDirectionOrWritingModeChanged && pagination.mode != Pagination::Unpaginated) {
            viewStyle.setColumnStylesFromPaginationMode(pagination.mode);
            if (view().multiColumnFlow())
                view().updateColumnProgressionFromStyle(viewStyle);
        }
        
        if (viewDirectionOrWritingModeChanged && view().multiColumnFlow())
            view().updateStylesForColumnChildren();
        
        if (rootStyleChanged && is<RenderBlockFlow>(rootRenderer) && downcast<RenderBlockFlow>(*rootRenderer).multiColumnFlow())
            downcast<RenderBlockFlow>(*rootRenderer).updateStylesForColumnChildren();
        
        if (isBodyRenderer && pagination.mode != Pagination::Unpaginated && page().paginationLineGridEnabled()) {
            // Propagate the body font back up to the RenderView and use it as
            // the basis of the grid.
            if (newStyle.fontDescription() != view().style().fontDescription()) {
                view().mutableStyle().setFontDescription(newStyle.fontDescription());
                view().mutableStyle().fontCascade().update(&document().fontSelector());
            }
        }

        if (diff != StyleDifferenceEqual)
            view().compositor().rootOrBodyStyleChanged(*this, oldStyle);
    }

    if ((oldStyle && oldStyle->shapeOutside()) || style().shapeOutside())
        updateShapeOutsideInfoAfterStyleChange(style(), oldStyle);
    updateGridPositionAfterStyleChange(style(), oldStyle);

     */
}

void RenderBox::updateGridPositionAfterStyleChange(const RenderStyle& style, const RenderStyle* oldStyle)
{
    if (!oldStyle || !is<RenderGrid>(parent()))
        return;

    if (oldStyle->gridItemColumnStart() == style.gridItemColumnStart()
        && oldStyle->gridItemColumnEnd() == style.gridItemColumnEnd()
        && oldStyle->gridItemRowStart() == style.gridItemRowStart()
        && oldStyle->gridItemRowEnd() == style.gridItemRowEnd()
        && oldStyle->order() == style.order()
        && oldStyle->hasOutOfFlowPosition() == style.hasOutOfFlowPosition())
        return;

    // Positioned items don't participate on the layout of the grid,
    // so we don't need to mark the grid as dirty if they change positions.
    if (oldStyle->hasOutOfFlowPosition() && style.hasOutOfFlowPosition())
        return;

    // It should be possible to not dirty the grid in some cases (like moving an
    // explicitly placed grid item).
    // For now, it's more simple to just always recompute the grid.
    //FIXME GRIDLAYOUT downcast<RenderGrid>(*parent()).dirtyGrid();
}



void RenderBox::updateFromStyle()
{

    /**
    RenderBoxModelObject::updateFromStyle();

    const RenderStyle& styleToUse = style();
    bool isDocElementRenderer = isDocumentElementRenderer();
    bool isViewObject = isRenderView();

    // The root and the RenderView always paint their backgrounds/borders.
    if (isDocElementRenderer || isViewObject)
        setHasVisibleBoxDecorations(true);

    setFloating(!isOutOfFlowPositioned() && styleToUse.isFloating());

    // We also handle <body> and <html>, whose overflow applies to the viewport.
    if (styleToUse.overflowX() != OVISIBLE && !isDocElementRenderer && isRenderBlock()) {
        bool boxHasOverflowClip = true;
        if (isBody()) {
            // Overflow on the body can propagate to the viewport under the following conditions.
            // (1) The root element is <html>.
            // (2) We are the primary <body> (can be checked by looking at document.body).
            // (3) The root element has visible overflow.
            if (is<HTMLHtmlElement>(*document().documentElement())
                && document().body() == element()
                && document().documentElement()->renderer()->style().overflowX() == OVISIBLE) {
                boxHasOverflowClip = false;
            }
        }
        // Check for overflow clip.
        // It's sufficient to just check one direction, since it's illegal to have visible on only one overflow value.
        if (boxHasOverflowClip) {
            if (!s_hadOverflowClip && hasRenderOverflow()) {
                // Erase the overflow.
                // Overflow changes have to result in immediate repaints of the entire layout overflow area because
                // repaints issued by removal of descendants get clipped using the updated style when they shouldn't.
                repaintRectangle(visualOverflowRect());
                repaintRectangle(layoutOverflowRect());
            }
            setHasOverflowClip();
        }
    }
    setHasTransformRelatedProperty(styleToUse.hasTransformRelatedProperty());
    setHasReflection(styleToUse.boxReflect());
     */
}

void RenderBox::layout()
{
    StackStats::LayoutCheckPoint layoutCheckPoint;
    ASSERT(needsLayout());

    RenderObject* child = firstChild();
    if (!child) {
        clearNeedsLayout();
        return;
    }

    LayoutStateMaintainer statePusher(*this, locationOffset(), style().isFlippedBlocksWritingMode());
    while (child) {
        if (child->needsLayout())
            downcast<RenderElement>(*child).layout();
        ASSERT(!child->needsLayout());
        child = child->nextSibling();
    }
    invalidateBackgroundObscurationStatus();
    clearNeedsLayout();
}

// More IE extensions.  clientWidth and clientHeight represent the interior of an object
// excluding border and scrollbar.
LayoutUnit RenderBox::clientWidth() const
{
    return width() - borderLeft() - borderRight() - verticalScrollbarWidth();
}

LayoutUnit RenderBox::clientHeight() const
{
    return height() - borderTop() - borderBottom() - horizontalScrollbarHeight();
}


int RenderBox::scrollWidth() const
{
    /**
    if (hasOverflowClip() && layer())
        return layer()->scrollWidth();
    // For objects with visible overflow, this matches IE.
    // FIXME: Need to work right with writing modes.
    if (style().isLeftToRightDirection()) {
        // FIXME: This should use snappedIntSize() instead with absolute coordinates.
        return roundToInt(std::max(clientWidth(), layoutOverflowRect().maxX() - borderLeft()));
    }
    return roundToInt(clientWidth() - std::min<LayoutUnit>(0, layoutOverflowRect().x() - borderLeft()));
     */
return 0;
}

int RenderBox::scrollHeight() const
{
    /**
    if (hasOverflowClip() && layer())
        return layer()->scrollHeight();
    // For objects with visible overflow, this matches IE.
    // FIXME: Need to work right with writing modes.
    // FIXME: This should use snappedIntSize() instead with absolute coordinates.
    return 0; roundToInt(std::max(clientHeight(), layoutOverflowRect().maxY() - borderTop()));*/

    return 0;
}

int RenderBox::scrollLeft() const
{
    return 0; //hasOverflowClip() && layer() ? layer()->scrollPosition().x() : 0;
}

int RenderBox::scrollTop() const
{
    return 0; //hasOverflowClip() && layer() ? layer()->scrollPosition().y() : 0;
}

static void setupWheelEventTestTrigger(RenderLayer& layer)
{
    /**
    Page& page = layer.renderer().page();
    if (!page.expectsWheelEventTriggers())
        return;
    layer.scrollAnimator().setWheelEventTestTrigger(page.testTrigger());
     */
}

void RenderBox::setScrollLeft(int newLeft, ScrollClamping clamping)
{
    /**
    if (!hasOverflowClip() || !layer())
        return;
    setupWheelEventTestTrigger(*layer());
    layer()->scrollToXPosition(newLeft, clamping);
     */
}

void RenderBox::setScrollTop(int newTop, ScrollClamping clamping)
{
    /**
    if (!hasOverflowClip() || !layer())
        return;
    setupWheelEventTestTrigger(*layer());
    layer()->scrollToYPosition(newTop, clamping);
     */
}
/**
void RenderBox::absoluteRects(Vector<IntRect>& rects, const LayoutPoint& accumulatedOffset) const
{
    rects.append(snappedIntRect(accumulatedOffset, size()));
}

void RenderBox::absoluteQuads(Vector<FloatQuad>& quads, bool* wasFixed) const
{

    FloatRect localRect(0, 0, width(), height());

    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();
    if (fragmentedFlow && fragmentedFlow->absoluteQuadsForBox(quads, wasFixed, this, localRect.y(), localRect.maxY()))
        return;

    quads.append(localToAbsoluteQuad(localRect, UseTransforms, wasFixed));

}
  */

void RenderBox::updateLayerTransform()
{
    /**
    // Transform-origin depends on box size, so we need to update the layer transform after layout.
    if (hasLayer())
        layer()->updateTransform();
        */
}

LayoutUnit RenderBox::constrainLogicalWidthInFragmentByMinMax(LayoutUnit logicalWidth, LayoutUnit availableWidth, RenderBlock& cb, RenderFragmentContainer* fragment) const
{
    const RenderStyle& styleToUse = style();
    if (!styleToUse.logicalMaxWidth().isUndefined())
        logicalWidth = std::min(logicalWidth, computeLogicalWidthInFragmentUsing(MaxSize, styleToUse.logicalMaxWidth(), availableWidth, cb, fragment));
    return std::max(logicalWidth, computeLogicalWidthInFragmentUsing(MinSize, styleToUse.logicalMinWidth(), availableWidth, cb, fragment));
}

LayoutUnit RenderBox::constrainLogicalHeightByMinMax(LayoutUnit logicalHeight, std::optional<LayoutUnit> intrinsicContentHeight) const
{
    const RenderStyle& styleToUse = style();
    if (!styleToUse.logicalMaxHeight().isUndefined()) {
        if (std::optional<LayoutUnit> maxH = computeLogicalHeightUsing(MaxSize, styleToUse.logicalMaxHeight(), intrinsicContentHeight))
            logicalHeight = std::min(logicalHeight, maxH.value());
    }
    if (std::optional<LayoutUnit> computedLogicalHeight = computeLogicalHeightUsing(MinSize, styleToUse.logicalMinHeight(), intrinsicContentHeight))
        return std::max(logicalHeight, computedLogicalHeight.value());
    return logicalHeight;
}

LayoutUnit RenderBox::constrainContentBoxLogicalHeightByMinMax(LayoutUnit logicalHeight, std::optional<LayoutUnit> intrinsicContentHeight) const
{
    const RenderStyle& styleToUse = style();
    if (!styleToUse.logicalMaxHeight().isUndefined()) {
        if (std::optional<LayoutUnit> maxH = computeContentLogicalHeight(MaxSize, styleToUse.logicalMaxHeight(), intrinsicContentHeight))
            logicalHeight = std::min(logicalHeight, maxH.value());
    }
    if (std::optional<LayoutUnit> computedContentLogicalHeight = computeContentLogicalHeight(MinSize, styleToUse.logicalMinHeight(), intrinsicContentHeight))
        return std::max(logicalHeight, computedContentLogicalHeight.value());
    return logicalHeight;
}

RoundedRect::Radii RenderBox::borderRadii() const
{
    auto& style = this->style();
    LayoutRect bounds = frameRect();

    unsigned borderLeft = style.borderLeftWidth();
    unsigned borderTop = style.borderTopWidth();
    bounds.moveBy(LayoutPoint(borderLeft, borderTop));
    bounds.contract(borderLeft + style.borderRightWidth(), borderTop + style.borderBottomWidth());
    return style.getRoundedBorderFor(bounds).radii();
}

LayoutRect RenderBox::contentBoxRect() const
{
    LayoutUnit x = borderLeft() + paddingLeft();

    LayoutUnit y = borderTop() + paddingTop();
    return LayoutRect(x, y, contentWidth(), contentHeight());
}

IntRect RenderBox::absoluteContentBox() const
{
    // This is wrong with transforms and flipped writing modes.
    IntRect rect = snappedIntRect(contentBoxRect());
    FloatPoint absPos = localToAbsolute();
    rect.move(absPos.x(), absPos.y());
    return rect;
}

FloatQuad RenderBox::absoluteContentQuad() const
{
    LayoutRect rect = contentBoxRect();
    return localToAbsoluteQuad(FloatRect(rect));
}

LayoutRect RenderBox::outlineBoundsForRepaint(const RenderElement* repaintContainer, const RenderGeometryMap* geometryMap) const
{
    /**
    LayoutRect box = borderBoundingBox();
    adjustRectForOutlineAndShadow(box);

    if (repaintContainer != this) {
        FloatQuad containerRelativeQuad;
        if (geometryMap)
            containerRelativeQuad = geometryMap->mapToContainer(box, repaintContainer);
        else
            containerRelativeQuad = localToContainerQuad(FloatRect(box), repaintContainer);

        box = LayoutRect(containerRelativeQuad.boundingBox());
    }
    
    // FIXME: layoutDelta needs to be applied in parts before/after transforms and
    // repaint containers. https://bugs.webkit.org/show_bug.cgi?id=23308
    box.move(view().frameView().layoutContext().layoutDelta());

    return LayoutRect(snapRectToDevicePixels(box, document().deviceScaleFactor()));
     */
    return  LayoutRect(0,0, 100, 200);
}

void RenderBox::addFocusRingRects(Vector<LayoutRect>& rects, const LayoutPoint& additionalOffset, const RenderElement*)
{
    if (!size().isEmpty())
        rects.append(LayoutRect(additionalOffset, size()));
}

int RenderBox::reflectionOffset() const
{
    return 0;
}

LayoutRect RenderBox::reflectedRect(const LayoutRect& r) const
{
    return LayoutRect();
}



bool RenderBox::includeVerticalScrollbarSize() const
{
    return hasOverflowClip()
        && (style().overflowY() == OSCROLL || style().overflowY() == OAUTO);
}

bool RenderBox::includeHorizontalScrollbarSize() const
{
    return hasOverflowClip()
        && (style().overflowX() == OSCROLL || style().overflowX() == OAUTO);
}

int RenderBox::verticalScrollbarWidth() const
{
    return 0;
}

int RenderBox::horizontalScrollbarHeight() const
{
    return  0;
}

int RenderBox::intrinsicScrollbarLogicalWidth() const
{

    if (!hasOverflowClip())
        return 0;

    /**
    if (isHorizontalWritingMode() && (style().overflowY() == OSCROLL && !hasVerticalScrollbarWithAutoBehavior())) {
        ASSERT(layer() && layer()->hasVerticalScrollbar());
        return verticalScrollbarWidth();
    }

    if (!isHorizontalWritingMode() && (style().overflowX() == OSCROLL && !hasHorizontalScrollbarWithAutoBehavior())) {
        ASSERT(layer() && layer()->hasHorizontalScrollbar());
        return horizontalScrollbarHeight();
    }*/

    return 0;
}






bool RenderBox::canBeScrolledAndHasScrollableArea() const
{
    return canBeProgramaticallyScrolled() && (hasHorizontalOverflow() || hasVerticalOverflow());
}

bool RenderBox::isScrollableOrRubberbandableBox() const
{
    return canBeScrolledAndHasScrollableArea();
}



bool RenderBox::usesCompositedScrolling() const
{
    return false;
}

void RenderBox::autoscroll(const IntPoint& position)
{
    /**
    if (layer())
        layer()->autoscroll(position);
        */
}

// There are two kinds of renderer that can autoscroll.
bool RenderBox::canAutoscroll() const
{
    /**
    if (isRenderView())
       // return view().frameView().isScrollable();

    // Check for a box that can be scrolled in its own right.
    if (canBeScrolledAndHasScrollableArea())
        return true;
*/
    return false;
}

// If specified point is in border belt, returned offset denotes direction of
// scrolling.
    /**
IntSize RenderBox::calculateAutoscrollDirection(const IntPoint& windowPoint) const
{

  IntRect box(absoluteBoundingBoxRect());
  box.moveBy(view().frameView().scrollPosition());
  IntRect windowBox = view().frameView().contentsToWindow(box);

  IntPoint windowAutoscrollPoint = windowPoint;

  if (windowAutoscrollPoint.x() < windowBox.x() + autoscrollBeltSize)
      windowAutoscrollPoint.move(-autoscrollBeltSize, 0);
  else if (windowAutoscrollPoint.x() > windowBox.maxX() - autoscrollBeltSize)
      windowAutoscrollPoint.move(autoscrollBeltSize, 0);

  if (windowAutoscrollPoint.y() < windowBox.y() + autoscrollBeltSize)
      windowAutoscrollPoint.move(0, -autoscrollBeltSize);
  else if (windowAutoscrollPoint.y() > windowBox.maxY() - autoscrollBeltSize)
      windowAutoscrollPoint.move(0, autoscrollBeltSize);

  return windowAutoscrollPoint - windowPoint;
}*/




bool RenderBox::needsPreferredWidthsRecalculation() const
{
  return style().paddingStart().isPercentOrCalculated() || style().paddingEnd().isPercentOrCalculated();
}




LayoutUnit RenderBox::minPreferredLogicalWidth() const
{
  if (preferredLogicalWidthsDirty()) {
#ifndef NDEBUG
      SetLayoutNeededForbiddenScope layoutForbiddenScope(const_cast<RenderBox*>(this));
#endif
      const_cast<RenderBox*>(this)->computePreferredLogicalWidths();
  }

  return m_minPreferredLogicalWidth;
}

LayoutUnit RenderBox::maxPreferredLogicalWidth() const
{
  if (preferredLogicalWidthsDirty()) {
#ifndef NDEBUG
      SetLayoutNeededForbiddenScope layoutForbiddenScope(const_cast<RenderBox*>(this));
#endif
      const_cast<RenderBox*>(this)->computePreferredLogicalWidths();
  }

  return m_maxPreferredLogicalWidth;
}

bool RenderBox::hasOverrideLogicalContentHeight() const
{
  return gOverrideHeightMap && gOverrideHeightMap->contains(this);
}

bool RenderBox::hasOverrideLogicalContentWidth() const
{
  return gOverrideWidthMap && gOverrideWidthMap->contains(this);
}

void RenderBox::setOverrideLogicalContentHeight(LayoutUnit height)
{
  if (!gOverrideHeightMap)
      gOverrideHeightMap = new OverrideSizeMap();
  gOverrideHeightMap->set(this, height);
}

void RenderBox::setOverrideLogicalContentWidth(LayoutUnit width)
{
  if (!gOverrideWidthMap)
      gOverrideWidthMap = new OverrideSizeMap();
  gOverrideWidthMap->set(this, width);
}

void RenderBox::clearOverrideLogicalContentHeight()
{
  if (gOverrideHeightMap)
      gOverrideHeightMap->remove(this);
}

void RenderBox::clearOverrideLogicalContentWidth()
{
  if (gOverrideWidthMap)
      gOverrideWidthMap->remove(this);
}

void RenderBox::clearOverrideSize()
{
  clearOverrideLogicalContentHeight();
  clearOverrideLogicalContentWidth();
}

LayoutUnit RenderBox::overrideLogicalContentWidth() const
{
  ASSERT(hasOverrideLogicalContentWidth());
  return gOverrideWidthMap->get(this);
}

LayoutUnit RenderBox::overrideLogicalContentHeight() const
{
  ASSERT(hasOverrideLogicalContentHeight());
  return gOverrideHeightMap->get(this);
}

std::optional<LayoutUnit> RenderBox::overrideContainingBlockContentLogicalWidth() const
{
  ASSERT(hasOverrideContainingBlockLogicalWidth());
  return gOverrideContainingBlockLogicalWidthMap->get(this);
}

std::optional<LayoutUnit> RenderBox::overrideContainingBlockContentLogicalHeight() const
{
  ASSERT(hasOverrideContainingBlockLogicalHeight());
  return gOverrideContainingBlockLogicalHeightMap->get(this);
}

bool RenderBox::hasOverrideContainingBlockLogicalWidth() const
{
  return gOverrideContainingBlockLogicalWidthMap && gOverrideContainingBlockLogicalWidthMap->contains(this);
}

bool RenderBox::hasOverrideContainingBlockLogicalHeight() const
{
  return gOverrideContainingBlockLogicalHeightMap && gOverrideContainingBlockLogicalHeightMap->contains(this);
}

void RenderBox::setOverrideContainingBlockContentLogicalWidth(std::optional<LayoutUnit> logicalWidth)
{
  if (!gOverrideContainingBlockLogicalWidthMap)
      gOverrideContainingBlockLogicalWidthMap = new OverrideOptionalSizeMap;
  gOverrideContainingBlockLogicalWidthMap->set(this, logicalWidth);
}

void RenderBox::setOverrideContainingBlockContentLogicalHeight(std::optional<LayoutUnit> logicalHeight)
{
  if (!gOverrideContainingBlockLogicalHeightMap)
      gOverrideContainingBlockLogicalHeightMap = new OverrideOptionalSizeMap;
  gOverrideContainingBlockLogicalHeightMap->set(this, logicalHeight);
}

void RenderBox::clearContainingBlockOverrideSize()
{
  if (gOverrideContainingBlockLogicalWidthMap)
      gOverrideContainingBlockLogicalWidthMap->remove(this);
  clearOverrideContainingBlockContentLogicalHeight();
}

void RenderBox::clearOverrideContainingBlockContentLogicalHeight()
{
  if (gOverrideContainingBlockLogicalHeightMap)
      gOverrideContainingBlockLogicalHeightMap->remove(this);
}

LayoutUnit RenderBox::adjustBorderBoxLogicalWidthForBoxSizing(LayoutUnit width) const
{
  LayoutUnit bordersPlusPadding = borderAndPaddingLogicalWidth();
  if (style().boxSizing() == CONTENT_BOX)
      return width + bordersPlusPadding;
  return std::max(width, bordersPlusPadding);
}

LayoutUnit RenderBox::adjustBorderBoxLogicalHeightForBoxSizing(LayoutUnit height) const
{
  LayoutUnit bordersPlusPadding = borderAndPaddingLogicalHeight();
  if (style().boxSizing() == CONTENT_BOX)
      return height + bordersPlusPadding;
  return std::max(height, bordersPlusPadding);
}

LayoutUnit RenderBox::adjustContentBoxLogicalWidthForBoxSizing(LayoutUnit width) const
{
  if (style().boxSizing() == BORDER_BOX)
      width -= borderAndPaddingLogicalWidth();
  return std::max<LayoutUnit>(0, width);
}

LayoutUnit RenderBox::adjustContentBoxLogicalHeightForBoxSizing(std::optional<LayoutUnit> height) const
{
  if (!height)
      return 0;
  LayoutUnit result = height.value();
  if (style().boxSizing() == BORDER_BOX)
      result -= borderAndPaddingLogicalHeight();
  return std::max(LayoutUnit(), result);
}

// Hit Testing
bool RenderBox::nodeAtPoint(const HitTestRequest& request, HitTestResult& result, const HitTestLocation& locationInContainer, const LayoutPoint& accumulatedOffset, HitTestAction action)
{
    /*
  LayoutPoint adjustedLocation = accumulatedOffset + location();

  // Check kids first.
  for (RenderObject* child = lastChild(); child; child = child->previousSibling()) {
      if (!child->hasLayer() && child->nodeAtPoint(request, result, locationInContainer, adjustedLocation, action)) {
          updateHitTestResult(result, locationInContainer.point() - toLayoutSize(adjustedLocation));
          return true;
      }
  }

  // Check our bounds next. For this purpose always assume that we can only be hit in the
  // foreground phase (which is true for replaced elements like images).
  LayoutRect boundsRect = borderBoxRectInFragment(nullptr);
  boundsRect.moveBy(adjustedLocation);
  if (visibleToHitTesting() && action == HitTestForeground && locationInContainer.intersects(boundsRect)) {
      updateHitTestResult(result, locationInContainer.point() - toLayoutSize(adjustedLocation));
      if (result.addNodeToListBasedTestResult(element(), request, locationInContainer, boundsRect) == HitTestProgress::Stop)
          return true;
  }*/

  return false;
}

// --------------------- painting stuff -------------------------------








LayoutRect RenderBox::clipRect(const LayoutPoint& location, RenderFragmentContainer* fragment)
{
  LayoutRect borderBoxRect = borderBoxRectInFragment(fragment);
  LayoutRect clipRect = LayoutRect(borderBoxRect.location() + location, borderBoxRect.size());

  if (!style().clipLeft().isAuto()) {
      LayoutUnit c = valueForLength(style().clipLeft(), borderBoxRect.width());
      clipRect.move(c, 0);
      clipRect.contract(c, 0);
  }

  // We don't use the fragment-specific border box's width and height since clip offsets are (stupidly) specified
  // from the left and top edges. Therefore it's better to avoid constraining to smaller widths and heights.

  if (!style().clipRight().isAuto())
      clipRect.contract(width() - valueForLength(style().clipRight(), width()), 0);

  if (!style().clipTop().isAuto()) {
      LayoutUnit c = valueForLength(style().clipTop(), borderBoxRect.height());
      clipRect.move(0, c);
      clipRect.contract(0, c);
  }

  if (!style().clipBottom().isAuto())
      clipRect.contract(0, height() - valueForLength(style().clipBottom(), height()));

  return clipRect;
}

LayoutUnit RenderBox::shrinkLogicalWidthToAvoidFloats(LayoutUnit childMarginStart, LayoutUnit childMarginEnd, const RenderBlock& cb, RenderFragmentContainer* fragment) const
{
  RenderFragmentContainer* containingBlockFragment = nullptr;
  LayoutUnit logicalTopPosition = logicalTop();
  if (fragment) {
      LayoutUnit offsetFromLogicalTopOfFragment = fragment ? fragment->logicalTopForFragmentedFlowContent() - offsetFromLogicalTopOfFirstPage() : LayoutUnit();
      logicalTopPosition = std::max(logicalTopPosition, logicalTopPosition + offsetFromLogicalTopOfFragment);
      containingBlockFragment = cb.clampToStartAndEndFragments(fragment);
  }

  LayoutUnit logicalHeight = cb.logicalHeightForChild(*this);
  LayoutUnit result = cb.availableLogicalWidthForLineInFragment(logicalTopPosition, DoNotIndentText, containingBlockFragment, logicalHeight) - childMarginStart - childMarginEnd;

  // We need to see if margins on either the start side or the end side can contain the floats in question. If they can,
  // then just using the line width is inaccurate. In the case where a float completely fits, we don't need to use the line
  // offset at all, but can instead push all the way to the content edge of the containing block. In the case where the float
  // doesn't fit, we can use the line offset, but we need to grow it by the margin to reflect the fact that the margin was
  // "consumed" by the float. Negative margins aren't consumed by the float, and so we ignore them.
  if (childMarginStart > 0) {
      LayoutUnit startContentSide = cb.startOffsetForContent(containingBlockFragment);
      LayoutUnit startContentSideWithMargin = startContentSide + childMarginStart;
      LayoutUnit startOffset = cb.startOffsetForLineInFragment(logicalTopPosition, DoNotIndentText, containingBlockFragment, logicalHeight);
      if (startOffset > startContentSideWithMargin)
          result += childMarginStart;
      else
          result += startOffset - startContentSide;
  }

  if (childMarginEnd > 0) {
      LayoutUnit endContentSide = cb.endOffsetForContent(containingBlockFragment);
      LayoutUnit endContentSideWithMargin = endContentSide + childMarginEnd;
      LayoutUnit endOffset = cb.endOffsetForLineInFragment(logicalTopPosition, DoNotIndentText, containingBlockFragment, logicalHeight);
      if (endOffset > endContentSideWithMargin)
          result += childMarginEnd;
      else
          result += endOffset - endContentSide;
  }

  return result;
}

LayoutUnit RenderBox::containingBlockLogicalWidthForContent() const
{
  if (hasOverrideContainingBlockLogicalWidth()) {
      if (auto overrideLogicalWidth = overrideContainingBlockContentLogicalWidth())
          return overrideLogicalWidth.value();
  }

  if (RenderBlock* cb = containingBlock())
      return cb->availableLogicalWidth();
  return LayoutUnit();
}

LayoutUnit RenderBox::containingBlockLogicalHeightForContent(AvailableLogicalHeightType heightType) const
{
  if (hasOverrideContainingBlockLogicalHeight()) {
      if (auto overrideLogicalHeight = overrideContainingBlockContentLogicalHeight())
          return overrideLogicalHeight.value();
  }

  if (RenderBlock* cb = containingBlock())
      return cb->availableLogicalHeight(heightType);
  return LayoutUnit();
}





LayoutUnit RenderBox::perpendicularContainingBlockLogicalHeight() const
{
  if (hasOverrideContainingBlockLogicalHeight()) {
      if (auto overrideLogicalHeight = overrideContainingBlockContentLogicalHeight())
          return overrideLogicalHeight.value();
  }

  RenderBlock* cb = containingBlock();
  if (cb->hasOverrideLogicalContentHeight())
      return cb->overrideLogicalContentHeight();

  const RenderStyle& containingBlockStyle = cb->style();
  Length logicalHeightLength = containingBlockStyle.logicalHeight();

    /**
  // FIXME: For now just support fixed heights.  Eventually should support percentage heights as well.
  if (!logicalHeightLength.isFixed()) {
      LayoutUnit fillFallbackExtent = containingBlockStyle.isHorizontalWritingMode() ? view().frameView().visibleHeight() : view().frameView().visibleWidth();
      LayoutUnit fillAvailableExtent = containingBlock()->availableLogicalHeight(ExcludeMarginBorderPadding);
      view().addPercentHeightDescendant(const_cast<RenderBox&>(*this));
      // FIXME: https://bugs.webkit.org/show_bug.cgi?id=158286 We also need to perform the same percentHeightDescendant treatment to the element which dictates the return value for containingBlock()->availableLogicalHeight() above.
      return std::min(fillAvailableExtent, fillFallbackExtent);
  }*/

  // Use the content box logical height as specified by the style.
  return cb->adjustContentBoxLogicalHeightForBoxSizing(LayoutUnit(logicalHeightLength.value()));
}



void RenderBox::mapAbsoluteToLocalPoint(MapCoordinatesFlags mode, TransformState& transformState) const
{
  bool isFixedPos = isFixedPositioned();
  if (hasTransform() && !isFixedPos) {
      // If this box has a transform, it acts as a fixed position container for fixed descendants,
      // and may itself also be fixed position. So propagate 'fixed' up only if this box is fixed position.
      mode &= ~IsFixed;
  } else if (isFixedPos)
      mode |= IsFixed;

  RenderBoxModelObject::mapAbsoluteToLocalPoint(mode, transformState);
}

LayoutSize RenderBox::offsetFromContainer(RenderElement& container, const LayoutPoint&, bool* offsetDependsOnPoint) const
{
  // A fragment "has" boxes inside it without being their container.
  ASSERT(&container == this->container() || is<RenderFragmentContainer>(container));

  LayoutSize offset;
  if (isInFlowPositioned())
      offset += offsetForInFlowPosition();

  if (!isInline() || isReplaced())
      offset += topLeftLocationOffset();

  //if (is<RenderBox>(container))
     // offset -= toLayoutSize(downcast<RenderBox>(container).scrollPosition());

  if (isAbsolutelyPositioned() && container.isInFlowPositioned() && is<RenderInline>(container))
      offset += downcast<RenderInline>(container).offsetForInFlowPositionedInline(this);

  //if (offsetDependsOnPoint)
      //*offsetDependsOnPoint |= is<RenderFragmentedFlow>(container);

  return offset;
}

    /**
std::unique_ptr<InlineElementBox> RenderBox::createInlineBox()
{
  return std::make_unique<InlineElementBox>(*this);
}*/


/**
void RenderBox::positionLineBox(InlineElementBox& box)
{
  if (isOutOfFlowPositioned()) {
      // Cache the x position only if we were an INLINE type originally.
      bool wasInline = style().isOriginalDisplayInlineType();
      if (wasInline) {
          // The value is cached in the xPos of the box.  We only need this value if
          // our object was inline originally, since otherwise it would have ended up underneath
          // the inlines.
          RootInlineBox& rootBox = box.root();
          rootBox.blockFlow().setStaticInlinePositionForChild(*this, rootBox.lineTopWithLeading(), LayoutUnit::fromFloatRound(box.logicalLeft()));
          if (style().hasStaticInlinePosition(box.isHorizontal()))
              setChildNeedsLayout(MarkOnlyThis); // Just mark the positioned object as needing layout, so it will update its position properly.
      } else {
          // Our object was a block originally, so we make our normal flow position be
          // just below the line box (as though all the inlines that came before us got
          // wrapped in an anonymous block, which is what would have happened had we been
          // in flow).  This value was cached in the y() of the box.
          layer()->setStaticBlockPosition(box.logicalTop());
          if (style().hasStaticBlockPosition(box.isHorizontal()))
              setChildNeedsLayout(MarkOnlyThis); // Just mark the positioned object as needing layout, so it will update its position properly.
      }
      return;
  }

  if (isReplaced()) {
      setLocation(LayoutPoint(box.topLeft()));
      setInlineBoxWrapper(&box);
  }
}*/

void RenderBox::deleteLineBoxWrapper()
{
    /**
  if (!m_inlineBoxWrapper)
      return;

  if (!renderTreeBeingDestroyed())
      m_inlineBoxWrapper->removeFromParent();
  delete m_inlineBoxWrapper;
  m_inlineBoxWrapper = nullptr;
     */
}



void RenderBox::repaintOverhangingFloats(bool)
{
}

void RenderBox::updateLogicalWidth()
{
  LogicalExtentComputedValues computedValues;
  computeLogicalWidthInFragment(computedValues);

  setLogicalWidth(computedValues.m_extent);
  setLogicalLeft(computedValues.m_position);
  setMarginStart(computedValues.m_margins.m_start);
  setMarginEnd(computedValues.m_margins.m_end);
}

void RenderBox::computeLogicalWidthInFragment(LogicalExtentComputedValues& computedValues, RenderFragmentContainer* fragment) const
{
    /**
  computedValues.m_extent = logicalWidth();
  computedValues.m_position = logicalLeft();
  computedValues.m_margins.m_start = marginStart();
  computedValues.m_margins.m_end = marginEnd();

  if (isOutOfFlowPositioned()) {
      // FIXME: This calculation is not patched for block-flow yet.
      // https://bugs.webkit.org/show_bug.cgi?id=46500
      computePositionedLogicalWidth(computedValues, fragment);
      return;
  }

  // If layout is limited to a subtree, the subtree root's logical width does not change.
  if (element() && !view().frameView().layoutContext().isLayoutPending() && view().frameView().layoutContext().subtreeLayoutRoot() == this)
      return;

  // The parent box is flexing us, so it has increased or decreased our
  // width.  Use the width from the style context.
  // FIXME: Account for block-flow in flexible boxes.
  // https://bugs.webkit.org/show_bug.cgi?id=46418
  if (hasOverrideLogicalContentWidth() && (isRubyRun() || style().borderFit() == BorderFitLines || (parent()->isFlexibleBoxIncludingDeprecated()))) {
      computedValues.m_extent = overrideLogicalContentWidth() + borderAndPaddingLogicalWidth();
      return;
  }

  // FIXME: Account for block-flow in flexible boxes.
  // https://bugs.webkit.org/show_bug.cgi?id=46418
  bool inVerticalBox = parent()->isDeprecatedFlexibleBox() && (parent()->style().boxOrient() == VERTICAL);
  bool stretching = (parent()->style().boxAlign() == BSTRETCH);
  // FIXME: Stretching is the only reason why we don't want the box to be treated as a replaced element, so we could perhaps
  // refactor all this logic, not only for flex and grid since alignment is intended to be applied to any block.
  bool treatAsReplaced = shouldComputeSizeAsReplaced() && (!inVerticalBox || !stretching);
  treatAsReplaced = treatAsReplaced && (!isGridItem() || !hasStretchedLogicalWidth());

  const RenderStyle& styleToUse = style();
  Length logicalWidthLength = treatAsReplaced ? Length(computeReplacedLogicalWidth(), Fixed) : styleToUse.logicalWidth();

  RenderBlock& cb = *containingBlock();
  LayoutUnit containerLogicalWidth = std::max<LayoutUnit>(0, containingBlockLogicalWidthForContentInFragment(fragment));
  bool hasPerpendicularContainingBlock = cb.isHorizontalWritingMode() != isHorizontalWritingMode();

  if (isInline() && !isInlineBlockOrInlineTable()) {
      // just calculate margins
      computedValues.m_margins.m_start = minimumValueForLength(styleToUse.marginStart(), containerLogicalWidth);
      computedValues.m_margins.m_end = minimumValueForLength(styleToUse.marginEnd(), containerLogicalWidth);
      if (treatAsReplaced)
          computedValues.m_extent = std::max<LayoutUnit>(floatValueForLength(logicalWidthLength, 0) + borderAndPaddingLogicalWidth(), minPreferredLogicalWidth());
      return;
  }

  LayoutUnit containerWidthInInlineDirection = containerLogicalWidth;
  if (hasPerpendicularContainingBlock)
      containerWidthInInlineDirection = perpendicularContainingBlockLogicalHeight();

  // Width calculations
  if (treatAsReplaced) {
      computedValues.m_extent = logicalWidthLength.value() + borderAndPaddingLogicalWidth();
  } else {
      LayoutUnit preferredWidth = computeLogicalWidthInFragmentUsing(MainOrPreferredSize, styleToUse.logicalWidth(), containerWidthInInlineDirection, cb, fragment);
      computedValues.m_extent = constrainLogicalWidthInFragmentByMinMax(preferredWidth, containerWidthInInlineDirection, cb, fragment);
  }

  // Margin calculations.
  if (hasPerpendicularContainingBlock || isFloating() || isInline()) {
      computedValues.m_margins.m_start = minimumValueForLength(styleToUse.marginStart(), containerLogicalWidth);
      computedValues.m_margins.m_end = minimumValueForLength(styleToUse.marginEnd(), containerLogicalWidth);
  } else {
      LayoutUnit containerLogicalWidthForAutoMargins = containerLogicalWidth;
      if (avoidsFloats() && cb.containsFloats())
          containerLogicalWidthForAutoMargins = containingBlockAvailableLineWidthInFragment(fragment);
      bool hasInvertedDirection = cb.style().isLeftToRightDirection() != style().isLeftToRightDirection();
      computeInlineDirectionMargins(cb, containerLogicalWidthForAutoMargins, computedValues.m_extent,
          hasInvertedDirection ? computedValues.m_margins.m_end : computedValues.m_margins.m_start,
          hasInvertedDirection ? computedValues.m_margins.m_start : computedValues.m_margins.m_end);
  }

  if (!hasPerpendicularContainingBlock && containerLogicalWidth && containerLogicalWidth != (computedValues.m_extent + computedValues.m_margins.m_start + computedValues.m_margins.m_end)
      && !isFloating() && !isInline() && !cb.isFlexibleBoxIncludingDeprecated()
#if ENABLE(MATHML)
      // RenderMathMLBlocks take the size of their content so we must not adjust the margin to fill the container size.
      && !cb.isRenderMathMLBlock()
#endif
      && !cb.isRenderGrid()
      ) {
      LayoutUnit newMarginTotal = containerLogicalWidth - computedValues.m_extent;
      bool hasInvertedDirection = cb.style().isLeftToRightDirection() != style().isLeftToRightDirection();
      if (hasInvertedDirection)
          computedValues.m_margins.m_start = newMarginTotal - computedValues.m_margins.m_end;
      else
          computedValues.m_margins.m_end = newMarginTotal - computedValues.m_margins.m_start;
  }*/
}

LayoutUnit RenderBox::fillAvailableMeasure(LayoutUnit availableLogicalWidth) const
{
  LayoutUnit marginStart = 0;
  LayoutUnit marginEnd = 0;
  return fillAvailableMeasure(availableLogicalWidth, marginStart, marginEnd);
}

LayoutUnit RenderBox::fillAvailableMeasure(LayoutUnit availableLogicalWidth, LayoutUnit& marginStart, LayoutUnit& marginEnd) const
{
  marginStart = minimumValueForLength(style().marginStart(), availableLogicalWidth);
  marginEnd = minimumValueForLength(style().marginEnd(), availableLogicalWidth);
  return availableLogicalWidth - marginStart - marginEnd;
}

LayoutUnit RenderBox::computeIntrinsicLogicalWidthUsing(Length logicalWidthLength, LayoutUnit availableLogicalWidth, LayoutUnit borderAndPadding) const
{
  if (logicalWidthLength.type() == FillAvailable)
      return std::max(borderAndPadding, fillAvailableMeasure(availableLogicalWidth));

  LayoutUnit minLogicalWidth = 0;
  LayoutUnit maxLogicalWidth = 0;
  computeIntrinsicLogicalWidths(minLogicalWidth, maxLogicalWidth);

  if (logicalWidthLength.type() == MinContent)
      return minLogicalWidth + borderAndPadding;

  if (logicalWidthLength.type() == MaxContent)
      return maxLogicalWidth + borderAndPadding;

  if (logicalWidthLength.type() == FitContent) {
      minLogicalWidth += borderAndPadding;
      maxLogicalWidth += borderAndPadding;
      return std::max(minLogicalWidth, std::min(maxLogicalWidth, fillAvailableMeasure(availableLogicalWidth)));
  }

  ASSERT_NOT_REACHED();
  return 0;
}

LayoutUnit RenderBox::computeLogicalWidthInFragmentUsing(SizeType widthType, Length logicalWidth, LayoutUnit availableLogicalWidth,
  const RenderBlock& cb, RenderFragmentContainer* fragment) const
{
  ASSERT(widthType == MinSize || widthType == MainOrPreferredSize || !logicalWidth.isAuto());
  if (widthType == MinSize && logicalWidth.isAuto())
      return adjustBorderBoxLogicalWidthForBoxSizing(0);

  if (!logicalWidth.isIntrinsicOrAuto()) {
      // FIXME: If the containing block flow is perpendicular to our direction we need to use the available logical height instead.
      return adjustBorderBoxLogicalWidthForBoxSizing(valueForLength(logicalWidth, availableLogicalWidth));
  }

  if (logicalWidth.isIntrinsic())
      return computeIntrinsicLogicalWidthUsing(logicalWidth, availableLogicalWidth, borderAndPaddingLogicalWidth());

  LayoutUnit marginStart = 0;
  LayoutUnit marginEnd = 0;
  LayoutUnit logicalWidthResult = fillAvailableMeasure(availableLogicalWidth, marginStart, marginEnd);

  if (shrinkToAvoidFloats() && cb.containsFloats())
      logicalWidthResult = std::min(logicalWidthResult, shrinkLogicalWidthToAvoidFloats(marginStart, marginEnd, cb, fragment));

  if (widthType == MainOrPreferredSize && sizesLogicalWidthToFitContent(widthType))
      return std::max(minPreferredLogicalWidth(), std::min(maxPreferredLogicalWidth(), logicalWidthResult));
  return logicalWidthResult;
}

bool RenderBox::columnFlexItemHasStretchAlignment() const
{
  // auto margins mean we don't stretch. Note that this function will only be
  // used for widths, so we don't have to check marginBefore/marginAfter.
  const auto& parentStyle = parent()->style();
  ASSERT(parentStyle.isColumnFlexDirection());
  if (style().marginStart().isAuto() || style().marginEnd().isAuto())
      return false;
  return style().resolvedAlignSelf(&parentStyle, containingBlock()->selfAlignmentNormalBehavior()).position() == ItemPositionStretch;
}

bool RenderBox::isStretchingColumnFlexItem() const
{
  if (parent()->isDeprecatedFlexibleBox() && parent()->style().boxOrient() == VERTICAL && parent()->style().boxAlign() == BSTRETCH)
      return true;

  // We don't stretch multiline flexboxes because they need to apply line spacing (align-content) first.
  if (parent()->isFlexibleBox() && parent()->style().flexWrap() == FlexNoWrap && parent()->style().isColumnFlexDirection() && columnFlexItemHasStretchAlignment())
      return true;
  return false;
}

// FIXME: Can/Should we move this inside specific layout classes (flex. grid)? Can we refactor columnFlexItemHasStretchAlignment logic?
bool RenderBox::hasStretchedLogicalWidth() const
{
  auto& style = this->style();
  if (!style.logicalWidth().isAuto() || style.marginStart().isAuto() || style.marginEnd().isAuto())
      return false;
  RenderBlock* containingBlock = this->containingBlock();
  if (!containingBlock) {
      // We are evaluating align-self/justify-self, which default to 'normal' for the root element.
      // The 'normal' value behaves like 'start' except for Flexbox Items, which obviously should have a container.
      return false;
  }
  if (containingBlock->isHorizontalWritingMode() != isHorizontalWritingMode())
      return style.resolvedAlignSelf(&containingBlock->style(), containingBlock->selfAlignmentNormalBehavior(this)).position() == ItemPositionStretch;
  return style.resolvedJustifySelf(&containingBlock->style(), containingBlock->selfAlignmentNormalBehavior(this)).position() == ItemPositionStretch;
}



void RenderBox::computeInlineDirectionMargins(const RenderBlock& containingBlock, LayoutUnit containerWidth, LayoutUnit childWidth, LayoutUnit& marginStart, LayoutUnit& marginEnd) const
{

  const RenderStyle& containingBlockStyle = containingBlock.style();
  Length marginStartLength = style().marginStartUsing(&containingBlockStyle);
  Length marginEndLength = style().marginEndUsing(&containingBlockStyle);

  if (isFloating() || isInline()) {
      // Inline blocks/tables and floats don't have their margins increased.
      marginStart = minimumValueForLength(marginStartLength, containerWidth);
      marginEnd = minimumValueForLength(marginEndLength, containerWidth);
      return;
  }

  if (containingBlock.isFlexibleBox()) {
      // We need to let flexbox handle the margin adjustment - otherwise, flexbox
      // will think we're wider than we actually are and calculate line sizes
      // wrong. See also http://dev.w3.org/csswg/css-flexbox/#auto-margins
      if (marginStartLength.isAuto())
          marginStartLength = Length(0, Fixed);
      if (marginEndLength.isAuto())
          marginEndLength = Length(0, Fixed);
  }

  // Case One: The object is being centered in the containing block's available logical width.
  if ((marginStartLength.isAuto() && marginEndLength.isAuto() && childWidth < containerWidth)
      || (!marginStartLength.isAuto() && !marginEndLength.isAuto() && containingBlock.style().textAlign() == WEBKIT_CENTER)) {
      // Other browsers center the margin box for align=center elements so we match them here.
      LayoutUnit marginStartWidth = minimumValueForLength(marginStartLength, containerWidth);
      LayoutUnit marginEndWidth = minimumValueForLength(marginEndLength, containerWidth);
      LayoutUnit centeredMarginBoxStart = std::max<LayoutUnit>(0, (containerWidth - childWidth - marginStartWidth - marginEndWidth) / 2);
      marginStart = centeredMarginBoxStart + marginStartWidth;
      marginEnd = containerWidth - childWidth - marginStart + marginEndWidth;
      return;
  }

  // Case Two: The object is being pushed to the start of the containing block's available logical width.
  if (marginEndLength.isAuto() && childWidth < containerWidth) {
      marginStart = valueForLength(marginStartLength, containerWidth);
      marginEnd = containerWidth - childWidth - marginStart;
      return;
  }

  // Case Three: The object is being pushed to the end of the containing block's available logical width.
  bool pushToEndFromTextAlign = !marginEndLength.isAuto() && ((!containingBlockStyle.isLeftToRightDirection() && containingBlockStyle.textAlign() == WEBKIT_LEFT)
      || (containingBlockStyle.isLeftToRightDirection() && containingBlockStyle.textAlign() == WEBKIT_RIGHT));
  if ((marginStartLength.isAuto() || pushToEndFromTextAlign) && childWidth < containerWidth) {
      marginEnd = valueForLength(marginEndLength, containerWidth);
      marginStart = containerWidth - childWidth - marginEnd;
      return;
  }

  // Case Four: Either no auto margins, or our width is >= the container width (css2.1, 10.3.3).  In that case
  // auto margins will just turn into 0.
  marginStart = minimumValueForLength(marginStartLength, containerWidth);
  marginEnd = minimumValueForLength(marginEndLength, containerWidth);
}

RenderBoxFragmentInfo* RenderBox::renderBoxFragmentInfo(RenderFragmentContainer* fragment, RenderBoxFragmentInfoFlags cacheFlag) const
{
  // Make sure nobody is trying to call this with a null fragment.
  if (!fragment)
      return nullptr;

  // If we have computed our width in this fragment already, it will be cached, and we can
  // just return it.
  RenderBoxFragmentInfo* boxInfo = fragment->renderBoxFragmentInfo(this);
  if (boxInfo && cacheFlag == CacheRenderBoxFragmentInfo)
      return boxInfo;

  return nullptr;
}

static bool shouldFlipBeforeAfterMargins(const RenderStyle& containingBlockStyle, const RenderStyle* childStyle)
{
  ASSERT(containingBlockStyle.isHorizontalWritingMode() != childStyle->isHorizontalWritingMode());
  WritingMode childWritingMode = childStyle->writingMode();
  bool shouldFlip = false;
  switch (containingBlockStyle.writingMode()) {
  case TopToBottomWritingMode:
      shouldFlip = (childWritingMode == RightToLeftWritingMode);
      break;
  case BottomToTopWritingMode:
      shouldFlip = (childWritingMode == RightToLeftWritingMode);
      break;
  case RightToLeftWritingMode:
      shouldFlip = (childWritingMode == BottomToTopWritingMode);
      break;
  case LeftToRightWritingMode:
      shouldFlip = (childWritingMode == BottomToTopWritingMode);
      break;
  }

  if (!containingBlockStyle.isLeftToRightDirection())
      shouldFlip = !shouldFlip;

  return shouldFlip;
}

void RenderBox::cacheIntrinsicContentLogicalHeightForFlexItem(LayoutUnit height) const
{
  if (isFloatingOrOutOfFlowPositioned() || !parent() || !parent()->isFlexibleBox())
      return;
  downcast<RenderFlexibleBox>(parent())->setCachedChildIntrinsicContentLogicalHeight(*this, height);
}

void RenderBox::updateLogicalHeight()
{
  cacheIntrinsicContentLogicalHeightForFlexItem(contentLogicalHeight());
  auto computedValues = computeLogicalHeight(logicalHeight(), logicalTop());
  setLogicalHeight(computedValues.m_extent);
  setLogicalTop(computedValues.m_position);
  setMarginBefore(computedValues.m_margins.m_before);
  setMarginAfter(computedValues.m_margins.m_after);
}



LayoutUnit RenderBox::computeLogicalHeightWithoutLayout() const
{
  // FIXME:: We should probably return something other than just
  // border + padding, but for now we have no good way to do anything else
  // without layout, so we just use that.
  LogicalExtentComputedValues computedValues = computeLogicalHeight(borderAndPaddingLogicalHeight(), LayoutUnit());
  return computedValues.m_extent;
}

std::optional<LayoutUnit> RenderBox::computeLogicalHeightUsing(SizeType heightType, const Length& height, std::optional<LayoutUnit> intrinsicContentHeight) const
{
  if (std::optional<LayoutUnit> logicalHeight = computeContentAndScrollbarLogicalHeightUsing(heightType, height, intrinsicContentHeight))
      return adjustBorderBoxLogicalHeightForBoxSizing(logicalHeight.value());
  return std::nullopt;
}

std::optional<LayoutUnit> RenderBox::computeContentLogicalHeight(SizeType heightType, const Length& height, std::optional<LayoutUnit> intrinsicContentHeight) const
{
  if (std::optional<LayoutUnit> heightIncludingScrollbar = computeContentAndScrollbarLogicalHeightUsing(heightType, height, intrinsicContentHeight))
      return std::max<LayoutUnit>(0, adjustContentBoxLogicalHeightForBoxSizing(heightIncludingScrollbar) - scrollbarLogicalHeight());
  return std::nullopt;
}

std::optional<LayoutUnit> RenderBox::computeIntrinsicLogicalContentHeightUsing(Length logicalHeightLength, std::optional<LayoutUnit> intrinsicContentHeight, LayoutUnit borderAndPadding) const
{
  // FIXME: The CSS sizing spec is considering changing what min-content/max-content should resolve to.
  // If that happens, this code will have to change.
  if (logicalHeightLength.isMinContent() || logicalHeightLength.isMaxContent() || logicalHeightLength.isFitContent()) {
      if (!intrinsicContentHeight)
          return intrinsicContentHeight;
      if (style().boxSizing() == BORDER_BOX)
          return intrinsicContentHeight.value() + borderAndPaddingLogicalHeight();
      return intrinsicContentHeight;
  }
  if (logicalHeightLength.isFillAvailable())
      return containingBlock()->availableLogicalHeight(ExcludeMarginBorderPadding) - borderAndPadding;
  ASSERT_NOT_REACHED();
  return LayoutUnit(0);
}

std::optional<LayoutUnit> RenderBox::computeContentAndScrollbarLogicalHeightUsing(SizeType heightType, const Length& height, std::optional<LayoutUnit> intrinsicContentHeight) const
{
  if (height.isAuto())
      return heightType == MinSize ? std::optional<LayoutUnit>(0) : std::nullopt;
  // FIXME: The CSS sizing spec is considering changing what min-content/max-content should resolve to.
  // If that happens, this code will have to change.
  if (height.isIntrinsic())
      return computeIntrinsicLogicalContentHeightUsing(height, intrinsicContentHeight, borderAndPaddingLogicalHeight());
  if (height.isFixed())
      return LayoutUnit(height.value());
  if (height.isPercentOrCalculated())
      return computePercentageLogicalHeight(height);
  return std::nullopt;
}

bool RenderBox::skipContainingBlockForPercentHeightCalculation(const RenderBox& containingBlock, bool isPerpendicularWritingMode) const
{
  // Flow threads for multicol or paged overflow should be skipped. They are invisible to the DOM,
  // and percent heights of children should be resolved against the multicol or paged container.
  if (containingBlock.isInFlowRenderFragmentedFlow() && !isPerpendicularWritingMode)
      return true;

  // Render view is not considered auto height.
  //if (is<RenderView>(containingBlock))
    //  return false;

  // If the writing mode of the containing block is orthogonal to ours, it means
  // that we shouldn't skip anything, since we're going to resolve the
  // percentage height against a containing block *width*.
  if (isPerpendicularWritingMode)
      return false;

  // Anonymous blocks should not impede percentage resolution on a child.
  // Examples of such anonymous blocks are blocks wrapped around inlines that
  // have block siblings (from the CSS spec) and multicol flow threads (an
  // implementation detail). Another implementation detail, ruby runs, create
  // anonymous inline-blocks, so skip those too. All other types of anonymous
  // objects, such as table-cells and flexboxes, will be treated as if they were
  // non-anonymous.
  if (containingBlock.isAnonymous())
      return containingBlock.style().display() == BLOCK || containingBlock.style().display() == INLINE_BLOCK;

  // For quirks mode, we skip most auto-height containing blocks when computing
  // percentages.
  return !containingBlock.isTableCell() && !containingBlock.isOutOfFlowPositioned() && !containingBlock.isRenderGrid() && containingBlock.style().logicalHeight().isAuto();
}



static bool tableCellShouldHaveZeroInitialSize(const RenderBlock& block, const RenderBox& child, bool scrollsOverflowY)
{
  // Normally we would let the cell size intrinsically, but scrolling overflow has to be
  // treated differently, since WinIE lets scrolled overflow fragments shrink as needed.
  // While we can't get all cases right, we can at least detect when the cell has a specified
  // height or when the table has a specified height. In these cases we want to initially have
  // no size and allow the flexing of the table or the cell to its specified height to cause us
  // to grow to fill the space. This could end up being wrong in some cases, but it is
  // preferable to the alternative (sizing intrinsically and making the row end up too big).
  const RenderTableCell& cell = downcast<RenderTableCell>(block);
  return scrollsOverflowY && !child.shouldTreatChildAsReplacedInTableCells() && (!cell.style().logicalHeight().isAuto() || !cell.table()->style().logicalHeight().isAuto());
}

std::optional<LayoutUnit> RenderBox::computePercentageLogicalHeight(const Length& height) const
{
  std::optional<LayoutUnit> availableHeight;

  bool skippedAutoHeightContainingBlock = false;
  RenderBlock* cb = containingBlock();
  const RenderBox* containingBlockChild = this;
  LayoutUnit rootMarginBorderPaddingHeight = 0;
  bool isHorizontal = isHorizontalWritingMode();
  while (cb  && skipContainingBlockForPercentHeightCalculation(*cb, isHorizontal != cb->isHorizontalWritingMode())) {
      if (cb->isBody())
          rootMarginBorderPaddingHeight += cb->marginBefore() + cb->marginAfter() + cb->borderAndPaddingLogicalHeight();
      skippedAutoHeightContainingBlock = true;
      containingBlockChild = cb;
      cb = cb->containingBlock();
  }
  cb->addPercentHeightDescendant(const_cast<RenderBox&>(*this));

  if (isHorizontal != cb->isHorizontalWritingMode())
      availableHeight = containingBlockChild->containingBlockLogicalWidthForContent();
  else if (hasOverrideContainingBlockLogicalHeight())
      availableHeight = overrideContainingBlockContentLogicalHeight();
  else if (is<RenderTableCell>(*cb)) {
      if (!skippedAutoHeightContainingBlock) {
          // Table cells violate what the CSS spec says to do with heights. Basically we
          // don't care if the cell specified a height or not. We just always make ourselves
          // be a percentage of the cell's current content height.
          if (!cb->hasOverrideLogicalContentHeight())
              return tableCellShouldHaveZeroInitialSize(*cb, *this, scrollsOverflowY()) ? std::optional<LayoutUnit>(0) : std::nullopt;

          availableHeight = cb->overrideLogicalContentHeight();
      }
  } else
      availableHeight = cb->availableLogicalHeightForPercentageComputation();

  if (!availableHeight)
      return availableHeight;

  LayoutUnit result = valueForLength(height, availableHeight.value() - rootMarginBorderPaddingHeight + (isTable() && isOutOfFlowPositioned() ? cb->paddingBefore() + cb->paddingAfter() : LayoutUnit()));

  // |overrideLogicalContentHeight| is the maximum height made available by the
  // cell to its percent height children when we decide they can determine the
  // height of the cell. If the percent height child is box-sizing:content-box
  // then we must subtract the border and padding from the cell's
  // |availableHeight| (given by |overrideLogicalContentHeight|) to arrive
  // at the child's computed height.
  bool subtractBorderAndPadding = isTable() || (is<RenderTableCell>(*cb) && !skippedAutoHeightContainingBlock && cb->hasOverrideLogicalContentHeight());
  if (subtractBorderAndPadding) {
      result -= borderAndPaddingLogicalHeight();
      return std::max(LayoutUnit(), result);
  }
  return result;
}

LayoutUnit RenderBox::computeReplacedLogicalWidth(ShouldComputePreferred shouldComputePreferred) const
{
  return computeReplacedLogicalWidthRespectingMinMaxWidth(computeReplacedLogicalWidthUsing(MainOrPreferredSize, style().logicalWidth()), shouldComputePreferred);
}

LayoutUnit RenderBox::computeReplacedLogicalWidthRespectingMinMaxWidth(LayoutUnit logicalWidth, ShouldComputePreferred shouldComputePreferred) const
{
  auto& logicalMinWidth = style().logicalMinWidth();
  auto& logicalMaxWidth = style().logicalMaxWidth();
  bool useLogicalWidthForMinWidth = (shouldComputePreferred == ComputePreferred && logicalMinWidth.isPercentOrCalculated()) || logicalMinWidth.isUndefined();
  bool useLogicalWidthForMaxWidth = (shouldComputePreferred == ComputePreferred && logicalMaxWidth.isPercentOrCalculated()) || logicalMaxWidth.isUndefined();
  auto minLogicalWidth =  useLogicalWidthForMinWidth ? logicalWidth : computeReplacedLogicalWidthUsing(MinSize, logicalMinWidth);
  auto maxLogicalWidth =  useLogicalWidthForMaxWidth ? logicalWidth : computeReplacedLogicalWidthUsing(MaxSize, logicalMaxWidth);
  return std::max(minLogicalWidth, std::min(logicalWidth, maxLogicalWidth));
}

LayoutUnit RenderBox::computeReplacedLogicalWidthUsing(SizeType widthType, Length logicalWidth) const
{
  ASSERT(widthType == MinSize || widthType == MainOrPreferredSize || !logicalWidth.isAuto());
  if (widthType == MinSize && logicalWidth.isAuto())
      return adjustContentBoxLogicalWidthForBoxSizing(0);

  switch (logicalWidth.type()) {
      case Fixed:
          return adjustContentBoxLogicalWidthForBoxSizing(logicalWidth.value());
      case MinContent:
      case MaxContent: {
          // MinContent/MaxContent don't need the availableLogicalWidth argument.
          LayoutUnit availableLogicalWidth = 0;
          return computeIntrinsicLogicalWidthUsing(logicalWidth, availableLogicalWidth, borderAndPaddingLogicalWidth()) - borderAndPaddingLogicalWidth();
      }
      case FitContent:
      case FillAvailable:
      case Percent:
      case Calculated: {
          // FIXME: containingBlockLogicalWidthForContent() is wrong if the replaced element's block-flow is perpendicular to the
          // containing block's block-flow.
          // https://bugs.webkit.org/show_bug.cgi?id=46496
          const LayoutUnit cw = isOutOfFlowPositioned() ? containingBlockLogicalWidthForPositioned(downcast<RenderBoxModelObject>(*container())) : containingBlockLogicalWidthForContent();
          Length containerLogicalWidth = containingBlock()->style().logicalWidth();
          // FIXME: Handle cases when containing block width is calculated or viewport percent.
          // https://bugs.webkit.org/show_bug.cgi?id=91071
          if (logicalWidth.isIntrinsic())
              return computeIntrinsicLogicalWidthUsing(logicalWidth, cw, borderAndPaddingLogicalWidth()) - borderAndPaddingLogicalWidth();
          if (cw > 0 || (!cw && (containerLogicalWidth.isFixed() || containerLogicalWidth.isPercentOrCalculated())))
              return adjustContentBoxLogicalWidthForBoxSizing(minimumValueForLength(logicalWidth, cw));
          return LayoutUnit();
      }
      case Intrinsic:
      case MinIntrinsic:
      case Auto:
      case Relative:
      case Undefined:
          return intrinsicLogicalWidth();
  }

  ASSERT_NOT_REACHED();
  return 0;
}

LayoutUnit RenderBox::computeReplacedLogicalHeight(std::optional<LayoutUnit>) const
{
  return computeReplacedLogicalHeightRespectingMinMaxHeight(computeReplacedLogicalHeightUsing(MainOrPreferredSize, style().logicalHeight()));
}

static bool allowMinMaxPercentagesInAutoHeightBlocksQuirk()
{
#if PLATFORM(MAC)
  return MacApplication::isIBooks();
#elif PLATFORM(IOS)
  return IOSApplication::isIBooks();
#endif
  return false;
}

bool RenderBox::replacedMinMaxLogicalHeightComputesAsNone(SizeType sizeType) const
{
  ASSERT(sizeType == MinSize || sizeType == MaxSize);

  auto logicalHeight = sizeType == MinSize ? style().logicalMinHeight() : style().logicalMaxHeight();
  auto initialLogicalHeight = sizeType == MinSize ? RenderStyle::initialMinSize() : RenderStyle::initialMaxSize();

  if (logicalHeight == initialLogicalHeight)
      return true;

  // Make sure % min-height and % max-height resolve to none if the containing block has auto height.
  // Note that the "height" case for replaced elements was handled by hasReplacedLogicalHeight, which is why
  // min and max-height are the only ones handled here.
  // FIXME: For now we put in a quirk for iBooks until we can move them to viewport units.
  if (auto* cb = containingBlockForAutoHeightDetection(logicalHeight))
      return allowMinMaxPercentagesInAutoHeightBlocksQuirk() ? false : cb->hasAutoHeightOrContainingBlockWithAutoHeight();

  return false;
}

LayoutUnit RenderBox::computeReplacedLogicalHeightRespectingMinMaxHeight(LayoutUnit logicalHeight) const
{
  LayoutUnit minLogicalHeight;
  if (!replacedMinMaxLogicalHeightComputesAsNone(MinSize))
      minLogicalHeight = computeReplacedLogicalHeightUsing(MinSize, style().logicalMinHeight());
  LayoutUnit maxLogicalHeight = logicalHeight;
  if (!replacedMinMaxLogicalHeightComputesAsNone(MaxSize))
      maxLogicalHeight = computeReplacedLogicalHeightUsing(MaxSize, style().logicalMaxHeight());
  return std::max(minLogicalHeight, std::min(logicalHeight, maxLogicalHeight));
}

LayoutUnit RenderBox::computeReplacedLogicalHeightUsing(SizeType heightType, Length logicalHeight) const
{
  ASSERT(heightType == MinSize || heightType == MainOrPreferredSize || !logicalHeight.isAuto());
  if (heightType == MinSize && logicalHeight.isAuto())
      return adjustContentBoxLogicalHeightForBoxSizing(std::optional<LayoutUnit>(0));

  switch (logicalHeight.type()) {
      case Fixed:
          return adjustContentBoxLogicalHeightForBoxSizing(LayoutUnit(logicalHeight.value()));
      case Percent:
      case Calculated:
      {
          auto* container = isOutOfFlowPositioned() ? this->container() : containingBlock();
          /**
          while (container && container->isAnonymous()) {
              // Stop at rendering context root.
              if (is<RenderView>(*container))
                  break;
              container = container->containingBlock();
          }*/
          std::optional<LayoutUnit> stretchedHeight;
          if (is<RenderBlock>(container)) {
              auto* block = downcast<RenderBlock>(container);
              block->addPercentHeightDescendant(*const_cast<RenderBox*>(this));
              if (block->isFlexItem())
                  stretchedHeight = downcast<RenderFlexibleBox>(block->parent())->childLogicalHeightForPercentageResolution(*block);
              else if (block->isGridItem() && block->hasOverrideLogicalContentHeight())
                  stretchedHeight = block->overrideLogicalContentHeight();
          }

          // FIXME: This calculation is not patched for block-flow yet.
          // https://bugs.webkit.org/show_bug.cgi?id=46500
          /**
          if (container->isOutOfFlowPositioned()
              && container->style().height().isAuto()
              && !(container->style().top().isAuto() || container->style().bottom().isAuto())) {
              ASSERT_WITH_SECURITY_IMPLICATION(container->isRenderBlock());
              auto& block = downcast<RenderBlock>(*container);
              auto computedValues = block.computeLogicalHeight(block.logicalHeight(), 0);
              LayoutUnit newContentHeight = computedValues.m_extent - block.borderAndPaddingLogicalHeight() - block.scrollbarLogicalHeight();
              return adjustContentBoxLogicalHeightForBoxSizing(valueForLength(logicalHeight, newContentHeight));
          }*/

          // FIXME: availableLogicalHeight() is wrong if the replaced element's block-flow is perpendicular to the
          // containing block's block-flow.
          // https://bugs.webkit.org/show_bug.cgi?id=46496
          LayoutUnit availableHeight;
          if (isOutOfFlowPositioned())
              availableHeight = containingBlockLogicalHeightForPositioned(downcast<RenderBoxModelObject>(*container));
          else if (stretchedHeight)
              availableHeight = stretchedHeight.value();
          else {
              availableHeight = containingBlockLogicalHeightForContent(IncludeMarginBorderPadding);
              // It is necessary to use the border-box to match WinIE's broken
              // box model.  This is essential for sizing inside
              // table cells using percentage heights.
              // FIXME: This needs to be made block-flow-aware.  If the cell and image are perpendicular block-flows, this isn't right.
              // https://bugs.webkit.org/show_bug.cgi?id=46997
              /**
              while (container && !is<RenderView>(*container)
                  && (container->style().logicalHeight().isAuto() || container->style().logicalHeight().isPercentOrCalculated())) {
                  if (container->isTableCell()) {
                      // Don't let table cells squeeze percent-height replaced elements
                      // <http://bugs.webkit.org/show_bug.cgi?id=15359>
                      availableHeight = std::max(availableHeight, intrinsicLogicalHeight());
                      return valueForLength(logicalHeight, availableHeight - borderAndPaddingLogicalHeight());
                  }
                  downcast<RenderBlock>(*container).addPercentHeightDescendant(const_cast<RenderBox&>(*this));
                  container = container->containingBlock();
              }*/
          }
          return adjustContentBoxLogicalHeightForBoxSizing(valueForLength(logicalHeight, availableHeight));
      }
      case MinContent:
      case MaxContent:
      case FitContent:
      case FillAvailable:
          return adjustContentBoxLogicalHeightForBoxSizing(computeIntrinsicLogicalContentHeightUsing(logicalHeight, intrinsicLogicalHeight(), borderAndPaddingLogicalHeight()));
      default:
          return intrinsicLogicalHeight();
  }
}

LayoutUnit RenderBox::availableLogicalHeight(AvailableLogicalHeightType heightType) const
{
  return constrainLogicalHeightByMinMax(availableLogicalHeightUsing(style().logicalHeight(), heightType), std::nullopt);
}

LayoutUnit RenderBox::availableLogicalHeightUsing(const Length& h, AvailableLogicalHeightType heightType) const
{
  // We need to stop here, since we don't want to increase the height of the table
  // artificially.  We're going to rely on this cell getting expanded to some new
  // height, and then when we lay out again we'll use the calculation below.
  if (isTableCell() && (h.isAuto() || h.isPercentOrCalculated())) {
      if (hasOverrideLogicalContentHeight())
          return overrideLogicalContentHeight();
      return logicalHeight() - borderAndPaddingLogicalHeight();
  }

  if (isFlexItem()) {
      auto& flexBox = downcast<RenderFlexibleBox>(*parent());
      auto stretchedHeight = flexBox.childLogicalHeightForPercentageResolution(*this);
      if (stretchedHeight)
          return stretchedHeight.value();
  }

  if (h.isPercentOrCalculated() && isOutOfFlowPositioned() && !isRenderFragmentedFlow()) {
      // FIXME: This is wrong if the containingBlock has a perpendicular writing mode.
      LayoutUnit availableHeight = containingBlockLogicalHeightForPositioned(*containingBlock());
      return adjustContentBoxLogicalHeightForBoxSizing(valueForLength(h, availableHeight));
  }

  if (std::optional<LayoutUnit> heightIncludingScrollbar = computeContentAndScrollbarLogicalHeightUsing(MainOrPreferredSize, h, std::nullopt))
      return std::max<LayoutUnit>(0, adjustContentBoxLogicalHeightForBoxSizing(heightIncludingScrollbar) - scrollbarLogicalHeight());

  // FIXME: Check logicalTop/logicalBottom here to correctly handle vertical writing-mode.
  // https://bugs.webkit.org/show_bug.cgi?id=46500
  if (is<RenderBlock>(*this) && isOutOfFlowPositioned() && style().height().isAuto() && !(style().top().isAuto() || style().bottom().isAuto())) {
      RenderBlock& block = const_cast<RenderBlock&>(downcast<RenderBlock>(*this));
      auto computedValues = block.computeLogicalHeight(block.logicalHeight(), 0);
      return computedValues.m_extent - block.borderAndPaddingLogicalHeight() - block.scrollbarLogicalHeight();
  }

  // FIXME: This is wrong if the containingBlock has a perpendicular writing mode.
  LayoutUnit availableHeight = containingBlockLogicalHeightForContent(heightType);
  if (heightType == ExcludeMarginBorderPadding) {
      // FIXME: Margin collapsing hasn't happened yet, so this incorrectly removes collapsed margins.
      availableHeight -= marginBefore() + marginAfter() + borderAndPaddingLogicalHeight();
  }
  return availableHeight;
}

void RenderBox::computeBlockDirectionMargins(const RenderBlock& containingBlock, LayoutUnit& marginBefore, LayoutUnit& marginAfter) const
{
  if (isTableCell()) {
      // FIXME: Not right if we allow cells to have different directionality than the table.  If we do allow this, though,
      // we may just do it with an extra anonymous block inside the cell.
      marginBefore = 0;
      marginAfter = 0;
      return;
  }

  // Margins are calculated with respect to the logical width of
  // the containing block (8.3)
  LayoutUnit cw = containingBlockLogicalWidthForContent();
  const RenderStyle& containingBlockStyle = containingBlock.style();
  marginBefore = minimumValueForLength(style().marginBeforeUsing(&containingBlockStyle), cw);
  marginAfter = minimumValueForLength(style().marginAfterUsing(&containingBlockStyle), cw);
}

void RenderBox::computeAndSetBlockDirectionMargins(const RenderBlock& containingBlock)
{
  LayoutUnit marginBefore;
  LayoutUnit marginAfter;
  computeBlockDirectionMargins(containingBlock, marginBefore, marginAfter);
  containingBlock.setMarginBeforeForChild(*this, marginBefore);
  containingBlock.setMarginAfterForChild(*this, marginAfter);
}




void RenderBox::computePositionedLogicalHeightUsing(SizeType heightType, Length logicalHeightLength, const RenderBoxModelObject& containerBlock,
                                                    LayoutUnit containerLogicalHeight, LayoutUnit bordersPlusPadding, LayoutUnit logicalHeight,
                                                    Length logicalTop, Length logicalBottom, Length marginBefore, Length marginAfter,
                                                    LogicalExtentComputedValues& computedValues) const
{
    ASSERT(heightType == MinSize || heightType == MainOrPreferredSize || !logicalHeightLength.isAuto());
    if (heightType == MinSize && logicalHeightLength.isAuto())
        logicalHeightLength = Length(0, Fixed);

    // 'top' and 'bottom' cannot both be 'auto' because 'top would of been
    // converted to the static position in computePositionedLogicalHeight()
    ASSERT(!(logicalTop.isAuto() && logicalBottom.isAuto()));

    LayoutUnit logicalHeightValue;
    LayoutUnit contentLogicalHeight = logicalHeight - bordersPlusPadding;

    const LayoutUnit containerRelativeLogicalWidth = containingBlockLogicalWidthForPositioned(containerBlock, nullptr, false);

    LayoutUnit logicalTopValue = 0;

    bool logicalHeightIsAuto = logicalHeightLength.isAuto();
    bool logicalTopIsAuto = logicalTop.isAuto();
    bool logicalBottomIsAuto = logicalBottom.isAuto();

    // Height is never unsolved for tables.
    LayoutUnit resolvedLogicalHeight;
    if (isTable()) {
        resolvedLogicalHeight = contentLogicalHeight;
        logicalHeightIsAuto = false;
    } else {
        if (logicalHeightLength.isIntrinsic())
            resolvedLogicalHeight = computeIntrinsicLogicalContentHeightUsing(logicalHeightLength, contentLogicalHeight, bordersPlusPadding).value();
        else
            resolvedLogicalHeight = adjustContentBoxLogicalHeightForBoxSizing(valueForLength(logicalHeightLength, containerLogicalHeight));
    }

    if (!logicalTopIsAuto && !logicalHeightIsAuto && !logicalBottomIsAuto) {
        /*-----------------------------------------------------------------------*\
         * If none of the three are 'auto': If both 'margin-top' and 'margin-
         * bottom' are 'auto', solve the equation under the extra constraint that
         * the two margins get equal values. If one of 'margin-top' or 'margin-
         * bottom' is 'auto', solve the equation for that value. If the values
         * are over-constrained, ignore the value for 'bottom' and solve for that
         * value.
        \*-----------------------------------------------------------------------*/
        // NOTE:  It is not necessary to solve for 'bottom' in the over constrained
        // case because the value is not used for any further calculations.

        logicalHeightValue = resolvedLogicalHeight;
        logicalTopValue = valueForLength(logicalTop, containerLogicalHeight);

        const LayoutUnit availableSpace = containerLogicalHeight - (logicalTopValue + logicalHeightValue + valueForLength(logicalBottom, containerLogicalHeight) + bordersPlusPadding);

        // Margins are now the only unknown
        if (marginBefore.isAuto() && marginAfter.isAuto()) {
            // Both margins auto, solve for equality
            // NOTE: This may result in negative values.
            computedValues.m_margins.m_before = availableSpace / 2; // split the difference
            computedValues.m_margins.m_after = availableSpace - computedValues.m_margins.m_before; // account for odd valued differences
        } else if (marginBefore.isAuto()) {
            // Solve for top margin
            computedValues.m_margins.m_after = valueForLength(marginAfter, containerRelativeLogicalWidth);
            computedValues.m_margins.m_before = availableSpace - computedValues.m_margins.m_after;
        } else if (marginAfter.isAuto()) {
            // Solve for bottom margin
            computedValues.m_margins.m_before = valueForLength(marginBefore, containerRelativeLogicalWidth);
            computedValues.m_margins.m_after = availableSpace - computedValues.m_margins.m_before;
        } else {
            // Over-constrained, (no need solve for bottom)
            computedValues.m_margins.m_before = valueForLength(marginBefore, containerRelativeLogicalWidth);
            computedValues.m_margins.m_after = valueForLength(marginAfter, containerRelativeLogicalWidth);
        }
    } else {
        /*--------------------------------------------------------------------*\
         * Otherwise, set 'auto' values for 'margin-top' and 'margin-bottom'
         * to 0, and pick the one of the following six rules that applies.
         *
         * 1. 'top' and 'height' are 'auto' and 'bottom' is not 'auto', then
         *    the height is based on the content, and solve for 'top'.
         *
         *              OMIT RULE 2 AS IT SHOULD NEVER BE HIT
         * ------------------------------------------------------------------
         * 2. 'top' and 'bottom' are 'auto' and 'height' is not 'auto', then
         *    set 'top' to the static position, and solve for 'bottom'.
         * ------------------------------------------------------------------
         *
         * 3. 'height' and 'bottom' are 'auto' and 'top' is not 'auto', then
         *    the height is based on the content, and solve for 'bottom'.
         * 4. 'top' is 'auto', 'height' and 'bottom' are not 'auto', and
         *    solve for 'top'.
         * 5. 'height' is 'auto', 'top' and 'bottom' are not 'auto', and
         *    solve for 'height'.
         * 6. 'bottom' is 'auto', 'top' and 'height' are not 'auto', and
         *    solve for 'bottom'.
        \*--------------------------------------------------------------------*/
        // NOTE: For rules 3 and 6 it is not necessary to solve for 'bottom'
        // because the value is not used for any further calculations.

        // Calculate margins, 'auto' margins are ignored.
        computedValues.m_margins.m_before = minimumValueForLength(marginBefore, containerRelativeLogicalWidth);
        computedValues.m_margins.m_after = minimumValueForLength(marginAfter, containerRelativeLogicalWidth);

        const LayoutUnit availableSpace = containerLogicalHeight - (computedValues.m_margins.m_before + computedValues.m_margins.m_after + bordersPlusPadding);

        // Use rule/case that applies.
        if (logicalTopIsAuto && logicalHeightIsAuto && !logicalBottomIsAuto) {
            // RULE 1: (height is content based, solve of top)
            logicalHeightValue = contentLogicalHeight;
            logicalTopValue = availableSpace - (logicalHeightValue + valueForLength(logicalBottom, containerLogicalHeight));
        } else if (!logicalTopIsAuto && logicalHeightIsAuto && logicalBottomIsAuto) {
            // RULE 3: (height is content based, no need solve of bottom)
            logicalTopValue = valueForLength(logicalTop, containerLogicalHeight);
            logicalHeightValue = contentLogicalHeight;
        } else if (logicalTopIsAuto && !logicalHeightIsAuto && !logicalBottomIsAuto) {
            // RULE 4: (solve of top)
            logicalHeightValue = resolvedLogicalHeight;
            logicalTopValue = availableSpace - (logicalHeightValue + valueForLength(logicalBottom, containerLogicalHeight));
        } else if (!logicalTopIsAuto && logicalHeightIsAuto && !logicalBottomIsAuto) {
            // RULE 5: (solve of height)
            logicalTopValue = valueForLength(logicalTop, containerLogicalHeight);
            logicalHeightValue = std::max<LayoutUnit>(0, availableSpace - (logicalTopValue + valueForLength(logicalBottom, containerLogicalHeight)));
        } else if (!logicalTopIsAuto && !logicalHeightIsAuto && logicalBottomIsAuto) {
            // RULE 6: (no need solve of bottom)
            logicalHeightValue = resolvedLogicalHeight;
            logicalTopValue = valueForLength(logicalTop, containerLogicalHeight);
        }
    }
    computedValues.m_extent = logicalHeightValue;

    // Use computed values to calculate the vertical position.
    computedValues.m_position = logicalTopValue + computedValues.m_margins.m_before;
    //computeLogicalTopPositionedOffset(computedValues.m_position, this, logicalHeightValue, containerBlock, containerLogicalHeight);
}

void RenderBox::computePositionedLogicalWidthReplaced(LogicalExtentComputedValues& computedValues) const
{
    // The following is based off of the W3C Working Draft from April 11, 2006 of
    // CSS 2.1: Section 10.3.8 "Absolutely positioned, replaced elements"
    // <http://www.w3.org/TR/2005/WD-CSS21-20050613/visudet.html#abs-replaced-width>
    // (block-style-comments in this function correspond to text from the spec and
    // the numbers correspond to numbers in spec)

    // We don't use containingBlock(), since we may be positioned by an enclosing
    // relative positioned inline.
    const RenderBoxModelObject& containerBlock = downcast<RenderBoxModelObject>(*container());

    const LayoutUnit containerLogicalWidth = containingBlockLogicalWidthForPositioned(containerBlock);
    const LayoutUnit containerRelativeLogicalWidth = containingBlockLogicalWidthForPositioned(containerBlock, nullptr, false);

    // To match WinIE, in quirks mode use the parent's 'direction' property
    // instead of the container block's.
    TextDirection containerDirection = containerBlock.style().direction();

    // Variables to solve.
    bool isHorizontal = isHorizontalWritingMode();
    Length logicalLeft = style().logicalLeft();
    Length logicalRight = style().logicalRight();
    Length marginLogicalLeft = isHorizontal ? style().marginLeft() : style().marginTop();
    Length marginLogicalRight = isHorizontal ? style().marginRight() : style().marginBottom();
    LayoutUnit& marginLogicalLeftAlias = style().isLeftToRightDirection() ? computedValues.m_margins.m_start : computedValues.m_margins.m_end;
    LayoutUnit& marginLogicalRightAlias = style().isLeftToRightDirection() ? computedValues.m_margins.m_end : computedValues.m_margins.m_start;

    /*-----------------------------------------------------------------------*\
     * 1. The used value of 'width' is determined as for inline replaced
     *    elements.
    \*-----------------------------------------------------------------------*/
    // NOTE: This value of width is final in that the min/max width calculations
    // are dealt with in computeReplacedWidth().  This means that the steps to produce
    // correct max/min in the non-replaced version, are not necessary.
    computedValues.m_extent = computeReplacedLogicalWidth() + borderAndPaddingLogicalWidth();

    const LayoutUnit availableSpace = containerLogicalWidth - computedValues.m_extent;

    /*-----------------------------------------------------------------------*\
     * 2. If both 'left' and 'right' have the value 'auto', then if 'direction'
     *    of the containing block is 'ltr', set 'left' to the static position;
     *    else if 'direction' is 'rtl', set 'right' to the static position.
    \*-----------------------------------------------------------------------*/
    // see FIXME 1
    //computeInlineStaticDistance(logicalLeft, logicalRight, this, containerBlock, containerLogicalWidth, nullptr); // FIXME: Pass the fragment.

    /*-----------------------------------------------------------------------*\
     * 3. If 'left' or 'right' are 'auto', replace any 'auto' on 'margin-left'
     *    or 'margin-right' with '0'.
    \*-----------------------------------------------------------------------*/
    if (logicalLeft.isAuto() || logicalRight.isAuto()) {
        if (marginLogicalLeft.isAuto())
            marginLogicalLeft.setValue(Fixed, 0);
        if (marginLogicalRight.isAuto())
            marginLogicalRight.setValue(Fixed, 0);
    }

    /*-----------------------------------------------------------------------*\
     * 4. If at this point both 'margin-left' and 'margin-right' are still
     *    'auto', solve the equation under the extra constraint that the two
     *    margins must get equal values, unless this would make them negative,
     *    in which case when the direction of the containing block is 'ltr'
     *    ('rtl'), set 'margin-left' ('margin-right') to zero and solve for
     *    'margin-right' ('margin-left').
    \*-----------------------------------------------------------------------*/
    LayoutUnit logicalLeftValue = 0;
    LayoutUnit logicalRightValue = 0;

    if (marginLogicalLeft.isAuto() && marginLogicalRight.isAuto()) {
        // 'left' and 'right' cannot be 'auto' due to step 3
        ASSERT(!(logicalLeft.isAuto() && logicalRight.isAuto()));

        logicalLeftValue = valueForLength(logicalLeft, containerLogicalWidth);
        logicalRightValue = valueForLength(logicalRight, containerLogicalWidth);

        LayoutUnit difference = availableSpace - (logicalLeftValue + logicalRightValue);
        if (difference > 0) {
            marginLogicalLeftAlias = difference / 2; // split the difference
            marginLogicalRightAlias = difference - marginLogicalLeftAlias; // account for odd valued differences
        } else {
            // Use the containing block's direction rather than the parent block's
            // per CSS 2.1 reference test abspos-replaced-width-margin-000.
            if (containerDirection == LTR) {
                marginLogicalLeftAlias = 0;
                marginLogicalRightAlias = difference; // will be negative
            } else {
                marginLogicalLeftAlias = difference; // will be negative
                marginLogicalRightAlias = 0;
            }
        }

    /*-----------------------------------------------------------------------*\
     * 5. If at this point there is an 'auto' left, solve the equation for
     *    that value.
    \*-----------------------------------------------------------------------*/
    } else if (logicalLeft.isAuto()) {
        marginLogicalLeftAlias = valueForLength(marginLogicalLeft, containerRelativeLogicalWidth);
        marginLogicalRightAlias = valueForLength(marginLogicalRight, containerRelativeLogicalWidth);
        logicalRightValue = valueForLength(logicalRight, containerLogicalWidth);

        // Solve for 'left'
        logicalLeftValue = availableSpace - (logicalRightValue + marginLogicalLeftAlias + marginLogicalRightAlias);
    } else if (logicalRight.isAuto()) {
        marginLogicalLeftAlias = valueForLength(marginLogicalLeft, containerRelativeLogicalWidth);
        marginLogicalRightAlias = valueForLength(marginLogicalRight, containerRelativeLogicalWidth);
        logicalLeftValue = valueForLength(logicalLeft, containerLogicalWidth);

        // Solve for 'right'
        logicalRightValue = availableSpace - (logicalLeftValue + marginLogicalLeftAlias + marginLogicalRightAlias);
    } else if (marginLogicalLeft.isAuto()) {
        marginLogicalRightAlias = valueForLength(marginLogicalRight, containerRelativeLogicalWidth);
        logicalLeftValue = valueForLength(logicalLeft, containerLogicalWidth);
        logicalRightValue = valueForLength(logicalRight, containerLogicalWidth);

        // Solve for 'margin-left'
        marginLogicalLeftAlias = availableSpace - (logicalLeftValue + logicalRightValue + marginLogicalRightAlias);
    } else if (marginLogicalRight.isAuto()) {
        marginLogicalLeftAlias = valueForLength(marginLogicalLeft, containerRelativeLogicalWidth);
        logicalLeftValue = valueForLength(logicalLeft, containerLogicalWidth);
        logicalRightValue = valueForLength(logicalRight, containerLogicalWidth);

        // Solve for 'margin-right'
        marginLogicalRightAlias = availableSpace - (logicalLeftValue + logicalRightValue + marginLogicalLeftAlias);
    } else {
        // Nothing is 'auto', just calculate the values.
        marginLogicalLeftAlias = valueForLength(marginLogicalLeft, containerRelativeLogicalWidth);
        marginLogicalRightAlias = valueForLength(marginLogicalRight, containerRelativeLogicalWidth);
        logicalRightValue = valueForLength(logicalRight, containerLogicalWidth);
        logicalLeftValue = valueForLength(logicalLeft, containerLogicalWidth);
        // If the containing block is right-to-left, then push the left position as far to the right as possible
        if (containerDirection == RTL) {
            int totalLogicalWidth = computedValues.m_extent + logicalLeftValue + logicalRightValue +  marginLogicalLeftAlias + marginLogicalRightAlias;
            logicalLeftValue = containerLogicalWidth - (totalLogicalWidth - logicalLeftValue);
        }
    }

    /*-----------------------------------------------------------------------*\
     * 6. If at this point the values are over-constrained, ignore the value
     *    for either 'left' (in case the 'direction' property of the
     *    containing block is 'rtl') or 'right' (in case 'direction' is
     *    'ltr') and solve for that value.
    \*-----------------------------------------------------------------------*/
    // NOTE: Constraints imposed by the width of the containing block and its content have already been accounted for above.

    // FIXME: Deal with differing writing modes here.  Our offset needs to be in the containing block's coordinate space, so that
    // can make the result here rather complicated to compute.

    // Use computed values to calculate the horizontal position.

    // FIXME: This hack is needed to calculate the logical left position for a 'rtl' relatively
    // positioned, inline containing block because right now, it is using the logical left position
    // of the first line box when really it should use the last line box.  When
    // this is fixed elsewhere, this block should be removed.
    if (is<RenderInline>(containerBlock) && !containerBlock.style().isLeftToRightDirection()) {
        const auto& flow = downcast<RenderInline>(containerBlock);
        InlineFlowBox* firstLine = flow.firstLineBox();
        InlineFlowBox* lastLine = flow.lastLineBox();
        if (firstLine && lastLine && firstLine != lastLine) {
            computedValues.m_position = logicalLeftValue + marginLogicalLeftAlias + lastLine->borderLogicalLeft() + (lastLine->logicalLeft() - firstLine->logicalLeft());
            return;
        }
    }

    LayoutUnit logicalLeftPos = logicalLeftValue + marginLogicalLeftAlias;
    //computeLogicalLeftPositionedOffset(logicalLeftPos, this, computedValues.m_extent, containerBlock, containerLogicalWidth);
    computedValues.m_position = logicalLeftPos;
}

void RenderBox::computePositionedLogicalHeightReplaced(LogicalExtentComputedValues& computedValues) const
{
    // The following is based off of the W3C Working Draft from April 11, 2006 of
    // CSS 2.1: Section 10.6.5 "Absolutely positioned, replaced elements"
    // <http://www.w3.org/TR/2005/WD-CSS21-20050613/visudet.html#abs-replaced-height>
    // (block-style-comments in this function correspond to text from the spec and
    // the numbers correspond to numbers in spec)

    // We don't use containingBlock(), since we may be positioned by an enclosing relpositioned inline.
    const RenderBoxModelObject& containerBlock = downcast<RenderBoxModelObject>(*container());

    const LayoutUnit containerLogicalHeight = containingBlockLogicalHeightForPositioned(containerBlock);
    const LayoutUnit containerRelativeLogicalWidth = containingBlockLogicalWidthForPositioned(containerBlock, nullptr, false);

    // Variables to solve.
    Length marginBefore = style().marginBefore();
    Length marginAfter = style().marginAfter();
    LayoutUnit& marginBeforeAlias = computedValues.m_margins.m_before;
    LayoutUnit& marginAfterAlias = computedValues.m_margins.m_after;

    Length logicalTop = style().logicalTop();
    Length logicalBottom = style().logicalBottom();

    /*-----------------------------------------------------------------------*\
     * 1. The used value of 'height' is determined as for inline replaced
     *    elements.
    \*-----------------------------------------------------------------------*/
    // NOTE: This value of height is final in that the min/max height calculations
    // are dealt with in computeReplacedHeight().  This means that the steps to produce
    // correct max/min in the non-replaced version, are not necessary.
    computedValues.m_extent = computeReplacedLogicalHeight() + borderAndPaddingLogicalHeight();
    const LayoutUnit availableSpace = containerLogicalHeight - computedValues.m_extent;

    /*-----------------------------------------------------------------------*\
     * 2. If both 'top' and 'bottom' have the value 'auto', replace 'top'
     *    with the element's static position.
    \*-----------------------------------------------------------------------*/
    // see FIXME 1
    //computeBlockStaticDistance(logicalTop, logicalBottom, this, containerBlock);

    /*-----------------------------------------------------------------------*\
     * 3. If 'bottom' is 'auto', replace any 'auto' on 'margin-top' or
     *    'margin-bottom' with '0'.
    \*-----------------------------------------------------------------------*/
    // FIXME: The spec. says that this step should only be taken when bottom is
    // auto, but if only top is auto, this makes step 4 impossible.
    if (logicalTop.isAuto() || logicalBottom.isAuto()) {
        if (marginBefore.isAuto())
            marginBefore.setValue(Fixed, 0);
        if (marginAfter.isAuto())
            marginAfter.setValue(Fixed, 0);
    }

    /*-----------------------------------------------------------------------*\
     * 4. If at this point both 'margin-top' and 'margin-bottom' are still
     *    'auto', solve the equation under the extra constraint that the two
     *    margins must get equal values.
    \*-----------------------------------------------------------------------*/
    LayoutUnit logicalTopValue = 0;
    LayoutUnit logicalBottomValue = 0;

    if (marginBefore.isAuto() && marginAfter.isAuto()) {
        // 'top' and 'bottom' cannot be 'auto' due to step 2 and 3 combined.
        ASSERT(!(logicalTop.isAuto() || logicalBottom.isAuto()));

        logicalTopValue = valueForLength(logicalTop, containerLogicalHeight);
        logicalBottomValue = valueForLength(logicalBottom, containerLogicalHeight);

        LayoutUnit difference = availableSpace - (logicalTopValue + logicalBottomValue);
        // NOTE: This may result in negative values.
        marginBeforeAlias =  difference / 2; // split the difference
        marginAfterAlias = difference - marginBeforeAlias; // account for odd valued differences

    /*-----------------------------------------------------------------------*\
     * 5. If at this point there is only one 'auto' left, solve the equation
     *    for that value.
    \*-----------------------------------------------------------------------*/
    } else if (logicalTop.isAuto()) {
        marginBeforeAlias = valueForLength(marginBefore, containerRelativeLogicalWidth);
        marginAfterAlias = valueForLength(marginAfter, containerRelativeLogicalWidth);
        logicalBottomValue = valueForLength(logicalBottom, containerLogicalHeight);

        // Solve for 'top'
        logicalTopValue = availableSpace - (logicalBottomValue + marginBeforeAlias + marginAfterAlias);
    } else if (logicalBottom.isAuto()) {
        marginBeforeAlias = valueForLength(marginBefore, containerRelativeLogicalWidth);
        marginAfterAlias = valueForLength(marginAfter, containerRelativeLogicalWidth);
        logicalTopValue = valueForLength(logicalTop, containerLogicalHeight);

        // Solve for 'bottom'
        // NOTE: It is not necessary to solve for 'bottom' because we don't ever
        // use the value.
    } else if (marginBefore.isAuto()) {
        marginAfterAlias = valueForLength(marginAfter, containerRelativeLogicalWidth);
        logicalTopValue = valueForLength(logicalTop, containerLogicalHeight);
        logicalBottomValue = valueForLength(logicalBottom, containerLogicalHeight);

        // Solve for 'margin-top'
        marginBeforeAlias = availableSpace - (logicalTopValue + logicalBottomValue + marginAfterAlias);
    } else if (marginAfter.isAuto()) {
        marginBeforeAlias = valueForLength(marginBefore, containerRelativeLogicalWidth);
        logicalTopValue = valueForLength(logicalTop, containerLogicalHeight);
        logicalBottomValue = valueForLength(logicalBottom, containerLogicalHeight);

        // Solve for 'margin-bottom'
        marginAfterAlias = availableSpace - (logicalTopValue + logicalBottomValue + marginBeforeAlias);
    } else {
        // Nothing is 'auto', just calculate the values.
        marginBeforeAlias = valueForLength(marginBefore, containerRelativeLogicalWidth);
        marginAfterAlias = valueForLength(marginAfter, containerRelativeLogicalWidth);
        logicalTopValue = valueForLength(logicalTop, containerLogicalHeight);
        // NOTE: It is not necessary to solve for 'bottom' because we don't ever
        // use the value.
     }

    /*-----------------------------------------------------------------------*\
     * 6. If at this point the values are over-constrained, ignore the value
     *    for 'bottom' and solve for that value.
    \*-----------------------------------------------------------------------*/
    // NOTE: It is not necessary to do this step because we don't end up using
    // the value of 'bottom' regardless of whether the values are over-constrained
    // or not.

    // Use computed values to calculate the vertical position.
    LayoutUnit logicalTopPos = logicalTopValue + marginBeforeAlias;
    //computeLogicalTopPositionedOffset(logicalTopPos, this, computedValues.m_extent, containerBlock, containerLogicalHeight);
    computedValues.m_position = logicalTopPos;
}



bool RenderBox::shrinkToAvoidFloats() const
{
    // Floating objects don't shrink.  Objects that don't avoid floats don't shrink.  Marquees don't shrink.
    if ((isInline() && !isHTMLMarquee()) || !avoidsFloats() || isFloating())
        return false;
    
    // Only auto width objects can possibly shrink to avoid floats.
    return style().width().isAuto();
}

bool RenderBox::createsNewFormattingContext() const
{
    return isInlineBlockOrInlineTable() || isFloatingOrOutOfFlowPositioned() || hasOverflowClip() || isFlexItemIncludingDeprecated()
        || isTableCell() || isTableCaption() || isFieldset() || isWritingModeRoot()  || isRenderFragmentedFlow() || isRenderFragmentContainer()
        || isGridItem() || style().specifiesColumns() || style().columnSpan();
}

bool RenderBox::avoidsFloats() const
{
    return isReplaced()  || isLegend() || isFieldset() || createsNewFormattingContext();
}



void RenderBox::addVisualOverflow(const LayoutRect& rect)
{
    LayoutRect borderBox = borderBoxRect();
    if (borderBox.contains(rect) || rect.isEmpty())
        return;
        
    if (!m_overflow)
        m_overflow = adoptRef(new RenderOverflow(flippedClientBoxRect(), borderBox));
    
    m_overflow->addVisualOverflow(rect);
}

void RenderBox::clearOverflow()
{
    m_overflow = nullptr;
    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();
    //if (fragmentedFlow)
        //fragmentedFlow->clearFragmentsOverflow(this);
}
    
bool RenderBox::percentageLogicalHeightIsResolvable() const
{
    // Do this to avoid duplicating all the logic that already exists when computing
    // an actual percentage height.
    Length fakeLength(100, Percent);
    return computePercentageLogicalHeight(fakeLength) != std::nullopt;
}

bool RenderBox::hasUnsplittableScrollingOverflow() const
{
    // We will paginate as long as we don't scroll overflow in the pagination direction.
    bool isHorizontal = isHorizontalWritingMode();
    if ((isHorizontal && !scrollsOverflowY()) || (!isHorizontal && !scrollsOverflowX()))
        return false;
    
    // We do have overflow. We'll still be willing to paginate as long as the block
    // has auto logical height, auto or undefined max-logical-height and a zero or auto min-logical-height.
    // Note this is just a heuristic, and it's still possible to have overflow under these
    // conditions, but it should work out to be good enough for common cases. Paginating overflow
    // with scrollbars present is not the end of the world and is what we used to do in the old model anyway.
    return !style().logicalHeight().isIntrinsicOrAuto()
        || (!style().logicalMaxHeight().isIntrinsicOrAuto() && !style().logicalMaxHeight().isUndefined() && (!style().logicalMaxHeight().isPercentOrCalculated() || percentageLogicalHeightIsResolvable()))
        || (!style().logicalMinHeight().isIntrinsicOrAuto() && style().logicalMinHeight().isPositive() && (!style().logicalMinHeight().isPercentOrCalculated() || percentageLogicalHeightIsResolvable()));
}

bool RenderBox::isUnsplittableForPagination() const
{
    return isReplaced()
        || hasUnsplittableScrollingOverflow()
        || (parent() && isWritingModeRoot())
        || (isFloating() && style().styleType() == FIRST_LETTER && style().initialLetterDrop() > 0);
}

LayoutUnit RenderBox::lineHeight(bool /*firstLine*/, LineDirectionMode direction, LinePositionMode /*linePositionMode*/) const
{
    if (isReplaced())
        return direction == HorizontalLine ? m_marginBox.top() + height() + m_marginBox.bottom() : m_marginBox.right() + width() + m_marginBox.left();
    return 0;
}

int RenderBox::baselinePosition(FontBaseline baselineType, bool /*firstLine*/, LineDirectionMode direction, LinePositionMode /*linePositionMode*/) const
{
    if (isReplaced()) {
        int result = direction == HorizontalLine ? m_marginBox.top() + height() + m_marginBox.bottom() : m_marginBox.right() + width() + m_marginBox.left();
        if (baselineType == AlphabeticBaseline)
            return result;
        return result - result / 2;
    }
    return 0;
}


RenderLayer* RenderBox::enclosingFloatPaintingLayer() const
{

    return nullptr;
}

LayoutRect RenderBox::logicalVisualOverflowRectForPropagation(const RenderStyle* parentStyle) const
{
    LayoutRect rect = visualOverflowRectForPropagation(parentStyle);
    if (!parentStyle->isHorizontalWritingMode())
        return rect.transposedRect();
    return rect;
}

LayoutRect RenderBox::visualOverflowRectForPropagation(const RenderStyle* parentStyle) const
{
    // If the writing modes of the child and parent match, then we don't have to 
    // do anything fancy. Just return the result.
    LayoutRect rect = visualOverflowRect();
    if (parentStyle->writingMode() == style().writingMode())
        return rect;
    
    // We are putting ourselves into our parent's coordinate space.  If there is a flipped block mismatch
    // in a particular axis, then we have to flip the rect along that axis.
    if (style().writingMode() == RightToLeftWritingMode || parentStyle->writingMode() == RightToLeftWritingMode)
        rect.setX(width() - rect.maxX());
    else if (style().writingMode() == BottomToTopWritingMode || parentStyle->writingMode() == BottomToTopWritingMode)
        rect.setY(height() - rect.maxY());

    return rect;
}

LayoutRect RenderBox::logicalLayoutOverflowRectForPropagation(const RenderStyle* parentStyle) const
{
    LayoutRect rect = layoutOverflowRectForPropagation(parentStyle);
    if (!parentStyle->isHorizontalWritingMode())
        return rect.transposedRect();
    return rect;
}

LayoutRect RenderBox::layoutOverflowRectForPropagation(const RenderStyle* parentStyle) const
{
    // Only propagate interior layout overflow if we don't clip it.
    LayoutRect rect = borderBoxRect();
    if (!hasOverflowClip())
        rect.unite(layoutOverflowRect());

    bool hasTransform = this->hasTransform();
    if (isInFlowPositioned() || hasTransform) {
        // If we are relatively positioned or if we have a transform, then we have to convert
        // this rectangle into physical coordinates, apply relative positioning and transforms
        // to it, and then convert it back.
        flipForWritingMode(rect);
        
        //if (hasTransform)
          //  rect = layer()->currentTransform().mapRect(rect);

        if (isInFlowPositioned())
            rect.move(offsetForInFlowPosition());
        
        // Now we need to flip back.
        flipForWritingMode(rect);
    }
    
    // If the writing modes of the child and parent match, then we don't have to 
    // do anything fancy. Just return the result.
    if (parentStyle->writingMode() == style().writingMode())
        return rect;
    
    // We are putting ourselves into our parent's coordinate space.  If there is a flipped block mismatch
    // in a particular axis, then we have to flip the rect along that axis.
    if (style().writingMode() == RightToLeftWritingMode || parentStyle->writingMode() == RightToLeftWritingMode)
        rect.setX(width() - rect.maxX());
    else if (style().writingMode() == BottomToTopWritingMode || parentStyle->writingMode() == BottomToTopWritingMode)
        rect.setY(height() - rect.maxY());

    return rect;
}

LayoutRect RenderBox::flippedClientBoxRect() const
{
    // Because of the special coordinate system used for overflow rectangles (not quite logical, not
    // quite physical), we need to flip the block progression coordinate in vertical-rl and
    // horizontal-bt writing modes. Apart from that, this method does the same as clientBoxRect().

    LayoutUnit left = borderLeft();
    LayoutUnit top = borderTop();
    LayoutUnit right = borderRight();
    LayoutUnit bottom = borderBottom();
    // Calculate physical padding box.
    LayoutRect rect(left, top, width() - left - right, height() - top - bottom);
    // Flip block progression axis if writing mode is vertical-rl or horizontal-bt.
    flipForWritingMode(rect);
    // Subtract space occupied by scrollbars. They are at their physical edge in this coordinate
    // system, so order is important here: first flip, then subtract scrollbars.
    //if (shouldPlaceBlockDirectionScrollbarOnLeft())
      //  rect.move(verticalScrollbarWidth(), 0);
    rect.contract(verticalScrollbarWidth(), horizontalScrollbarHeight());
    return rect;
}

LayoutRect RenderBox::overflowRectForPaintRejection() const
{
    LayoutRect overflowRect = visualOverflowRect();
    
    if (!m_overflow || !usesCompositedScrolling())
        return overflowRect;

    overflowRect.unite(layoutOverflowRect());
    //overflowRect.moveBy(-scrollPosition());
    return overflowRect;
}

LayoutUnit RenderBox::offsetLeft() const
{
    return adjustedPositionRelativeToOffsetParent(topLeftLocation()).x();
}

LayoutUnit RenderBox::offsetTop() const
{
    return adjustedPositionRelativeToOffsetParent(topLeftLocation()).y();
}

LayoutPoint RenderBox::flipForWritingModeForChild(const RenderBox* child, const LayoutPoint& point) const
{
    if (!style().isFlippedBlocksWritingMode())
        return point;
    
    // The child is going to add in its x() and y(), so we have to make sure it ends up in
    // the right place.
    if (isHorizontalWritingMode())
        return LayoutPoint(point.x(), point.y() + height() - child->height() - (2 * child->y()));
    return LayoutPoint(point.x() + width() - child->width() - (2 * child->x()), point.y());
}

void RenderBox::flipForWritingMode(LayoutRect& rect) const
{
    if (!style().isFlippedBlocksWritingMode())
        return;

    if (isHorizontalWritingMode())
        rect.setY(height() - rect.maxY());
    else
        rect.setX(width() - rect.maxX());
}

LayoutUnit RenderBox::flipForWritingMode(LayoutUnit position) const
{
    if (!style().isFlippedBlocksWritingMode())
        return position;
    return logicalHeight() - position;
}

LayoutPoint RenderBox::flipForWritingMode(const LayoutPoint& position) const
{
    if (!style().isFlippedBlocksWritingMode())
        return position;
    return isHorizontalWritingMode() ? LayoutPoint(position.x(), height() - position.y()) : LayoutPoint(width() - position.x(), position.y());
}

LayoutSize RenderBox::flipForWritingMode(const LayoutSize& offset) const
{
    if (!style().isFlippedBlocksWritingMode())
        return offset;
    return isHorizontalWritingMode() ? LayoutSize(offset.width(), height() - offset.height()) : LayoutSize(width() - offset.width(), offset.height());
}

FloatPoint RenderBox::flipForWritingMode(const FloatPoint& position) const
{
    if (!style().isFlippedBlocksWritingMode())
        return position;
    return isHorizontalWritingMode() ? FloatPoint(position.x(), height() - position.y()) : FloatPoint(width() - position.x(), position.y());
}

void RenderBox::flipForWritingMode(FloatRect& rect) const
{
    if (!style().isFlippedBlocksWritingMode())
        return;

    if (isHorizontalWritingMode())
        rect.setY(height() - rect.maxY());
    else
        rect.setX(width() - rect.maxX());
}

LayoutPoint RenderBox::topLeftLocation() const
{

    //if (!view().frameView().hasFlippedBlockRenderers())
        return location();
    
    RenderBlock* containerBlock = containingBlock();
    if (!containerBlock || containerBlock == this)
        return location();
    return containerBlock->flipForWritingModeForChild(this, location());
}

LayoutSize RenderBox::topLeftLocationOffset() const
{
    return locationOffset();
    /*
    if (!view().frameView().hasFlippedBlockRenderers())
        return locationOffset();

    RenderBlock* containerBlock = containingBlock();
    if (!containerBlock || containerBlock == this)
        return locationOffset();
    
    LayoutRect rect(frameRect());
    containerBlock->flipForWritingMode(rect); // FIXME: This is wrong if we are an absolutely positioned object enclosed by a relative-positioned inline.
    return LayoutSize(rect.x(), rect.y());*/
}

void RenderBox::applyTopLeftLocationOffsetWithFlipping(LayoutPoint& point) const
{
    RenderBlock* containerBlock = containingBlock();
    if (!containerBlock || containerBlock == this) {
        point.move(m_frameRect.x(), m_frameRect.y());
        return;
    }
    
    LayoutRect rect(frameRect());
    containerBlock->flipForWritingMode(rect); // FIXME: This is wrong if we are an absolutely positioned object  enclosed by a relative-positioned inline.
    point.move(rect.x(), rect.y());
}

bool RenderBox::hasRelativeDimensions() const
{
    return style().height().isPercentOrCalculated() || style().width().isPercentOrCalculated()
        || style().maxHeight().isPercentOrCalculated() || style().maxWidth().isPercentOrCalculated()
        || style().minHeight().isPercentOrCalculated() || style().minWidth().isPercentOrCalculated();
}

bool RenderBox::hasRelativeLogicalHeight() const
{
    return style().logicalHeight().isPercentOrCalculated()
        || style().logicalMinHeight().isPercentOrCalculated()
        || style().logicalMaxHeight().isPercentOrCalculated();
}

bool RenderBox::hasRelativeLogicalWidth() const
{
    return style().logicalWidth().isPercentOrCalculated()
        || style().logicalMinWidth().isPercentOrCalculated()
        || style().logicalMaxWidth().isPercentOrCalculated();
}

static void markBoxForRelayoutAfterSplit(RenderBox& box)
{
    // FIXME: The table code should handle that automatically. If not,
    // we should fix it and remove the table part checks.
    if (is<RenderTable>(box)) {
        // Because we may have added some sections with already computed column structures, we need to
        // sync the table structure with them now. This avoids crashes when adding new cells to the table.
        downcast<RenderTable>(box).forceSectionsRecalc();
    } else if (is<RenderTableSection>(box))
        downcast<RenderTableSection>(box).setNeedsCellRecalc();

    box.setNeedsLayoutAndPrefWidthsRecalc();
}

RenderObject* RenderBox::splitAnonymousBoxesAroundChild(RenderObject* beforeChild)
{
    bool didSplitParentAnonymousBoxes = false;

    while (beforeChild->parent() != this) {
        auto& boxToSplit = downcast<RenderBox>(*beforeChild->parent());
        if (boxToSplit.firstChild() != beforeChild && boxToSplit.isAnonymous()) {
            didSplitParentAnonymousBoxes = true;

            // We have to split the parent box into two boxes and move children
            // from |beforeChild| to end into the new post box.
            auto newPostBox = boxToSplit.createAnonymousBoxWithSameTypeAs(*this);
            auto& postBox = *newPostBox;
            postBox.setChildrenInline(boxToSplit.childrenInline());
            RenderBox* parentBox = downcast<RenderBox>(boxToSplit.parent());
            // We need to invalidate the |parentBox| before inserting the new node
            // so that the table repainting logic knows the structure is dirty.
            // See for example RenderTableCell:clippedOverflowRectForRepaint.
            markBoxForRelayoutAfterSplit(*parentBox);
            parentBox->insertChildInternal(WTFMove(newPostBox), boxToSplit.nextSibling());
            boxToSplit.moveChildrenTo(&postBox, beforeChild, nullptr, RenderBoxModelObject::NormalizeAfterInsertion::Yes);

            markBoxForRelayoutAfterSplit(boxToSplit);
            markBoxForRelayoutAfterSplit(postBox);

            beforeChild = &postBox;
        } else
            beforeChild = &boxToSplit;
    }

    if (didSplitParentAnonymousBoxes)
        markBoxForRelayoutAfterSplit(*this);

    ASSERT(beforeChild->parent() == this);
    return beforeChild;
}

LayoutUnit RenderBox::offsetFromLogicalTopOfFirstPage() const
{
    return 0;
    /**
    auto* layoutState = view().frameView().layoutContext().layoutState();
    if ((layoutState && !layoutState->isPaginated()) || (!layoutState && !enclosingFragmentedFlow()))
        return 0;

    RenderBlock* containerBlock = containingBlock();
    return containerBlock->offsetFromLogicalTopOfFirstPage() + logicalTop();
     */
}

const RenderBox* RenderBox::findEnclosingScrollableContainer() const
{
    /**
    for (auto& candidate : lineageOfType<RenderBox>(*this)) {
        if (candidate.hasOverflowClip())
            return &candidate;
    }
    // If all parent elements are not overflow scrollable, check the body.
    if (document().body() && frame().mainFrame().view() && frame().mainFrame().view()->isScrollable())
        return document().body()->renderBox();*/
    
    return nullptr;
}

} // namespace WebCore
