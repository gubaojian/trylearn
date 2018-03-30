/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2005 Allan Sandfeld Jensen (kde@carewolf.com)
 *           (C) 2005, 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2013, 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2010, 2012 Google Inc. All rights reserved.
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
 */

#include "config.h"
#include "RenderElement.h"

#include "PathUtilities.h"
#include "RenderBlock.h"
#include "RenderChildIterator.h"
//#include "RenderCounter.h"
#include "RenderDeprecatedFlexibleBox.h"
#include "RenderDescendantIterator.h"
#include "RenderFlexibleBox.h"
#include "RenderFragmentedFlow.h"
#include "RenderInline.h"
#include "RenderIterator.h"
#include "RenderLineBreak.h"
//#include "RenderListItem.h"
#if !ASSERT_DISABLED
//#include "RenderListMarker.h"
#endif
#include "RenderFragmentContainer.h"
#include "RenderTableCaption.h"
#include "RenderTableCell.h"
#include "RenderTableCol.h"
#include "RenderTableRow.h"
//#include "RenderText.h"
//#include "RenderTheme.h"
//#include "RenderView.h"
//#include "SVGRenderSupport.h"
//#include "Settings.h"
//#include "ShadowRoot.h"
//#include "StylePendingResources.h"
//#include "StyleResolver.h"
//#include "TextAutoSizing.h"
#include <wtf/IsoMallocInlines.h>
#include <wtf/MathExtras.h>
#include <wtf/StackStats.h>

#include "RenderGrid.h"

namespace WebCore {

WTF_MAKE_ISO_ALLOCATED_IMPL(RenderElement);

struct SameSizeAsRenderElement : public RenderObject {
    unsigned bitfields : 25;
    void* firstChild;
    void* lastChild;
    RenderStyle style;
};

static_assert(sizeof(RenderElement) == sizeof(SameSizeAsRenderElement), "RenderElement should stay small");

bool RenderElement::s_affectsParentBlock = false;
bool RenderElement::s_noLongerAffectsParentBlock = false;
    
inline RenderElement::RenderElement(RenderStyle&& style, BaseTypeFlags baseTypeFlags)
    : RenderObject()
    , m_baseTypeFlags(baseTypeFlags)
    , m_ancestorLineBoxDirty(false)
    , m_hasInitializedStyle(false)
    , m_renderInlineAlwaysCreatesLineBoxes(false)
    , m_renderBoxNeedsLazyRepaint(false)
    , m_hasPausedImageAnimations(false)
    , m_hasCounterNodeMap(false)
    , m_hasContinuationChainNode(false)
    , m_isContinuation(false)
    , m_isFirstLetter(false)
    , m_hasValidCachedFirstLineStyle(false)
    , m_renderBlockHasMarginBeforeQuirk(false)
    , m_renderBlockHasMarginAfterQuirk(false)
    , m_renderBlockShouldForceRelayoutChildren(false)
    , m_renderBlockFlowHasMarkupTruncation(false)
    , m_renderBlockFlowLineLayoutPath(RenderBlockFlow::UndeterminedPath)
    , m_isRegisteredForVisibleInViewportCallback(false)
    , m_visibleInViewportState(static_cast<unsigned>(VisibleInViewportState::Unknown))
    , m_firstChild(nullptr)
    , m_lastChild(nullptr)
    , m_style(WTFMove(style))
{
}


RenderElement::~RenderElement()
{
    // Do not add any code here. Add it to willBeDestroyed() instead.
}

RenderPtr<RenderElement> RenderElement::createFor(RenderStyle&& style, RendererCreationType creationType)
{
    // Minimal support for content properties replacing an entire element.
    // Works only if we have exactly one piece of content and it's a URL.
    // Otherwise acts as if we didn't support this feature.

    switch (style.display()) {
    case NONE:
    case CONTENTS:
        return nullptr;
    case INLINE:
        if (creationType == CreateAllRenderers)
            return createRenderer<RenderInline>(WTFMove(style));
        FALLTHROUGH; // Fieldsets should make a block flow if display:inline is set.
    case BLOCK:
    case INLINE_BLOCK:
    case COMPACT:
        return createRenderer<RenderBlockFlow>(WTFMove(style));
    //case LIST_ITEM:
      //  return createRenderer<RenderListItem>(WTFMove(style));
    case FLEX:
    case INLINE_FLEX:
    case WEBKIT_FLEX:
    case WEBKIT_INLINE_FLEX:
        return createRenderer<RenderFlexibleBox>(WTFMove(style));
    case GRID:
    case INLINE_GRID:
        return createRenderer<RenderGrid>(WTFMove(style));
    case BOX:
    case INLINE_BOX:
        return createRenderer<RenderDeprecatedFlexibleBox>(WTFMove(style));
    default: {
        if (creationType == OnlyCreateBlockAndFlexboxRenderers)
            return createRenderer<RenderBlockFlow>(WTFMove(style));
        switch (style.display()) {
        case TABLE:
        case INLINE_TABLE:
            return createRenderer<RenderTable>( WTFMove(style));
        case TABLE_CELL:
            return createRenderer<RenderTableCell>(WTFMove(style));
        case TABLE_CAPTION:
            return createRenderer<RenderTableCaption>(WTFMove(style));
        case TABLE_ROW_GROUP:
        case TABLE_HEADER_GROUP:
        case TABLE_FOOTER_GROUP:
            return createRenderer<RenderTableSection>(WTFMove(style));
        case TABLE_ROW:
            return createRenderer<RenderTableRow>(WTFMove(style));
        case TABLE_COLUMN_GROUP:
        case TABLE_COLUMN:
            return createRenderer<RenderTableCol>(WTFMove(style));
        default:
            break;
        }
        break;
    }
    }
    ASSERT_NOT_REACHED();
    return nullptr;
}

const RenderStyle& RenderElement::firstLineStyle() const
{
    return style();
}

StyleDifference RenderElement::adjustStyleDifference(StyleDifference diff, unsigned contextSensitiveProperties) const
{
    // If transform changed, and we are not composited, need to do a layout.
    if (contextSensitiveProperties & ContextSensitivePropertyTransform) {
        // FIXME: when transforms are taken into account for overflow, we will need to do a layout.
        if (!hasLayer()) {
            if (!hasLayer())
                diff = std::max(diff, StyleDifferenceLayout);
            else {
                // We need to set at least SimplifiedLayout, but if PositionedMovementOnly is already set
                // then we actually need SimplifiedLayoutAndPositionedMovement.
                diff = std::max(diff, (diff == StyleDifferenceLayoutPositionedMovementOnly) ? StyleDifferenceSimplifiedLayoutAndPositionedMovement : StyleDifferenceSimplifiedLayout);
            }
        
        } else
            diff = std::max(diff, StyleDifferenceRecompositeLayer);
    }

    if (contextSensitiveProperties & ContextSensitivePropertyOpacity) {
        if (!hasLayer())
            diff = std::max(diff, StyleDifferenceRepaintLayer);
        else
            diff = std::max(diff, StyleDifferenceRecompositeLayer);
    }

    if (contextSensitiveProperties & ContextSensitivePropertyClipPath) {
        if (hasLayer())
            diff = std::max(diff, StyleDifferenceRecompositeLayer);
        else
            diff = std::max(diff, StyleDifferenceRepaint);
    }
    
    if (contextSensitiveProperties & ContextSensitivePropertyWillChange) {
        //if (style().willChange() && style().willChange()->canTriggerCompositing())
            diff = std::max(diff, StyleDifferenceRecompositeLayer);
    }
    

    // The answer to requiresLayer() for plugins, iframes, and canvas can change without the actual
    // style changing, since it depends on whether we decide to composite these elements. When the
    // layer status of one of these elements changes, we need to force a layout.
    if (diff < StyleDifferenceLayout && isRenderElement()) {
            diff = StyleDifferenceLayout;
    }

    // If we have no layer(), just treat a RepaintLayer hint as a normal Repaint.
    if (diff == StyleDifferenceRepaintLayer && !hasLayer())
        diff = StyleDifferenceRepaint;

    return diff;
}

inline bool RenderElement::hasImmediateNonWhitespaceTextChildOrBorderOrOutline() const
{
    for (auto& child : childrenOfType<RenderObject>(*this)) {
        if (child.style().hasOutline() || child.style().hasBorder())
            return true;
    }
    return false;
}

inline bool RenderElement::shouldRepaintForStyleDifference(StyleDifference diff) const
{
    return diff == StyleDifferenceRepaint || (diff == StyleDifferenceRepaintIfTextOrBorderOrOutline && hasImmediateNonWhitespaceTextChildOrBorderOrOutline());
}



void RenderElement::initializeStyle()
{
    //Style::loadPendingResources(m_style, document(), element());

    //styleWillChange(StyleDifferenceNewStyle, style());
    m_hasInitializedStyle = true;
    styleDidChange(StyleDifferenceNewStyle, nullptr);


    // It would be nice to assert that !parent() here, but some RenderLayer subrenderers
    // have their parent set before getting a call to initializeStyle() :|
}

void RenderElement::setStyle(RenderStyle&& style, StyleDifference minimalStyleDifference)
{
    // FIXME: Should change RenderView so it can use initializeStyle too.
    // If we do that, we can assert m_hasInitializedStyle unconditionally,
    // and remove the check of m_hasInitializedStyle below too.
    ASSERT(m_hasInitializedStyle || isRenderView());

    StyleDifference diff = StyleDifferenceEqual;
    unsigned contextSensitiveProperties = ContextSensitivePropertyNone;
    if (m_hasInitializedStyle)
        diff = m_style.diff(style, contextSensitiveProperties);

    diff = std::max(diff, minimalStyleDifference);

    diff = adjustStyleDifference(diff, contextSensitiveProperties);

    //Style::loadPendingResources(style, document(), element());

    styleWillChange(diff, style);
    auto oldStyle = m_style.replace(WTFMove(style));
    bool detachedFromParent = !parent();

    // Make sure we invalidate the containing block cache for flows when the contianing block context changes
    // so that styleDidChange can safely use RenderBlock::locateEnclosingFragmentedFlow()
    if (oldStyle.position() != m_style.position())
        adjustFragmentedFlowStateOnContainingBlockChangeIfNeeded();

    styleDidChange(diff, &oldStyle);

    // Text renderers use their parent style. Notify them about the change.
    //for (auto& child : childrenOfType<RenderText>(*this))
        //child.styleDidChange(diff, &oldStyle);

    // FIXME: |this| might be destroyed here. This can currently happen for a RenderTextFragment when
    // its first-letter block gets an update in RenderTextFragment::styleDidChange. For RenderTextFragment(s),
    // we will safely bail out with the detachedFromParent flag. We might want to broaden this condition
    // in the future as we move renderer changes out of layout and into style changes.
    if (detachedFromParent)
        return;

    // Now that the layer (if any) has been updated, we need to adjust the diff again,
    // check whether we should layout now, and decide if we need to repaint.
    StyleDifference updatedDiff = adjustStyleDifference(diff, contextSensitiveProperties);
    
    if (diff <= StyleDifferenceLayoutPositionedMovementOnly) {
        if (updatedDiff == StyleDifferenceLayout)
            setNeedsLayoutAndPrefWidthsRecalc();
        else if (updatedDiff == StyleDifferenceLayoutPositionedMovementOnly)
            setNeedsPositionedMovementLayout(&oldStyle);
        else if (updatedDiff == StyleDifferenceSimplifiedLayoutAndPositionedMovement) {
            setNeedsPositionedMovementLayout(&oldStyle);
            setNeedsSimplifiedNormalFlowLayout();
        } else if (updatedDiff == StyleDifferenceSimplifiedLayout)
            setNeedsSimplifiedNormalFlowLayout();
    }

    if (updatedDiff == StyleDifferenceRepaintLayer || shouldRepaintForStyleDifference(updatedDiff)) {
        // Do a repaint with the new style now, e.g., for example if we go from
        // not having an outline to having an outline.
        repaint();
    }
}

bool RenderElement::childRequiresTable(const RenderObject& child) const
{
    if (is<RenderTableCol>(child)) {
        const RenderTableCol& newTableColumn = downcast<RenderTableCol>(child);
        bool isColumnInColumnGroup = newTableColumn.isTableColumn() && is<RenderTableCol>(*this);
        return !is<RenderTable>(*this) && !isColumnInColumnGroup;
    }
    if (is<RenderTableCaption>(child))
        return !is<RenderTable>(*this);

    if (is<RenderTableSection>(child))
        return !is<RenderTable>(*this);

    if (is<RenderTableRow>(child))
        return !is<RenderTableSection>(*this);

    if (is<RenderTableCell>(child))
        return !is<RenderTableRow>(*this);

    return false;
}

void RenderElement::addChild(RenderPtr<RenderObject> newChild, RenderObject* beforeChild)
{
    auto& child = *newChild;
    if (childRequiresTable(child)) {
        RenderTable* table;
        RenderObject* afterChild = beforeChild ? beforeChild->previousSibling() : m_lastChild;
        if (afterChild && afterChild->isAnonymous() && is<RenderTable>(*afterChild) && !afterChild->isBeforeContent())
            table = downcast<RenderTable>(afterChild);
        else {
            auto newTable =  RenderTable::createAnonymousWithParentRenderer(*this);
            table = newTable.get();
            addChild(WTFMove(newTable), beforeChild);
        }

        table->addChild(WTFMove(newChild));
    } else
        insertChildInternal(WTFMove(newChild), beforeChild);

    //if (is<RenderText>(child))
      //  downcast<RenderText>(child).styleDidChange(StyleDifferenceEqual, nullptr);

    // SVG creates renderers for <g display="none">, as SVG requires children of hidden
    // <g>s to have renderers - at least that's how our implementation works. Consider:
    // <g display="none"><foreignObject><body style="position: relative">FOO...
    // - requiresLayer() would return true for the <body>, creating a new RenderLayer
    // - when the document is painted, both layers are painted. The <body> layer doesn't
    //   know that it's inside a "hidden SVG subtree", and thus paints, even if it shouldn't.
    // To avoid the problem alltogether, detect early if we're inside a hidden SVG subtree
    // and stop creating layers at all for these cases - they're not used anyways.
   // if (child.hasLayer() && !layerCreationAllowedForSubtree())
     //   downcast<RenderElement>(child).layer()->removeOnlyThisLayer();

    //SVGRenderSupport::childAdded(*this, child);
}

RenderPtr<RenderObject> RenderElement::takeChild(RenderObject& oldChild)
{
    return takeChildInternal(oldChild);
}

void RenderElement::removeAndDestroyChild(RenderObject& oldChild)
{
    auto toDestroy = takeChild(oldChild);
}

void RenderElement::destroyLeftoverChildren()
{
    while (m_firstChild) {
        //if (auto* node = m_firstChild->node())
          //  node->setRenderer(nullptr);
        removeAndDestroyChild(*m_firstChild);
    }
}

void RenderElement::insertChildInternal(RenderPtr<RenderObject> newChildPtr, RenderObject* beforeChild)
{
    //RELEASE_ASSERT_WITH_MESSAGE(!view().frameView().layoutContext().layoutState(), "Layout must not mutate render tree");

    ASSERT(canHaveChildren() || canHaveGeneratedChildren());
    ASSERT(!newChildPtr->parent());
    ASSERT(!isRenderBlockFlow() || (!newChildPtr->isTableSection() && !newChildPtr->isTableRow() && !newChildPtr->isTableCell()));

    while (beforeChild && beforeChild->parent() && beforeChild->parent() != this)
        beforeChild = beforeChild->parent();

    ASSERT(!beforeChild || beforeChild->parent() == this);
    //ASSERT(!is<RenderText>(beforeChild) || !downcast<RenderText>(*beforeChild).inlineWrapperForDisplayContents());

    // Take the ownership.
    auto* newChild = newChildPtr.release();

    newChild->setParent(this);

    if (m_firstChild == beforeChild)
        m_firstChild = newChild;

    if (beforeChild) {
        RenderObject* previousSibling = beforeChild->previousSibling();
        if (previousSibling)
            previousSibling->setNextSibling(newChild);
        newChild->setPreviousSibling(previousSibling);
        newChild->setNextSibling(beforeChild);
        beforeChild->setPreviousSibling(newChild);
    } else {
        if (lastChild())
            lastChild()->setNextSibling(newChild);
        newChild->setPreviousSibling(lastChild());
        m_lastChild = newChild;
    }

    newChild->initializeFragmentedFlowStateOnInsertion();
    if (!renderTreeBeingDestroyed()) {
        newChild->insertedIntoTree();
        //if (is<RenderElement>(*newChild))
            //RenderCounter::rendererSubtreeAttached(downcast<RenderElement>(*newChild));
    }

    newChild->setNeedsLayoutAndPrefWidthsRecalc();
    setPreferredLogicalWidthsDirty(true);
    if (!normalChildNeedsLayout())
        setChildNeedsLayout(); // We may supply the static position for an absolute positioned child.

    //if (AXObjectCache* cache = document().axObjectCache())
      //  cache->childrenChanged(this, newChild);
    if (is<RenderBlockFlow>(*this))
        downcast<RenderBlockFlow>(*this).invalidateLineLayoutPath();
    if (hasOutlineAutoAncestor() || outlineStyleForRepaint().outlineStyleIsAuto())
        newChild->setHasOutlineAutoAncestor();
}

RenderPtr<RenderObject> RenderElement::takeChildInternal(RenderObject& oldChild)
{
    //RELEASE_ASSERT_WITH_MESSAGE(!view().frameView().layoutContext().layoutState(), "Layout must not mutate render tree");

    ASSERT(canHaveChildren() || canHaveGeneratedChildren());
    ASSERT(oldChild.parent() == this);

    if (oldChild.isFloatingOrOutOfFlowPositioned())
        downcast<RenderBox>(oldChild).removeFloatingOrPositionedChildFromBlockLists();

    // So that we'll get the appropriate dirty bit set (either that a normal flow child got yanked or
    // that a positioned child got yanked). We also repaint, so that the area exposed when the child
    // disappears gets repainted properly.
    if (!renderTreeBeingDestroyed() && oldChild.everHadLayout()) {
        oldChild.setNeedsLayoutAndPrefWidthsRecalc();
        // We only repaint |oldChild| if we have a RenderLayer as its visual overflow may not be tracked by its parent.
        //if (oldChild.isBody())
          //  view().repaintRootContents();
        //else
            oldChild.repaint();
    }

    // If we have a line box wrapper, delete it.
    if (is<RenderBox>(oldChild))
        downcast<RenderBox>(oldChild).deleteLineBoxWrapper();
    else if (is<RenderLineBreak>(oldChild))
        downcast<RenderLineBreak>(oldChild).deleteInlineBoxWrapper();
    
    if (!renderTreeBeingDestroyed() && is<RenderFlexibleBox>(this) && !oldChild.isFloatingOrOutOfFlowPositioned() && oldChild.isBox())
        downcast<RenderFlexibleBox>(this)->clearCachedChildIntrinsicContentLogicalHeight(downcast<RenderBox>(oldChild));

    // If oldChild is the start or end of the selection, then clear the selection to
    // avoid problems of invalid pointers.
    //if (!renderTreeBeingDestroyed() && oldChild.isSelectionBorder())
        //frame().selection().setNeedsSelectionUpdate();

    if (!renderTreeBeingDestroyed())
        oldChild.willBeRemovedFromTree();

    oldChild.resetFragmentedFlowStateOnRemoval();

    // WARNING: There should be no code running between willBeRemovedFromTree and the actual removal below.
    // This is needed to avoid race conditions where willBeRemovedFromTree would dirty the tree's structure
    // and the code running here would force an untimely rebuilding, leaving |oldChild| dangling.
    
    RenderObject* nextSibling = oldChild.nextSibling();

    if (oldChild.previousSibling())
        oldChild.previousSibling()->setNextSibling(nextSibling);
    if (nextSibling)
        nextSibling->setPreviousSibling(oldChild.previousSibling());

    if (m_firstChild == &oldChild)
        m_firstChild = nextSibling;
    if (m_lastChild == &oldChild)
        m_lastChild = oldChild.previousSibling();

    oldChild.setPreviousSibling(nullptr);
    oldChild.setNextSibling(nullptr);
    oldChild.setParent(nullptr);

    // rendererRemovedFromTree walks the whole subtree. We can improve performance
    // by skipping this step when destroying the entire tree.
    //if (!renderTreeBeingDestroyed() && is<RenderElement>(oldChild))
        //RenderCounter::rendererRemovedFromTree(downcast<RenderElement>(oldChild));

    //if (AXObjectCache* cache = document().existingAXObjectCache())
    //    cache->childrenChanged(this);

    return RenderPtr<RenderObject>(&oldChild);
}

RenderBlock* RenderElement::containingBlockForFixedPosition() const
{
    auto* renderer = parent();
    while (renderer && !renderer->canContainFixedPositionObjects())
        renderer = renderer->parent();

    ASSERT(!renderer || !renderer->isAnonymousBlock());
    return downcast<RenderBlock>(renderer);
}

RenderBlock* RenderElement::containingBlockForAbsolutePosition() const
{
    // A relatively positioned RenderInline forwards its absolute positioned descendants to
    // its nearest non-anonymous containing block (to avoid having a positioned objects list in all RenderInlines).
    auto* renderer = isRenderInline() ? const_cast<RenderElement*>(downcast<RenderElement>(this)) : parent();
    while (renderer && !renderer->canContainAbsolutelyPositionedObjects())
        renderer = renderer->parent();
    // Make sure we only return non-anonymous RenderBlock as containing block.
    while (renderer && (!is<RenderBlock>(*renderer) || renderer->isAnonymousBlock()))
        renderer = renderer->containingBlock();
    return downcast<RenderBlock>(renderer);
}

static void addLayers(RenderElement& renderer, RenderLayer* parentLayer, RenderElement*& newObject, RenderLayer*& beforeChild)
{
    if (renderer.hasLayer()) {
        if (!beforeChild && newObject) {
            // We need to figure out the layer that follows newObject. We only do
            // this the first time we find a child layer, and then we update the
            // pointer values for newObject and beforeChild used by everyone else.
            beforeChild = newObject->parent()->findNextLayer(parentLayer, newObject);
            newObject = nullptr;
        }
        //parentLayer->addChild(downcast<RenderElement>(renderer).layer(), beforeChild);
        return;
    }

    for (auto& child : childrenOfType<RenderElement>(renderer))
        addLayers(child, parentLayer, newObject, beforeChild);
}

void RenderElement::addLayers(RenderLayer* parentLayer)
{
    if (!parentLayer)
        return;

    RenderElement* renderer = this;
    RenderLayer* beforeChild = nullptr;
    WebCore::addLayers(*this, parentLayer, renderer, beforeChild);
}

void RenderElement::removeLayers(RenderLayer* parentLayer)
{
    if (!parentLayer)
        return;

    //if (hasLayer()) {
    //    parentLayer->removeChild(downcast<RenderElement>(*this).layer());
    //    return;
    //}

    for (auto& child : childrenOfType<RenderElement>(*this))
        child.removeLayers(parentLayer);
}

void RenderElement::moveLayers(RenderLayer* oldParent, RenderLayer* newParent)
{
    if (!newParent)
        return;



    for (auto& child : childrenOfType<RenderElement>(*this))
        child.moveLayers(oldParent, newParent);
}



void RenderElement::propagateStyleToAnonymousChildren(StylePropagationType propagationType)
{
    // FIXME: We could save this call when the change only affected non-inherited properties.
    for (auto& elementChild : childrenOfType<RenderElement>(*this)) {
        if (!elementChild.isAnonymous() || elementChild.style().styleType() != NOPSEUDO)
            continue;

        if (propagationType == PropagateToBlockChildrenOnly && !is<RenderBlock>(elementChild))
            continue;

#if ENABLE(FULLSCREEN_API)
        if (elementChild.isRenderFullScreen() || elementChild.isRenderFullScreenPlaceholder())
            continue;
#endif

        // RenderFragmentedFlows are updated through the RenderView::styleDidChange function.
        if (is<RenderFragmentedFlow>(elementChild))
            continue;

        auto newStyle = RenderStyle::createAnonymousStyleWithDisplay(style(), elementChild.style().display());
        if (style().specifiesColumns()) {
            if (elementChild.style().specifiesColumns())
                newStyle.inheritColumnPropertiesFrom(style());
            if (elementChild.style().columnSpan())
                newStyle.setColumnSpan(ColumnSpanAll);
        }

        // Preserve the position style of anonymous block continuations as they can have relative or sticky position when
        // they contain block descendants of relative or sticky positioned inlines.
        if (elementChild.isInFlowPositioned() && elementChild.isContinuation())
            newStyle.setPosition(elementChild.style().position());

        updateAnonymousChildStyle(elementChild, newStyle);
        
        elementChild.setStyle(WTFMove(newStyle));
    }
}

static inline bool rendererHasBackground(const RenderElement* renderer)
{
    return renderer && renderer->hasBackground();
}

void RenderElement::invalidateCachedFirstLineStyle()
{
    if (!m_hasValidCachedFirstLineStyle)
        return;
    m_hasValidCachedFirstLineStyle = false;
    // Invalidate the subtree as descendant's first line style may depend on ancestor's.
    for (auto& descendant : descendantsOfType<RenderElement>(*this))
        descendant.m_hasValidCachedFirstLineStyle = false;
}

void RenderElement::styleWillChange(StyleDifference diff, const RenderStyle& newStyle)
{
    auto* oldStyle = hasInitializedStyle() ? &style() : nullptr;
    if (oldStyle) {
        // If our z-index changes value or our visibility changes,
        // we need to dirty our stacking context's z-order list.
        bool visibilityChanged = m_style.visibility() != newStyle.visibility()
            || m_style.zIndex() != newStyle.zIndex()
            || m_style.hasAutoZIndex() != newStyle.hasAutoZIndex();
#if ENABLE(DASHBOARD_SUPPORT)
        if (visibilityChanged)
            document().setAnnotatedRegionsDirty(true);
#endif
#if PLATFORM(IOS) && ENABLE(TOUCH_EVENTS)
        if (visibilityChanged)
            document().setTouchEventRegionsNeedUpdate();
#endif
        if (visibilityChanged) {
            //if (AXObjectCache* cache = document().existingAXObjectCache())
              //  cache->childrenChanged(parent(), this);
        }

        // Keep layer hierarchy visibility bits up to date if visibility changes.
        if (m_style.visibility() != newStyle.visibility()) {
            if (RenderLayer* layer = enclosingLayer()) {
                /**
                if (newStyle.visibility() == VISIBLE)
                    layer->setHasVisibleContent();
                else if (layer->hasVisibleContent() && (this == &layer->renderer() || layer->renderer().style().visibility() != VISIBLE)) {
                    layer->dirtyVisibleContentStatus();
                    if (diff > StyleDifferenceRepaintLayer)
                        repaint();
                }*/
            }
        }

        if (m_parent && (newStyle.outlineSize() < m_style.outlineSize() || shouldRepaintForStyleDifference(diff)))
            repaint();
        if (isFloating() && m_style.floating() != newStyle.floating()) {
            // For changes in float styles, we need to conceivably remove ourselves
            // from the floating objects list.
            downcast<RenderBox>(*this).removeFloatingOrPositionedChildFromBlockLists();
        } else if (isOutOfFlowPositioned() && m_style.position() != newStyle.position()) {
            // For changes in positioning styles, we need to conceivably remove ourselves
            // from the positioned objects list.
            downcast<RenderBox>(*this).removeFloatingOrPositionedChildFromBlockLists();
        }

        s_affectsParentBlock = isFloatingOrOutOfFlowPositioned()
            && (!newStyle.isFloating() && !newStyle.hasOutOfFlowPosition())
            && parent() && (parent()->isRenderBlockFlow() || parent()->isRenderInline());

        s_noLongerAffectsParentBlock = ((!isFloating() && newStyle.isFloating()) || (!isOutOfFlowPositioned() && newStyle.hasOutOfFlowPosition()))
            && parent() && parent()->isRenderBlock();

        // reset style flags
        if (diff == StyleDifferenceLayout || diff == StyleDifferenceLayoutPositionedMovementOnly) {
            setFloating(false);
            clearPositionedState();
        }
        if (newStyle.hasPseudoStyle(FIRST_LINE) || oldStyle->hasPseudoStyle(FIRST_LINE))
            invalidateCachedFirstLineStyle();

        setHorizontalWritingMode(true);
        setHasVisibleBoxDecorations(false);
        setHasOverflowClip(false);
        setHasTransformRelatedProperty(false);
        setHasReflection(false);
    } else {
        s_affectsParentBlock = false;
        s_noLongerAffectsParentBlock = false;
    }

    /**
    bool newStyleSlowScroll = false;
    if (newStyle.hasFixedBackgroundImage() && !settings().fixedBackgroundsPaintRelativeToDocument()) {
        newStyleSlowScroll = true;
        bool drawsRootBackground = isDocumentElementRenderer() || (isBody() && !rendererHasBackground(document().documentElement()->renderer()));
        if (drawsRootBackground && newStyle.hasEntirelyFixedBackground() && view().compositor().supportsFixedRootBackgroundCompositing())
            newStyleSlowScroll = false;
    }

    if (view().frameView().hasSlowRepaintObject(*this)) {
        if (!newStyleSlowScroll)
            view().frameView().removeSlowRepaintObject(*this);
    } else if (newStyleSlowScroll)
        view().frameView().addSlowRepaintObject(*this);

    if (isDocumentElementRenderer() || isBody())
        view().frameView().updateExtendBackgroundIfNecessary();
        */
}

void RenderElement::handleDynamicFloatPositionChange()
{
    // We have gone from not affecting the inline status of the parent flow to suddenly
    // having an impact.  See if there is a mismatch between the parent flow's
    // childrenInline() state and our state.
    setInline(style().isDisplayInlineType());
    if (isInline() != parent()->childrenInline()) {
        if (!isInline())
            downcast<RenderBoxModelObject>(*parent()).childBecameNonInline(*this);
        else {
            // An anonymous block must be made to wrap this inline.
            auto newBlock = downcast<RenderBlock>(*parent()).createAnonymousBlock();
            auto& block = *newBlock;
            parent()->insertChildInternal(WTFMove(newBlock), this);
            auto thisToMove = parent()->takeChildInternal(*this);
            block.insertChildInternal(WTFMove(thisToMove), nullptr);
        }
    }
}

void RenderElement::removeAnonymousWrappersForInlinesIfNecessary()
{
    // FIXME: Move to RenderBlock.
    if (!is<RenderBlock>(*this))
        return;
    RenderBlock& thisBlock = downcast<RenderBlock>(*this);
    if (!thisBlock.canDropAnonymousBlockChild())
        return;

    // We have changed to floated or out-of-flow positioning so maybe all our parent's
    // children can be inline now. Bail if there are any block children left on the line,
    // otherwise we can proceed to stripping solitary anonymous wrappers from the inlines.
    // FIXME: We should also handle split inlines here - we exclude them at the moment by returning
    // if we find a continuation.
    RenderObject* current = firstChild();
    while (current && ((current->isAnonymousBlock() && !downcast<RenderBlock>(*current).isContinuation()) || current->style().isFloating() || current->style().hasOutOfFlowPosition()))
        current = current->nextSibling();

    if (current)
        return;

    RenderObject* next;
    for (current = firstChild(); current; current = next) {
        next = current->nextSibling();
        if (current->isAnonymousBlock())
            thisBlock.dropAnonymousBoxChild(downcast<RenderBlock>(*current));
    }
}

#if !PLATFORM(IOS)
static bool areNonIdenticalCursorListsEqual(const RenderStyle* a, const RenderStyle* b)
{
    //ASSERT(a->cursors() != b->cursors());
    //return a->cursors() && b->cursors() && *a->cursors() == *b->cursors();
    return true;
}

static inline bool areCursorsEqual(const RenderStyle* a, const RenderStyle* b)
{
    return true; //a->cursor() == b->cursor() && (a->cursors() == b->cursors() || areNonIdenticalCursorListsEqual(a, b));
}
#endif

void RenderElement::styleDidChange(StyleDifference diff, const RenderStyle* oldStyle)
{
    /**
    updateFillImages(oldStyle ? &oldStyle->backgroundLayers() : nullptr, m_style.backgroundLayers());
    updateFillImages(oldStyle ? &oldStyle->maskLayers() : nullptr, m_style.maskLayers());
    updateImage(oldStyle ? oldStyle->borderImage().image() : nullptr, m_style.borderImage().image());
    updateImage(oldStyle ? oldStyle->maskBoxImage().image() : nullptr, m_style.maskBoxImage().image());
    updateShapeImage(oldStyle ? oldStyle->shapeOutside() : nullptr, m_style.shapeOutside());

    if (s_affectsParentBlock)
        handleDynamicFloatPositionChange();

    if (s_noLongerAffectsParentBlock)
        parent()->removeAnonymousWrappersForInlinesIfNecessary();

    SVGRenderSupport::styleChanged(*this, oldStyle);

    if (!m_parent)
        return;
    
    if (diff == StyleDifferenceLayout || diff == StyleDifferenceSimplifiedLayout) {
        RenderCounter::rendererStyleChanged(*this, oldStyle, &m_style);

        // If the object already needs layout, then setNeedsLayout won't do
        // any work. But if the containing block has changed, then we may need
        // to mark the new containing blocks for layout. The change that can
        // directly affect the containing block of this object is a change to
        // the position style.
        if (needsLayout() && oldStyle->position() != m_style.position())
            markContainingBlocksForLayout();

        if (diff == StyleDifferenceLayout)
            setNeedsLayoutAndPrefWidthsRecalc();
        else
            setNeedsSimplifiedNormalFlowLayout();
    } else if (diff == StyleDifferenceSimplifiedLayoutAndPositionedMovement) {
        setNeedsPositionedMovementLayout(oldStyle);
        setNeedsSimplifiedNormalFlowLayout();
    } else if (diff == StyleDifferenceLayoutPositionedMovementOnly)
        setNeedsPositionedMovementLayout(oldStyle);

    // Don't check for repaint here; we need to wait until the layer has been
    // updated by subclasses before we know if we have to repaint (in setStyle()).

#if !PLATFORM(IOS)
    if (oldStyle && !areCursorsEqual(oldStyle, &style()))
        frame().eventHandler().scheduleCursorUpdate();
#endif
    bool hadOutlineAuto = oldStyle && oldStyle->outlineStyleIsAuto();
    bool hasOutlineAuto = outlineStyleForRepaint().outlineStyleIsAuto();
    if (hasOutlineAuto != hadOutlineAuto) {
        updateOutlineAutoAncestor(hasOutlineAuto);
        issueRepaintForOutlineAuto(hasOutlineAuto ? outlineStyleForRepaint().outlineSize() : oldStyle->outlineSize());
    }*/
}

void RenderElement::insertedIntoTree()
{
    // Keep our layer hierarchy updated. Optimize for the common case where we don't have any children
    // and don't have a layer attached to ourselves.
    RenderLayer* layer = nullptr;
    if (firstChild() || hasLayer()) {
        layer = parent()->enclosingLayer();
        addLayers(layer);
    }

    // If |this| is visible but this object was not, tell the layer it has some visible content
    // that needs to be drawn and layer visibility optimization can't be used
    if (parent()->style().visibility() != VISIBLE && style().visibility() == VISIBLE && !hasLayer()) {
        if (!layer)
            layer = parent()->enclosingLayer();
        //if (layer)
        //    layer->setHasVisibleContent();
    }

    RenderObject::insertedIntoTree();
}

void RenderElement::willBeRemovedFromTree()
{
    // If we remove a visible child from an invisible parent, we don't know the layer visibility any more.
    RenderLayer* layer = nullptr;
    if (parent()->style().visibility() != VISIBLE && style().visibility() == VISIBLE && !hasLayer()) {
        //if ((layer = parent()->enclosingLayer()))
            //layer->dirtyVisibleContentStatus();
    }
    // Keep our layer hierarchy updated.
    if (firstChild() || hasLayer()) {
        if (!layer)
            layer = parent()->enclosingLayer();
        removeLayers(layer);
    }

    if (isOutOfFlowPositioned() && parent()->childrenInline())
        parent()->dirtyLinesFromChangedChild(*this);

    RenderObject::willBeRemovedFromTree();
}

inline void RenderElement::clearSubtreeLayoutRootIfNeeded() const
{
    if (renderTreeBeingDestroyed())
        return;

    //if (view().frameView().layoutContext().subtreeLayoutRoot() != this)
    //    return;

    // Normally when a renderer is detached from the tree, the appropriate dirty bits get set
    // which ensures that this renderer is no longer the layout root.
    ASSERT_NOT_REACHED();
    
    // This indicates a failure to layout the child, which is why
    // the layout root is still set to |this|. Make sure to clear it
    // since we are getting destroyed.
    //view().frameView().layoutContext().clearSubtreeLayoutRoot();
}

void RenderElement::willBeDestroyed()
{
    //if (m_style.hasFixedBackgroundImage() && !settings().fixedBackgroundsPaintRelativeToDocument())
        //view().frameView().removeSlowRepaintObject(*this);

    unregisterForVisibleInViewportCallback();

    //if (hasCounterNodeMap())
    //    RenderCounter::destroyCounterNodes(*this);

    RenderObject::willBeDestroyed();

    clearSubtreeLayoutRootIfNeeded();

    /**
    if (hasInitializedStyle()) {
        for (auto* bgLayer = &m_style.backgroundLayers(); bgLayer; bgLayer = bgLayer->next()) {
            if (auto* backgroundImage = bgLayer->image())
                backgroundImage->removeClient(this);
        }
        for (auto* maskLayer = &m_style.maskLayers(); maskLayer; maskLayer = maskLayer->next()) {
            if (auto* maskImage = maskLayer->image())
                maskImage->removeClient(this);
        }
        if (auto* borderImage = m_style.borderImage().image())
            borderImage->removeClient(this);
        if (auto* maskBoxImage = m_style.maskBoxImage().image())
            maskBoxImage->removeClient(this);
        if (auto shapeValue = m_style.shapeOutside()) {
            if (auto shapeImage = shapeValue->image())
                shapeImage->removeClient(this);
        }
    }
    if (m_hasPausedImageAnimations)
        view().removeRendererWithPausedImageAnimations(*this);
        */
}

void RenderElement::setNeedsPositionedMovementLayout(const RenderStyle* oldStyle)
{
    ASSERT(!isSetNeedsLayoutForbidden());
    if (needsPositionedMovementLayout())
        return;
    setNeedsPositionedMovementLayoutBit(true);
    markContainingBlocksForLayout();
    if (hasLayer()) {
        if (oldStyle)
            setLayerNeedsFullRepaint();
        else
            setLayerNeedsFullRepaintForPositionedMovementLayout();
    }
}

void RenderElement::clearChildNeedsLayout()
{
    setNormalChildNeedsLayoutBit(false);
    setPosChildNeedsLayoutBit(false);
    setNeedsSimplifiedNormalFlowLayoutBit(false);
    setNormalChildNeedsLayoutBit(false);
    setNeedsPositionedMovementLayoutBit(false);
}

void RenderElement::setNeedsSimplifiedNormalFlowLayout()
{
    ASSERT(!isSetNeedsLayoutForbidden());
    if (needsSimplifiedNormalFlowLayout())
        return;
    setNeedsSimplifiedNormalFlowLayoutBit(true);
    markContainingBlocksForLayout();
    if (hasLayer())
        setLayerNeedsFullRepaint();
}

RenderElement* RenderElement::hoverAncestor() const
{
    return parent();
}



void RenderElement::layout()
{
    StackStats::LayoutCheckPoint layoutCheckPoint;
    ASSERT(needsLayout());
    for (auto* child = firstChild(); child; child = child->nextSibling()) {
        if (child->needsLayout())
            downcast<RenderElement>(*child).layout();
        ASSERT(!child->needsLayout());
    }
    clearNeedsLayout();
}








void RenderElement::setVisibleInViewportState(VisibleInViewportState state)
{
    if (state == visibleInViewportState())
        return;
    m_visibleInViewportState = static_cast<unsigned>(state);
    visibleInViewportStateChanged();
}

void RenderElement::visibleInViewportStateChanged()
{
    ASSERT_NOT_REACHED();
}







Color RenderElement::selectionForegroundColor() const
{
    return selectionColor(CSSPropertyWebkitTextFillColor);
}

Color RenderElement::selectionEmphasisMarkColor() const
{
    return selectionColor(CSSPropertyWebkitTextEmphasisColor);
}



bool RenderElement::getLeadingCorner(FloatPoint& point, bool& insideFixed) const
{
    if (!isInline() || isReplaced()) {
        point = localToAbsolute(FloatPoint(), UseTransforms, &insideFixed);
        return true;
    }

    // find the next text/image child, to get a position
    const RenderObject* o = this;
    while (o) {
        const RenderObject* p = o;
        if (RenderObject* child = o->firstChildSlow())
            o = child;
        else if (o->nextSibling())
            o = o->nextSibling();
        else {
            RenderObject* next = 0;
            while (!next && o->parent()) {
                o = o->parent();
                next = o->nextSibling();
            }
            o = next;

            if (!o)
                break;
        }
        ASSERT(o);

        if (!o->isInline() || o->isReplaced()) {
            point = o->localToAbsolute(FloatPoint(), UseTransforms, &insideFixed);
            return true;
        }

    }

    return false;
}

bool RenderElement::getTrailingCorner(FloatPoint& point, bool& insideFixed) const
{
    if (!isInline() || isReplaced()) {
        point = localToAbsolute(LayoutPoint(downcast<RenderBox>(*this).size()), UseTransforms, &insideFixed);
        return true;
    }

    // find the last text/image child, to get a position
    const RenderObject* o = this;
    while (o) {
        if (RenderObject* child = o->lastChildSlow())
            o = child;
        else if (o->previousSibling())
            o = o->previousSibling();
        else {
            RenderObject* prev = 0;
            while (!prev) {
                o = o->parent();
                if (!o)
                    return false;
                prev = o->previousSibling();
            }
            o = prev;
        }
        ASSERT(o);
        if (o->isReplaced()) {
            point = FloatPoint();
            point.moveBy(downcast<RenderBox>(*o).frameRect().maxXMaxYCorner());
            point = o->container()->localToAbsolute(point, UseTransforms, &insideFixed);
            return true;
        }
    }
    return true;
}

LayoutRect RenderElement::absoluteAnchorRect(bool* insideFixed) const
{
    FloatPoint leading, trailing;
    bool leadingInFixed = false;
    bool trailingInFixed = false;
    getLeadingCorner(leading, leadingInFixed);
    getTrailingCorner(trailing, trailingInFixed);

    FloatPoint upperLeft = leading;
    FloatPoint lowerRight = trailing;

    // Vertical writing modes might mean the leading point is not in the top left
    if (!isInline() || isReplaced()) {
        upperLeft = FloatPoint(std::min(leading.x(), trailing.x()), std::min(leading.y(), trailing.y()));
        lowerRight = FloatPoint(std::max(leading.x(), trailing.x()), std::max(leading.y(), trailing.y()));
    } // Otherwise, it's not obvious what to do.

    if (insideFixed) {
        // For now, just look at the leading corner. Handling one inside fixed and one not would be tricky.
        *insideFixed = leadingInFixed;
    }

    return enclosingLayoutRect(FloatRect(upperLeft, lowerRight.expandedTo(upperLeft) - upperLeft));
}







void RenderElement::updateOutlineAutoAncestor(bool hasOutlineAuto)
{
    for (auto& child : childrenOfType<RenderObject>(*this)) {
        if (hasOutlineAuto == child.hasOutlineAutoAncestor())
            continue;
        child.setHasOutlineAutoAncestor(hasOutlineAuto);
        bool childHasOutlineAuto = child.outlineStyleForRepaint().outlineStyleIsAuto();
        if (childHasOutlineAuto)
            continue;
        if (!is<RenderElement>(child))
            continue;
        downcast<RenderElement>(child).updateOutlineAutoAncestor(hasOutlineAuto);
    }
    if (is<RenderBoxModelObject>(*this)) {
        if (auto* continuation = downcast<RenderBoxModelObject>(*this).continuation())
            continuation->updateOutlineAutoAncestor(hasOutlineAuto);
    }
}

bool RenderElement::hasSelfPaintingLayer() const
{
    if (!hasLayer())
        return false;
    auto& layerModelObject = downcast<RenderElement>(*this);
    return layerModelObject.hasSelfPaintingLayer();
}


void RenderElement::adjustFragmentedFlowStateOnContainingBlockChangeIfNeeded()
{
    if (fragmentedFlowState() == NotInsideFragmentedFlow)
        return;

    // Invalidate the containing block caches.
    if (is<RenderBlock>(*this))
        downcast<RenderBlock>(*this).resetEnclosingFragmentedFlowAndChildInfoIncludingDescendants();
    
    // Adjust the flow tread state on the subtree.
    setFragmentedFlowState(RenderObject::computedFragmentedFlowState(*this));
    for (auto& descendant : descendantsOfType<RenderObject>(*this))
        descendant.setFragmentedFlowState(RenderObject::computedFragmentedFlowState(descendant));
}

void RenderElement::removeFromRenderFragmentedFlow()
{
    ASSERT(fragmentedFlowState() != NotInsideFragmentedFlow);
    // Sometimes we remove the element from the flow, but it's not destroyed at that time.
    // It's only until later when we actually destroy it and remove all the children from it.
    // Currently, that happens for firstLetter elements and list markers.
    // Pass in the flow thread so that we don't have to look it up for all the children.
    removeFromRenderFragmentedFlowIncludingDescendants(true);
}

void RenderElement::removeFromRenderFragmentedFlowIncludingDescendants(bool shouldUpdateState)
{
    // Once we reach another flow thread we don't need to update the flow thread state
    // but we have to continue cleanup the flow thread info.
    if (isRenderFragmentedFlow())
        shouldUpdateState = false;

    for (auto& child : childrenOfType<RenderObject>(*this)) {
        if (is<RenderElement>(child)) {
            downcast<RenderElement>(child).removeFromRenderFragmentedFlowIncludingDescendants(shouldUpdateState);
            continue;
        }
        if (shouldUpdateState)
            child.setFragmentedFlowState(NotInsideFragmentedFlow);
    }

    // We have to ask for our containing flow thread as it may be above the removed sub-tree.
    RenderFragmentedFlow* enclosingFragmentedFlow = this->enclosingFragmentedFlow();
    while (enclosingFragmentedFlow) {
        enclosingFragmentedFlow->removeFlowChildInfo(*this);

        if (enclosingFragmentedFlow->fragmentedFlowState() == NotInsideFragmentedFlow)
            break;
        auto* parent = enclosingFragmentedFlow->parent();
        if (!parent)
            break;
        enclosingFragmentedFlow = parent->enclosingFragmentedFlow();
    }
    if (is<RenderBlock>(*this))
        downcast<RenderBlock>(*this).setCachedEnclosingFragmentedFlowNeedsUpdate();

    if (shouldUpdateState)
        setFragmentedFlowState(NotInsideFragmentedFlow);
}

void RenderElement::resetEnclosingFragmentedFlowAndChildInfoIncludingDescendants(RenderFragmentedFlow* fragmentedFlow)
{
    if (fragmentedFlow)
        fragmentedFlow->removeFlowChildInfo(*this);

    for (auto& child : childrenOfType<RenderElement>(*this))
        child.resetEnclosingFragmentedFlowAndChildInfoIncludingDescendants(fragmentedFlow);
}

#if ENABLE(TEXT_AUTOSIZING)
static RenderObject::BlockContentHeightType includeNonFixedHeight(const RenderObject& renderer)
{
    const RenderStyle& style = renderer.style();
    if (style.height().type() == Fixed) {
        if (is<RenderBlock>(renderer)) {
            // For fixed height styles, if the overflow size of the element spills out of the specified
            // height, assume we can apply text auto-sizing.
            if (style.overflowY() == OVISIBLE
                && style.height().value() < downcast<RenderBlock>(renderer).layoutOverflowRect().maxY())
                return RenderObject::OverflowHeight;
        }
        return RenderObject::FixedHeight;
    }
    return RenderObject::FlexibleHeight;
}

void RenderElement::adjustComputedFontSizesOnBlocks(float size, float visibleWidth)
{
    Document* document = view().frameView().frame().document();
    if (!document)
        return;

    Vector<int> depthStack;
    int currentDepth = 0;
    int newFixedDepth = 0;

    // We don't apply autosizing to nodes with fixed height normally.
    // But we apply it to nodes which are located deep enough
    // (nesting depth is greater than some const) inside of a parent block
    // which has fixed height but its content overflows intentionally.
    for (RenderObject* descendent = traverseNext(this, includeNonFixedHeight, currentDepth, newFixedDepth); descendent; descendent = descendent->traverseNext(this, includeNonFixedHeight, currentDepth, newFixedDepth)) {
        while (depthStack.size() > 0 && currentDepth <= depthStack[depthStack.size() - 1])
            depthStack.remove(depthStack.size() - 1);
        if (newFixedDepth)
            depthStack.append(newFixedDepth);

        int stackSize = depthStack.size();
        if (is<RenderBlockFlow>(*descendent) && !descendent->isListItem() && (!stackSize || currentDepth - depthStack[stackSize - 1] > TextAutoSizingFixedHeightDepth))
            downcast<RenderBlockFlow>(*descendent).adjustComputedFontSizes(size, visibleWidth);
        newFixedDepth = 0;
    }

    // Remove style from auto-sizing table that are no longer valid.
    document->textAutoSizing().updateRenderTree();
}

void RenderElement::resetTextAutosizing()
{
    Document* document = view().frameView().frame().document();
    if (!document)
        return;

    LOG(TextAutosizing, "RenderElement::resetTextAutosizing()");

    document->textAutoSizing().reset();

    Vector<int> depthStack;
    int currentDepth = 0;
    int newFixedDepth = 0;

    for (RenderObject* descendent = traverseNext(this, includeNonFixedHeight, currentDepth, newFixedDepth); descendent; descendent = descendent->traverseNext(this, includeNonFixedHeight, currentDepth, newFixedDepth)) {
        while (depthStack.size() > 0 && currentDepth <= depthStack[depthStack.size() - 1])
            depthStack.remove(depthStack.size() - 1);
        if (newFixedDepth)
            depthStack.append(newFixedDepth);

        int stackSize = depthStack.size();
        if (is<RenderBlockFlow>(*descendent) && !descendent->isListItem() && (!stackSize || currentDepth - depthStack[stackSize - 1] > TextAutoSizingFixedHeightDepth))
            downcast<RenderBlockFlow>(*descendent).resetComputedFontSize();
        newFixedDepth = 0;
    }
}
#endif // ENABLE(TEXT_AUTOSIZING)

}
