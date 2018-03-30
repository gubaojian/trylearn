/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2005 Allan Sandfeld Jensen (kde@carewolf.com)
 *           (C) 2005, 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
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
#include "RenderBoxModelObject.h"

//#include "BitmapImage.h"
#include "BorderEdge.h"
//#include "CachedImage.h"
#include "FloatRoundedRect.h"
//#include "Frame.h"
//#include "FrameView.h"
#include "GeometryUtilities.h"
//#include "GraphicsContext.h"
//#include "HTMLFrameOwnerElement.h"
//#include "HTMLFrameSetElement.h"
//#include "HTMLImageElement.h"
//#include "HTMLNames.h"
//#include "ImageBuffer.h"
//#include "ImageQualityController.h"
#include "Path.h"
#include "RenderBlock.h"
#include "RenderFlexibleBox.h"
#include "RenderFragmentContainer.h"
#include "RenderInline.h"
//#include "RenderLayer.h"
//#include "RenderLayerBacking.h"
//#include "RenderLayerCompositor.h"
#include "RenderMultiColumnFlow.h"
#include "RenderTable.h"
#include "RenderTableRow.h"
//#include "RenderText.h"
//#include "RenderTextFragment.h"
//#include "RenderView.h"
//#include "ScrollingConstraints.h"
//#include "Settings.h"
//#include "TransformState.h"
#include <wtf/IsoMallocInlines.h>
#include <wtf/NeverDestroyed.h>
#if !ASSERT_DISABLED
#include <wtf/SetForScope.h>
#endif

#if PLATFORM(IOS)
#include "RuntimeApplicationChecks.h"
#endif

namespace WebCore {

//using namespace HTMLNames;

WTF_MAKE_ISO_ALLOCATED_IMPL(RenderBoxModelObject);

// The HashMap for storing continuation pointers.
// An inline can be split with blocks occuring in between the inline content.
// When this occurs we need a pointer to the next object. We can basically be
// split into a sequence of inlines and blocks. The continuation will either be
// an anonymous block (that houses other blocks) or it will be an inline flow.
// <b><i><p>Hello</p></i></b>. In this example the <i> will have a block as
// its continuation but the <b> will just have an inline as its continuation.

struct RenderBoxModelObject::ContinuationChainNode {
    WeakPtr<RenderBoxModelObject> renderer;
    ContinuationChainNode* previous { nullptr };
    ContinuationChainNode* next { nullptr };

    ContinuationChainNode(RenderBoxModelObject&);
    ~ContinuationChainNode();

    void insertAfter(ContinuationChainNode&);

    WTF_MAKE_FAST_ALLOCATED;
};

RenderBoxModelObject::ContinuationChainNode::ContinuationChainNode(RenderBoxModelObject& renderer)
    : renderer(makeWeakPtr(renderer))
{
}

RenderBoxModelObject::ContinuationChainNode::~ContinuationChainNode()
{
    if (next) {
        ASSERT(previous);
        ASSERT(next->previous == this);
        next->previous = previous;
    }
    if (previous) {
        ASSERT(previous->next == this);
        previous->next = next;
    }
}

void RenderBoxModelObject::ContinuationChainNode::insertAfter(ContinuationChainNode& after)
{
    ASSERT(!previous);
    ASSERT(!next);
    if ((next = after.next)) {
        ASSERT(next->previous == &after);
        next->previous = this;
    }
    previous = &after;
    after.next = this;
}

using ContinuationChainNodeMap = HashMap<const RenderBoxModelObject*, std::unique_ptr<RenderBoxModelObject::ContinuationChainNode>>;

static ContinuationChainNodeMap& continuationChainNodeMap()
{
    static NeverDestroyed<ContinuationChainNodeMap> map;
    return map;
}

using FirstLetterRemainingTextMap = HashMap<const RenderBoxModelObject*, WeakPtr<RenderTextFragment>>;

static FirstLetterRemainingTextMap& firstLetterRemainingTextMap()
{
    static NeverDestroyed<FirstLetterRemainingTextMap> map;
    return map;
}

void RenderBoxModelObject::setSelectionState(SelectionState state)
{
    if (state == SelectionInside && selectionState() != SelectionNone)
        return;

    if ((state == SelectionStart && selectionState() == SelectionEnd)
        || (state == SelectionEnd && selectionState() == SelectionStart))
        RenderElement::setSelectionState(SelectionBoth);
    else
        RenderElement::setSelectionState(state);

    // FIXME: We should consider whether it is OK propagating to ancestor RenderInlines.
    // This is a workaround for http://webkit.org/b/32123
    // The containing block can be null in case of an orphaned tree.
    RenderBlock* containingBlock = this->containingBlock();
    if (containingBlock && !containingBlock->isRenderView())
        containingBlock->setSelectionState(state);
}

void RenderBoxModelObject::contentChanged(ContentChangeType changeType)
{
    if (!hasLayer())
        return;

    //layer()->contentChanged(changeType);
}

bool RenderBoxModelObject::hasAcceleratedCompositing() const
{
    return false; //view().compositor().hasAcceleratedCompositing();
}

bool RenderBoxModelObject::startTransition(double timeOffset, CSSPropertyID propertyId, const RenderStyle* fromStyle, const RenderStyle* toStyle)
{
    ASSERT(hasLayer());
    ASSERT(isComposited());
    return  false; //layer()->backing()->startTransition(timeOffset, propertyId, fromStyle, toStyle);
}

void RenderBoxModelObject::transitionPaused(double timeOffset, CSSPropertyID propertyId)
{
    /**
    ASSERT(hasLayer());
    ASSERT(isComposited());
    layer()->backing()->transitionPaused(timeOffset, propertyId);
     */
}

void RenderBoxModelObject::transitionFinished(CSSPropertyID propertyId)
{
    /**
    ASSERT(hasLayer());
    ASSERT(isComposited());
    layer()->backing()->transitionFinished(propertyId);
     */
}
/**
bool RenderBoxModelObject::startAnimation(double timeOffset, const Animation* animation, const KeyframeList& keyframes)
{
    ASSERT(hasLayer());
    ASSERT(isComposited());
    return layer()->backing()->startAnimation(timeOffset, animation, keyframes);
}*/

void RenderBoxModelObject::animationPaused(double timeOffset, const String& name)
{
    /**
    ASSERT(hasLayer());
    ASSERT(isComposited());
    layer()->backing()->animationPaused(timeOffset, name);
     */
}

void RenderBoxModelObject::animationFinished(const String& name)
{
    /**
    ASSERT(hasLayer());
    ASSERT(isComposited());
    layer()->backing()->animationFinished(name);
     */
}

void RenderBoxModelObject::suspendAnimations(double time)
{
    /**
   ASSERT(hasLayer());
   ASSERT(isComposited());
   layer()->backing()->suspendAnimations(time);
     */
}

RenderBoxModelObject::RenderBoxModelObject(RenderStyle&& style, BaseTypeFlags baseTypeFlags)
   : RenderElement(WTFMove(style), baseTypeFlags | RenderBoxModelObjectFlag)
{
}


RenderBoxModelObject::~RenderBoxModelObject()
{
   // Do not add any code here. Add it to willBeDestroyed() instead.
}

void RenderBoxModelObject::willBeDestroyed()
{
   if (continuation() && !isContinuation()) {
       removeAndDestroyAllContinuations();
       ASSERT(!continuation());
   }
   if (hasContinuationChainNode())
       removeFromContinuationChain();

   if (isFirstLetter())
       clearFirstLetterRemainingText();

   if (!renderTreeBeingDestroyed())
       //view().imageQualityController().rendererWillBeDestroyed(*this);

   RenderElement::willBeDestroyed();
}

bool RenderBoxModelObject::hasVisibleBoxDecorationStyle() const
{
   return hasBackground() || style().hasVisibleBorderDecoration();
}

void RenderBoxModelObject::updateFromStyle()
{
   //FIXME RenderElement::updateFromStyle();

   // Set the appropriate bits for a box model object.  Since all bits are cleared in styleWillChange,
   // we only check for bits that could possibly be set to true.
   const RenderStyle& styleToUse = style();
   setHasVisibleBoxDecorations(hasVisibleBoxDecorationStyle());
   setInline(styleToUse.isDisplayInlineType());
   setPositionState(styleToUse.position());
   setHorizontalWritingMode(styleToUse.isHorizontalWritingMode());
   //if (styleToUse.isFlippedBlocksWritingMode())
       //view().frameView().setHasFlippedBlockRenderers(true);
}

static LayoutSize accumulateInFlowPositionOffsets(const RenderObject* child)
{
   if (!child->isAnonymousBlock() || !child->isInFlowPositioned())
       return LayoutSize();
   LayoutSize offset;
   for (RenderElement* parent = downcast<RenderBlock>(*child).inlineContinuation(); is<RenderInline>(parent); parent = parent->parent()) {
       if (parent->isInFlowPositioned())
           offset += downcast<RenderInline>(*parent).offsetForInFlowPosition();
   }
   return offset;
}

static inline bool isOutOfFlowPositionedWithImplicitHeight(const RenderBoxModelObject& child)
{
   return child.isOutOfFlowPositioned() && !child.style().logicalTop().isAuto() && !child.style().logicalBottom().isAuto();
}

RenderBlock* RenderBoxModelObject::containingBlockForAutoHeightDetection(Length logicalHeight) const
{
   // For percentage heights: The percentage is calculated with respect to the
   // height of the generated box's containing block. If the height of the
   // containing block is not specified explicitly (i.e., it depends on content
   // height), and this element is not absolutely positioned, the used height is
   // calculated as if 'auto' was specified.
   if (!logicalHeight.isPercentOrCalculated() || isOutOfFlowPositioned())
       return nullptr;

   // Anonymous block boxes are ignored when resolving percentage values that
   // would refer to it: the closest non-anonymous ancestor box is used instead.
   auto* cb = containingBlock();
   while (cb && cb->isAnonymous())
       cb = cb->containingBlock();
   if (!cb)
       return nullptr;

   // Matching RenderBox::percentageLogicalHeightIsResolvable() by
   // ignoring table cell's attribute value, where it says that table cells
   // violate what the CSS spec says to do with heights. Basically we don't care
   // if the cell specified a height or not.
   if (cb->isTableCell())
       return nullptr;

   // Match RenderBox::availableLogicalHeightUsing by special casing the layout
   // view. The available height is taken from the frame.
   if (cb->isRenderView())
       return nullptr;

   if (isOutOfFlowPositionedWithImplicitHeight(*cb))
       return nullptr;

   return cb;
}

bool RenderBoxModelObject::hasAutoHeightOrContainingBlockWithAutoHeight() const
{
   const auto* thisBox = isBox() ? downcast<RenderBox>(this) : nullptr;
   Length logicalHeightLength = style().logicalHeight();
   auto* cb = containingBlockForAutoHeightDetection(logicalHeightLength);

   if (logicalHeightLength.isPercentOrCalculated() && cb && isBox())
       cb->addPercentHeightDescendant(*const_cast<RenderBox*>(downcast<RenderBox>(this)));

   if (thisBox && thisBox->isFlexItem()) {
       auto& flexBox = downcast<RenderFlexibleBox>(*parent());
       if (flexBox.childLogicalHeightForPercentageResolution(*thisBox))
           return false;
   }

   if (thisBox && thisBox->isGridItem() && thisBox->hasOverrideContainingBlockLogicalHeight())
       return false;

   if (logicalHeightLength.isAuto() && !isOutOfFlowPositionedWithImplicitHeight(*this))
       return true;

   if (cb)
       return !cb->hasDefiniteLogicalHeight();

   return false;
}

    /**
DecodingMode RenderBoxModelObject::decodingModeForImageDraw(const Image& image, const PaintInfo& paintInfo) const
{
   if (!is<BitmapImage>(image))
       return DecodingMode::Synchronous;

   const BitmapImage& bitmapImage = downcast<BitmapImage>(image);
   if (bitmapImage.canAnimate()) {
       // The DecodingMode for the current frame has to be Synchronous. The DecodingMode
       // for the next frame will be calculated in BitmapImage::internalStartAnimation().
       return DecodingMode::Synchronous;
   }

   // Large image case.
#if PLATFORM(IOS)
   if (IOSApplication::isIBooksStorytime())
       return DecodingMode::Synchronous;
#endif
   if (bitmapImage.isLargeImageAsyncDecodingEnabledForTesting())
       return DecodingMode::Asynchronous;
   if (is<HTMLImageElement>(element()) && element()->hasAttribute(asyncAttr))
       return DecodingMode::Asynchronous;
   if (document().isImageDocument())
       return DecodingMode::Synchronous;
   if (paintInfo.paintBehavior & PaintBehaviorSnapshotting)
       return DecodingMode::Synchronous;
   if (!settings().largeImageAsyncDecodingEnabled())
       return DecodingMode::Synchronous;
   if (!bitmapImage.canUseAsyncDecodingForLargeImages())
       return DecodingMode::Synchronous;
   if (paintInfo.paintBehavior & PaintBehaviorTileFirstPaint)
       return DecodingMode::Asynchronous;
   // FIXME: isVisibleInViewport() is not cheap. Find a way to make this condition faster.
   if (!isVisibleInViewport())
       return DecodingMode::Asynchronous;
   return DecodingMode::Synchronous;
}
     */


LayoutSize RenderBoxModelObject::relativePositionOffset() const
{
   // This function has been optimized to avoid calls to containingBlock() in the common case
   // where all values are either auto or fixed.

   LayoutSize offset = accumulateInFlowPositionOffsets(this);

   // Objects that shrink to avoid floats normally use available line width when computing containing block width.  However
   // in the case of relative positioning using percentages, we can't do this.  The offset should always be resolved using the
   // available width of the containing block.  Therefore we don't use containingBlockLogicalWidthForContent() here, but instead explicitly
   // call availableWidth on our containing block.
   if (!style().left().isAuto()) {
       if (!style().right().isAuto() && !containingBlock()->style().isLeftToRightDirection())
           offset.setWidth(-valueForLength(style().right(), !style().right().isFixed() ? containingBlock()->availableWidth() : LayoutUnit()));
       else
           offset.expand(valueForLength(style().left(), !style().left().isFixed() ? containingBlock()->availableWidth() : LayoutUnit()), 0);
   } else if (!style().right().isAuto()) {
       offset.expand(-valueForLength(style().right(), !style().right().isFixed() ? containingBlock()->availableWidth() : LayoutUnit()), 0);
   }

   // If the containing block of a relatively positioned element does not
   // specify a height, a percentage top or bottom offset should be resolved as
   // auto. An exception to this is if the containing block has the WinIE quirk
   // where <html> and <body> assume the size of the viewport. In this case,
   // calculate the percent offset based on this height.
   // See <https://bugs.webkit.org/show_bug.cgi?id=26396>.
   if (!style().top().isAuto()
       && (!style().top().isPercentOrCalculated()
           || !containingBlock()->hasAutoHeightOrContainingBlockWithAutoHeight()
           || containingBlock()->stretchesToViewport()))
       offset.expand(0, valueForLength(style().top(), !style().top().isFixed() ? containingBlock()->availableHeight() : LayoutUnit()));

   else if (!style().bottom().isAuto()
       && (!style().bottom().isPercentOrCalculated()
           || !containingBlock()->hasAutoHeightOrContainingBlockWithAutoHeight()
           || containingBlock()->stretchesToViewport()))
       offset.expand(0, -valueForLength(style().bottom(), !style().bottom().isFixed() ? containingBlock()->availableHeight() : LayoutUnit()));

   return offset;
}

LayoutPoint RenderBoxModelObject::adjustedPositionRelativeToOffsetParent(const LayoutPoint& startPoint) const
{
   // If the element is the HTML body element or doesn't have a parent
   // return 0 and stop this algorithm.
   if (isBody() || !parent())
       return LayoutPoint();

   LayoutPoint referencePoint = startPoint;

   // If the offsetParent of the element is null, or is the HTML body element,
   // return the distance between the canvas origin and the left border edge
   // of the element and stop this algorithm.
   if (const RenderBoxModelObject* offsetParent = this->offsetParent()) {
       if (is<RenderBox>(*offsetParent) && !offsetParent->isBody() && !is<RenderTable>(*offsetParent))
           referencePoint.move(-downcast<RenderBox>(*offsetParent).borderLeft(), -downcast<RenderBox>(*offsetParent).borderTop());
       if (!isOutOfFlowPositioned() || enclosingFragmentedFlow()) {
           if (isRelativelyPositioned())
               referencePoint.move(relativePositionOffset());
           else if (isStickilyPositioned())
               referencePoint.move(stickyPositionOffset());

           // CSS regions specification says that region flows should return the body element as their offsetParent.
           // Since we will bypass the body’s renderer anyway, just end the loop if we encounter a region flow (named flow thread).
           // See http://dev.w3.org/csswg/css-regions/#cssomview-offset-attributes
           auto* ancestor = parent();
           while (ancestor != offsetParent) {
               // FIXME: What are we supposed to do inside SVG content?

               if (is<RenderMultiColumnFlow>(*ancestor)) {
                   // We need to apply a translation based off what region we are inside.
                   RenderFragmentContainer* fragment = downcast<RenderMultiColumnFlow>(*ancestor).physicalTranslationFromFlowToFragment(referencePoint);
                   if (fragment)
                       referencePoint.moveBy(fragment->topLeftLocation());
               } else if (!isOutOfFlowPositioned()) {
                   if (is<RenderBox>(*ancestor) && !is<RenderTableRow>(*ancestor))
                       referencePoint.moveBy(downcast<RenderBox>(*ancestor).topLeftLocation());
               }

               ancestor = ancestor->parent();
           }

           if (is<RenderBox>(*offsetParent) && offsetParent->isBody() && !offsetParent->isPositioned())
               referencePoint.moveBy(downcast<RenderBox>(*offsetParent).topLeftLocation());
       }
   }

   return referencePoint;
}

void RenderBoxModelObject::computeStickyPositionConstraints(StickyPositionViewportConstraints& constraints, const FloatRect& constrainingRect) const
{
    /**
   constraints.setConstrainingRectAtLastLayout(constrainingRect);

   RenderBlock* containingBlock = this->containingBlock();
   RenderLayer* enclosingClippingLayer = layer()->enclosingOverflowClipLayer(ExcludeSelf);
   RenderBox& enclosingClippingBox = enclosingClippingLayer ? downcast<RenderBox>(enclosingClippingLayer->renderer()) : view();

   LayoutRect containerContentRect;
   if (!enclosingClippingLayer || (containingBlock != &enclosingClippingBox))
       containerContentRect = containingBlock->contentBoxRect();
   else {
       containerContentRect = containingBlock->layoutOverflowRect();
       LayoutPoint containerLocation = containerContentRect.location() + LayoutPoint(containingBlock->borderLeft() + containingBlock->paddingLeft(),
           containingBlock->borderTop() + containingBlock->paddingTop());
       containerContentRect.setLocation(containerLocation);
   }

   LayoutUnit maxWidth = containingBlock->availableLogicalWidth();

   // Sticky positioned element ignore any override logical width on the containing block (as they don't call
   // containingBlockLogicalWidthForContent). It's unclear whether this is totally fine.
   LayoutBoxExtent minMargin(minimumValueForLength(style().marginTop(), maxWidth),
       minimumValueForLength(style().marginRight(), maxWidth),
       minimumValueForLength(style().marginBottom(), maxWidth),
       minimumValueForLength(style().marginLeft(), maxWidth));

   // Compute the container-relative area within which the sticky element is allowed to move.
   containerContentRect.contract(minMargin);

   // Finally compute container rect relative to the scrolling ancestor.
   FloatRect containerRectRelativeToScrollingAncestor = containingBlock->localToContainerQuad(FloatRect(containerContentRect), &enclosingClippingBox).boundingBox();
   if (enclosingClippingLayer) {
       FloatPoint containerLocationRelativeToScrollingAncestor = containerRectRelativeToScrollingAncestor.location() -
           FloatSize(enclosingClippingBox.borderLeft() + enclosingClippingBox.paddingLeft(),
           enclosingClippingBox.borderTop() + enclosingClippingBox.paddingTop());
       if (&enclosingClippingBox != containingBlock)
           containerLocationRelativeToScrollingAncestor += enclosingClippingLayer->scrollOffset();
       containerRectRelativeToScrollingAncestor.setLocation(containerLocationRelativeToScrollingAncestor);
   }
   constraints.setContainingBlockRect(containerRectRelativeToScrollingAncestor);

   // Now compute the sticky box rect, also relative to the scrolling ancestor.
   LayoutRect stickyBoxRect = frameRectForStickyPositioning();
   LayoutRect flippedStickyBoxRect = stickyBoxRect;
   containingBlock->flipForWritingMode(flippedStickyBoxRect);
   FloatRect stickyBoxRelativeToScrollingAnecstor = flippedStickyBoxRect;

   // FIXME: sucks to call localToContainerQuad again, but we can't just offset from the previously computed rect if there are transforms.
   // Map to the view to avoid including page scale factor.
   FloatPoint stickyLocationRelativeToScrollingAncestor = flippedStickyBoxRect.location() + containingBlock->localToContainerQuad(FloatRect(FloatPoint(), containingBlock->size()), &enclosingClippingBox).boundingBox().location();
   if (enclosingClippingLayer) {
       stickyLocationRelativeToScrollingAncestor -= FloatSize(enclosingClippingBox.borderLeft() + enclosingClippingBox.paddingLeft(),
           enclosingClippingBox.borderTop() + enclosingClippingBox.paddingTop());
       if (&enclosingClippingBox != containingBlock)
           stickyLocationRelativeToScrollingAncestor += enclosingClippingLayer->scrollOffset();
   }
   // FIXME: For now, assume that |this| is not transformed.
   stickyBoxRelativeToScrollingAnecstor.setLocation(stickyLocationRelativeToScrollingAncestor);
   constraints.setStickyBoxRect(stickyBoxRelativeToScrollingAnecstor);

   if (!style().left().isAuto()) {
       constraints.setLeftOffset(valueForLength(style().left(), constrainingRect.width()));
       constraints.addAnchorEdge(ViewportConstraints::AnchorEdgeLeft);
   }

   if (!style().right().isAuto()) {
       constraints.setRightOffset(valueForLength(style().right(), constrainingRect.width()));
       constraints.addAnchorEdge(ViewportConstraints::AnchorEdgeRight);
   }

   if (!style().top().isAuto()) {
       constraints.setTopOffset(valueForLength(style().top(), constrainingRect.height()));
       constraints.addAnchorEdge(ViewportConstraints::AnchorEdgeTop);
   }

   if (!style().bottom().isAuto()) {
       constraints.setBottomOffset(valueForLength(style().bottom(), constrainingRect.height()));
       constraints.addAnchorEdge(ViewportConstraints::AnchorEdgeBottom);
   }*/
}

FloatRect RenderBoxModelObject::constrainingRectForStickyPosition() const
{

    /**
   RenderLayer* enclosingClippingLayer = layer()->enclosingOverflowClipLayer(ExcludeSelf);
   if (enclosingClippingLayer) {
       RenderBox& enclosingClippingBox = downcast<RenderBox>(enclosingClippingLayer->renderer());
       LayoutRect clipRect = enclosingClippingBox.overflowClipRect(LayoutPoint(), nullptr); // FIXME: make this work in regions.
       clipRect.contract(LayoutSize(enclosingClippingBox.paddingLeft() + enclosingClippingBox.paddingRight(),
           enclosingClippingBox.paddingTop() + enclosingClippingBox.paddingBottom()));

       FloatRect constrainingRect = enclosingClippingBox.localToContainerQuad(FloatRect(clipRect), &view()).boundingBox();

       FloatPoint scrollOffset = FloatPoint() + enclosingClippingLayer->scrollOffset();

       float scrollbarOffset = 0;
       if (enclosingClippingBox.hasLayer() && enclosingClippingBox.shouldPlaceBlockDirectionScrollbarOnLeft())
           scrollbarOffset = enclosingClippingBox.layer()->verticalScrollbarWidth(IgnoreOverlayScrollbarSize);

       constrainingRect.setLocation(FloatPoint(scrollOffset.x() + scrollbarOffset, scrollOffset.y()));
       return constrainingRect;
   }

   return view().frameView().rectForFixedPositionLayout();
     */
    return FloatRect(0, 0, 10, 10);
}

LayoutSize RenderBoxModelObject::stickyPositionOffset() const
{
   ASSERT(hasLayer());


   /**
   FloatRect constrainingRect = constrainingRectForStickyPosition();
   StickyPositionViewportConstraints constraints;
   computeStickyPositionConstraints(constraints, constrainingRect);
    */

   // The sticky offset is physical, so we can just return the delta computed in absolute coords (though it may be wrong with transforms).
   return LayoutSize(100, 100);
}

LayoutSize RenderBoxModelObject::offsetForInFlowPosition() const
{
   if (isRelativelyPositioned())
       return relativePositionOffset();

   if (isStickilyPositioned())
       return stickyPositionOffset();

   return LayoutSize();
}

LayoutUnit RenderBoxModelObject::offsetLeft() const
{
   // Note that RenderInline and RenderBox override this to pass a different
   // startPoint to adjustedPositionRelativeToOffsetParent.
   return adjustedPositionRelativeToOffsetParent(LayoutPoint()).x();
}

LayoutUnit RenderBoxModelObject::offsetTop() const
{
   // Note that RenderInline and RenderBox override this to pass a different
   // startPoint to adjustedPositionRelativeToOffsetParent.
   return adjustedPositionRelativeToOffsetParent(LayoutPoint()).y();
}

LayoutUnit RenderBoxModelObject::computedCSSPadding(const Length& padding) const
{
   LayoutUnit w = 0;
   if (padding.isPercentOrCalculated())
       w = containingBlockLogicalWidthForContent();
   return minimumValueForLength(padding, w);
}

RoundedRect RenderBoxModelObject::getBackgroundRoundedRect(const LayoutRect& borderRect, InlineFlowBox* box, LayoutUnit inlineBoxWidth, LayoutUnit inlineBoxHeight,
   bool includeLogicalLeftEdge, bool includeLogicalRightEdge) const
{
   RoundedRect border = style().getRoundedBorderFor(borderRect, includeLogicalLeftEdge, includeLogicalRightEdge);
   if (box && (box->nextLineBox() || box->prevLineBox())) {
       RoundedRect segmentBorder = style().getRoundedBorderFor(LayoutRect(0, 0, inlineBoxWidth, inlineBoxHeight), includeLogicalLeftEdge, includeLogicalRightEdge);
       border.setRadii(segmentBorder.radii());
   }
   return border;
}




static inline LayoutUnit resolveWidthForRatio(LayoutUnit height, const LayoutSize& intrinsicRatio)
{
  return height * intrinsicRatio.width() / intrinsicRatio.height();
}

static inline LayoutUnit resolveHeightForRatio(LayoutUnit width, const LayoutSize& intrinsicRatio)
{
  return width * intrinsicRatio.height() / intrinsicRatio.width();
}

static inline LayoutSize resolveAgainstIntrinsicWidthOrHeightAndRatio(const LayoutSize& size, const LayoutSize& intrinsicRatio, LayoutUnit useWidth, LayoutUnit useHeight)
{
  if (intrinsicRatio.isEmpty()) {
      if (useWidth)
          return LayoutSize(useWidth, size.height());
      return LayoutSize(size.width(), useHeight);
  }

  if (useWidth)
      return LayoutSize(useWidth, resolveHeightForRatio(useWidth, intrinsicRatio));
  return LayoutSize(resolveWidthForRatio(useHeight, intrinsicRatio), useHeight);
}

static inline LayoutSize resolveAgainstIntrinsicRatio(const LayoutSize& size, const LayoutSize& intrinsicRatio)
{
  // Two possible solutions: (size.width(), solutionHeight) or (solutionWidth, size.height())
  // "... must be assumed to be the largest dimensions..." = easiest answer: the rect with the largest surface area.

  LayoutUnit solutionWidth = resolveWidthForRatio(size.height(), intrinsicRatio);
  LayoutUnit solutionHeight = resolveHeightForRatio(size.width(), intrinsicRatio);
  if (solutionWidth <= size.width()) {
      if (solutionHeight <= size.height()) {
          // If both solutions fit, choose the one covering the larger area.
          LayoutUnit areaOne = solutionWidth * size.height();
          LayoutUnit areaTwo = size.width() * solutionHeight;
          if (areaOne < areaTwo)
              return LayoutSize(size.width(), solutionHeight);
          return LayoutSize(solutionWidth, size.height());
      }

      // Only the first solution fits.
      return LayoutSize(solutionWidth, size.height());
  }

  // Only the second solution fits, assert that.
  ASSERT(solutionHeight <= size.height());
  return LayoutSize(size.width(), solutionHeight);
}




static void pixelSnapBackgroundImageGeometryForPainting(LayoutRect& destinationRect, LayoutSize& tileSize, LayoutSize& phase, LayoutSize& space, float scaleFactor)
{
  tileSize = LayoutSize(snapRectToDevicePixels(LayoutRect(destinationRect.location(), tileSize), scaleFactor).size());
  phase = LayoutSize(snapRectToDevicePixels(LayoutRect(destinationRect.location(), phase), scaleFactor).size());
  space = LayoutSize(snapRectToDevicePixels(LayoutRect(LayoutPoint(), space), scaleFactor).size());
  destinationRect = LayoutRect(snapRectToDevicePixels(destinationRect, scaleFactor));
}



static inline LayoutUnit getSpace(LayoutUnit areaSize, LayoutUnit tileSize)
{
  int numberOfTiles = areaSize / tileSize;
  LayoutUnit space = -1;

  if (numberOfTiles > 1)
      space = (areaSize - numberOfTiles * tileSize) / (numberOfTiles - 1);

  return space;
}

static LayoutUnit resolveEdgeRelativeLength(const Length& length, Edge edge, LayoutUnit availableSpace, const LayoutSize& areaSize, const LayoutSize& tileSize)
{
  LayoutUnit result = minimumValueForLength(length, availableSpace);

  if (edge == Edge::Right)
      return areaSize.width() - tileSize.width() - result;

  if (edge == Edge::Bottom)
      return areaSize.height() - tileSize.height() - result;

  return result;
}
    /**
  BackgroundImageGeometry RenderBoxModelObject::calculateBackgroundImageGeometry(const RenderElement* paintContainer, const FillLayer& fillLayer, const LayoutPoint& paintOffset,
    const LayoutRect& borderBoxRect, RenderElement* backgroundObject) const
  {
    LayoutUnit left = 0;
    LayoutUnit top = 0;
    LayoutSize positioningAreaSize;
    // Determine the background positioning area and set destination rect to the background painting area.
    // Destination rect will be adjusted later if the background is non-repeating.
    // FIXME: transforms spec says that fixed backgrounds behave like scroll inside transforms. https://bugs.webkit.org/show_bug.cgi?id=15679
    LayoutRect destinationRect(borderBoxRect);
    bool fixedAttachment = fillLayer.attachment() == FixedBackgroundAttachment;
    float deviceScaleFactor = document().deviceScaleFactor();
    if (!fixedAttachment) {
        LayoutUnit right = 0;
        LayoutUnit bottom = 0;
        // Scroll and Local.
        if (fillLayer.origin() != BorderFillBox) {
            left = borderLeft();
            right = borderRight();
            top = borderTop();
            bottom = borderBottom();
            if (fillLayer.origin() == ContentFillBox) {
                left += paddingLeft();
                right += paddingRight();
                top += paddingTop();
                bottom += paddingBottom();
            }
        }

        // The background of the box generated by the root element covers the entire canvas including
        // its margins. Since those were added in already, we have to factor them out when computing
        // the background positioning area.
        if (isDocumentElementRenderer()) {
            positioningAreaSize = downcast<RenderBox>(*this).size() - LayoutSize(left + right, top + bottom);
            positioningAreaSize = LayoutSize(snapSizeToDevicePixel(positioningAreaSize, LayoutPoint(), deviceScaleFactor));
            if (view().frameView().hasExtendedBackgroundRectForPainting()) {
                LayoutRect extendedBackgroundRect = view().frameView().extendedBackgroundRectForPainting();
                left += (marginLeft() - extendedBackgroundRect.x());
                top += (marginTop() - extendedBackgroundRect.y());
            }
        } else {
            positioningAreaSize = borderBoxRect.size() - LayoutSize(left + right, top + bottom);
            positioningAreaSize = LayoutSize(snapRectToDevicePixels(LayoutRect(paintOffset, positioningAreaSize), deviceScaleFactor).size());
        }
    } else {
        LayoutRect viewportRect;
        float topContentInset = 0;
        if (settings().fixedBackgroundsPaintRelativeToDocument())
            viewportRect = view().unscaledDocumentRect();
        else {
            FrameView& frameView = view().frameView();
            bool useFixedLayout = frameView.useFixedLayout() && !frameView.fixedLayoutSize().isEmpty();

            if (useFixedLayout) {
                // Use the fixedLayoutSize() when useFixedLayout() because the rendering will scale
                // down the frameView to to fit in the current viewport.
                viewportRect.setSize(frameView.fixedLayoutSize());
            } else
                viewportRect.setSize(frameView.sizeForVisibleContent());

            if (fixedBackgroundPaintsInLocalCoordinates()) {
                if (!useFixedLayout) {
                    // Shifting location up by topContentInset is needed for layout tests which expect
                    // layout to be shifted down when calling window.internals.setTopContentInset().
                    topContentInset = frameView.topContentInset(ScrollView::TopContentInsetType::WebCoreOrPlatformContentInset);
                    viewportRect.setLocation(LayoutPoint(0, -topContentInset));
                }
            } else if (useFixedLayout || frameView.frameScaleFactor() != 1) {
                // scrollPositionForFixedPosition() is adjusted for page scale and it does not include
                // topContentInset so do not add it to the calculation below.
                viewportRect.setLocation(frameView.scrollPositionForFixedPosition());
            } else {
                // documentScrollPositionRelativeToViewOrigin() includes -topContentInset in its height
                // so we need to account for that in calculating the phase size
                topContentInset = frameView.topContentInset(ScrollView::TopContentInsetType::WebCoreOrPlatformContentInset);
                viewportRect.setLocation(frameView.documentScrollPositionRelativeToViewOrigin());
            }

            top += topContentInset;
        }

        if (paintContainer)
            viewportRect.moveBy(LayoutPoint(-paintContainer->localToAbsolute(FloatPoint())));

        destinationRect = viewportRect;
        positioningAreaSize = destinationRect.size();
        positioningAreaSize.setHeight(positioningAreaSize.height() - topContentInset);
        positioningAreaSize = LayoutSize(snapRectToDevicePixels(LayoutRect(destinationRect.location(), positioningAreaSize), deviceScaleFactor).size());
    }

    auto clientForBackgroundImage = backgroundObject ? backgroundObject : this;
    LayoutSize tileSize = calculateFillTileSize(fillLayer, positioningAreaSize);
    if (StyleImage* layerImage = fillLayer.image())
        layerImage->setContainerContextForRenderer(*clientForBackgroundImage, tileSize, style().effectiveZoom());

    EFillRepeat backgroundRepeatX = fillLayer.repeatX();
    EFillRepeat backgroundRepeatY = fillLayer.repeatY();
    LayoutUnit availableWidth = positioningAreaSize.width() - tileSize.width();
    LayoutUnit availableHeight = positioningAreaSize.height() - tileSize.height();

    LayoutSize spaceSize;
    LayoutSize phase;
    LayoutSize noRepeat;
    LayoutUnit computedXPosition = resolveEdgeRelativeLength(fillLayer.xPosition(), fillLayer.backgroundXOrigin(), availableWidth, positioningAreaSize, tileSize);
    if (backgroundRepeatX == RoundFill && positioningAreaSize.width() > 0 && tileSize.width() > 0) {
        int numTiles = std::max(1, roundToInt(positioningAreaSize.width() / tileSize.width()));
        if (fillLayer.size().size.height.isAuto() && backgroundRepeatY != RoundFill)
            tileSize.setHeight(tileSize.height() * positioningAreaSize.width() / (numTiles * tileSize.width()));

        tileSize.setWidth(positioningAreaSize.width() / numTiles);
        phase.setWidth(tileSize.width() ? tileSize.width() - fmodf((computedXPosition + left), tileSize.width()) : 0);
    }

    LayoutUnit computedYPosition = resolveEdgeRelativeLength(fillLayer.yPosition(), fillLayer.backgroundYOrigin(), availableHeight, positioningAreaSize, tileSize);
    if (backgroundRepeatY == RoundFill && positioningAreaSize.height() > 0 && tileSize.height() > 0) {
        int numTiles = std::max(1, roundToInt(positioningAreaSize.height() / tileSize.height()));
        if (fillLayer.size().size.width.isAuto() && backgroundRepeatX != RoundFill)
            tileSize.setWidth(tileSize.width() * positioningAreaSize.height() / (numTiles * tileSize.height()));

        tileSize.setHeight(positioningAreaSize.height() / numTiles);
        phase.setHeight(tileSize.height() ? tileSize.height() - fmodf((computedYPosition + top), tileSize.height()) : 0);
    }

    if (backgroundRepeatX == RepeatFill) {
        phase.setWidth(tileSize.width() ? tileSize.width() - fmodf(computedXPosition + left, tileSize.width()) : 0);
        spaceSize.setWidth(0);
    } else if (backgroundRepeatX == SpaceFill && tileSize.width() > 0) {
        LayoutUnit space = getSpace(positioningAreaSize.width(), tileSize.width());
        if (space >= 0) {
            LayoutUnit actualWidth = tileSize.width() + space;
            computedXPosition = minimumValueForLength(Length(), availableWidth);
            spaceSize.setWidth(space);
            spaceSize.setHeight(0);
            phase.setWidth(actualWidth ? actualWidth - fmodf((computedXPosition + left), actualWidth) : 0);
        } else
            backgroundRepeatX = NoRepeatFill;
    }

    if (backgroundRepeatX == NoRepeatFill) {
        LayoutUnit xOffset = left + computedXPosition;
        if (xOffset > 0)
            destinationRect.move(xOffset, 0);
        xOffset = std::min<LayoutUnit>(xOffset, 0);
        phase.setWidth(-xOffset);
        destinationRect.setWidth(tileSize.width() + xOffset);
        spaceSize.setWidth(0);
    }

    if (backgroundRepeatY == RepeatFill) {
        phase.setHeight(tileSize.height() ? tileSize.height() - fmodf(computedYPosition + top, tileSize.height()) : 0);
        spaceSize.setHeight(0);
    } else if (backgroundRepeatY == SpaceFill && tileSize.height() > 0) {
        LayoutUnit space = getSpace(positioningAreaSize.height(), tileSize.height());

        if (space >= 0) {
            LayoutUnit actualHeight = tileSize.height() + space;
            computedYPosition = minimumValueForLength(Length(), availableHeight);
            spaceSize.setHeight(space);
            phase.setHeight(actualHeight ? actualHeight - fmodf((computedYPosition + top), actualHeight) : 0);
        } else
            backgroundRepeatY = NoRepeatFill;
    }
    if (backgroundRepeatY == NoRepeatFill) {
        LayoutUnit yOffset = top + computedYPosition;
        if (yOffset > 0)
            destinationRect.move(0, yOffset);
        yOffset = std::min<LayoutUnit>(yOffset, 0);
        phase.setHeight(-yOffset);
        destinationRect.setHeight(tileSize.height() + yOffset);
        spaceSize.setHeight(0);
    }

    if (fixedAttachment) {
        LayoutPoint attachmentPoint = borderBoxRect.location();
        phase.expand(std::max<LayoutUnit>(attachmentPoint.x() - destinationRect.x(), 0), std::max<LayoutUnit>(attachmentPoint.y() - destinationRect.y(), 0));
    }

    destinationRect.intersect(borderBoxRect);
    pixelSnapBackgroundImageGeometryForPainting(destinationRect, tileSize, phase, spaceSize, deviceScaleFactor);
    return BackgroundImageGeometry(destinationRect, tileSize, phase, spaceSize, fixedAttachment);

}
       */


static bool allCornersClippedOut(const RoundedRect& border, const LayoutRect& clipRect)
{
  LayoutRect boundingRect = border.rect();
  if (clipRect.contains(boundingRect))
      return false;

  RoundedRect::Radii radii = border.radii();

  LayoutRect topLeftRect(boundingRect.location(), radii.topLeft());
  if (clipRect.intersects(topLeftRect))
      return false;

  LayoutRect topRightRect(boundingRect.location(), radii.topRight());
  topRightRect.setX(boundingRect.maxX() - topRightRect.width());
  if (clipRect.intersects(topRightRect))
      return false;

  LayoutRect bottomLeftRect(boundingRect.location(), radii.bottomLeft());
  bottomLeftRect.setY(boundingRect.maxY() - bottomLeftRect.height());
  if (clipRect.intersects(bottomLeftRect))
      return false;

  LayoutRect bottomRightRect(boundingRect.location(), radii.bottomRight());
  bottomRightRect.setX(boundingRect.maxX() - bottomRightRect.width());
  bottomRightRect.setY(boundingRect.maxY() - bottomRightRect.height());
  if (clipRect.intersects(bottomRightRect))
      return false;

  return true;
}

static bool borderWillArcInnerEdge(const LayoutSize& firstRadius, const FloatSize& secondRadius)
{
  return !firstRadius.isZero() || !secondRadius.isZero();
}

inline bool styleRequiresClipPolygon(EBorderStyle style)
{
  return style == DOTTED || style == DASHED; // These are drawn with a stroke, so we have to clip to get corner miters.
}

static bool borderStyleFillsBorderArea(EBorderStyle style)
{
  return !(style == DOTTED || style == DASHED || style == DOUBLE);
}

static bool borderStyleHasInnerDetail(EBorderStyle style)
{
  return style == GROOVE || style == RIDGE || style == DOUBLE;
}

static bool borderStyleIsDottedOrDashed(EBorderStyle style)
{
  return style == DOTTED || style == DASHED;
}

// OUTSET darkens the bottom and right (and maybe lightens the top and left)
// INSET darkens the top and left (and maybe lightens the bottom and right)
static inline bool borderStyleHasUnmatchedColorsAtCorner(EBorderStyle style, BoxSide side, BoxSide adjacentSide)
{
  // These styles match at the top/left and bottom/right.
  if (style == INSET || style == GROOVE || style == RIDGE || style == OUTSET) {
      const BorderEdgeFlags topRightFlags = edgeFlagForSide(BSTop) | edgeFlagForSide(BSRight);
      const BorderEdgeFlags bottomLeftFlags = edgeFlagForSide(BSBottom) | edgeFlagForSide(BSLeft);

      BorderEdgeFlags flags = edgeFlagForSide(side) | edgeFlagForSide(adjacentSide);
      return flags == topRightFlags || flags == bottomLeftFlags;
  }
  return false;
}

static inline bool colorsMatchAtCorner(BoxSide side, BoxSide adjacentSide, const BorderEdge edges[])
{
  if (edges[side].shouldRender() != edges[adjacentSide].shouldRender())
      return false;

  if (!edgesShareColor(edges[side], edges[adjacentSide]))
      return false;

  return !borderStyleHasUnmatchedColorsAtCorner(edges[side].style(), side, adjacentSide);
}


static inline bool colorNeedsAntiAliasAtCorner(BoxSide side, BoxSide adjacentSide, const BorderEdge edges[])
{
  if (edges[side].color().isOpaque())
      return false;

  if (edges[side].shouldRender() != edges[adjacentSide].shouldRender())
      return false;

  if (!edgesShareColor(edges[side], edges[adjacentSide]))
      return true;

  return borderStyleHasUnmatchedColorsAtCorner(edges[side].style(), side, adjacentSide);
}

// This assumes that we draw in order: top, bottom, left, right.
static inline bool willBeOverdrawn(BoxSide side, BoxSide adjacentSide, const BorderEdge edges[])
{
  switch (side) {
  case BSTop:
  case BSBottom:
      if (edges[adjacentSide].presentButInvisible())
          return false;

      if (!edgesShareColor(edges[side], edges[adjacentSide]) && !edges[adjacentSide].color().isOpaque())
          return false;

      if (!borderStyleFillsBorderArea(edges[adjacentSide].style()))
          return false;

      return true;

  case BSLeft:
  case BSRight:
      // These draw last, so are never overdrawn.
      return false;
  }
  return false;
}

static inline bool borderStylesRequireMitre(BoxSide side, BoxSide adjacentSide, EBorderStyle style, EBorderStyle adjacentStyle)
{
  if (style == DOUBLE || adjacentStyle == DOUBLE || adjacentStyle == GROOVE || adjacentStyle == RIDGE)
      return true;

  if (borderStyleIsDottedOrDashed(style) != borderStyleIsDottedOrDashed(adjacentStyle))
      return true;

  if (style != adjacentStyle)
      return true;

  return borderStyleHasUnmatchedColorsAtCorner(style, side, adjacentSide);
}

static bool joinRequiresMitre(BoxSide side, BoxSide adjacentSide, const BorderEdge edges[], bool allowOverdraw)
{
  if ((edges[side].isTransparent() && edges[adjacentSide].isTransparent()) || !edges[adjacentSide].isPresent())
      return false;

  if (allowOverdraw && willBeOverdrawn(side, adjacentSide, edges))
      return false;

  if (!edgesShareColor(edges[side], edges[adjacentSide]))
      return true;

  if (borderStylesRequireMitre(side, adjacentSide, edges[side].style(), edges[adjacentSide].style()))
      return true;

  return false;
}

static RoundedRect calculateAdjustedInnerBorder(const RoundedRect&innerBorder, BoxSide side)
{
  // Expand the inner border as necessary to make it a rounded rect (i.e. radii contained within each edge).
  // This function relies on the fact we only get radii not contained within each edge if one of the radii
  // for an edge is zero, so we can shift the arc towards the zero radius corner.
  RoundedRect::Radii newRadii = innerBorder.radii();
  LayoutRect newRect = innerBorder.rect();

  float overshoot;
  float maxRadii;

  switch (side) {
  case BSTop:
      overshoot = newRadii.topLeft().width() + newRadii.topRight().width() - newRect.width();
      if (overshoot > 0) {
          ASSERT(!(newRadii.topLeft().width() && newRadii.topRight().width()));
          newRect.setWidth(newRect.width() + overshoot);
          if (!newRadii.topLeft().width())
              newRect.move(-overshoot, 0);
      }
      newRadii.setBottomLeft(IntSize(0, 0));
      newRadii.setBottomRight(IntSize(0, 0));
      maxRadii = std::max(newRadii.topLeft().height(), newRadii.topRight().height());
      if (maxRadii > newRect.height())
          newRect.setHeight(maxRadii);
      break;

  case BSBottom:
      overshoot = newRadii.bottomLeft().width() + newRadii.bottomRight().width() - newRect.width();
      if (overshoot > 0) {
          ASSERT(!(newRadii.bottomLeft().width() && newRadii.bottomRight().width()));
          newRect.setWidth(newRect.width() + overshoot);
          if (!newRadii.bottomLeft().width())
              newRect.move(-overshoot, 0);
      }
      newRadii.setTopLeft(IntSize(0, 0));
      newRadii.setTopRight(IntSize(0, 0));
      maxRadii = std::max(newRadii.bottomLeft().height(), newRadii.bottomRight().height());
      if (maxRadii > newRect.height()) {
          newRect.move(0, newRect.height() - maxRadii);
          newRect.setHeight(maxRadii);
      }
      break;

  case BSLeft:
      overshoot = newRadii.topLeft().height() + newRadii.bottomLeft().height() - newRect.height();
      if (overshoot > 0) {
          ASSERT(!(newRadii.topLeft().height() && newRadii.bottomLeft().height()));
          newRect.setHeight(newRect.height() + overshoot);
          if (!newRadii.topLeft().height())
              newRect.move(0, -overshoot);
      }
      newRadii.setTopRight(IntSize(0, 0));
      newRadii.setBottomRight(IntSize(0, 0));
      maxRadii = std::max(newRadii.topLeft().width(), newRadii.bottomLeft().width());
      if (maxRadii > newRect.width())
          newRect.setWidth(maxRadii);
      break;

  case BSRight:
      overshoot = newRadii.topRight().height() + newRadii.bottomRight().height() - newRect.height();
      if (overshoot > 0) {
          ASSERT(!(newRadii.topRight().height() && newRadii.bottomRight().height()));
          newRect.setHeight(newRect.height() + overshoot);
          if (!newRadii.topRight().height())
              newRect.move(0, -overshoot);
      }
      newRadii.setTopLeft(IntSize(0, 0));
      newRadii.setBottomLeft(IntSize(0, 0));
      maxRadii = std::max(newRadii.topRight().width(), newRadii.bottomRight().width());
      if (maxRadii > newRect.width()) {
          newRect.move(newRect.width() - maxRadii, 0);
          newRect.setWidth(maxRadii);
      }
      break;
  }

  return RoundedRect(newRect, newRadii);
}



static LayoutRect calculateSideRect(const RoundedRect& outerBorder, const BorderEdge edges[], int side)
{
  LayoutRect sideRect = outerBorder.rect();
  float width = edges[side].widthForPainting();

  if (side == BSTop)
      sideRect.setHeight(width);
  else if (side == BSBottom)
      sideRect.shiftYEdgeTo(sideRect.maxY() - width);
  else if (side == BSLeft)
      sideRect.setWidth(width);
  else
      sideRect.shiftXEdgeTo(sideRect.maxX() - width);

  return sideRect;
}




bool RenderBoxModelObject::borderObscuresBackgroundEdge(const FloatSize& contextScale) const
{
    /**
  BorderEdge edges[4];
  BorderEdge::getBorderEdgeInfo(edges, style(), document().deviceScaleFactor());

  for (int i = BSTop; i <= BSLeft; ++i) {
      const BorderEdge& currEdge = edges[i];
      // FIXME: for vertical text
      float axisScale = (i == BSTop || i == BSBottom) ? contextScale.height() : contextScale.width();
      if (!currEdge.obscuresBackgroundEdge(axisScale))
          return false;
  }*/

  return true;
}

bool RenderBoxModelObject::borderObscuresBackground() const
{
    /**
  if (!style().hasBorder())
      return false;

  // Bail if we have any border-image for now. We could look at the image alpha to improve this.
  //if (style().borderImage().image())
    //  return false;

  BorderEdge edges[4];
  BorderEdge::getBorderEdgeInfo(edges, style(), document().deviceScaleFactor());

  for (int i = BSTop; i <= BSLeft; ++i) {
      const BorderEdge& currEdge = edges[i];
      if (!currEdge.obscuresBackground())
          return false;
  }*/

  return true;
}

bool RenderBoxModelObject::boxShadowShouldBeAppliedToBackground(const LayoutPoint&, BackgroundBleedAvoidance bleedAvoidance, InlineFlowBox* inlineFlowBox) const
{
    /**
  if (bleedAvoidance != BackgroundBleedNone)
      return false;

  if (style().hasAppearance())
      return false;

  bool hasOneNormalBoxShadow = false;
  for (const ShadowData* currentShadow = style().boxShadow(); currentShadow; currentShadow = currentShadow->next()) {
      if (currentShadow->style() != Normal)
          continue;

      if (hasOneNormalBoxShadow)
          return false;
      hasOneNormalBoxShadow = true;

      if (currentShadow->spread())
          return false;
  }

  if (!hasOneNormalBoxShadow)
      return false;

  Color backgroundColor = style().visitedDependentColor(CSSPropertyBackgroundColor);
  if (!backgroundColor.isOpaque())
      return false;

  auto* lastBackgroundLayer = &style().backgroundLayers();
  while (auto* next = lastBackgroundLayer->next())
      lastBackgroundLayer = next;

  if (lastBackgroundLayer->clip() != BorderFillBox)
      return false;

  if (lastBackgroundLayer->image() && style().hasBorderRadius())
      return false;

  if (inlineFlowBox && !inlineFlowBox->boxShadowCanBeAppliedToBackground(*lastBackgroundLayer))
      return false;

  if (hasOverflowClip() && lastBackgroundLayer->attachment() == LocalBackgroundAttachment)
      return false;
*/
  return true;
}

static inline LayoutRect areaCastingShadowInHole(const LayoutRect& holeRect, int shadowExtent, int shadowSpread, const IntSize& shadowOffset)
{
  LayoutRect bounds(holeRect);

  bounds.inflate(shadowExtent);

  if (shadowSpread < 0)
      bounds.inflate(-shadowSpread);

  LayoutRect offsetBounds = bounds;
  offsetBounds.move(-shadowOffset);
  return unionRect(bounds, offsetBounds);
}



LayoutUnit RenderBoxModelObject::containingBlockLogicalWidthForContent() const
{
  return containingBlock()->availableLogicalWidth();
}

RenderBoxModelObject* RenderBoxModelObject::continuation() const
{
  if (!hasContinuationChainNode())
      return nullptr;

  auto& continuationChainNode = *continuationChainNodeMap().get(this);
  if (!continuationChainNode.next)
      return nullptr;
  return continuationChainNode.next->renderer.get();
}

RenderInline* RenderBoxModelObject::inlineContinuation() const
{
  if (!hasContinuationChainNode())
      return nullptr;

  for (auto* next = continuationChainNodeMap().get(this)->next; next; next = next->next) {
      if (is<RenderInline>(*next->renderer))
          return downcast<RenderInline>(next->renderer.get());
  }
  return nullptr;
}


void RenderBoxModelObject::insertIntoContinuationChainAfter(RenderBoxModelObject& afterRenderer)
{
  ASSERT(isContinuation());
  ASSERT(!continuationChainNodeMap().contains(this));

  auto& after = afterRenderer.ensureContinuationChainNode();
  ensureContinuationChainNode().insertAfter(after);
}

void RenderBoxModelObject::removeFromContinuationChain()
{
  ASSERT(hasContinuationChainNode());
  ASSERT(continuationChainNodeMap().contains(this));
  setHasContinuationChainNode(false);
  continuationChainNodeMap().remove(this);
}

auto RenderBoxModelObject::ensureContinuationChainNode() -> ContinuationChainNode&
{
  setHasContinuationChainNode(true);
  return *continuationChainNodeMap().ensure(this, [&] {
      return std::make_unique<ContinuationChainNode>(*this);
  }).iterator->value;
}

void RenderBoxModelObject::removeAndDestroyAllContinuations()
{
  ASSERT(!isContinuation());
  ASSERT(hasContinuationChainNode());
  ASSERT(continuationChainNodeMap().contains(this));
  auto& continuationChainNode = *continuationChainNodeMap().get(this);
  while (continuationChainNode.next)
      continuationChainNode.next->renderer->removeFromParentAndDestroy();
  removeFromContinuationChain();
}

RenderTextFragment* RenderBoxModelObject::firstLetterRemainingText() const
{
  if (!isFirstLetter())
      return nullptr;
  return firstLetterRemainingTextMap().get(this).get();
}

void RenderBoxModelObject::setFirstLetterRemainingText(RenderTextFragment& remainingText)
{
  //ASSERT(isFirstLetter());
  //firstLetterRemainingTextMap().set(this, makeWeakPtr(remainingText));
}

void RenderBoxModelObject::clearFirstLetterRemainingText()
{
  ASSERT(isFirstLetter());
  firstLetterRemainingTextMap().remove(this);
}

LayoutRect RenderBoxModelObject::localCaretRectForEmptyElement(LayoutUnit width, LayoutUnit textIndentOffset)
{
  ASSERT(!firstChild());

  // FIXME: This does not take into account either :first-line or :first-letter
  // However, as soon as some content is entered, the line boxes will be
  // constructed and this kludge is not called any more. So only the caret size
  // of an empty :first-line'd block is wrong. I think we can live with that.
  const RenderStyle& currentStyle = firstLineStyle();
  LayoutUnit height = lineHeight(true, currentStyle.isHorizontalWritingMode() ? HorizontalLine : VerticalLine);

  enum CaretAlignment { alignLeft, alignRight, alignCenter };

  CaretAlignment alignment = alignLeft;

  switch (currentStyle.textAlign()) {
  case LEFT:
  case WEBKIT_LEFT:
      break;
  case CENTER:
  case WEBKIT_CENTER:
      alignment = alignCenter;
      break;
  case RIGHT:
  case WEBKIT_RIGHT:
      alignment = alignRight;
      break;
  case JUSTIFY:
  case TASTART:
      if (!currentStyle.isLeftToRightDirection())
          alignment = alignRight;
      break;
  case TAEND:
      if (currentStyle.isLeftToRightDirection())
          alignment = alignRight;
      break;
  }

  LayoutUnit x = borderLeft() + paddingLeft();
  LayoutUnit maxX = width - borderRight() - paddingRight();

  switch (alignment) {
  case alignLeft:
      if (currentStyle.isLeftToRightDirection())
          x += textIndentOffset;
      break;
  case alignCenter:
      x = (x + maxX) / 2;
      if (currentStyle.isLeftToRightDirection())
          x += textIndentOffset / 2;
      else
          x -= textIndentOffset / 2;
      break;
  case alignRight:
      x = maxX - caretWidth;
      if (!currentStyle.isLeftToRightDirection())
          x -= textIndentOffset;
      break;
  }
  x = std::min(x, std::max<LayoutUnit>(maxX - caretWidth, 0));

  LayoutUnit y = paddingTop() + borderTop();

  return currentStyle.isHorizontalWritingMode() ? LayoutRect(x, y, caretWidth, height) : LayoutRect(y, x, height, caretWidth);
}

    /**
bool RenderBoxModelObject::shouldAntialiasLines(GraphicsContext& context)
{
  // FIXME: We may want to not antialias when scaled by an integral value,
  // and we may want to antialias when translated by a non-integral value.
  return !context.getCTM().isIdentityOrTranslationOrFlipped();
}*/

void RenderBoxModelObject::mapAbsoluteToLocalPoint(MapCoordinatesFlags mode, TransformState& transformState) const
{
    /**
  RenderElement* container = this->container();
  if (!container)
      return;

  // FIXME: This code is wrong for named flow threads since it only works for content in the first region.
  // We also don't want to run it for multicolumn flow threads, since we can use our knowledge of column
  // geometry to actually get a better result.
  // The point inside a box that's inside a region has its coordinates relative to the region,
  // not the FragmentedFlow that is its container in the RenderObject tree.
  if (is<RenderBox>(*this) && container->isOutOfFlowRenderFragmentedFlow()) {
      RenderFragmentContainer* startFragment = nullptr;
      RenderFragmentContainer* endFragment = nullptr;
      if (downcast<RenderFragmentedFlow>(*container).getFragmentRangeForBox(downcast<RenderBox>(this), startFragment, endFragment))
          container = startFragment;
  }

  container->mapAbsoluteToLocalPoint(mode, transformState);

  LayoutSize containerOffset = offsetFromContainer(*container, LayoutPoint());

  bool preserve3D = mode & UseTransforms && (container->style().preserves3D() || style().preserves3D());
  if (mode & UseTransforms && shouldUseTransformFromContainer(container)) {
      TransformationMatrix t;
      getTransformFromContainer(container, containerOffset, t);
      transformState.applyTransform(t, preserve3D ? TransformState::AccumulateTransform : TransformState::FlattenTransform);
  } else
      transformState.move(containerOffset.width(), containerOffset.height(), preserve3D ? TransformState::AccumulateTransform : TransformState::FlattenTransform);
     */
 }

void RenderBoxModelObject::moveChildTo(RenderBoxModelObject* toBoxModelObject, RenderObject* child, RenderObject* beforeChild, NormalizeAfterInsertion normalizeAfterInsertion)
{
  // We assume that callers have cleared their positioned objects list for child moves so the
  // positioned renderer maps don't become stale. It would be too slow to do the map lookup on each call.
  ASSERT(normalizeAfterInsertion == NormalizeAfterInsertion::No || !is<RenderBlock>(*this) || !downcast<RenderBlock>(*this).hasPositionedObjects());

  ASSERT(this == child->parent());
  ASSERT(!beforeChild || toBoxModelObject == beforeChild->parent());
  if (normalizeAfterInsertion == NormalizeAfterInsertion::Yes && (toBoxModelObject->isRenderBlock() || toBoxModelObject->isRenderInline())) {
      // Takes care of adding the new child correctly if toBlock and fromBlock
      // have different kind of children (block vs inline).
      auto childToMove = takeChildInternal(*child);
      toBoxModelObject->addChild(WTFMove(childToMove), beforeChild);
  } else {
      auto childToMove = takeChildInternal(*child);
      toBoxModelObject->insertChildInternal(WTFMove(childToMove), beforeChild);
  }
}

void RenderBoxModelObject::moveChildrenTo(RenderBoxModelObject* toBoxModelObject, RenderObject* startChild, RenderObject* endChild, RenderObject* beforeChild, NormalizeAfterInsertion normalizeAfterInsertion)
{
  // This condition is rarely hit since this function is usually called on
  // anonymous blocks which can no longer carry positioned objects (see r120761)
  // or when fullRemoveInsert is false.
  if (normalizeAfterInsertion == NormalizeAfterInsertion::Yes && is<RenderBlock>(*this)) {
      downcast<RenderBlock>(*this).removePositionedObjects(nullptr);
      if (is<RenderBlockFlow>(*this))
          downcast<RenderBlockFlow>(*this).removeFloatingObjects();
  }

  ASSERT(!beforeChild || toBoxModelObject == beforeChild->parent());
  for (RenderObject* child = startChild; child && child != endChild; ) {
      // Save our next sibling as moveChildTo will clear it.
      RenderObject* nextSibling = child->nextSibling();

      // FIXME: This logic here fails to detect the first letter in certain cases
      // and skips a valid sibling renderer (see webkit.org/b/163737).
      // Check to make sure we're not saving the firstLetter as the nextSibling.
      // When the |child| object will be moved, its firstLetter will be recreated,
      // so saving it now in nextSibling would leave us with a stale object.


      moveChildTo(toBoxModelObject, child, beforeChild, normalizeAfterInsertion);
      child = nextSibling;
  }
}

} // namespace WebCore
