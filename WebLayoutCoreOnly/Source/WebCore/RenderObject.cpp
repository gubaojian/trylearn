/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2004 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2011, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
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
#include "RenderObject.h"

#include "FloatQuad.h"
#include "GeometryUtilities.h"
#include "HitTestResult.h"
#include "LogicalSelectionOffsetCaches.h"
#include "RenderChildIterator.h"
//#include "RenderCounter.h"
//#include "RenderFragmentedFlow.h"
#include "RenderGeometryMap.h"
#include "RenderInline.h"
#include "RenderIterator.h"
#include "RenderMultiColumnFlow.h"
//#include "RenderRuby.h"
//#include "RenderScrollbarPart.h"
#include "RenderTableRow.h"
//#include "RenderTheme.h"
//#include "RenderView.h"
//#include "RenderWidget.h"
//#include "SVGRenderSupport.h"
//#include "StyleResolver.h"
#include "TransformState.h"
#include <algorithm>
#include <stdio.h>
#include <wtf/IsoMallocInlines.h>
#include <wtf/RefCountedLeakCounter.h>
#include <wtf/text/TextStream.h>

#if PLATFORM(IOS)
#include "SelectionRect.h"
#endif

namespace WebCore {


WTF_MAKE_ISO_ALLOCATED_IMPL(RenderObject);

#ifndef NDEBUG

RenderObject::SetLayoutNeededForbiddenScope::SetLayoutNeededForbiddenScope(RenderObject* renderObject, bool isForbidden)
    : m_renderObject(renderObject)
    , m_preexistingForbidden(m_renderObject->isSetNeedsLayoutForbidden())
{
    m_renderObject->setNeedsLayoutIsForbidden(isForbidden);
}

RenderObject::SetLayoutNeededForbiddenScope::~SetLayoutNeededForbiddenScope()
{
    m_renderObject->setNeedsLayoutIsForbidden(m_preexistingForbidden);
}
#endif

struct SameSizeAsRenderObject {
    virtual ~SameSizeAsRenderObject() = default; // Allocate vtable pointer.
    void* pointers[5];
#ifndef NDEBUG
    unsigned m_debugBitfields : 2;
#endif
    unsigned m_bitfields;
};

//COMPILE_ASSERT(sizeof(RenderObject) == sizeof(SameSizeAsRenderObject), RenderObject_should_stay_small);

DEFINE_DEBUG_ONLY_GLOBAL(WTF::RefCountedLeakCounter, renderObjectCounter, ("RenderObject"));

void RenderObjectDeleter::operator() (RenderObject* renderer) const
{
    renderer->destroy();
}

RenderObject::RenderObject()
    :m_parent(nullptr)
    , m_previous(nullptr)
    , m_next(nullptr)
#ifndef NDEBUG
    , m_hasAXObject(false)
    , m_setNeedsLayoutForbidden(false)
#endif
{
#ifndef NDEBUG
    renderObjectCounter.increment();
#endif
}

RenderObject::~RenderObject()
{
#ifndef NDEBUG
    ASSERT(!m_hasAXObject);
    renderObjectCounter.decrement();
#endif
    ASSERT(!hasRareData());
}



bool RenderObject::isDescendantOf(const RenderObject* ancestor) const
{
    for (const RenderObject* renderer = this; renderer; renderer = renderer->m_parent) {
        if (renderer == ancestor)
            return true;
    }
    return false;
}

bool RenderObject::isLegend() const
{
    return false;
}

    
bool RenderObject::isFieldset() const
{
    return false;
}

bool RenderObject::isHTMLMarquee() const
{
    return false;
}

void RenderObject::setFragmentedFlowStateIncludingDescendants(FragmentedFlowState state)
{
    setFragmentedFlowState(state);

    if (!is<RenderElement>(*this))
        return;

    for (auto& child : childrenOfType<RenderObject>(downcast<RenderElement>(*this))) {
        // If the child is a fragmentation context it already updated the descendants flag accordingly.
        if (child.isRenderFragmentedFlow())
            continue;
        ASSERT(state != child.fragmentedFlowState());
        child.setFragmentedFlowStateIncludingDescendants(state);
    }
}

RenderObject::FragmentedFlowState RenderObject::computedFragmentedFlowState(const RenderObject& renderer)
{
    if (!renderer.parent())
        return renderer.fragmentedFlowState();

    auto inheritedFlowState = RenderObject::NotInsideFragmentedFlow;
    if (auto* container = renderer.container())
        inheritedFlowState = container->fragmentedFlowState();
    else {
        // Splitting lines or doing continuation, so just keep the current state.
        inheritedFlowState = renderer.fragmentedFlowState();
    }
    return inheritedFlowState;
}

void RenderObject::initializeFragmentedFlowStateOnInsertion()
{
    ASSERT(parent());

    // A RenderFragmentedFlow is always considered to be inside itself, so it never has to change its state in response to parent changes.
    if (isRenderFragmentedFlow())
        return;

    auto computedState = computedFragmentedFlowState(*this);
    if (fragmentedFlowState() == computedState)
        return;

    setFragmentedFlowStateIncludingDescendants(computedState);
}

void RenderObject::resetFragmentedFlowStateOnRemoval()
{
    if (fragmentedFlowState() == NotInsideFragmentedFlow)
        return;

    if (!renderTreeBeingDestroyed() && is<RenderElement>(*this)) {
        downcast<RenderElement>(*this).removeFromRenderFragmentedFlow();
        return;
    }

    // A RenderFragmentedFlow is always considered to be inside itself, so it never has to change its state in response to parent changes.
    if (isRenderFragmentedFlow())
        return;

    setFragmentedFlowStateIncludingDescendants(NotInsideFragmentedFlow);
}

void RenderObject::setParent(RenderElement* parent)
{
    m_parent = parent;
}

void RenderObject::removeFromParentAndDestroy()
{
    ASSERT(m_parent);
    m_parent->removeAndDestroyChild(*this);
}

RenderObject* RenderObject::nextInPreOrder() const
{
    if (RenderObject* o = firstChildSlow())
        return o;

    return nextInPreOrderAfterChildren();
}

RenderObject* RenderObject::nextInPreOrderAfterChildren() const
{
    RenderObject* o;
    if (!(o = nextSibling())) {
        o = parent();
        while (o && !o->nextSibling())
            o = o->parent();
        if (o)
            o = o->nextSibling();
    }

    return o;
}

RenderObject* RenderObject::nextInPreOrder(const RenderObject* stayWithin) const
{
    if (RenderObject* o = firstChildSlow())
        return o;

    return nextInPreOrderAfterChildren(stayWithin);
}

RenderObject* RenderObject::nextInPreOrderAfterChildren(const RenderObject* stayWithin) const
{
    if (this == stayWithin)
        return nullptr;

    const RenderObject* current = this;
    RenderObject* next;
    while (!(next = current->nextSibling())) {
        current = current->parent();
        if (!current || current == stayWithin)
            return nullptr;
    }
    return next;
}

RenderObject* RenderObject::previousInPreOrder() const
{
    if (RenderObject* o = previousSibling()) {
        while (RenderObject* last = o->lastChildSlow())
            o = last;
        return o;
    }

    return parent();
}

RenderObject* RenderObject::previousInPreOrder(const RenderObject* stayWithin) const
{
    if (this == stayWithin)
        return nullptr;

    return previousInPreOrder();
}

RenderObject* RenderObject::childAt(unsigned index) const
{
    RenderObject* child = firstChildSlow();
    for (unsigned i = 0; child && i < index; i++)
        child = child->nextSibling();
    return child;
}

RenderObject* RenderObject::firstLeafChild() const
{
    RenderObject* r = firstChildSlow();
    while (r) {
        RenderObject* n = nullptr;
        n = r->firstChildSlow();
        if (!n)
            break;
        r = n;
    }
    return r;
}

RenderObject* RenderObject::lastLeafChild() const
{
    RenderObject* r = lastChildSlow();
    while (r) {
        RenderObject* n = nullptr;
        n = r->lastChildSlow();
        if (!n)
            break;
        r = n;
    }
    return r;
}

#if ENABLE(TEXT_AUTOSIZING)

// Non-recursive version of the DFS search.
RenderObject* RenderObject::traverseNext(const RenderObject* stayWithin, HeightTypeTraverseNextInclusionFunction inclusionFunction, int& currentDepth, int& newFixedDepth) const
{
    BlockContentHeightType overflowType;

    // Check for suitable children.
    for (RenderObject* child = firstChildSlow(); child; child = child->nextSibling()) {
        overflowType = inclusionFunction(*child);
        if (overflowType != FixedHeight) {
            currentDepth++;
            if (overflowType == OverflowHeight)
                newFixedDepth = currentDepth;
            ASSERT(!stayWithin || child->isDescendantOf(stayWithin));
            return child;
        }
    }

    if (this == stayWithin)
        return nullptr;

    // Now we traverse other nodes if they exist, otherwise
    // we go to the parent node and try doing the same.
    const RenderObject* n = this;
    while (n) {
        while (n && !n->nextSibling() && (!stayWithin || n->parent() != stayWithin)) {
            n = n->parent();
            currentDepth--;
        }
        if (!n)
            return nullptr;
        for (RenderObject* sibling = n->nextSibling(); sibling; sibling = sibling->nextSibling()) {
            overflowType = inclusionFunction(*sibling);
            if (overflowType != FixedHeight) {
                if (overflowType == OverflowHeight)
                    newFixedDepth = currentDepth;
                ASSERT(!stayWithin || !n->nextSibling() || n->nextSibling()->isDescendantOf(stayWithin));
                return sibling;
            }
        }
        if (!stayWithin || n->parent() != stayWithin) {
            n = n->parent();
            currentDepth--;
        } else
            return nullptr;
    }
    return nullptr;
}

#endif // ENABLE(TEXT_AUTOSIZING)

RenderLayer* RenderObject::enclosingLayer() const
{
    for (auto& renderer : lineageOfType<RenderElement>(*this)) {
        //if (renderer.hasLayer())
          //  return renderer.layer();
    }
    return nullptr;
}


RenderBox& RenderObject::enclosingBox() const
{
    return *lineageOfType<RenderBox>(const_cast<RenderObject&>(*this)).first();
}

RenderBoxModelObject& RenderObject::enclosingBoxModelObject() const
{
    return *lineageOfType<RenderBoxModelObject>(const_cast<RenderObject&>(*this)).first();
}

RenderBlock* RenderObject::firstLineBlock() const
{
    return nullptr;
}

static inline bool objectIsRelayoutBoundary(const RenderElement* object)
{
    // FIXME: In future it may be possible to broaden these conditions in order to improve performance.
    if (object->isRenderView())
        return true;

    if (object->isTextControl())
        return true;

    if (object->isSVGRoot())
        return true;

    if (!object->hasOverflowClip())
        return false;

    if (object->style().width().isIntrinsicOrAuto() || object->style().height().isIntrinsicOrAuto() || object->style().height().isPercentOrCalculated())
        return false;

    // Table parts can't be relayout roots since the table is responsible for layouting all the parts.
    if (object->isTablePart())
        return false;

    return true;
}

void RenderObject::clearNeedsLayout()
{
    m_bitfields.setNeedsLayout(false);
    setEverHadLayout(true);
    setPosChildNeedsLayoutBit(false);
    setNeedsSimplifiedNormalFlowLayoutBit(false);
    setNormalChildNeedsLayoutBit(false);
    setNeedsPositionedMovementLayoutBit(false);
    if (is<RenderElement>(*this))
        downcast<RenderElement>(*this).setAncestorLineBoxDirty(false);
#ifndef NDEBUG
    checkBlockPositionedObjectsNeedLayout();
#endif
}

static void scheduleRelayoutForSubtree(RenderElement& renderer)
{
    /*
    if (is<RenderView>(renderer)) {
        downcast<RenderView>(renderer).frameView().layoutContext().scheduleLayout();
        return;
    }

    if (renderer.isRooted())
        renderer.view().frameView().layoutContext().scheduleSubtreeLayout(renderer);
        */
}

void RenderObject::markContainingBlocksForLayout(ScheduleRelayout scheduleRelayout, RenderElement* newRoot)
{
    ASSERT(scheduleRelayout == ScheduleRelayout::No || !newRoot);
    ASSERT(!isSetNeedsLayoutForbidden());

    auto ancestor = container();

    bool simplifiedNormalFlowLayout = needsSimplifiedNormalFlowLayout() && !selfNeedsLayout() && !normalChildNeedsLayout();
    bool hasOutOfFlowPosition = !isText() && style().hasOutOfFlowPosition();

    while (ancestor) {
#ifndef NDEBUG
        // FIXME: Remove this once we remove the special cases for counters, quotes and mathml
        // calling setNeedsLayout during preferred width computation.
        SetLayoutNeededForbiddenScope layoutForbiddenScope(ancestor, isSetNeedsLayoutForbidden());
#endif
        // Don't mark the outermost object of an unrooted subtree. That object will be
        // marked when the subtree is added to the document.
        auto container = ancestor->container();
        if (!container && !ancestor->isRenderView())
            return;
        if (hasOutOfFlowPosition) {
            bool willSkipRelativelyPositionedInlines = !ancestor->isRenderBlock() || ancestor->isAnonymousBlock();
            // Skip relatively positioned inlines and anonymous blocks to get to the enclosing RenderBlock.
            while (ancestor && (!ancestor->isRenderBlock() || ancestor->isAnonymousBlock()))
                ancestor = ancestor->container();
            if (!ancestor || ancestor->posChildNeedsLayout())
                return;
            if (willSkipRelativelyPositionedInlines)
                container = ancestor->container();
            ancestor->setPosChildNeedsLayoutBit(true);
            simplifiedNormalFlowLayout = true;
        } else if (simplifiedNormalFlowLayout) {
            if (ancestor->needsSimplifiedNormalFlowLayout())
                return;
            ancestor->setNeedsSimplifiedNormalFlowLayoutBit(true);
        } else {
            if (ancestor->normalChildNeedsLayout())
                return;
            ancestor->setNormalChildNeedsLayoutBit(true);
        }
        ASSERT(!ancestor->isSetNeedsLayoutForbidden());

        if (ancestor == newRoot)
            return;

        if (scheduleRelayout == ScheduleRelayout::Yes && objectIsRelayoutBoundary(ancestor))
            break;

        hasOutOfFlowPosition = ancestor->style().hasOutOfFlowPosition();
        ancestor = container;
    }

    if (scheduleRelayout == ScheduleRelayout::Yes && ancestor)
        scheduleRelayoutForSubtree(*ancestor);
}

#ifndef NDEBUG
void RenderObject::checkBlockPositionedObjectsNeedLayout()
{
    ASSERT(!needsLayout());

    if (is<RenderBlock>(*this))
        downcast<RenderBlock>(*this).checkPositionedObjectsNeedLayout();
}
#endif

void RenderObject::setPreferredLogicalWidthsDirty(bool shouldBeDirty, MarkingBehavior markParents)
{
    bool alreadyDirty = preferredLogicalWidthsDirty();
    m_bitfields.setPreferredLogicalWidthsDirty(shouldBeDirty);
    if (shouldBeDirty && !alreadyDirty && markParents == MarkContainingBlockChain && (isText() || !style().hasOutOfFlowPosition()))
        invalidateContainerPreferredLogicalWidths();
}

void RenderObject::invalidateContainerPreferredLogicalWidths()
{
    // In order to avoid pathological behavior when inlines are deeply nested, we do include them
    // in the chain that we mark dirty (even though they're kind of irrelevant).
    auto o = isTableCell() ? containingBlock() : container();
    while (o && !o->preferredLogicalWidthsDirty()) {
        // Don't invalidate the outermost object of an unrooted subtree. That object will be 
        // invalidated when the subtree is added to the document.
        auto container = o->isTableCell() ? o->containingBlock() : o->container();
        if (!container && !o->isRenderView())
            break;

        o->m_bitfields.setPreferredLogicalWidthsDirty(true);
        if (o->style().hasOutOfFlowPosition())
            // A positioned object has no effect on the min/max width of its containing block ever.
            // We can optimize this case and not go up any further.
            break;
        o = container;
    }
}

void RenderObject::setLayerNeedsFullRepaint()
{
    ASSERT(hasLayer());
    //downcast<RenderElement>(*this).layer()->setRepaintStatus(NeedsFullRepaint);
}

void RenderObject::setLayerNeedsFullRepaintForPositionedMovementLayout()
{
    ASSERT(hasLayer());
    //downcast<RenderElement>(*this).layer()->setRepaintStatus(NeedsFullRepaintForPositionedMovementLayout);
}

RenderBlock* RenderObject::containingBlock() const
{
    auto containingBlockForRenderer = [](const RenderElement& renderer)
    {
        if (renderer.isAbsolutelyPositioned())
            return renderer.containingBlockForAbsolutePosition();
        if (renderer.isFixedPositioned())
            return renderer.containingBlockForFixedPosition();
        return renderer.containingBlockForObjectInFlow();
    };


    return containingBlockForRenderer(downcast<RenderElement>(*this));
}

RenderBlock* RenderObject::containingBlockForObjectInFlow() const
{
    auto* renderer = parent();
    while (renderer && ((renderer->isInline() && !renderer->isReplaced()) || !renderer->isRenderBlock()))
        renderer = renderer->parent();
    return downcast<RenderBlock>(renderer);
}



#if PLATFORM(IOS)
// This function is similar in spirit to RenderText::absoluteRectsForRange, but returns rectangles
// which are annotated with additional state which helps iOS draw selections in its unique way.
// No annotations are added in this class.
// FIXME: Move to RenderText with absoluteRectsForRange()?
void RenderObject::collectSelectionRects(Vector<SelectionRect>& rects, unsigned start, unsigned end)
{
    Vector<FloatQuad> quads;

    if (!firstChildSlow()) {
        // FIXME: WebKit's position for an empty span after a BR is incorrect, so we can't trust 
        // quads for them. We don't need selection rects for those anyway though, since they 
        // are just empty containers. See <https://bugs.webkit.org/show_bug.cgi?id=49358>.
        RenderObject* previous = previousSibling();
        Node* node = this->node();
        if (!previous || !previous->isBR() || !node || !node->isContainerNode() || !isInline()) {
            // For inline elements we don't use absoluteQuads, since it takes into account continuations and leads to wrong results.
            absoluteQuadsForSelection(quads);
        }
    } else {
        unsigned offset = start;
        for (RenderObject* child = childAt(start); child && offset < end; child = child->nextSibling(), ++offset)
            child->absoluteQuads(quads);
    }

    unsigned numberOfQuads = quads.size();
    for (unsigned i = 0; i < numberOfQuads; ++i)
        rects.append(SelectionRect(quads[i].enclosingBoundingBox(), isHorizontalWritingMode(), view().pageNumberForBlockProgressionOffset(quads[i].enclosingBoundingBox().x())));
}
#endif

IntRect RenderObject::absoluteBoundingBoxRect(bool useTransforms, bool* wasFixed) const
{
    if (useTransforms) {
        Vector<FloatQuad> quads;
        absoluteQuads(quads, wasFixed);

        size_t n = quads.size();
        if (!n)
            return IntRect();
    
        IntRect result = quads[0].enclosingBoundingBox();
        for (size_t i = 1; i < n; ++i)
            result.unite(quads[i].enclosingBoundingBox());
        return result;
    }

    FloatPoint absPos = localToAbsolute(FloatPoint(), 0 /* ignore transforms */, wasFixed);
    Vector<IntRect> rects;
    absoluteRects(rects, flooredLayoutPoint(absPos));

    size_t n = rects.size();
    if (!n)
        return IntRect();

    LayoutRect result = rects[0];
    for (size_t i = 1; i < n; ++i)
        result.unite(rects[i]);
    return snappedIntRect(result);
}

void RenderObject::absoluteFocusRingQuads(Vector<FloatQuad>& quads)
{
    Vector<LayoutRect> rects;
    // FIXME: addFocusRingRects() needs to be passed this transform-unaware
    // localToAbsolute() offset here because RenderInline::addFocusRingRects()
    // implicitly assumes that. This doesn't work correctly with transformed
    // descendants.
    FloatPoint absolutePoint = localToAbsolute();
    addFocusRingRects(rects, flooredLayoutPoint(absolutePoint));
    float deviceScaleFactor = 10; //document().deviceScaleFactor();
    for (auto rect : rects) {
        rect.moveBy(LayoutPoint(-absolutePoint));
        quads.append(localToAbsoluteQuad(FloatQuad(snapRectToDevicePixels(rect, deviceScaleFactor))));
    }
}


void RenderObject::addAbsoluteRectForLayer(LayoutRect& result)
{
    if (hasLayer())
        result.unite(absoluteBoundingBoxRectIgnoringTransforms());

    if (!is<RenderElement>(*this))
        return;

    for (auto& child : childrenOfType<RenderObject>(downcast<RenderElement>(*this)))
        child.addAbsoluteRectForLayer(result);
}

// FIXME: change this to use the subtreePaint terminology
LayoutRect RenderObject::paintingRootRect(LayoutRect& topLevelRect)
{
    LayoutRect result = absoluteBoundingBoxRectIgnoringTransforms();
    topLevelRect = result;
    if (is<RenderElement>(*this)) {
        for (auto& child : childrenOfType<RenderObject>(downcast<RenderElement>(*this)))
            child.addAbsoluteRectForLayer(result);
    }
    return result;
}


IntRect RenderObject::pixelSnappedAbsoluteClippedOverflowRect() const
{
    return snappedIntRect(absoluteClippedOverflowRect());
}
    
LayoutRect RenderObject::rectWithOutlineForRepaint(const RenderElement* repaintContainer, LayoutUnit outlineWidth) const
{
    LayoutRect r(clippedOverflowRectForRepaint(repaintContainer));
    r.inflate(outlineWidth);
    return r;
}

LayoutRect RenderObject::clippedOverflowRectForRepaint(const RenderElement*) const
{
    ASSERT_NOT_REACHED();
    return LayoutRect();
}



FloatRect RenderObject::computeFloatRectForRepaint(const FloatRect&, const RenderElement*, bool) const
{
    ASSERT_NOT_REACHED();
    return FloatRect();
}

#if ENABLE(TREE_DEBUGGING)

static void outputRenderTreeLegend(TextStream& stream)
{
    stream.nextLine();
    stream << "(B)lock/(I)nline/I(N)line-block, (A)bsolute/Fi(X)ed/(R)elative/Stic(K)y, (F)loating, (O)verflow clip, Anon(Y)mous, (G)enerated, has(L)ayer, (C)omposited, (+)Dirty style, (+)Dirty layout";
    stream.nextLine();
}

void RenderObject::showNodeTreeForThis() const
{

}

void RenderObject::showRenderTreeForThis() const
{
    const WebCore::RenderObject* root = this;
    while (root->parent())
        root = root->parent();
    TextStream stream(TextStream::LineMode::MultipleLine, TextStream::Formatting::SVGStyleRect);
    outputRenderTreeLegend(stream);
    root->outputRenderSubTreeAndMark(stream, this, 1);
    WTFLogAlways("%s", stream.release().utf8().data());
}

void RenderObject::showLineTreeForThis() const
{
    if (!is<RenderBlockFlow>(*this))
        return;
    TextStream stream(TextStream::LineMode::MultipleLine, TextStream::Formatting::SVGStyleRect);
    outputRenderTreeLegend(stream);
    outputRenderObject(stream, false, 1);
    downcast<RenderBlockFlow>(*this).outputLineTreeAndMark(stream, nullptr, 2);
    WTFLogAlways("%s", stream.release().utf8().data());
}

static const RenderFragmentedFlow* enclosingFragmentedFlowFromRenderer(const RenderObject* renderer)
{
    if (!renderer)
        return nullptr;

    if (renderer->fragmentedFlowState() == RenderObject::NotInsideFragmentedFlow)
        return nullptr;

    if (is<RenderFragmentedFlow>(*renderer))
        return downcast<RenderFragmentedFlow>(renderer);

    if (is<RenderBlock>(*renderer))
        return downcast<RenderBlock>(*renderer).cachedEnclosingFragmentedFlow();

    return nullptr;
}

void RenderObject::outputRegionsInformation(TextStream& stream) const
{
    const RenderFragmentedFlow* ftcb = enclosingFragmentedFlowFromRenderer(this);

    if (!ftcb) {
        // Only the boxes have region range information.
        // Try to get the flow thread containing block information
        // from the containing block of this box.
        if (is<RenderBox>(*this))
            ftcb = enclosingFragmentedFlowFromRenderer(containingBlock());
    }

    if (!ftcb)
        return;

    RenderFragmentContainer* startRegion = nullptr;
    RenderFragmentContainer* endRegion = nullptr;
    ftcb->getFragmentRangeForBox(downcast<RenderBox>(this), startRegion, endRegion);
    stream << " [Rs:" << startRegion << " Re:" << endRegion << "]";
}

void RenderObject::outputRenderObject(TextStream& stream, bool mark, int depth) const
{
    if (isInlineBlockOrInlineTable())
        stream << "N";
    else if (isInline())
        stream << "I";
    else
        stream << "B";

    if (isPositioned()) {
        if (isRelativelyPositioned())
            stream << "R";
        else if (isStickilyPositioned())
            stream << "K";
        else if (isOutOfFlowPositioned()) {
            if (isAbsolutelyPositioned())
                stream << "A";
            else
                stream << "X";
        }
    } else
        stream << "-";

    if (isFloating())
        stream << "F";
    else
        stream << "-";

    if (hasOverflowClip())
        stream << "O";
    else
        stream << "-";

    if (isAnonymous())
        stream << "Y";
    else
        stream << "-";

    if (isPseudoElement() || isAnonymous())
        stream << "G";
    else
        stream << "-";

    if (hasLayer())
        stream << "L";
    else
        stream << "-";

    if (isComposited())
        stream << "C";
    else
        stream << "-";

    stream << " ";


    if (needsLayout())
        stream << "+";
    else
        stream << "-";

    int printedCharacters = 0;
    if (mark) {
        stream << "*";
        ++printedCharacters;
    }

    while (++printedCharacters <= depth * 2)
        stream << " ";


    String name = renderName();
    // FIXME: Renderer's name should not include property value listing.
    int pos = name.find('(');
    if (pos > 0)
        stream << name.left(pos - 1).utf8().data();
    else
        stream << name.utf8().data();

    if (is<RenderBox>(*this)) {
        auto& renderBox = downcast<RenderBox>(*this);
        FloatRect boxRect = renderBox.frameRect();
        if (renderBox.isInFlowPositioned())
            boxRect.move(renderBox.offsetForInFlowPosition());
        stream << " " << boxRect;
    } else if (is<RenderInline>(*this) && isInFlowPositioned()) {
        FloatSize inlineOffset = downcast<RenderInline>(*this).offsetForInFlowPosition();
        stream << "  (" << inlineOffset.width() << ", " << inlineOffset.height() << ")";
    }

    stream << " renderer->(" << this << ")";

    if (is<RenderBoxModelObject>(*this)) {
        auto& renderer = downcast<RenderBoxModelObject>(*this);
        if (renderer.continuation())
            stream << " continuation->(" << renderer.continuation() << ")";
    }
    outputRegionsInformation(stream);
    if (needsLayout()) {
        stream << " layout->";
        if (selfNeedsLayout())
            stream << "[self]";
        if (normalChildNeedsLayout())
            stream << "[normal child]";
        if (posChildNeedsLayout())
            stream << "[positioned child]";
        if (needsSimplifiedNormalFlowLayout())
            stream << "[simplified]";
        if (needsPositionedMovementLayout())
            stream << "[positioned movement]";
    }
    stream.nextLine();
}

void RenderObject::outputRenderSubTreeAndMark(TextStream& stream, const RenderObject* markedObject, int depth) const
{
    outputRenderObject(stream, markedObject == this, depth);
    if (is<RenderBlockFlow>(*this))
        downcast<RenderBlockFlow>(*this).outputLineTreeAndMark(stream, nullptr, depth + 1);

    for (auto* child = firstChildSlow(); child; child = child->nextSibling())
        child->outputRenderSubTreeAndMark(stream, markedObject, depth + 1);
}

#endif // NDEBUG

FloatPoint RenderObject::localToAbsolute(const FloatPoint& localPoint, MapCoordinatesFlags mode, bool* wasFixed) const
{
    TransformState transformState(TransformState::ApplyTransformDirection, localPoint);
    mapLocalToContainer(nullptr, transformState, mode | ApplyContainerFlip, wasFixed);
    transformState.flatten();
    
    return transformState.lastPlanarPoint();
}

FloatPoint RenderObject::absoluteToLocal(const FloatPoint& containerPoint, MapCoordinatesFlags mode) const
{
    TransformState transformState(TransformState::UnapplyInverseTransformDirection, containerPoint);
    mapAbsoluteToLocalPoint(mode, transformState);
    transformState.flatten();
    
    return transformState.lastPlanarPoint();
}

FloatQuad RenderObject::absoluteToLocalQuad(const FloatQuad& quad, MapCoordinatesFlags mode) const
{
    TransformState transformState(TransformState::UnapplyInverseTransformDirection, quad.boundingBox().center(), quad);
    mapAbsoluteToLocalPoint(mode, transformState);
    transformState.flatten();
    return transformState.lastPlanarQuad();
}

void RenderObject::mapLocalToContainer(const RenderElement* repaintContainer, TransformState& transformState, MapCoordinatesFlags mode, bool* wasFixed) const
{
    if (repaintContainer == this)
        return;

    auto* parent = this->parent();
    if (!parent)
        return;

    // FIXME: this should call offsetFromContainer to share code, but I'm not sure it's ever called.
    LayoutPoint centerPoint(transformState.mappedPoint());
    if (mode & ApplyContainerFlip && is<RenderBox>(*parent)) {
        if (parent->style().isFlippedBlocksWritingMode())
            transformState.move(downcast<RenderBox>(parent)->flipForWritingMode(LayoutPoint(transformState.mappedPoint())) - centerPoint);
        mode &= ~ApplyContainerFlip;
    }

    //if (is<RenderBox>(*parent))
        //transformState.move(-toLayoutSize(downcast<RenderBox>(*parent).scrollPosition()));

    parent->mapLocalToContainer(repaintContainer, transformState, mode, wasFixed);
}

const RenderObject* RenderObject::pushMappingToContainer(const RenderElement* ancestorToStopAt, RenderGeometryMap& geometryMap) const
{
    ASSERT_UNUSED(ancestorToStopAt, ancestorToStopAt != this);

    auto* container = parent();
    if (!container)
        return nullptr;

    // FIXME: this should call offsetFromContainer to share code, but I'm not sure it's ever called.
    LayoutSize offset;
    if (is<RenderBox>(*container))
        //offset = -toLayoutSize(downcast<RenderBox>(*container).scrollPosition());

    geometryMap.push(this, offset, false);
    
    return container;
}

void RenderObject::mapAbsoluteToLocalPoint(MapCoordinatesFlags mode, TransformState& transformState) const
{
    if (auto* parent = this->parent()) {
        parent->mapAbsoluteToLocalPoint(mode, transformState);
        //if (is<RenderBox>(*parent))
            //transformState.move(toLayoutSize(downcast<RenderBox>(*parent).scrollPosition()));
    }
}

bool RenderObject::shouldUseTransformFromContainer(const RenderObject* containerObject) const
{
#if ENABLE(3D_TRANSFORMS)
    return hasTransform() || (containerObject && containerObject->style().hasPerspective());
#else
    UNUSED_PARAM(containerObject);
    return hasTransform();
#endif
}

void RenderObject::getTransformFromContainer(const RenderObject* containerObject, const LayoutSize& offsetInContainer, TransformationMatrix& transform) const
{
    transform.makeIdentity();
    transform.translate(offsetInContainer.width(), offsetInContainer.height());
    RenderLayer* layer;
   // if (hasLayer() && (layer = downcast<RenderElement>(*this).layer()) && layer->transform())
     //   transform.multiply(layer->currentTransform());
    
#if ENABLE(3D_TRANSFORMS)
    if (containerObject && containerObject->hasLayer() && containerObject->style().hasPerspective()) {
        // Perpsective on the container affects us, so we have to factor it in here.
        ASSERT(containerObject->hasLayer());
        FloatPoint perspectiveOrigin = downcast<RenderElement>(*containerObject).layer()->perspectiveOrigin();

        TransformationMatrix perspectiveMatrix;
        perspectiveMatrix.applyPerspective(containerObject->style().perspective());
        
        transform.translateRight3d(-perspectiveOrigin.x(), -perspectiveOrigin.y(), 0);
        transform = perspectiveMatrix * transform;
        transform.translateRight3d(perspectiveOrigin.x(), perspectiveOrigin.y(), 0);
    }
#else
    UNUSED_PARAM(containerObject);
#endif
}

FloatQuad RenderObject::localToContainerQuad(const FloatQuad& localQuad, const RenderElement* repaintContainer, MapCoordinatesFlags mode, bool* wasFixed) const
{
    // Track the point at the center of the quad's bounding box. As mapLocalToContainer() calls offsetFromContainer(),
    // it will use that point as the reference point to decide which column's transform to apply in multiple-column blocks.
    TransformState transformState(TransformState::ApplyTransformDirection, localQuad.boundingBox().center(), localQuad);
    mapLocalToContainer(repaintContainer, transformState, mode | ApplyContainerFlip, wasFixed);
    transformState.flatten();
    
    return transformState.lastPlanarQuad();
}

FloatPoint RenderObject::localToContainerPoint(const FloatPoint& localPoint, const RenderElement* repaintContainer, MapCoordinatesFlags mode, bool* wasFixed) const
{
    TransformState transformState(TransformState::ApplyTransformDirection, localPoint);
    mapLocalToContainer(repaintContainer, transformState, mode | ApplyContainerFlip, wasFixed);
    transformState.flatten();

    return transformState.lastPlanarPoint();
}

LayoutSize RenderObject::offsetFromContainer(RenderElement& container, const LayoutPoint&, bool* offsetDependsOnPoint) const
{
    ASSERT(&container == this->container());

    LayoutSize offset;
    if (is<RenderBox>(container))
        //offset -= toLayoutSize(downcast<RenderBox>(container).scrollPosition());

    if (offsetDependsOnPoint)
        *offsetDependsOnPoint = is<RenderFragmentedFlow>(container);

    return offset;
}

LayoutSize RenderObject::offsetFromAncestorContainer(RenderElement& container) const
{
    LayoutSize offset;
    LayoutPoint referencePoint;
    const RenderObject* currContainer = this;
    do {
        RenderElement* nextContainer = currContainer->container();
        ASSERT(nextContainer);  // This means we reached the top without finding container.
        if (!nextContainer)
            break;
        ASSERT(!currContainer->hasTransform());
        LayoutSize currentOffset = currContainer->offsetFromContainer(*nextContainer, referencePoint);
        offset += currentOffset;
        referencePoint.move(currentOffset);
        currContainer = nextContainer;
    } while (currContainer != &container);

    return offset;
}

LayoutRect RenderObject::localCaretRect(InlineBox*, unsigned, LayoutUnit* extraWidthToEndOfLine)
{
    if (extraWidthToEndOfLine)
        *extraWidthToEndOfLine = 0;

    return LayoutRect();
}

bool RenderObject::isRooted() const
{
    return false;
}

static inline RenderElement* containerForElement(const RenderObject& renderer, const RenderElement* repaintContainer, bool* repaintContainerSkipped)
{
    // This method is extremely similar to containingBlock(), but with a few notable
    // exceptions.
    // (1) For normal flow elements, it just returns the parent.
    // (2) For absolute positioned elements, it will return a relative positioned inline, while
    // containingBlock() skips to the non-anonymous containing block.
    // This does mean that computePositionedLogicalWidth and computePositionedLogicalHeight have to use container().
    EPosition pos = renderer.style().position();
    auto* parent = renderer.parent();
    if ((pos != FixedPosition && pos != AbsolutePosition))
        return parent;
    for (; parent && (pos == AbsolutePosition ? !parent->canContainAbsolutelyPositionedObjects() : !parent->canContainFixedPositionObjects()); parent = parent->parent()) {
        if (repaintContainerSkipped && repaintContainer == parent)
            *repaintContainerSkipped = true;
    }
    return parent;
}

RenderElement* RenderObject::container() const
{
    return containerForElement(*this, nullptr, nullptr);
}

RenderElement* RenderObject::container(const RenderElement* repaintContainer, bool& repaintContainerSkipped) const
{
    repaintContainerSkipped = false;
    return containerForElement(*this, repaintContainer, &repaintContainerSkipped);
}

bool RenderObject::isSelectionBorder() const
{
    SelectionState st = selectionState();
    return st == SelectionStart
        || st == SelectionEnd
        || st == SelectionBoth;
}

void RenderObject::willBeDestroyed()
{
    ASSERT(!m_parent);
    ASSERT(renderTreeBeingDestroyed() || !is<RenderElement>(*this));



    removeRareData();
}

void RenderObject::insertedIntoTree()
{
    // FIXME: We should ASSERT(isRooted()) here but generated content makes some out-of-order insertion.

    if (!isFloating() && parent()->childrenInline())
        parent()->dirtyLinesFromChangedChild(*this);

    if (RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow())
        fragmentedFlow->fragmentedFlowDescendantInserted(*this);
}

void RenderObject::willBeRemovedFromTree()
{
    // FIXME: We should ASSERT(isRooted()) but we have some out-of-order removals which would need to be fixed first.
    // Update cached boundaries in SVG renderers, if a child is removed.
    parent()->setNeedsBoundariesUpdate();
}

static bool isAnonymousAndSafeToDelete(RenderElement& element)
{
    if (!element.isAnonymous())
        return false;
    if (element.isRenderView() || element.isRenderFragmentedFlow())
        return false;
    return true;
}

static RenderObject& findDestroyRootIncludingAnonymous(RenderObject& renderer)
{
    auto* destroyRoot = &renderer;
    while (true) {
        auto& destroyRootParent = *destroyRoot->parent();
        if (!isAnonymousAndSafeToDelete(destroyRootParent))
            break;
        bool destroyingOnlyChild = destroyRootParent.firstChild() == destroyRoot && destroyRootParent.lastChild() == destroyRoot;
        if (!destroyingOnlyChild)
            break;
        destroyRoot = &destroyRootParent;
    }
    return *destroyRoot;
}

void RenderObject::removeFromParentAndDestroyCleaningUpAnonymousWrappers()
{
    // If the tree is destroyed, there is no need for a clean-up phase.
    if (renderTreeBeingDestroyed()) {
        removeFromParentAndDestroy();
        return;
    }

    auto& destroyRoot = findDestroyRootIncludingAnonymous(*this);

    if (is<RenderTableRow>(destroyRoot))
        downcast<RenderTableRow>(destroyRoot).collapseAndDestroyAnonymousSiblingRows();

    auto& destroyRootParent = *destroyRoot.parent();
    destroyRootParent.removeAndDestroyChild(destroyRoot);
    destroyRootParent.removeAnonymousWrappersForInlinesIfNecessary();

    // Anonymous parent might have become empty, try to delete it too.
    if (isAnonymousAndSafeToDelete(destroyRootParent) && !destroyRootParent.firstChild())
        destroyRootParent.removeFromParentAndDestroyCleaningUpAnonymousWrappers();

    // WARNING: |this| is deleted here.
}

void RenderObject::destroy()
{
    RELEASE_ASSERT(!m_parent);
    RELEASE_ASSERT(!m_next);
    RELEASE_ASSERT(!m_previous);
    RELEASE_ASSERT(!m_bitfields.beingDestroyed());

    if (is<RenderElement>(*this))
        downcast<RenderElement>(*this).destroyLeftoverChildren();

    m_bitfields.setBeingDestroyed(true);

#if PLATFORM(IOS)
    if (hasLayer())
        downcast<RenderBoxModelObject>(*this).layer()->willBeDestroyed();
#endif

    willBeDestroyed();

    delete this;
}


bool RenderObject::hitTest(const HitTestRequest& request, HitTestResult& result, const HitTestLocation& locationInContainer, const LayoutPoint& accumulatedOffset, HitTestFilter hitTestFilter)
{
    bool inside = false;
    if (hitTestFilter != HitTestSelf) {
        // First test the foreground layer (lines and inlines).
        inside = nodeAtPoint(request, result, locationInContainer, accumulatedOffset, HitTestForeground);

        // Test floats next.
        if (!inside)
            inside = nodeAtPoint(request, result, locationInContainer, accumulatedOffset, HitTestFloat);

        // Finally test to see if the mouse is in the background (within a child block's background).
        if (!inside)
            inside = nodeAtPoint(request, result, locationInContainer, accumulatedOffset, HitTestChildBlockBackgrounds);
    }

    // See if the mouse is inside us but not any of our descendants
    if (hitTestFilter != HitTestDescendants && !inside)
        inside = nodeAtPoint(request, result, locationInContainer, accumulatedOffset, HitTestBlockBackground);

    return inside;
}

void RenderObject::updateHitTestResult(HitTestResult& result, const LayoutPoint& point)
{
    /**
    if (result.innerNode())
        return;

    Node* node = this->node();

    // If we hit the anonymous renderers inside generated content we should
    // actually hit the generated content so walk up to the PseudoElement.
    if (!node && parent() && parent()->isBeforeOrAfterContent()) {
        for (auto* renderer = parent(); renderer && !node; renderer = renderer->parent())
            node = renderer->element();
    }

    if (node) {
        result.setInnerNode(node);
        if (!result.innerNonSharedNode())
            result.setInnerNonSharedNode(node);
        result.setLocalPoint(point);
    }
    */
}

bool RenderObject::nodeAtPoint(const HitTestRequest&, HitTestResult&, const HitTestLocation& /*locationInContainer*/, const LayoutPoint& /*accumulatedOffset*/, HitTestAction)
{
    return false;
}

int RenderObject::innerLineHeight() const
{
    return style().computedLineHeight();
}

#if ENABLE(DASHBOARD_SUPPORT)
void RenderObject::addAnnotatedRegions(Vector<AnnotatedRegionValue>& regions)
{
    // Convert the style regions to absolute coordinates.
    if (style().visibility() != VISIBLE || !is<RenderBox>(*this))
        return;
    
    auto& box = downcast<RenderBox>(*this);
    FloatPoint absPos = localToAbsolute();

    const Vector<StyleDashboardRegion>& styleRegions = style().dashboardRegions();
    for (const auto& styleRegion : styleRegions) {
        LayoutUnit w = box.width();
        LayoutUnit h = box.height();

        AnnotatedRegionValue region;
        region.label = styleRegion.label;
        region.bounds = LayoutRect(styleRegion.offset.left().value(),
                                   styleRegion.offset.top().value(),
                                   w - styleRegion.offset.left().value() - styleRegion.offset.right().value(),
                                   h - styleRegion.offset.top().value() - styleRegion.offset.bottom().value());
        region.type = styleRegion.type;

        region.clip = computeAbsoluteRepaintRect(region.bounds);
        if (region.clip.height() < 0) {
            region.clip.setHeight(0);
            region.clip.setWidth(0);
        }

        region.bounds.setX(absPos.x() + styleRegion.offset.left().value());
        region.bounds.setY(absPos.y() + styleRegion.offset.top().value());

        regions.append(region);
    }
}

void RenderObject::collectAnnotatedRegions(Vector<AnnotatedRegionValue>& regions)
{
    // RenderTexts don't have their own style, they just use their parent's style,
    // so we don't want to include them.
    if (is<RenderText>(*this))
        return;

    addAnnotatedRegions(regions);
    for (RenderObject* current = downcast<RenderElement>(*this).firstChild(); current; current = current->nextSibling())
        current->collectAnnotatedRegions(regions);
}
#endif

int RenderObject::caretMinOffset() const
{
    return 0;
}

int RenderObject::caretMaxOffset() const
{
    /**
    if (isReplaced())
        return node() ? std::max(1U, node()->countChildNodes()) : 1;
    if (isHR())
        return 1;*/
    return 0;
}

int RenderObject::previousOffset(int current) const
{
    return current - 1;
}

int RenderObject::previousOffsetForBackwardDeletion(int current) const
{
    return current - 1;
}

int RenderObject::nextOffset(int current) const
{
    return current + 1;
}

void RenderObject::adjustRectForOutlineAndShadow(LayoutRect& rect) const
{
    /**
    LayoutUnit outlineSize = outlineStyleForRepaint().outlineSize();
    if (const ShadowData* boxShadow = style().boxShadow()) {
        boxShadow->adjustRectForShadow(rect, outlineSize);
        return;
    }
    rect.inflate(outlineSize);
     */
}


CursorDirective RenderObject::getCursor(const LayoutPoint&, Cursor&) const
{
    return SetCursorBasedOnStyle;
}

bool RenderObject::canUpdateSelectionOnRootLineBoxes()
{
    if (needsLayout())
        return false;

    RenderBlock* containingBlock = this->containingBlock();
    return containingBlock ? !containingBlock->needsLayout() : true;
}

// We only create "generated" child renderers like one for first-letter if:
// - the firstLetterBlock can have children in the DOM and
// - the block doesn't have any special assumption on its text children.
// This correctly prevents form controls from having such renderers.
bool RenderObject::canHaveGeneratedChildren() const
{
    return canHaveChildren();
}


void RenderObject::setNeedsBoundariesUpdate()
{
    if (auto renderer = parent())
        renderer->setNeedsBoundariesUpdate();
}

FloatRect RenderObject::objectBoundingBox() const
{
    ASSERT_NOT_REACHED();
    return FloatRect();
}

FloatRect RenderObject::strokeBoundingBox() const
{
    ASSERT_NOT_REACHED();
    return FloatRect();
}

// Returns the smallest rectangle enclosing all of the painted content
// respecting clipping, masking, filters, opacity, stroke-width and markers
FloatRect RenderObject::repaintRectInLocalCoordinates() const
{
    ASSERT_NOT_REACHED();
    return FloatRect();
}

AffineTransform RenderObject::localTransform() const
{
    static const AffineTransform identity;
    return identity;
}

const AffineTransform& RenderObject::localToParentTransform() const
{
    static const AffineTransform identity;
    return identity;
}

bool RenderObject::nodeAtFloatPoint(const HitTestRequest&, HitTestResult&, const FloatPoint&, HitTestAction)
{
    ASSERT_NOT_REACHED();
    return false;
}

RenderFragmentedFlow* RenderObject::locateEnclosingFragmentedFlow() const
{
    RenderBlock* containingBlock = this->containingBlock();
    return containingBlock ? containingBlock->enclosingFragmentedFlow() : nullptr;
}

void RenderObject::calculateBorderStyleColor(const EBorderStyle& style, const BoxSide& side, Color& color)
{
    ASSERT(style == INSET || style == OUTSET);
    // This values were derived empirically.
    const RGBA32 baseDarkColor = 0xFF202020;
    const RGBA32 baseLightColor = 0xFFEBEBEB;
    enum Operation { Darken, Lighten };

    Operation operation = (side == BSTop || side == BSLeft) == (style == INSET) ? Darken : Lighten;

    // Here we will darken the border decoration color when needed. This will yield a similar behavior as in FF.
    if (operation == Darken) {
        if (differenceSquared(color, Color::black) > differenceSquared(baseDarkColor, Color::black))
            color = color.dark();
    } else {
        if (differenceSquared(color, Color::white) > differenceSquared(baseLightColor, Color::white))
            color = color.light();
    }
}

void RenderObject::setIsDragging(bool isDragging)
{
    if (isDragging || hasRareData())
        ensureRareData().setIsDragging(isDragging);
}

void RenderObject::setHasReflection(bool hasReflection)
{
    if (hasReflection || hasRareData())
        ensureRareData().setHasReflection(hasReflection);
}

void RenderObject::setIsRenderFragmentedFlow(bool isFragmentedFlow)
{
    if (isFragmentedFlow || hasRareData())
        ensureRareData().setIsRenderFragmentedFlow(isFragmentedFlow);
}

void RenderObject::setHasOutlineAutoAncestor(bool hasOutlineAutoAncestor)
{
    if (hasOutlineAutoAncestor || hasRareData())
        ensureRareData().setHasOutlineAutoAncestor(hasOutlineAutoAncestor);
}

RenderObject::RareDataMap& RenderObject::rareDataMap()
{
    static NeverDestroyed<RareDataMap> map;
    return map;
}

const RenderObject::RenderObjectRareData& RenderObject::rareData() const
{
    ASSERT(hasRareData());
    return *rareDataMap().get(this);
}

RenderObject::RenderObjectRareData& RenderObject::ensureRareData()
{
    setHasRareData(true);
    return *rareDataMap().ensure(this, [] { return std::make_unique<RenderObjectRareData>(); }).iterator->value;
}

void RenderObject::removeRareData()
{
    rareDataMap().remove(this);
    setHasRareData(false);
}

#if ENABLE(TREE_DEBUGGING)

void printRenderTreeForLiveDocuments()
{

}

void printLayerTreeForLiveDocuments()
{

}

#endif // ENABLE(TREE_DEBUGGING)

} // namespace WebCore

#if ENABLE(TREE_DEBUGGING)

void showNodeTree(const WebCore::RenderObject* object)
{
    if (!object)
        return;
    object->showNodeTreeForThis();
}

void showLineTree(const WebCore::RenderObject* object)
{
    if (!object)
        return;
    object->showLineTreeForThis();
}

void showRenderTree(const WebCore::RenderObject* object)
{
    if (!object)
        return;
    object->showRenderTreeForThis();
}

#endif
