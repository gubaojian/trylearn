/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2007 David Smith (catfish.man@gmail.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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
#include "RenderBlock.h"

//#include "AXObjectCache.h"
//#include "Document.h"
//#include "Editor.h"
//#include "Element.h"
#include "FloatQuad.h"
//#include "Frame.h"
//#include "FrameSelection.h"
//#include "FrameView.h"
//#include "GraphicsContext.h"
//#include "HTMLNames.h"
#include "HitTestLocation.h"
#include "HitTestResult.h"
//#include "InlineElementBox.h"
//#include "InlineIterator.h"
//#include "InlineTextBox.h"
//#include "LayoutRepainter.h"
#include "LogicalSelectionOffsetCaches.h"
//#include "OverflowEvent.h"
//#include "Page.h"
//#include "PaintInfo.h"
#include "RenderBlockFlow.h"
//#include "RenderBoxFragmentInfo.h"
//#include "RenderButton.h"
#include "RenderChildIterator.h"
//#include "RenderCombineText.h"
#include "RenderDeprecatedFlexibleBox.h"
#include "RenderFlexibleBox.h"
#include "RenderFragmentContainer.h"
#include "RenderInline.h"
#include "RenderIterator.h"
//#include "RenderLayer.h"
//#include "RenderListMarker.h"
//#include "RenderMenuList.h"
//#include "RenderSVGResourceClipper.h"
#include "RenderTableCell.h"
//#include "RenderTextFragment.h"
//#include "RenderTheme.h"
//#include "RenderTreePosition.h"
//#include "RenderView.h"
//#include "Settings.h"
//#include "ShadowRoot.h"
//#include "ShapeOutsideInfo.h"
//#include "TransformState.h"

#include "RenderTreePosition.h"
#include "LayoutState.h"

#include <wtf/IsoMallocInlines.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/Optional.h>
#include <wtf/SetForScope.h>
#include <wtf/StackStats.h>


namespace WebCore {
using namespace WTF;
//using namespace Unicode;

//using namespace HTMLNames;

WTF_MAKE_ISO_ALLOCATED_IMPL(RenderBlock);

struct SameSizeAsRenderBlock : public RenderBox {
};

COMPILE_ASSERT(sizeof(RenderBlock) == sizeof(SameSizeAsRenderBlock), RenderBlock_should_stay_small);

typedef HashMap<const RenderBlock*, std::unique_ptr<TrackedRendererListHashSet>> TrackedDescendantsMap;
typedef HashMap<const RenderBox*, std::unique_ptr<HashSet<const RenderBlock*>>> TrackedContainerMap;

static TrackedDescendantsMap* percentHeightDescendantsMap;
static TrackedContainerMap* percentHeightContainerMap;

static void insertIntoTrackedRendererMaps(const RenderBlock& container, RenderBox& descendant)
{
    if (!percentHeightDescendantsMap) {
        percentHeightDescendantsMap = new TrackedDescendantsMap;
        percentHeightContainerMap = new TrackedContainerMap;
    }
    
    auto& descendantSet = percentHeightDescendantsMap->ensure(&container, [] {
        return std::make_unique<TrackedRendererListHashSet>();
    }).iterator->value;

    bool added = descendantSet->add(&descendant).isNewEntry;
    if (!added) {
        ASSERT(percentHeightContainerMap->get(&descendant));
        ASSERT(percentHeightContainerMap->get(&descendant)->contains(&container));
        return;
    }
    
    auto& containerSet = percentHeightContainerMap->ensure(&descendant, [] {
        return std::make_unique<HashSet<const RenderBlock*>>();
    }).iterator->value;

    ASSERT(!containerSet->contains(&container));
    containerSet->add(&container);
}

static void removeFromTrackedRendererMaps(RenderBox& descendant)
{
    if (!percentHeightDescendantsMap)
        return;
    
    std::unique_ptr<HashSet<const RenderBlock*>> containerSet = percentHeightContainerMap->take(&descendant);
    if (!containerSet)
        return;
    
    for (auto* container : *containerSet) {
        // FIXME: Disabling this assert temporarily until we fix the layout
        // bugs associated with positioned objects not properly cleared from
        // their ancestor chain before being moved. See webkit bug 93766.
        // ASSERT(descendant->isDescendantOf(container));
        auto descendantsMapIterator = percentHeightDescendantsMap->find(container);
        ASSERT(descendantsMapIterator != percentHeightDescendantsMap->end());
        if (descendantsMapIterator == percentHeightDescendantsMap->end())
            continue;
        auto& descendantSet = descendantsMapIterator->value;
        ASSERT(descendantSet->contains(&descendant));
        descendantSet->remove(&descendant);
        if (descendantSet->isEmpty())
            percentHeightDescendantsMap->remove(descendantsMapIterator);
    }
}

class PositionedDescendantsMap {
public:
    enum class MoveDescendantToEnd { No, Yes };
    void addDescendant(const RenderBlock& containingBlock, RenderBox& positionedDescendant, MoveDescendantToEnd moveDescendantToEnd)
    {
        // Protect against double insert where a descendant would end up with multiple containing blocks.
        auto* previousContainingBlock = m_containerMap.get(&positionedDescendant);
        if (previousContainingBlock && previousContainingBlock != &containingBlock) {
            if (auto* descendants = m_descendantsMap.get(previousContainingBlock))
                descendants->remove(&positionedDescendant);
        }

        auto& descendants = m_descendantsMap.ensure(&containingBlock, [] {
            return std::make_unique<TrackedRendererListHashSet>();
        }).iterator->value;

        bool isNewEntry = moveDescendantToEnd == MoveDescendantToEnd::Yes ? descendants->appendOrMoveToLast(&positionedDescendant).isNewEntry
            : descendants->add(&positionedDescendant).isNewEntry;
        if (!isNewEntry) {
            ASSERT(m_containerMap.contains(&positionedDescendant));
            return;
        }
        m_containerMap.set(&positionedDescendant, &containingBlock);
    }

    void removeDescendant(const RenderBox& positionedDescendant)
    {
        auto* containingBlock = m_containerMap.take(&positionedDescendant);
        if (!containingBlock)
            return;

        auto descendantsIterator = m_descendantsMap.find(containingBlock);
        ASSERT(descendantsIterator != m_descendantsMap.end());
        if (descendantsIterator == m_descendantsMap.end())
            return;

        auto& descendants = descendantsIterator->value;
        ASSERT(descendants->contains(const_cast<RenderBox*>(&positionedDescendant)));

        descendants->remove(const_cast<RenderBox*>(&positionedDescendant));
        if (descendants->isEmpty())
            m_descendantsMap.remove(descendantsIterator);
    }
    
    void removeContainingBlock(const RenderBlock& containingBlock)
    {
        auto descendants = m_descendantsMap.take(&containingBlock);
        if (!descendants)
            return;

        for (auto* renderer : *descendants)
            m_containerMap.remove(renderer);
    }
    
    TrackedRendererListHashSet* positionedRenderers(const RenderBlock& containingBlock) const
    {
        return m_descendantsMap.get(&containingBlock);
    }

private:
    using DescendantsMap = HashMap<const RenderBlock*, std::unique_ptr<TrackedRendererListHashSet>>;
    using ContainerMap = HashMap<const RenderBox*, const RenderBlock*>;
    
    DescendantsMap m_descendantsMap;
    ContainerMap m_containerMap;
};

static PositionedDescendantsMap& positionedDescendantsMap()
{
    static NeverDestroyed<PositionedDescendantsMap> mapForPositionedDescendants;
    return mapForPositionedDescendants;
}

typedef HashMap<RenderBlock*, std::unique_ptr<ListHashSet<RenderInline*>>> ContinuationOutlineTableMap;

struct UpdateScrollInfoAfterLayoutTransaction {
    UpdateScrollInfoAfterLayoutTransaction(const RenderView& view)
        : nestedCount(0)
        , view(&view)
    {
    }

    int nestedCount;
    const RenderView* view;
    HashSet<RenderBlock*> blocks;
};

typedef Vector<UpdateScrollInfoAfterLayoutTransaction> DelayedUpdateScrollInfoStack;
static std::unique_ptr<DelayedUpdateScrollInfoStack>& updateScrollInfoAfterLayoutTransactionStack()
{
    static NeverDestroyed<std::unique_ptr<DelayedUpdateScrollInfoStack>> delayedUpdatedScrollInfoStack;
    return delayedUpdatedScrollInfoStack;
}

// Allocated only when some of these fields have non-default values

struct RenderBlockRareData {
    WTF_MAKE_NONCOPYABLE(RenderBlockRareData); WTF_MAKE_FAST_ALLOCATED;
public:
    RenderBlockRareData()
    {
    }

    LayoutUnit m_paginationStrut;
    LayoutUnit m_pageLogicalOffset;
    LayoutUnit m_intrinsicBorderForFieldset;
    
    std::optional<RenderFragmentedFlow*> m_enclosingFragmentedFlow;
};

typedef HashMap<const RenderBlock*, std::unique_ptr<RenderBlockRareData>> RenderBlockRareDataMap;
static RenderBlockRareDataMap* gRareDataMap;

// This class helps dispatching the 'overflow' event on layout change. overflow can be set on RenderBoxes, yet the existing code
// only works on RenderBlocks. If this change, this class should be shared with other RenderBoxes.
class OverflowEventDispatcher {
    WTF_MAKE_NONCOPYABLE(OverflowEventDispatcher);
public:
    OverflowEventDispatcher(const RenderBlock* block)
        : m_block(block)
        , m_hadHorizontalLayoutOverflow(false)
        , m_hadVerticalLayoutOverflow(false)
    {
        m_shouldDispatchEvent = false;
        if (m_shouldDispatchEvent) {
            m_hadHorizontalLayoutOverflow = m_block->hasHorizontalLayoutOverflow();
            m_hadVerticalLayoutOverflow = m_block->hasVerticalLayoutOverflow();
        }
    }

    ~OverflowEventDispatcher()
    {
        if (!m_shouldDispatchEvent)
            return;

        bool hasHorizontalLayoutOverflow = m_block->hasHorizontalLayoutOverflow();
        bool hasVerticalLayoutOverflow = m_block->hasVerticalLayoutOverflow();

        bool horizontalLayoutOverflowChanged = hasHorizontalLayoutOverflow != m_hadHorizontalLayoutOverflow;
        bool verticalLayoutOverflowChanged = hasVerticalLayoutOverflow != m_hadVerticalLayoutOverflow;
        if (!horizontalLayoutOverflowChanged && !verticalLayoutOverflowChanged)
            return;

        //Ref<OverflowEvent> overflowEvent = OverflowEvent::create(horizontalLayoutOverflowChanged, hasHorizontalLayoutOverflow, verticalLayoutOverflowChanged, hasVerticalLayoutOverflow);
        //overflowEvent->setTarget(m_block->element());
        //m_block->document().enqueueOverflowEvent(WTFMove(overflowEvent));
    }

private:
    const RenderBlock* m_block;
    bool m_shouldDispatchEvent;
    bool m_hadHorizontalLayoutOverflow;
    bool m_hadVerticalLayoutOverflow;
};

RenderBlock::RenderBlock(RenderStyle&& style, BaseTypeFlags baseTypeFlags)
    : RenderBox(WTFMove(style), baseTypeFlags | RenderBlockFlag)
{
}


static void removeBlockFromPercentageDescendantAndContainerMaps(RenderBlock* block)
{
    if (!percentHeightDescendantsMap)
        return;
    std::unique_ptr<TrackedRendererListHashSet> descendantSet = percentHeightDescendantsMap->take(block);
    if (!descendantSet)
        return;
    
    for (auto* descendant : *descendantSet) {
        auto it = percentHeightContainerMap->find(descendant);
        ASSERT(it != percentHeightContainerMap->end());
        if (it == percentHeightContainerMap->end())
            continue;
        auto* containerSet = it->value.get();
        ASSERT(containerSet->contains(block));
        containerSet->remove(block);
        if (containerSet->isEmpty())
            percentHeightContainerMap->remove(it);
    }
}

RenderBlock::~RenderBlock()
{
    // Blocks can be added to gRareDataMap during willBeDestroyed(), so this code can't move there.
    if (gRareDataMap)
        gRareDataMap->remove(this);

    // Do not add any more code here. Add it to willBeDestroyed() instead.
}

// Note that this is not called for RenderBlockFlows.
void RenderBlock::willBeDestroyed()
{
    if (!renderTreeBeingDestroyed()) {
        if (parent())
            parent()->dirtyLinesFromChangedChild(*this);
    }

    blockWillBeDestroyed();

    RenderBox::willBeDestroyed();
}

void RenderBlock::blockWillBeDestroyed()
{
    removeFromUpdateScrollInfoAfterLayoutTransaction();

    removeBlockFromPercentageDescendantAndContainerMaps(this);
    positionedDescendantsMap().removeContainingBlock(*this);
}

bool RenderBlock::hasRareData() const
{
    return gRareDataMap ? gRareDataMap->contains(this) : false;
}



static bool borderOrPaddingLogicalWidthChanged(const RenderStyle& oldStyle, const RenderStyle& newStyle)
{
    if (newStyle.isHorizontalWritingMode()) {
        return oldStyle.borderLeftWidth() != newStyle.borderLeftWidth()
            || oldStyle.borderRightWidth() != newStyle.borderRightWidth()
            || oldStyle.paddingLeft() != newStyle.paddingLeft()
            || oldStyle.paddingRight() != newStyle.paddingRight();
    }

    return oldStyle.borderTopWidth() != newStyle.borderTopWidth()
        || oldStyle.borderBottomWidth() != newStyle.borderBottomWidth()
        || oldStyle.paddingTop() != newStyle.paddingTop()
        || oldStyle.paddingBottom() != newStyle.paddingBottom();
}

void RenderBlock::styleDidChange(StyleDifference diff, const RenderStyle* oldStyle)
{
    bool hadTransform = hasTransform();
    RenderBox::styleDidChange(diff, oldStyle);

    if (hadTransform != hasTransform())
        adjustFragmentedFlowStateOnContainingBlockChangeIfNeeded();

    propagateStyleToAnonymousChildren(PropagateToBlockChildrenOnly);

    // It's possible for our border/padding to change, but for the overall logical width of the block to
    // end up being the same. We keep track of this change so in layoutBlock, we can know to set relayoutChildren=true.
    setShouldForceRelayoutChildren(oldStyle && diff == StyleDifferenceLayout && needsLayout() && borderOrPaddingLogicalWidthChanged(*oldStyle, style()));
}

RenderBlock* RenderBlock::continuationBefore(RenderObject* beforeChild)
{
    if (beforeChild && beforeChild->parent() == this)
        return this;

    RenderBlock* nextToLast = this;
    RenderBlock* last = this;
    for (auto* current = downcast<RenderBlock>(continuation()); current; current = downcast<RenderBlock>(current->continuation())) {
        if (beforeChild && beforeChild->parent() == current) {
            if (current->firstChild() == beforeChild)
                return last;
            return current;
        }

        nextToLast = last;
        last = current;
    }

    if (!beforeChild && !last->firstChild())
        return nextToLast;
    return last;
}

void RenderBlock::addChildToContinuation(RenderPtr<RenderObject> newChild, RenderObject* beforeChild)
{
    RenderBlock* flow = continuationBefore(beforeChild);
    ASSERT(!beforeChild || is<RenderBlock>(*beforeChild->parent()));
    RenderBoxModelObject* beforeChildParent = nullptr;
    if (beforeChild)
        beforeChildParent = downcast<RenderBoxModelObject>(beforeChild->parent());
    else {
        RenderBoxModelObject* continuation = flow->continuation();
        if (continuation)
            beforeChildParent = continuation;
        else
            beforeChildParent = flow;
    }

    if (newChild->isFloatingOrOutOfFlowPositioned()) {
        beforeChildParent->addChildIgnoringContinuation(WTFMove(newChild), beforeChild);
        return;
    }

    bool childIsNormal = newChild->isInline() || !newChild->style().columnSpan();
    bool bcpIsNormal = beforeChildParent->isInline() || !beforeChildParent->style().columnSpan();
    bool flowIsNormal = flow->isInline() || !flow->style().columnSpan();

    if (flow == beforeChildParent) {
        flow->addChildIgnoringContinuation(WTFMove(newChild), beforeChild);
        return;
    }
    
    // The goal here is to match up if we can, so that we can coalesce and create the
    // minimal # of continuations needed for the inline.
    if (childIsNormal == bcpIsNormal) {
        beforeChildParent->addChildIgnoringContinuation(WTFMove(newChild), beforeChild);
        return;
    }
    if (flowIsNormal == childIsNormal) {
        flow->addChildIgnoringContinuation(WTFMove(newChild), 0); // Just treat like an append.
        return;
    }
    beforeChildParent->addChildIgnoringContinuation(WTFMove(newChild), beforeChild);
}



void RenderBlock::addChild(RenderPtr<RenderObject> newChild, RenderObject* beforeChild)
{
    if (continuation() && !isAnonymousBlock())
        addChildToContinuation(WTFMove(newChild), beforeChild);
    else
        addChildIgnoringContinuation(WTFMove(newChild), beforeChild);
}

void RenderBlock::addChildIgnoringContinuation(RenderPtr<RenderObject> newChild, RenderObject* beforeChild)
{
    if (beforeChild && beforeChild->parent() != this) {
        RenderElement* beforeChildContainer = beforeChild->parent();
        while (beforeChildContainer->parent() != this)
            beforeChildContainer = beforeChildContainer->parent();
        ASSERT(beforeChildContainer);

        if (beforeChildContainer->isAnonymous()) {
            RELEASE_ASSERT_WITH_SECURITY_IMPLICATION(!beforeChildContainer->isInline());

            // If the requested beforeChild is not one of our children, then this is because
            // there is an anonymous container within this object that contains the beforeChild.
            RenderElement* beforeChildAnonymousContainer = beforeChildContainer;
            if (beforeChildAnonymousContainer->isAnonymousBlock()
#if ENABLE(FULLSCREEN_API)
                // Full screen renderers and full screen placeholders act as anonymous blocks, not tables:
                || beforeChildAnonymousContainer->isRenderFullScreen()
                || beforeChildAnonymousContainer->isRenderFullScreenPlaceholder()
#endif
                ) {
                // Insert the child into the anonymous block box instead of here.
                if (newChild->isInline() || beforeChild->parent()->firstChild() != beforeChild)
                    beforeChild->parent()->addChild(WTFMove(newChild), beforeChild);
                else
                    addChild(WTFMove(newChild), beforeChild->parent());
                return;
            }

            ASSERT(beforeChildAnonymousContainer->isTable());

            if (newChild->isTablePart()) {
                // Insert into the anonymous table.
                beforeChildAnonymousContainer->addChild(WTFMove(newChild), beforeChild);
                return;
            }

            beforeChild = splitAnonymousBoxesAroundChild(beforeChild);

            RELEASE_ASSERT_WITH_SECURITY_IMPLICATION(beforeChild->parent() == this);
        }
    }

    bool madeBoxesNonInline = false;

    // A block has to either have all of its children inline, or all of its children as blocks.
    // So, if our children are currently inline and a block child has to be inserted, we move all our
    // inline children into anonymous block boxes.
    if (childrenInline() && !newChild->isInline() && !newChild->isFloatingOrOutOfFlowPositioned()) {
        // This is a block with inline content. Wrap the inline content in anonymous blocks.
        makeChildrenNonInline(beforeChild);
        madeBoxesNonInline = true;

        if (beforeChild && beforeChild->parent() != this) {
            beforeChild = beforeChild->parent();
            ASSERT(beforeChild->isAnonymousBlock());
            ASSERT(beforeChild->parent() == this);
        }
    } else if (!childrenInline() && (newChild->isFloatingOrOutOfFlowPositioned() || newChild->isInline())) {
        // If we're inserting an inline child but all of our children are blocks, then we have to make sure
        // it is put into an anomyous block box. We try to use an existing anonymous box if possible, otherwise
        // a new one is created and inserted into our list of children in the appropriate position.
        RenderObject* afterChild = beforeChild ? beforeChild->previousSibling() : lastChild();

        if (afterChild && afterChild->isAnonymousBlock()) {
            downcast<RenderBlock>(*afterChild).addChild(WTFMove(newChild));
            return;
        }

        if (newChild->isInline()) {
            // No suitable existing anonymous box - create a new one.
            auto newBox = createAnonymousBlock();
            auto& box = *newBox;
            RenderBox::addChild(WTFMove(newBox), beforeChild);
            box.addChild(WTFMove(newChild));
            return;
        }
    }

    invalidateLineLayoutPath();

    RenderBox::addChild(WTFMove(newChild), beforeChild);
 
    if (madeBoxesNonInline && is<RenderBlock>(parent()) && isAnonymousBlock())
        downcast<RenderBlock>(*parent()).removeLeftoverAnonymousBlock(this);
    // this object may be dead here
}

static void getInlineRun(RenderObject* start, RenderObject* boundary,
                         RenderObject*& inlineRunStart,
                         RenderObject*& inlineRunEnd)
{
    // Beginning at |start| we find the largest contiguous run of inlines that
    // we can.  We denote the run with start and end points, |inlineRunStart|
    // and |inlineRunEnd|.  Note that these two values may be the same if
    // we encounter only one inline.
    //
    // We skip any non-inlines we encounter as long as we haven't found any
    // inlines yet.
    //
    // |boundary| indicates a non-inclusive boundary point.  Regardless of whether |boundary|
    // is inline or not, we will not include it in a run with inlines before it.  It's as though we encountered
    // a non-inline.
    
    // Start by skipping as many non-inlines as we can.
    RenderObject * curr = start;
    bool sawInline;
    do {
        while (curr && !(curr->isInline() || curr->isFloatingOrOutOfFlowPositioned()))
            curr = curr->nextSibling();
        
        inlineRunStart = inlineRunEnd = curr;
        
        if (!curr)
            return; // No more inline children to be found.
        
        sawInline = curr->isInline();
        
        curr = curr->nextSibling();
        while (curr && (curr->isInline() || curr->isFloatingOrOutOfFlowPositioned()) && (curr != boundary)) {
            inlineRunEnd = curr;
            if (curr->isInline())
                sawInline = true;
            curr = curr->nextSibling();
        }
    } while (!sawInline);
}

void RenderBlock::deleteLines()
{
}

void RenderBlock::makeChildrenNonInline(RenderObject* insertionPoint)
{    
    // makeChildrenNonInline takes a block whose children are *all* inline and it
    // makes sure that inline children are coalesced under anonymous
    // blocks.  If |insertionPoint| is defined, then it represents the insertion point for
    // the new block child that is causing us to have to wrap all the inlines.  This
    // means that we cannot coalesce inlines before |insertionPoint| with inlines following
    // |insertionPoint|, because the new child is going to be inserted in between the inlines,
    // splitting them.
    ASSERT(isInlineBlockOrInlineTable() || !isInline());
    ASSERT(!insertionPoint || insertionPoint->parent() == this);

    setChildrenInline(false);

    RenderObject* child = firstChild();
    if (!child)
        return;

    deleteLines();

    while (child) {
        RenderObject* inlineRunStart;
        RenderObject* inlineRunEnd;
        getInlineRun(child, insertionPoint, inlineRunStart, inlineRunEnd);

        if (!inlineRunStart)
            break;

        child = inlineRunEnd->nextSibling();

        auto newBlock = createAnonymousBlock();
        auto& block = *newBlock;
        insertChildInternal(WTFMove(newBlock), inlineRunStart);
        moveChildrenTo(&block, inlineRunStart, child, RenderBoxModelObject::NormalizeAfterInsertion::No);
    }

#ifndef NDEBUG
    for (RenderObject* c = firstChild(); c; c = c->nextSibling())
        ASSERT(!c->isInline());
#endif

    repaint();
}

void RenderBlock::removeLeftoverAnonymousBlock(RenderBlock* child)
{
    ASSERT(child->isAnonymousBlock());
    ASSERT(!child->childrenInline());
    
    if (child->continuation())
        return;
    
    RenderObject* firstAnChild = child->firstChild();
    RenderObject* lastAnChild = child->lastChild();
    if (firstAnChild) {
        RenderObject* o = firstAnChild;
        while (o) {
            o->setParent(this);
            o = o->nextSibling();
        }
        firstAnChild->setPreviousSibling(child->previousSibling());
        lastAnChild->setNextSibling(child->nextSibling());
        if (child->previousSibling())
            child->previousSibling()->setNextSibling(firstAnChild);
        if (child->nextSibling())
            child->nextSibling()->setPreviousSibling(lastAnChild);
            
        if (child == firstChild())
            setFirstChild(firstAnChild);
        if (child == lastChild())
            setLastChild(lastAnChild);
    } else {
        if (child == firstChild())
            setFirstChild(child->nextSibling());
        if (child == lastChild())
            setLastChild(child->previousSibling());

        if (child->previousSibling())
            child->previousSibling()->setNextSibling(child->nextSibling());
        if (child->nextSibling())
            child->nextSibling()->setPreviousSibling(child->previousSibling());
    }

    child->setFirstChild(nullptr);
    child->m_next = nullptr;

    // Remove all the information in the flow thread associated with the leftover anonymous block.
    child->resetFragmentedFlowStateOnRemoval();

    child->setParent(nullptr);
    child->setPreviousSibling(nullptr);
    child->setNextSibling(nullptr);

    child->destroy();
}

static bool canDropAnonymousBlock(const RenderBlock& anonymousBlock)
{
    if (anonymousBlock.beingDestroyed() || anonymousBlock.continuation())
        return false;
    if (anonymousBlock.isRubyRun() || anonymousBlock.isRubyBase())
        return false;
    return true;
}

static bool canMergeContiguousAnonymousBlocks(RenderObject& oldChild, RenderObject* previous, RenderObject* next)
{
    ASSERT(!oldChild.renderTreeBeingDestroyed());

    if (oldChild.isInline())
        return false;

    if (is<RenderBoxModelObject>(oldChild) && downcast<RenderBoxModelObject>(oldChild).continuation())
        return false;

    if (previous) {
        if (!previous->isAnonymousBlock())
            return false;
        RenderBlock& previousAnonymousBlock = downcast<RenderBlock>(*previous);
        if (!canDropAnonymousBlock(previousAnonymousBlock))
            return false;
    }
    if (next) {
        if (!next->isAnonymousBlock())
            return false;
        RenderBlock& nextAnonymousBlock = downcast<RenderBlock>(*next);
        if (!canDropAnonymousBlock(nextAnonymousBlock))
            return false;
    }
    return true;
}

void RenderBlock::dropAnonymousBoxChild(RenderBlock& child)
{
    setNeedsLayoutAndPrefWidthsRecalc();
    setChildrenInline(child.childrenInline());
    RenderObject* nextSibling = child.nextSibling();

    auto toBeDeleted = takeChildInternal(child);
    child.moveAllChildrenTo(this, nextSibling, RenderBoxModelObject::NormalizeAfterInsertion::No);
    // Delete the now-empty block's lines and nuke it.
    child.deleteLines();
}

RenderPtr<RenderObject> RenderBlock::takeChild(RenderObject& oldChild)
{
    // No need to waste time in merging or removing empty anonymous blocks.
    // We can just bail out if our document is getting destroyed.
    if (renderTreeBeingDestroyed())
        return RenderBox::takeChild(oldChild);

    // If this child is a block, and if our previous and next siblings are both anonymous blocks
    // with inline content, then we can fold the inline content back together.
    RenderObject* prev = oldChild.previousSibling();
    RenderObject* next = oldChild.nextSibling();
    bool canMergeAnonymousBlocks = canMergeContiguousAnonymousBlocks(oldChild, prev, next);
    if (canMergeAnonymousBlocks && prev && next) {
        prev->setNeedsLayoutAndPrefWidthsRecalc();
        RenderBlock& nextBlock = downcast<RenderBlock>(*next);
        RenderBlock& prevBlock = downcast<RenderBlock>(*prev);
       
        if (prev->childrenInline() != next->childrenInline()) {
            RenderBlock& inlineChildrenBlock = prev->childrenInline() ? prevBlock : nextBlock;
            RenderBlock& blockChildrenBlock = prev->childrenInline() ? nextBlock : prevBlock;
            
            // Place the inline children block inside of the block children block instead of deleting it.
            // In order to reuse it, we have to reset it to just be a generic anonymous block.  Make sure
            // to clear out inherited column properties by just making a new style, and to also clear the
            // column span flag if it is set.
            ASSERT(!inlineChildrenBlock.continuation());
            // Cache this value as it might get changed in setStyle() call.
            inlineChildrenBlock.setStyle(RenderStyle::createAnonymousStyleWithDisplay(style(), BLOCK));
            auto blockToMove = takeChildInternal(inlineChildrenBlock);
            
            // Now just put the inlineChildrenBlock inside the blockChildrenBlock.
            RenderObject* beforeChild = prev == &inlineChildrenBlock ? blockChildrenBlock.firstChild() : nullptr;
            blockChildrenBlock.insertChildInternal(WTFMove(blockToMove), beforeChild);
            next->setNeedsLayoutAndPrefWidthsRecalc();
            
            // inlineChildrenBlock got reparented to blockChildrenBlock, so it is no longer a child
            // of "this". we null out prev or next so that is not used later in the function.
            if (&inlineChildrenBlock == &prevBlock)
                prev = nullptr;
            else
                next = nullptr;
        } else {
            // Take all the children out of the |next| block and put them in
            // the |prev| block.
            nextBlock.moveAllChildrenIncludingFloatsTo(prevBlock, RenderBoxModelObject::NormalizeAfterInsertion::No);
            
            // Delete the now-empty block's lines and nuke it.
            nextBlock.deleteLines();
            nextBlock.removeFromParentAndDestroy();
            next = nullptr;
        }
    }

    invalidateLineLayoutPath();

    auto takenChild = RenderBox::takeChild(oldChild);

    RenderObject* child = prev ? prev : next;
    if (canMergeAnonymousBlocks && child && !child->previousSibling() && !child->nextSibling() && canDropAnonymousBlockChild()) {
        // The removal has knocked us down to containing only a single anonymous
        // box. We can pull the content right back up into our box.
        dropAnonymousBoxChild(downcast<RenderBlock>(*child));
    } else if (((prev && prev->isAnonymousBlock()) || (next && next->isAnonymousBlock())) && canDropAnonymousBlockChild()) {
        // It's possible that the removal has knocked us down to a single anonymous
        // block with floating siblings.
        RenderBlock& anonBlock = downcast<RenderBlock>((prev && prev->isAnonymousBlock()) ? *prev : *next);
        if (canDropAnonymousBlock(anonBlock)) {
            bool dropAnonymousBlock = true;
            for (auto& sibling : childrenOfType<RenderObject>(*this)) {
                if (&sibling == &anonBlock)
                    continue;
                if (!sibling.isFloating()) {
                    dropAnonymousBlock = false;
                    break;
                }
            }
            if (dropAnonymousBlock)
                dropAnonymousBoxChild(anonBlock);
        }
    }

    if (!firstChild()) {
        // If this was our last child be sure to clear out our line boxes.
        if (childrenInline())
            deleteLines();
    }
    return takenChild;
}

bool RenderBlock::childrenPreventSelfCollapsing() const
{
    // Whether or not we collapse is dependent on whether all our normal flow children
    // are also self-collapsing.
    for (RenderBox* child = firstChildBox(); child; child = child->nextSiblingBox()) {
        if (child->isFloatingOrOutOfFlowPositioned())
            continue;
        if (!child->isSelfCollapsingBlock())
            return true;
    }
    return false;
}



static inline UpdateScrollInfoAfterLayoutTransaction* currentUpdateScrollInfoAfterLayoutTransaction()
{
    if (!updateScrollInfoAfterLayoutTransactionStack())
        return nullptr;
    return &updateScrollInfoAfterLayoutTransactionStack()->last();
}





void RenderBlock::layout()
{
    StackStats::LayoutCheckPoint layoutCheckPoint;

    // Table cells call layoutBlock directly, so don't add any logic here.  Put code into
    // layoutBlock().
    layoutBlock(false);
    
    // It's safe to check for control clip here, since controls can never be table cells.
    // If we have a lightweight clip, there can never be any overflow from children.
    if (hasControlClip() && m_overflow)
        clearLayoutOverflow();

    invalidateBackgroundObscurationStatus();
}

static RenderBlockRareData* getBlockRareData(const RenderBlock& block)
{
    return gRareDataMap ? gRareDataMap->get(&block) : nullptr;
}

static RenderBlockRareData& ensureBlockRareData(const RenderBlock& block)
{
    if (!gRareDataMap)
        gRareDataMap = new RenderBlockRareDataMap;
    
    auto& rareData = gRareDataMap->add(&block, nullptr).iterator->value;
    if (!rareData)
        rareData = std::make_unique<RenderBlockRareData>();
    return *rareData.get();
}

void RenderBlock::preparePaginationBeforeBlockLayout(bool& relayoutChildren)
{
    // Fragments changing widths can force us to relayout our children.
    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();
    if (fragmentedFlow)
        fragmentedFlow->logicalWidthChangedInFragmentsForBlock(this, relayoutChildren);
}

bool RenderBlock::recomputeLogicalWidth()
{
    LayoutUnit oldWidth = logicalWidth();
    
    updateLogicalWidth();
    
    bool hasBorderOrPaddingLogicalWidthChanged = this->hasBorderOrPaddingLogicalWidthChanged();
    setShouldForceRelayoutChildren(false);

    return oldWidth != logicalWidth() || hasBorderOrPaddingLogicalWidthChanged;
}

void RenderBlock::layoutBlock(bool, LayoutUnit)
{
    ASSERT_NOT_REACHED();
    clearNeedsLayout();
}

void RenderBlock::addOverflowFromChildren()
{
    if (childrenInline())
        addOverflowFromInlineChildren();
    else
        addOverflowFromBlockChildren();
    
    // If this block is flowed inside a flow thread, make sure its overflow is propagated to the containing fragments.
    if (m_overflow) {
        if (RenderFragmentedFlow* containingFragmentedFlow = enclosingFragmentedFlow())
            containingFragmentedFlow->addFragmentsVisualOverflow(this, m_overflow->visualOverflowRect());
    }
}

// Overflow is always relative to the border-box of the element in question.
// Therefore, if the element has a vertical scrollbar placed on the left, an overflow rect at x=2px would conceptually intersect the scrollbar.
void RenderBlock::computeOverflow(LayoutUnit oldClientAfterEdge, bool)
{
    clearOverflow();
    addOverflowFromChildren();

    addOverflowFromPositionedObjects();

    if (hasOverflowClip()) {
        // When we have overflow clip, propagate the original spillout since it will include collapsed bottom margins
        // and bottom padding.  Set the axis we don't care about to be 1, since we want this overflow to always
        // be considered reachable.
        LayoutRect clientRect(flippedClientBoxRect());
        LayoutRect rectToApply;
        if (isHorizontalWritingMode())
            rectToApply = LayoutRect(clientRect.x(), clientRect.y(), 1, std::max<LayoutUnit>(0, oldClientAfterEdge - clientRect.y()));
        else
            rectToApply = LayoutRect(clientRect.x(), clientRect.y(), std::max<LayoutUnit>(0, oldClientAfterEdge - clientRect.x()), 1);
        addLayoutOverflow(rectToApply);
        if (hasRenderOverflow())
            m_overflow->setLayoutClientAfterEdge(oldClientAfterEdge);
    }
        
    // Add visual overflow from box-shadow, border-image-outset and outline.
    addVisualEffectOverflow();

    // Add visual overflow from theme.
    addVisualOverflowFromTheme();
}

void RenderBlock::clearLayoutOverflow()
{
    if (!m_overflow)
        return;
    
    if (visualOverflowRect() == borderBoxRect()) {
        // FIXME: Implement complete solution for fragments overflow.
        clearOverflow();
        return;
    }
    
    m_overflow->setLayoutOverflow(borderBoxRect());
}

void RenderBlock::addOverflowFromBlockChildren()
{
    for (auto* child = firstChildBox(); child; child = child->nextSiblingBox()) {
        if (!child->isFloatingOrOutOfFlowPositioned())
            addOverflowFromChild(child);
    }
}

void RenderBlock::addOverflowFromPositionedObjects()
{
    TrackedRendererListHashSet* positionedDescendants = positionedObjects();
    if (!positionedDescendants)
        return;

    for (auto it = positionedDescendants->begin(), end = positionedDescendants->end(); it != end; ++it) {
        RenderBox* positionedObject = *it;
        
        // Fixed positioned elements don't contribute to layout overflow, since they don't scroll with the content.
        if (positionedObject->style().position() != FixedPosition)
            addOverflowFromChild(positionedObject, { positionedObject->x(), positionedObject->y() });
    }
}



LayoutUnit RenderBlock::computeStartPositionDeltaForChildAvoidingFloats(const RenderBox& child, LayoutUnit childMarginStart, RenderFragmentContainer* fragment)
{
    LayoutUnit startPosition = startOffsetForContent(fragment);

    // Add in our start margin.
    LayoutUnit oldPosition = startPosition + childMarginStart;
    LayoutUnit newPosition = oldPosition;

    LayoutUnit blockOffset = logicalTopForChild(child);
    if (fragment)
        blockOffset = std::max(blockOffset, blockOffset + (fragment->logicalTopForFragmentedFlowContent() - offsetFromLogicalTopOfFirstPage()));

    LayoutUnit startOff = startOffsetForLineInFragment(blockOffset, DoNotIndentText, fragment, logicalHeightForChild(child));

    if (style().textAlign() != WEBKIT_CENTER && !child.style().marginStartUsing(&style()).isAuto()) {
        if (childMarginStart < 0)
            startOff += childMarginStart;
        newPosition = std::max(newPosition, startOff); // Let the float sit in the child's margin if it can fit.
    } else if (startOff != startPosition)
        newPosition = startOff + childMarginStart;

    return newPosition - oldPosition;
}




void RenderBlock::updateBlockChildDirtyBitsBeforeLayout(bool relayoutChildren, RenderBox& child)
{
    if (child.isOutOfFlowPositioned())
        return;

    // FIXME: Technically percentage height objects only need a relayout if their percentage isn't going to be turned into
    // an auto value. Add a method to determine this, so that we can avoid the relayout.
    if (relayoutChildren || (child.hasRelativeLogicalHeight() && !isRenderView()))
        child.setChildNeedsLayout(MarkOnlyThis);

    // If relayoutChildren is set and the child has percentage padding or an embedded content box, we also need to invalidate the childs pref widths.
    if (relayoutChildren && child.needsPreferredWidthsRecalculation())
        child.setPreferredLogicalWidthsDirty(true, MarkOnlyThis);
}

void RenderBlock::dirtyForLayoutFromPercentageHeightDescendants()
{
    if (!percentHeightDescendantsMap)
        return;

    TrackedRendererListHashSet* descendants = percentHeightDescendantsMap->get(this);
    if (!descendants)
        return;

    for (auto it = descendants->begin(), end = descendants->end(); it != end; ++it) {
        RenderBox* box = *it;
        while (box != this) {
            if (box->normalChildNeedsLayout())
                break;
            box->setChildNeedsLayout(MarkOnlyThis);
            
            // If the width of an image is affected by the height of a child (e.g., an image with an aspect ratio),
            // then we have to dirty preferred widths, since even enclosing blocks can become dirty as a result.
            // (A horizontal flexbox that contains an inline image wrapped in an anonymous block for example.)
            if (box->hasAspectRatio()) 
                box->setPreferredLogicalWidthsDirty(true);
            
            box = box->containingBlock();
            ASSERT(box);
            if (!box)
                break;
        }
    }
}


bool RenderBlock::canPerformSimplifiedLayout() const
{
    return (posChildNeedsLayout() || needsSimplifiedNormalFlowLayout()) && !normalChildNeedsLayout() && !selfNeedsLayout();
}

bool RenderBlock::simplifiedLayout()
{
    if (!canPerformSimplifiedLayout())
        return false;

    LayoutStateMaintainer statePusher(*this, locationOffset(), hasTransform() || hasReflection() || style().isFlippedBlocksWritingMode());
    if (needsPositionedMovementLayout() && !tryLayoutDoingPositionedMovementOnly())
        return false;

    // Lay out positioned descendants or objects that just need to recompute overflow.
    if (needsSimplifiedNormalFlowLayout())
        simplifiedNormalFlowLayout();

    // Make sure a forced break is applied after the content if we are a flow thread in a simplified layout.
    // This ensures the size information is correctly computed for the last auto-height fragment receiving content.
    if (is<RenderFragmentedFlow>(*this))
        downcast<RenderFragmentedFlow>(*this).applyBreakAfterContent(clientLogicalBottom());

    // Lay out our positioned objects if our positioned child bit is set.
    // Also, if an absolute position element inside a relative positioned container moves, and the absolute element has a fixed position
    // child, neither the fixed element nor its container learn of the movement since posChildNeedsLayout() is only marked as far as the 
    // relative positioned container. So if we can have fixed pos objects in our positioned objects list check if any of them
    // are statically positioned and thus need to move with their absolute ancestors.
    bool canContainFixedPosObjects = canContainFixedPositionObjects();
    if (posChildNeedsLayout() || canContainFixedPosObjects)
        layoutPositionedObjects(false, !posChildNeedsLayout() && canContainFixedPosObjects);

    // Recompute our overflow information.
    // FIXME: We could do better here by computing a temporary overflow object from layoutPositionedObjects and only
    // updating our overflow if we either used to have overflow or if the new temporary object has overflow.
    // For now just always recompute overflow.  This is no worse performance-wise than the old code that called rightmostPosition and
    // lowestPosition on every relayout so it's not a regression.
    // computeOverflow expects the bottom edge before we clamp our height. Since this information isn't available during
    // simplifiedLayout, we cache the value in m_overflow.
    LayoutUnit oldClientAfterEdge = hasRenderOverflow() ? m_overflow->layoutClientAfterEdge() : clientLogicalBottom();
    computeOverflow(oldClientAfterEdge, true);

    updateLayerTransform();

    updateScrollInfoAfterLayout();

    clearNeedsLayout();
    return true;
}

void RenderBlock::markFixedPositionObjectForLayoutIfNeeded(RenderBox& positionedChild)
{
    if (positionedChild.style().position() != FixedPosition)
        return;

    bool hasStaticBlockPosition = positionedChild.style().hasStaticBlockPosition(isHorizontalWritingMode());
    bool hasStaticInlinePosition = positionedChild.style().hasStaticInlinePosition(isHorizontalWritingMode());
    if (!hasStaticBlockPosition && !hasStaticInlinePosition)
        return;

    auto* parent = positionedChild.parent();
    while (parent && parent->style().position() != AbsolutePosition)
        parent = parent->parent();
    if (!parent || parent->style().position() != AbsolutePosition)
        return;

    if (hasStaticInlinePosition) {
        LogicalExtentComputedValues computedValues;
        positionedChild.computeLogicalWidthInFragment(computedValues);
        LayoutUnit newLeft = computedValues.m_position;
        if (newLeft != positionedChild.logicalLeft())
            positionedChild.setChildNeedsLayout(MarkOnlyThis);
    } else if (hasStaticBlockPosition) {
        LayoutUnit oldTop = positionedChild.logicalTop();
        positionedChild.updateLogicalHeight();
        if (positionedChild.logicalTop() != oldTop)
            positionedChild.setChildNeedsLayout(MarkOnlyThis);
    }
}

LayoutUnit RenderBlock::marginIntrinsicLogicalWidthForChild(RenderBox& child) const
{
    // A margin has three types: fixed, percentage, and auto (variable).
    // Auto and percentage margins become 0 when computing min/max width.
    // Fixed margins can be added in as is.
    Length marginLeft = child.style().marginStartUsing(&style());
    Length marginRight = child.style().marginEndUsing(&style());
    LayoutUnit margin = 0;
    if (marginLeft.isFixed())
        margin += marginLeft.value();
    if (marginRight.isFixed())
        margin += marginRight.value();
    return margin;
}

void RenderBlock::layoutPositionedObject(RenderBox& r, bool relayoutChildren, bool fixedPositionObjectsOnly)
{
    estimateFragmentRangeForBoxChild(r);

    // A fixed position element with an absolute positioned ancestor has no way of knowing if the latter has changed position. So
    // if this is a fixed position element, mark it for layout if it has an abspos ancestor and needs to move with that ancestor, i.e. 
    // it has static position.
    markFixedPositionObjectForLayoutIfNeeded(r);
    if (fixedPositionObjectsOnly) {
        r.layoutIfNeeded();
        return;
    }

    // When a non-positioned block element moves, it may have positioned children that are implicitly positioned relative to the
    // non-positioned block.  Rather than trying to detect all of these movement cases, we just always lay out positioned
    // objects that are positioned implicitly like this.  Such objects are rare, and so in typical DHTML menu usage (where everything is
    // positioned explicitly) this should not incur a performance penalty.
    if (relayoutChildren || (r.style().hasStaticBlockPosition(isHorizontalWritingMode()) && r.parent() != this))
        r.setChildNeedsLayout(MarkOnlyThis);
        
    // If relayoutChildren is set and the child has percentage padding or an embedded content box, we also need to invalidate the childs pref widths.
    if (relayoutChildren && r.needsPreferredWidthsRecalculation())
        r.setPreferredLogicalWidthsDirty(true, MarkOnlyThis);
    
    r.markForPaginationRelayoutIfNeeded();
    
    // We don't have to do a full layout.  We just have to update our position. Try that first. If we have shrink-to-fit width
    // and we hit the available width constraint, the layoutIfNeeded() will catch it and do a full layout.
    if (r.needsPositionedMovementLayoutOnly() && r.tryLayoutDoingPositionedMovementOnly())
        r.clearNeedsLayout();
        
    // If we are paginated or in a line grid, compute a vertical position for our object now.
    // If it's wrong we'll lay out again.
    LayoutUnit oldLogicalTop = 0;
    bool needsBlockDirectionLocationSetBeforeLayout = r.needsLayout();
    if (needsBlockDirectionLocationSetBeforeLayout) {
        if (isHorizontalWritingMode() == r.isHorizontalWritingMode())
            r.updateLogicalHeight();
        else
            r.updateLogicalWidth();
        oldLogicalTop = logicalTopForChild(r);
    }

    r.layoutIfNeeded();
    
    auto* parent = r.parent();
    bool layoutChanged = false;
    if (parent->isFlexibleBox() && downcast<RenderFlexibleBox>(parent)->setStaticPositionForPositionedLayout(r)) {
        // The static position of an abspos child of a flexbox depends on its size
        // (for example, they can be centered). So we may have to reposition the
        // item after layout.
        // FIXME: We could probably avoid a layout here and just reposition?
        layoutChanged = true;
    }

    // Lay out again if our estimate was wrong.
    if (layoutChanged || (needsBlockDirectionLocationSetBeforeLayout && logicalTopForChild(r) != oldLogicalTop)) {
        r.setChildNeedsLayout(MarkOnlyThis);
        r.layoutIfNeeded();
    }

    if (updateFragmentRangeForBoxChild(r)) {
        r.setNeedsLayout(MarkOnlyThis);
        r.layoutIfNeeded();
    }
}

void RenderBlock::layoutPositionedObjects(bool relayoutChildren, bool fixedPositionObjectsOnly)
{
    TrackedRendererListHashSet* positionedDescendants = positionedObjects();
    if (!positionedDescendants)
        return;
    
    // Do not cache positionedDescendants->end() in a local variable, since |positionedDescendants| can be mutated
    // as it is walked. We always need to fetch the new end() value dynamically.
    for (auto it = positionedDescendants->begin(); it != positionedDescendants->end(); ++it)
        layoutPositionedObject(**it, relayoutChildren, fixedPositionObjectsOnly);
}

void RenderBlock::markPositionedObjectsForLayout()
{
    TrackedRendererListHashSet* positionedDescendants = positionedObjects();
    if (!positionedDescendants)
        return;

    for (auto it = positionedDescendants->begin(), end = positionedDescendants->end(); it != end; ++it) {
        RenderBox* r = *it;
        r->setChildNeedsLayout();
    }
}

void RenderBlock::markForPaginationRelayoutIfNeeded()
{
    /**
    auto* layoutState = view().frameView().layoutContext().layoutState();
    if (needsLayout() || !layoutState->isPaginated())
        return;

    if (layoutState->pageLogicalHeightChanged() || (layoutState->pageLogicalHeight() && layoutState->pageLogicalOffset(this, logicalTop()) != pageLogicalOffset()))
        setChildNeedsLayout(MarkOnlyThis);*/
}


static ContinuationOutlineTableMap* continuationOutlineTable()
{
    static NeverDestroyed<ContinuationOutlineTableMap> table;
    return &table.get();
}

LayoutUnit blockDirectionOffset(RenderBlock& rootBlock, const LayoutSize& offsetFromRootBlock)
{
    return rootBlock.isHorizontalWritingMode() ? offsetFromRootBlock.height() : offsetFromRootBlock.width();
}

LayoutUnit inlineDirectionOffset(RenderBlock& rootBlock, const LayoutSize& offsetFromRootBlock)
{
    return rootBlock.isHorizontalWritingMode() ? offsetFromRootBlock.width() : offsetFromRootBlock.height();
}

LayoutRect RenderBlock::logicalRectToPhysicalRect(const LayoutPoint& rootBlockPhysicalPosition, const LayoutRect& logicalRect)
{
    LayoutRect result;
    if (isHorizontalWritingMode())
        result = logicalRect;
    else
        result = LayoutRect(logicalRect.y(), logicalRect.x(), logicalRect.height(), logicalRect.width());
    flipForWritingMode(result);
    result.moveBy(rootBlockPhysicalPosition);
    return result;
}


GapRects RenderBlock::inlineSelectionGaps(RenderBlock&, const LayoutPoint&, const LayoutSize&, LayoutUnit&, LayoutUnit&, LayoutUnit&, const LogicalSelectionOffsetCaches&, const PaintInfo*)
{
    ASSERT_NOT_REACHED();
    return GapRects();
}


void RenderBlock::addPercentHeightDescendant(RenderBox& descendant)
{
    insertIntoTrackedRendererMaps(*this, descendant);
}

void RenderBlock::removePercentHeightDescendant(RenderBox& descendant)
{
    removeFromTrackedRendererMaps(descendant);
}

TrackedRendererListHashSet* RenderBlock::percentHeightDescendants() const
{
    return percentHeightDescendantsMap ? percentHeightDescendantsMap->get(this) : nullptr;
}

bool RenderBlock::hasPercentHeightContainerMap()
{
    return percentHeightContainerMap;
}

bool RenderBlock::hasPercentHeightDescendant(RenderBox& descendant)
{
    // We don't null check percentHeightContainerMap since the caller
    // already ensures this and we need to call this function on every
    // descendant in clearPercentHeightDescendantsFrom().
    ASSERT(percentHeightContainerMap);
    return percentHeightContainerMap->contains(&descendant);
}

void RenderBlock::removePercentHeightDescendantIfNeeded(RenderBox& descendant)
{
    // We query the map directly, rather than looking at style's
    // logicalHeight()/logicalMinHeight()/logicalMaxHeight() since those
    // can change with writing mode/directional changes.
    if (!hasPercentHeightContainerMap())
        return;

    if (!hasPercentHeightDescendant(descendant))
        return;

    removePercentHeightDescendant(descendant);
}

void RenderBlock::clearPercentHeightDescendantsFrom(RenderBox& parent)
{
    ASSERT(percentHeightContainerMap);
    for (RenderObject* child = parent.firstChild(); child; child = child->nextInPreOrder(&parent)) {
        if (!is<RenderBox>(*child))
            continue;
 
        auto& box = downcast<RenderBox>(*child);
        if (!hasPercentHeightDescendant(box))
            continue;

        removePercentHeightDescendant(box);
    }
}

LayoutUnit RenderBlock::textIndentOffset() const
{
    LayoutUnit cw = 0;
    if (style().textIndent().isPercentOrCalculated())
        cw = containingBlock()->availableLogicalWidth();
    return minimumValueForLength(style().textIndent(), cw);
}


bool RenderBlock::nodeAtPoint(const HitTestRequest& request, HitTestResult& result, const HitTestLocation& locationInContainer, const LayoutPoint& accumulatedOffset, HitTestAction hitTestAction)
{
    /**
    LayoutPoint adjustedLocation(accumulatedOffset + location());
    LayoutSize localOffset = toLayoutSize(adjustedLocation);

    if (!isRenderView()) {
        // Check if we need to do anything at all.
        LayoutRect overflowBox = visualOverflowRect();
        flipForWritingMode(overflowBox);
        overflowBox.moveBy(adjustedLocation);
        if (!locationInContainer.intersects(overflowBox))
            return false;
    }

    if ((hitTestAction == HitTestBlockBackground || hitTestAction == HitTestChildBlockBackground) && isPointInOverflowControl(result, locationInContainer.point(), adjustedLocation)) {
        updateHitTestResult(result, locationInContainer.point() - localOffset);
        // FIXME: isPointInOverflowControl() doesn't handle rect-based tests yet.
        if (result.addNodeToListBasedTestResult(nodeForHitTest(), request, locationInContainer) == HitTestProgress::Stop)
           return true;
    }

    if (style().clipPath()) {
        switch (style().clipPath()->type()) {
        case ClipPathOperation::Shape: {
            auto& clipPath = downcast<ShapeClipPathOperation>(*style().clipPath());

            LayoutRect referenceBoxRect;
            switch (clipPath.referenceBox()) {
            case CSSBoxType::MarginBox:
                referenceBoxRect = marginBoxRect();
                break;
            case CSSBoxType::BorderBox:
                referenceBoxRect = borderBoxRect();
                break;
            case CSSBoxType::PaddingBox:
                referenceBoxRect = paddingBoxRect();
                break;
            case CSSBoxType::ContentBox:
                referenceBoxRect = contentBoxRect();
                break;
            case CSSBoxType::BoxMissing:
            case CSSBoxType::Fill:
            case CSSBoxType::Stroke:
            case CSSBoxType::ViewBox:
                referenceBoxRect = borderBoxRect();
            }
            if (!clipPath.pathForReferenceRect(referenceBoxRect).contains(locationInContainer.point() - localOffset, clipPath.windRule()))
                return false;
            break;
        }
        case ClipPathOperation::Reference: {
            const auto& referenceClipPathOperation = downcast<ReferenceClipPathOperation>(*style().clipPath());
            auto* element = document().getElementById(referenceClipPathOperation.fragment());
            if (!element || !element->renderer())
                break;
            if (!is<SVGClipPathElement>(*element))
                break;
            auto& clipper = downcast<RenderSVGResourceClipper>(*element->renderer());
            if (!clipper.hitTestClipContent(FloatRect(borderBoxRect()), FloatPoint(locationInContainer.point() - localOffset)))
                return false;
            break;
        }
        case ClipPathOperation::Box:
            break;
        }
    }

    // If we have clipping, then we can't have any spillout.
    bool useOverflowClip = hasOverflowClip() && !hasSelfPaintingLayer();
    bool useClip = (hasControlClip() || useOverflowClip);
    bool checkChildren = !useClip || (hasControlClip() ? locationInContainer.intersects(controlClipRect(adjustedLocation)) : locationInContainer.intersects(overflowClipRect(adjustedLocation, nullptr, IncludeOverlayScrollbarSize)));
    if (checkChildren) {
        // Hit test descendants first.
        LayoutSize scrolledOffset(localOffset - toLayoutSize(scrollPosition()));

        if (hitTestAction == HitTestFloat && hitTestFloats(request, result, locationInContainer, toLayoutPoint(scrolledOffset)))
            return true;
        if (hitTestContents(request, result, locationInContainer, toLayoutPoint(scrolledOffset), hitTestAction)) {
            updateHitTestResult(result, flipForWritingMode(locationInContainer.point() - localOffset));
            return true;
        }
    }

    // Check if the point is outside radii.
    if (!isRenderView() && style().hasBorderRadius()) {
        LayoutRect borderRect = borderBoxRect();
        borderRect.moveBy(adjustedLocation);
        RoundedRect border = style().getRoundedBorderFor(borderRect);
        if (!locationInContainer.intersects(border))
            return false;
    }

    // Now hit test our background
    if (hitTestAction == HitTestBlockBackground || hitTestAction == HitTestChildBlockBackground) {
        LayoutRect boundsRect(adjustedLocation, size());
        if (visibleToHitTesting() && locationInContainer.intersects(boundsRect)) {
            updateHitTestResult(result, flipForWritingMode(locationInContainer.point() - localOffset));
            if (result.addNodeToListBasedTestResult(nodeForHitTest(), request, locationInContainer, boundsRect) == HitTestProgress::Stop)
                return true;
        }
    }*/

    return false;
}

bool RenderBlock::hitTestContents(const HitTestRequest& request, HitTestResult& result, const HitTestLocation& locationInContainer, const LayoutPoint& accumulatedOffset, HitTestAction hitTestAction)
{
    if (childrenInline() && !isTable())
        return hitTestInlineChildren(request, result, locationInContainer, accumulatedOffset, hitTestAction);

    // Hit test our children.
    HitTestAction childHitTest = hitTestAction;
    if (hitTestAction == HitTestChildBlockBackgrounds)
        childHitTest = HitTestChildBlockBackground;
    for (auto* child = lastChildBox(); child; child = child->previousSiblingBox()) {
        LayoutPoint childPoint = flipForWritingModeForChild(child, accumulatedOffset);
        if (!child->hasSelfPaintingLayer() && !child->isFloating() && child->nodeAtPoint(request, result, locationInContainer, childPoint, childHitTest))
            return true;
    }

    return false;
}





static inline bool isChildHitTestCandidate(const RenderBox& box)
{
    return box.height() && box.style().visibility() == VISIBLE && !box.isOutOfFlowPositioned() && !box.isInFlowRenderFragmentedFlow();
}

// Valid candidates in a FragmentedFlow must be rendered by the fragment.
static inline bool isChildHitTestCandidate(const RenderBox& box, const RenderFragmentContainer* fragment, const LayoutPoint& point)
{
    if (!isChildHitTestCandidate(box))
        return false;
    if (!fragment)
        return true;
    const RenderBlock& block = is<RenderBlock>(box) ? downcast<RenderBlock>(box) : *box.containingBlock();
    return block.fragmentAtBlockOffset(point.y()) == fragment;
}


void RenderBlock::computeIntrinsicLogicalWidths(LayoutUnit& minLogicalWidth, LayoutUnit& maxLogicalWidth) const
{
    ASSERT(!childrenInline());
    
    computeBlockPreferredLogicalWidths(minLogicalWidth, maxLogicalWidth);

    maxLogicalWidth = std::max(minLogicalWidth, maxLogicalWidth);

    int scrollbarWidth = intrinsicScrollbarLogicalWidth();
    maxLogicalWidth += scrollbarWidth;
    minLogicalWidth += scrollbarWidth;
}

void RenderBlock::computePreferredLogicalWidths()
{
    ASSERT(preferredLogicalWidthsDirty());

    m_minPreferredLogicalWidth = 0;
    m_maxPreferredLogicalWidth = 0;

    const RenderStyle& styleToUse = style();
    if (!isTableCell() && styleToUse.logicalWidth().isFixed() && styleToUse.logicalWidth().value() >= 0
        && !(isDeprecatedFlexItem() && !styleToUse.logicalWidth().intValue()))
        m_minPreferredLogicalWidth = m_maxPreferredLogicalWidth = adjustContentBoxLogicalWidthForBoxSizing(styleToUse.logicalWidth().value());
    else
        computeIntrinsicLogicalWidths(m_minPreferredLogicalWidth, m_maxPreferredLogicalWidth);
    
    if (styleToUse.logicalMinWidth().isFixed() && styleToUse.logicalMinWidth().value() > 0) {
        m_maxPreferredLogicalWidth = std::max(m_maxPreferredLogicalWidth, adjustContentBoxLogicalWidthForBoxSizing(styleToUse.logicalMinWidth().value()));
        m_minPreferredLogicalWidth = std::max(m_minPreferredLogicalWidth, adjustContentBoxLogicalWidthForBoxSizing(styleToUse.logicalMinWidth().value()));
    }
    
    if (styleToUse.logicalMaxWidth().isFixed()) {
        m_maxPreferredLogicalWidth = std::min(m_maxPreferredLogicalWidth, adjustContentBoxLogicalWidthForBoxSizing(styleToUse.logicalMaxWidth().value()));
        m_minPreferredLogicalWidth = std::min(m_minPreferredLogicalWidth, adjustContentBoxLogicalWidthForBoxSizing(styleToUse.logicalMaxWidth().value()));
    }
    
    LayoutUnit borderAndPadding = borderAndPaddingLogicalWidth();
    m_minPreferredLogicalWidth += borderAndPadding;
    m_maxPreferredLogicalWidth += borderAndPadding;

    setPreferredLogicalWidthsDirty(false);
}

void RenderBlock::computeBlockPreferredLogicalWidths(LayoutUnit& minLogicalWidth, LayoutUnit& maxLogicalWidth) const
{
    const RenderStyle& styleToUse = style();
    bool nowrap = styleToUse.whiteSpace() == NOWRAP;

    RenderObject* child = firstChild();
    RenderBlock* containingBlock = this->containingBlock();
    LayoutUnit floatLeftWidth = 0, floatRightWidth = 0;

    LayoutUnit childMinWidth;
    LayoutUnit childMaxWidth;
    bool hadExcludedChildren = computePreferredWidthsForExcludedChildren(childMinWidth, childMaxWidth);
    if (hadExcludedChildren) {
        minLogicalWidth = std::max(childMinWidth, minLogicalWidth);
        maxLogicalWidth = std::max(childMaxWidth, maxLogicalWidth);
    }

    while (child) {
        // Positioned children don't affect the min/max width. Legends in fieldsets are skipped here
        // since they compute outside of any one layout system. Other children excluded from
        // normal layout are only used with block flows, so it's ok to calculate them here.
        if (child->isOutOfFlowPositioned() || child->isExcludedAndPlacedInBorder()) {
            child = child->nextSibling();
            continue;
        }

        const RenderStyle& childStyle = child->style();
        if (child->isFloating() || (is<RenderBox>(*child) && downcast<RenderBox>(*child).avoidsFloats())) {
            LayoutUnit floatTotalWidth = floatLeftWidth + floatRightWidth;
            if (childStyle.clear() & CLEFT) {
                maxLogicalWidth = std::max(floatTotalWidth, maxLogicalWidth);
                floatLeftWidth = 0;
            }
            if (childStyle.clear() & CRIGHT) {
                maxLogicalWidth = std::max(floatTotalWidth, maxLogicalWidth);
                floatRightWidth = 0;
            }
        }

        // A margin basically has three types: fixed, percentage, and auto (variable).
        // Auto and percentage margins simply become 0 when computing min/max width.
        // Fixed margins can be added in as is.
        Length startMarginLength = childStyle.marginStartUsing(&styleToUse);
        Length endMarginLength = childStyle.marginEndUsing(&styleToUse);
        LayoutUnit margin = 0;
        LayoutUnit marginStart = 0;
        LayoutUnit marginEnd = 0;
        if (startMarginLength.isFixed())
            marginStart += startMarginLength.value();
        if (endMarginLength.isFixed())
            marginEnd += endMarginLength.value();
        margin = marginStart + marginEnd;

        LayoutUnit childMinPreferredLogicalWidth, childMaxPreferredLogicalWidth;
        computeChildPreferredLogicalWidths(*child, childMinPreferredLogicalWidth, childMaxPreferredLogicalWidth);

        LayoutUnit w = childMinPreferredLogicalWidth + margin;
        minLogicalWidth = std::max(w, minLogicalWidth);
        
        // IE ignores tables for calculation of nowrap. Makes some sense.
        if (nowrap && !child->isTable())
            maxLogicalWidth = std::max(w, maxLogicalWidth);

        w = childMaxPreferredLogicalWidth + margin;

        if (!child->isFloating()) {
            if (is<RenderBox>(*child) && downcast<RenderBox>(*child).avoidsFloats()) {
                // Determine a left and right max value based off whether or not the floats can fit in the
                // margins of the object.  For negative margins, we will attempt to overlap the float if the negative margin
                // is smaller than the float width.
                bool ltr = containingBlock ? containingBlock->style().isLeftToRightDirection() : styleToUse.isLeftToRightDirection();
                LayoutUnit marginLogicalLeft = ltr ? marginStart : marginEnd;
                LayoutUnit marginLogicalRight = ltr ? marginEnd : marginStart;
                LayoutUnit maxLeft = marginLogicalLeft > 0 ? std::max(floatLeftWidth, marginLogicalLeft) : floatLeftWidth + marginLogicalLeft;
                LayoutUnit maxRight = marginLogicalRight > 0 ? std::max(floatRightWidth, marginLogicalRight) : floatRightWidth + marginLogicalRight;
                w = childMaxPreferredLogicalWidth + maxLeft + maxRight;
                w = std::max(w, floatLeftWidth + floatRightWidth);
            }
            else
                maxLogicalWidth = std::max(floatLeftWidth + floatRightWidth, maxLogicalWidth);
            floatLeftWidth = floatRightWidth = 0;
        }
        
        if (child->isFloating()) {
            if (childStyle.floating() == LeftFloat)
                floatLeftWidth += w;
            else
                floatRightWidth += w;
        } else
            maxLogicalWidth = std::max(w, maxLogicalWidth);
        
        child = child->nextSibling();
    }

    // Always make sure these values are non-negative.
    minLogicalWidth = std::max<LayoutUnit>(0, minLogicalWidth);
    maxLogicalWidth = std::max<LayoutUnit>(0, maxLogicalWidth);

    maxLogicalWidth = std::max(floatLeftWidth + floatRightWidth, maxLogicalWidth);
}

void RenderBlock::computeChildPreferredLogicalWidths(RenderObject& child, LayoutUnit& minPreferredLogicalWidth, LayoutUnit& maxPreferredLogicalWidth) const
{
    if (child.isBox() && child.isHorizontalWritingMode() != isHorizontalWritingMode()) {
        // If the child is an orthogonal flow, child's height determines the width,
        // but the height is not available until layout.
        // http://dev.w3.org/csswg/css-writing-modes-3/#orthogonal-shrink-to-fit
        if (!child.needsLayout()) {
            minPreferredLogicalWidth = maxPreferredLogicalWidth = downcast<RenderBox>(child).logicalHeight();
            return;
        }
        minPreferredLogicalWidth = maxPreferredLogicalWidth = downcast<RenderBox>(child).computeLogicalHeightWithoutLayout();
        return;
    }
    
    // The preferred widths of flexbox children should never depend on override sizes. They should
    // always be computed without regard for any overrides that are present.
    std::optional<LayoutUnit> overrideHeight;
    std::optional<LayoutUnit> overrideWidth;
    
    if (child.isBox()) {
        auto& box = downcast<RenderBox>(child);
        if (box.isFlexItem()) {
            if (box.hasOverrideLogicalContentHeight())
                overrideHeight = std::optional<LayoutUnit>(box.overrideLogicalContentHeight());
            if (box.hasOverrideLogicalContentWidth())
                overrideWidth = std::optional<LayoutUnit>(box.overrideLogicalContentWidth());
            box.clearOverrideSize();
        }
    }
    
    minPreferredLogicalWidth = child.minPreferredLogicalWidth();
    maxPreferredLogicalWidth = child.maxPreferredLogicalWidth();
    
    if (child.isBox()) {
        auto& box = downcast<RenderBox>(child);
        if (overrideHeight)
            box.setOverrideLogicalContentHeight(overrideHeight.value());
        if (overrideWidth)
            box.setOverrideLogicalContentWidth(overrideWidth.value());
    }

    // For non-replaced blocks if the inline size is min|max-content or a definite
    // size the min|max-content contribution is that size plus border, padding and
    // margin https://drafts.csswg.org/css-sizing/#block-intrinsic
    if (child.isRenderBlock()) {
        const Length& computedInlineSize = child.style().logicalWidth();
        if (computedInlineSize.isMaxContent())
            minPreferredLogicalWidth = maxPreferredLogicalWidth;
        else if (computedInlineSize.isMinContent())
            maxPreferredLogicalWidth = minPreferredLogicalWidth;
    }
}



LayoutUnit RenderBlock::lineHeight(bool firstLine, LineDirectionMode direction, LinePositionMode linePositionMode) const
{
    /**
    // Inline blocks are replaced elements. Otherwise, just pass off to
    // the base class.  If we're being queried as though we're the root line
    // box, then the fact that we're an inline-block is irrelevant, and we behave
    // just like a block.
    if (isReplaced() && linePositionMode == PositionOnContainingLine)
        return RenderBox::lineHeight(firstLine, direction, linePositionMode);

    if (firstLine && view().usesFirstLineRules()) {
        auto& s = firstLineStyle();
        if (&s != &style())
            return s.computedLineHeight();
    }*/
    
    return style().computedLineHeight();
}






static inline bool isRenderBlockFlowOrRenderButton(RenderElement& renderElement)
{
    // We include isRenderButton in this check because buttons are implemented
    // using flex box but should still support first-line|first-letter.
    // The flex box and specs require that flex box and grid do not support
    // first-line|first-letter, though.
    // FIXME: Remove when buttons are implemented with align-items instead of
    // flex box.
    return renderElement.isRenderBlockFlow() || renderElement.isRenderButton();
}

RenderBlock* RenderBlock::firstLineBlock() const
{
    RenderBlock* firstLineBlock = const_cast<RenderBlock*>(this);
    bool hasPseudo = false;
    while (true) {
        hasPseudo = firstLineBlock->style().hasPseudoStyle(FIRST_LINE);
        if (hasPseudo)
            break;
        RenderElement* parentBlock = firstLineBlock->parent();
        if (firstLineBlock->isReplaced() || firstLineBlock->isFloating()
            || !parentBlock || parentBlock->firstChild() != firstLineBlock || !isRenderBlockFlowOrRenderButton(*parentBlock))
            break;
        firstLineBlock = downcast<RenderBlock>(parentBlock);
    } 
    
    if (!hasPseudo)
        return nullptr;
    
    return firstLineBlock;
}

static inline RenderBlock* findFirstLetterBlock(RenderBlock* start)
{
    RenderBlock* firstLetterBlock = start;
    while (true) {
        bool canHaveFirstLetterRenderer = firstLetterBlock->style().hasPseudoStyle(FIRST_LETTER)
            && firstLetterBlock->canHaveGeneratedChildren()
            && isRenderBlockFlowOrRenderButton(*firstLetterBlock);
        if (canHaveFirstLetterRenderer)
            return firstLetterBlock;

        RenderElement* parentBlock = firstLetterBlock->parent();
        if (firstLetterBlock->isReplaced() || !parentBlock || parentBlock->firstChild() != firstLetterBlock
            || !isRenderBlockFlowOrRenderButton(*parentBlock))
            return nullptr;
        firstLetterBlock = downcast<RenderBlock>(parentBlock);
    } 

    return nullptr;
}



bool RenderBlock::cachedEnclosingFragmentedFlowNeedsUpdate() const
{
    RenderBlockRareData* rareData = getBlockRareData(*this);

    if (!rareData || !rareData->m_enclosingFragmentedFlow)
        return true;

    return false;
}

void RenderBlock::setCachedEnclosingFragmentedFlowNeedsUpdate()
{
    RenderBlockRareData& rareData = ensureBlockRareData(*this);
    rareData.m_enclosingFragmentedFlow = std::nullopt;
}

RenderFragmentedFlow* RenderBlock::updateCachedEnclosingFragmentedFlow(RenderFragmentedFlow* fragmentedFlow) const
{
    RenderBlockRareData& rareData = ensureBlockRareData(*this);
    rareData.m_enclosingFragmentedFlow = fragmentedFlow;

    return fragmentedFlow;
}

RenderFragmentedFlow* RenderBlock::locateEnclosingFragmentedFlow() const
{
    RenderBlockRareData* rareData = getBlockRareData(*this);
    if (!rareData || !rareData->m_enclosingFragmentedFlow)
        return updateCachedEnclosingFragmentedFlow(RenderBox::locateEnclosingFragmentedFlow());

    ASSERT(rareData->m_enclosingFragmentedFlow.value() == RenderBox::locateEnclosingFragmentedFlow());
    return rareData->m_enclosingFragmentedFlow.value();
}

void RenderBlock::resetEnclosingFragmentedFlowAndChildInfoIncludingDescendants(RenderFragmentedFlow*)
{
    if (fragmentedFlowState() == NotInsideFragmentedFlow)
        return;

    if (cachedEnclosingFragmentedFlowNeedsUpdate())
        return;

    auto* fragmentedFlow = cachedEnclosingFragmentedFlow();
    setCachedEnclosingFragmentedFlowNeedsUpdate();
    RenderElement::resetEnclosingFragmentedFlowAndChildInfoIncludingDescendants(fragmentedFlow);
}

LayoutUnit RenderBlock::paginationStrut() const
{
    RenderBlockRareData* rareData = getBlockRareData(*this);
    return rareData ? rareData->m_paginationStrut : LayoutUnit();
}

LayoutUnit RenderBlock::pageLogicalOffset() const
{
    RenderBlockRareData* rareData = getBlockRareData(*this);
    return rareData ? rareData->m_pageLogicalOffset : LayoutUnit();
}

void RenderBlock::setPaginationStrut(LayoutUnit strut)
{
    RenderBlockRareData* rareData = getBlockRareData(*this);
    if (!rareData) {
        if (!strut)
            return;
        rareData = &ensureBlockRareData(*this);
    }
    rareData->m_paginationStrut = strut;
}

void RenderBlock::setPageLogicalOffset(LayoutUnit logicalOffset)
{
    RenderBlockRareData* rareData = getBlockRareData(*this);
    if (!rareData) {
        if (!logicalOffset)
            return;
        rareData = &ensureBlockRareData(*this);
    }
    rareData->m_pageLogicalOffset = logicalOffset;
}

void RenderBlock::absoluteRects(Vector<IntRect>& rects, const LayoutPoint& accumulatedOffset) const
{
    // For blocks inside inlines, we include margins so that we run right up to the inline boxes
    // above and below us (thus getting merged with them to form a single irregular shape).
    if (auto* continuation = this->continuation()) {
        // FIXME: This is wrong for block-flows that are horizontal.
        // https://bugs.webkit.org/show_bug.cgi?id=46781
        rects.append(snappedIntRect(accumulatedOffset.x(), accumulatedOffset.y() - collapsedMarginBefore(), width(), height() + collapsedMarginBefore() + collapsedMarginAfter()));
        continuation->absoluteRects(rects, accumulatedOffset - toLayoutSize(location() + inlineContinuation()->containingBlock()->location()));
    } else
        rects.append(snappedIntRect(accumulatedOffset, size()));
}

void RenderBlock::absoluteQuads(Vector<FloatQuad>& quads, bool* wasFixed) const
{
    // For blocks inside inlines, we include margins so that we run right up to the inline boxes
    // above and below us (thus getting merged with them to form a single irregular shape).
    auto* continuation = this->continuation();
    FloatRect localRect = continuation
        ? FloatRect(0, -collapsedMarginBefore(), width(), height() + collapsedMarginBefore() + collapsedMarginAfter())
        : FloatRect(0, 0, width(), height());
    
    // FIXME: This is wrong for block-flows that are horizontal.
    // https://bugs.webkit.org/show_bug.cgi?id=46781
    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();
    if (!fragmentedFlow || !fragmentedFlow->absoluteQuadsForBox(quads, wasFixed, this, localRect.y(), localRect.maxY()))
        quads.append(localToAbsoluteQuad(localRect, UseTransforms, wasFixed));

    if (continuation)
        continuation->absoluteQuads(quads, wasFixed);
}

LayoutRect RenderBlock::rectWithOutlineForRepaint(const RenderElement* repaintContainer, LayoutUnit outlineWidth) const
{
    LayoutRect r(RenderBox::rectWithOutlineForRepaint(repaintContainer, outlineWidth));
    if (isContinuation())
        r.inflateY(collapsedMarginBefore()); // FIXME: This is wrong for block-flows that are horizontal.
    return r;
}

RenderElement* RenderBlock::hoverAncestor() const
{
    if (auto* continuation = this->continuation())
        return continuation;
    return RenderBox::hoverAncestor();
}

void RenderBlock::updateDragState(bool dragOn)
{
    RenderBox::updateDragState(dragOn);
    if (RenderBoxModelObject* continuation = this->continuation())
        continuation->updateDragState(dragOn);
}

const RenderStyle& RenderBlock::outlineStyleForRepaint() const
{
    if (auto* continuation = this->continuation())
        return continuation->style();
    return RenderElement::outlineStyleForRepaint();
}

void RenderBlock::childBecameNonInline(RenderElement&)
{
    makeChildrenNonInline();
    if (isAnonymousBlock() && is<RenderBlock>(parent()))
        downcast<RenderBlock>(*parent()).removeLeftoverAnonymousBlock(this);
    // |this| may be dead here
}

void RenderBlock::updateHitTestResult(HitTestResult& result, const LayoutPoint& point)
{
    /**
    if (result.innerNode())
        return;

    if (Node* n = nodeForHitTest()) {
        result.setInnerNode(n);
        if (!result.innerNonSharedNode())
            result.setInnerNonSharedNode(n);
        result.setLocalPoint(point);
    }*/
}

LayoutRect RenderBlock::localCaretRect(InlineBox* inlineBox, unsigned caretOffset, LayoutUnit* extraWidthToEndOfLine)
{
    // Do the normal calculation in most cases.
    if (firstChild())
        return RenderBox::localCaretRect(inlineBox, caretOffset, extraWidthToEndOfLine);

    LayoutRect caretRect = localCaretRectForEmptyElement(width(), textIndentOffset());

    // FIXME: Does this need to adjust for vertical orientation?
    if (extraWidthToEndOfLine)
        *extraWidthToEndOfLine = width() - caretRect.maxX();

    return caretRect;
}

void RenderBlock::addFocusRingRectsForInlineChildren(Vector<LayoutRect>&, const LayoutPoint&, const RenderElement*)
{
    ASSERT_NOT_REACHED();
}





RenderFragmentContainer* RenderBlock::fragmentAtBlockOffset(LayoutUnit blockOffset) const
{
    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();
    if (!fragmentedFlow || !fragmentedFlow->hasValidFragmentInfo())
        return 0;

    return fragmentedFlow->fragmentAtBlockOffset(this, offsetFromLogicalTopOfFirstPage() + blockOffset, true);
}

static bool canComputeFragmentRangeForBox(const RenderBlock& parentBlock, const RenderBox& childBox, const RenderFragmentedFlow* enclosingFragmentedFlow)
{
    if (!enclosingFragmentedFlow)
        return false;

    if (!enclosingFragmentedFlow->hasFragments())
        return false;

    if (!childBox.canHaveOutsideFragmentRange())
        return false;

    return enclosingFragmentedFlow->hasCachedFragmentRangeForBox(parentBlock);
}


void RenderBlock::computeFragmentRangeForBoxChild(const RenderBox& box) const
{
    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();
    ASSERT(canComputeFragmentRangeForBox(*this, box, fragmentedFlow));

    RenderFragmentContainer* startFragment;
    RenderFragmentContainer* endFragment;
    LayoutUnit offsetFromLogicalTopOfFirstFragment = box.offsetFromLogicalTopOfFirstPage();
    if (childBoxIsUnsplittableForFragmentation(box))
        startFragment = endFragment = fragmentedFlow->fragmentAtBlockOffset(this, offsetFromLogicalTopOfFirstFragment, true);
    else {
        startFragment = fragmentedFlow->fragmentAtBlockOffset(this, offsetFromLogicalTopOfFirstFragment, true);
        endFragment = fragmentedFlow->fragmentAtBlockOffset(this, offsetFromLogicalTopOfFirstFragment + logicalHeightForChild(box), true);
    }

    fragmentedFlow->setFragmentRangeForBox(box, startFragment, endFragment);
}

void RenderBlock::estimateFragmentRangeForBoxChild(const RenderBox& box) const
{
    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();
    if (!canComputeFragmentRangeForBox(*this, box, fragmentedFlow))
        return;

    if (childBoxIsUnsplittableForFragmentation(box)) {
        computeFragmentRangeForBoxChild(box);
        return;
    }

    auto estimatedValues = box.computeLogicalHeight(RenderFragmentedFlow::maxLogicalHeight(), logicalTopForChild(box));
    LayoutUnit offsetFromLogicalTopOfFirstFragment = box.offsetFromLogicalTopOfFirstPage();
    RenderFragmentContainer* startFragment = fragmentedFlow->fragmentAtBlockOffset(this, offsetFromLogicalTopOfFirstFragment, true);
    RenderFragmentContainer* endFragment = fragmentedFlow->fragmentAtBlockOffset(this, offsetFromLogicalTopOfFirstFragment + estimatedValues.m_extent, true);

    fragmentedFlow->setFragmentRangeForBox(box, startFragment, endFragment);
}

bool RenderBlock::updateFragmentRangeForBoxChild(const RenderBox& box) const
{
    RenderFragmentedFlow* fragmentedFlow = enclosingFragmentedFlow();
    if (!canComputeFragmentRangeForBox(*this, box, fragmentedFlow))
        return false;

    RenderFragmentContainer* startFragment = nullptr;
    RenderFragmentContainer* endFragment = nullptr;
    fragmentedFlow->getFragmentRangeForBox(&box, startFragment, endFragment);

    computeFragmentRangeForBoxChild(box);

    RenderFragmentContainer* newStartFragment = nullptr;
    RenderFragmentContainer* newEndFragment = nullptr;
    fragmentedFlow->getFragmentRangeForBox(&box, newStartFragment, newEndFragment);


    // Changing the start fragment means we shift everything and a relayout is needed.
    if (newStartFragment != startFragment)
        return true;

    // The fragment range of the box has changed. Some boxes (e.g floats) may have been positioned assuming
    // a different range.
    if (box.needsLayoutAfterFragmentRangeChange() && newEndFragment != endFragment)
        return true;

    return false;
}

LayoutUnit RenderBlock::collapsedMarginBeforeForChild(const RenderBox& child) const
{
    // If the child has the same directionality as we do, then we can just return its
    // collapsed margin.
    if (!child.isWritingModeRoot())
        return child.collapsedMarginBefore();
    
    // The child has a different directionality.  If the child is parallel, then it's just
    // flipped relative to us.  We can use the collapsed margin for the opposite edge.
    if (child.isHorizontalWritingMode() == isHorizontalWritingMode())
        return child.collapsedMarginAfter();
    
    // The child is perpendicular to us, which means its margins don't collapse but are on the
    // "logical left/right" sides of the child box.  We can just return the raw margin in this case.  
    return marginBeforeForChild(child);
}

LayoutUnit RenderBlock::collapsedMarginAfterForChild(const RenderBox& child) const
{
    // If the child has the same directionality as we do, then we can just return its
    // collapsed margin.
    if (!child.isWritingModeRoot())
        return child.collapsedMarginAfter();
    
    // The child has a different directionality.  If the child is parallel, then it's just
    // flipped relative to us.  We can use the collapsed margin for the opposite edge.
    if (child.isHorizontalWritingMode() == isHorizontalWritingMode())
        return child.collapsedMarginBefore();
    
    // The child is perpendicular to us, which means its margins don't collapse but are on the
    // "logical left/right" side of the child box.  We can just return the raw margin in this case.  
    return marginAfterForChild(child);
}

bool RenderBlock::hasMarginBeforeQuirk(const RenderBox& child) const
{
    // If the child has the same directionality as we do, then we can just return its
    // margin quirk.
    if (!child.isWritingModeRoot())
        return is<RenderBlock>(child) ? downcast<RenderBlock>(child).hasMarginBeforeQuirk() : child.style().hasMarginBeforeQuirk();
    
    // The child has a different directionality. If the child is parallel, then it's just
    // flipped relative to us. We can use the opposite edge.
    if (child.isHorizontalWritingMode() == isHorizontalWritingMode())
        return is<RenderBlock>(child) ? downcast<RenderBlock>(child).hasMarginAfterQuirk() : child.style().hasMarginAfterQuirk();
    
    // The child is perpendicular to us and box sides are never quirky in html.css, and we don't really care about
    // whether or not authors specified quirky ems, since they're an implementation detail.
    return false;
}

bool RenderBlock::hasMarginAfterQuirk(const RenderBox& child) const
{
    // If the child has the same directionality as we do, then we can just return its
    // margin quirk.
    if (!child.isWritingModeRoot())
        return is<RenderBlock>(child) ? downcast<RenderBlock>(child).hasMarginAfterQuirk() : child.style().hasMarginAfterQuirk();
    
    // The child has a different directionality. If the child is parallel, then it's just
    // flipped relative to us. We can use the opposite edge.
    if (child.isHorizontalWritingMode() == isHorizontalWritingMode())
        return is<RenderBlock>(child) ? downcast<RenderBlock>(child).hasMarginBeforeQuirk() : child.style().hasMarginBeforeQuirk();
    
    // The child is perpendicular to us and box sides are never quirky in html.css, and we don't really care about
    // whether or not authors specified quirky ems, since they're an implementation detail.
    return false;
}

const char* RenderBlock::renderName() const
{
    if (isBody())
        return "RenderBody"; // FIXME: Temporary hack until we know that the regression tests pass.
    if (isFieldset())
        return "RenderFieldSet"; // FIXME: Remove eventually, but done to keep tests from breaking.
    if (isFloating())
        return "RenderBlock (floating)";
    if (isOutOfFlowPositioned())
        return "RenderBlock (positioned)";
    if (isAnonymousBlock())
        return "RenderBlock (anonymous)";
    // FIXME: Temporary hack while the new generated content system is being implemented.
    if (isPseudoElement())
        return "RenderBlock (generated)";
    if (isAnonymous())
        return "RenderBlock (generated)";
    if (isRelativelyPositioned())
        return "RenderBlock (relative positioned)";
    if (isStickilyPositioned())
        return "RenderBlock (sticky positioned)";
    return "RenderBlock";
}



#ifndef NDEBUG
void RenderBlock::checkPositionedObjectsNeedLayout()
{
    auto* positionedDescendants = positionedObjects();
    if (!positionedDescendants)
        return;

    for (auto* renderer : *positionedDescendants)
        ASSERT(!renderer->needsLayout());
}

#endif

bool RenderBlock::hasDefiniteLogicalHeight() const
{
    return (bool)availableLogicalHeightForPercentageComputation();
}

std::optional<LayoutUnit> RenderBlock::availableLogicalHeightForPercentageComputation() const
{
    std::optional<LayoutUnit> availableHeight;
    
    // For anonymous blocks that are skipped during percentage height calculation,
    // we consider them to have an indefinite height.
    if (skipContainingBlockForPercentHeightCalculation(*this, false))
        return availableHeight;
    
    const auto& styleToUse = style();
    
    // A positioned element that specified both top/bottom or that specifies
    // height should be treated as though it has a height explicitly specified
    // that can be used for any percentage computations.
    bool isOutOfFlowPositionedWithSpecifiedHeight = isOutOfFlowPositioned() && (!styleToUse.logicalHeight().isAuto() || (!styleToUse.logicalTop().isAuto() && !styleToUse.logicalBottom().isAuto()));
    
    std::optional<LayoutUnit> stretchedFlexHeight;
    if (isFlexItem())
        stretchedFlexHeight = downcast<RenderFlexibleBox>(parent())->childLogicalHeightForPercentageResolution(*this);
    
    if (stretchedFlexHeight)
        availableHeight = stretchedFlexHeight;
    else if (isGridItem() && hasOverrideLogicalContentHeight())
        availableHeight = overrideLogicalContentHeight();
    else if (styleToUse.logicalHeight().isFixed()) {
        LayoutUnit contentBoxHeight = adjustContentBoxLogicalHeightForBoxSizing((LayoutUnit)styleToUse.logicalHeight().value());
        availableHeight = std::max(LayoutUnit(), constrainContentBoxLogicalHeightByMinMax(contentBoxHeight - scrollbarLogicalHeight(), std::nullopt));
    } else if (styleToUse.logicalHeight().isPercentOrCalculated() && !isOutOfFlowPositionedWithSpecifiedHeight) {
        std::optional<LayoutUnit> heightWithScrollbar = computePercentageLogicalHeight(styleToUse.logicalHeight());
        if (heightWithScrollbar) {
            LayoutUnit contentBoxHeightWithScrollbar = adjustContentBoxLogicalHeightForBoxSizing(heightWithScrollbar.value());
            // We need to adjust for min/max height because this method does not
            // handle the min/max of the current block, its caller does. So the
            // return value from the recursive call will not have been adjusted
            // yet.
            LayoutUnit contentBoxHeight = constrainContentBoxLogicalHeightByMinMax(contentBoxHeightWithScrollbar - scrollbarLogicalHeight(), std::nullopt);
            availableHeight = std::max(LayoutUnit(), contentBoxHeight);
        }
    } else if (isOutOfFlowPositionedWithSpecifiedHeight) {
        // Don't allow this to affect the block' size() member variable, since this
        // can get called while the block is still laying out its kids.
        LogicalExtentComputedValues computedValues = computeLogicalHeight(logicalHeight(), LayoutUnit());
        availableHeight = computedValues.m_extent - borderAndPaddingLogicalHeight() - scrollbarLogicalHeight();
    } //else if (isRenderView())
       // availableHeight = view().pageOrViewLogicalHeight();
    
    return availableHeight;
}
    
void RenderBlock::layoutExcludedChildren(bool relayoutChildren)
{
    if (!isFieldset())
        return;

    setIntrinsicBorderForFieldset(0);

    RenderBox* box = findFieldsetLegend();
    if (!box)
        return;

    box->setIsExcludedFromNormalLayout(true);
    for (auto& child : childrenOfType<RenderBox>(*this)) {
        if (&child == box || !child.isLegend())
            continue;
        child.setIsExcludedFromNormalLayout(false);
    }

    RenderBox& legend = *box;
    if (relayoutChildren)
        legend.setChildNeedsLayout(MarkOnlyThis);
    legend.layoutIfNeeded();
    
    LayoutUnit logicalLeft;
    if (style().isLeftToRightDirection()) {
        switch (legend.style().textAlign()) {
        case CENTER:
            logicalLeft = (logicalWidth() - logicalWidthForChild(legend)) / 2;
            break;
        case RIGHT:
            logicalLeft = logicalWidth() - borderEnd() - paddingEnd() - logicalWidthForChild(legend);
            break;
        default:
            logicalLeft = borderStart() + paddingStart() + marginStartForChild(legend);
            break;
        }
    } else {
        switch (legend.style().textAlign()) {
        case LEFT:
            logicalLeft = borderStart() + paddingStart();
            break;
        case CENTER: {
            // Make sure that the extra pixel goes to the end side in RTL (since it went to the end side
            // in LTR).
            LayoutUnit centeredWidth = logicalWidth() - logicalWidthForChild(legend);
            logicalLeft = centeredWidth - centeredWidth / 2;
            break;
        }
        default:
            logicalLeft = logicalWidth() - borderStart() - paddingStart() - marginStartForChild(legend) - logicalWidthForChild(legend);
            break;
        }
    }
    
    setLogicalLeftForChild(legend, logicalLeft);
    
    LayoutUnit fieldsetBorderBefore = borderBefore();
    LayoutUnit legendLogicalHeight = logicalHeightForChild(legend);
    LayoutUnit legendAfterMargin = marginAfterForChild(legend);
    LayoutUnit topPositionForLegend = std::max(LayoutUnit(), (fieldsetBorderBefore - legendLogicalHeight) / 2);
    LayoutUnit bottomPositionForLegend = topPositionForLegend + legendLogicalHeight + legendAfterMargin;

    // Place the legend now.
    setLogicalTopForChild(legend, topPositionForLegend);

    // If the bottom of the legend (including its after margin) is below the fieldset border,
    // then we need to add in sufficient intrinsic border to account for this gap.
    // FIXME: Should we support the before margin of the legend? Not entirely clear.
    // FIXME: Consider dropping support for the after margin of the legend. Not sure other
    // browsers support that anyway.
    if (bottomPositionForLegend > fieldsetBorderBefore)
        setIntrinsicBorderForFieldset(bottomPositionForLegend - fieldsetBorderBefore);
    
    // Now that the legend is included in the border extent, we can set our logical height
    // to the borderBefore (which includes the legend and its after margin if they were bigger
    // than the actual fieldset border) and then add in our padding before.
    setLogicalHeight(borderBefore() + paddingBefore());
}

RenderBox* RenderBlock::findFieldsetLegend(FieldsetFindLegendOption option) const
{
    for (auto& legend : childrenOfType<RenderBox>(*this)) {
        if (option == FieldsetIgnoreFloatingOrOutOfFlow && legend.isFloatingOrOutOfFlowPositioned())
            continue;
        if (legend.isLegend())
            return const_cast<RenderBox*>(&legend);
    }
    return nullptr;
}

void RenderBlock::adjustBorderBoxRectForPainting(LayoutRect& paintRect)
{
    if (!isFieldset() || !intrinsicBorderForFieldset())
        return;
    
    auto* legend = findFieldsetLegend();
    if (!legend)
        return;

    if (style().isHorizontalWritingMode()) {
        LayoutUnit yOff = std::max(LayoutUnit(), (legend->height() - RenderBox::borderBefore()) / 2);
        paintRect.setHeight(paintRect.height() - yOff);
        if (style().writingMode() == TopToBottomWritingMode)
            paintRect.setY(paintRect.y() + yOff);
    } else {
        LayoutUnit xOff = std::max(LayoutUnit(), (legend->width() - RenderBox::borderBefore()) / 2);
        paintRect.setWidth(paintRect.width() - xOff);
        if (style().writingMode() == LeftToRightWritingMode)
            paintRect.setX(paintRect.x() + xOff);
    }
}

LayoutRect RenderBlock::paintRectToClipOutFromBorder(const LayoutRect& paintRect)
{
    LayoutRect clipRect;
    if (!isFieldset())
        return clipRect;
    auto* legend = findFieldsetLegend();
    if (!legend)
        return clipRect;

    LayoutUnit borderExtent = RenderBox::borderBefore();
    if (style().isHorizontalWritingMode()) {
        clipRect.setX(paintRect.x() + legend->x());
        clipRect.setY(style().writingMode() == TopToBottomWritingMode ? paintRect.y() : paintRect.y() + paintRect.height() - borderExtent);
        clipRect.setWidth(legend->width());
        clipRect.setHeight(borderExtent);
    } else {
        clipRect.setX(style().writingMode() == LeftToRightWritingMode ? paintRect.x() : paintRect.x() + paintRect.width() - borderExtent);
        clipRect.setY(paintRect.y() + legend->y());
        clipRect.setWidth(borderExtent);
        clipRect.setHeight(legend->height());
    }
    return clipRect;
}

LayoutUnit RenderBlock::intrinsicBorderForFieldset() const
{
    auto* rareData = getBlockRareData(*this);
    return rareData ? rareData->m_intrinsicBorderForFieldset : LayoutUnit();
}

void RenderBlock::setIntrinsicBorderForFieldset(LayoutUnit padding)
{
    auto* rareData = getBlockRareData(*this);
    if (!rareData) {
        if (!padding)
            return;
        rareData = &ensureBlockRareData(*this);
    }
    rareData->m_intrinsicBorderForFieldset = padding;
}

LayoutUnit RenderBlock::borderTop() const
{
    if (style().writingMode() != TopToBottomWritingMode || !intrinsicBorderForFieldset())
        return RenderBox::borderTop();
    return RenderBox::borderTop() + intrinsicBorderForFieldset();
}

LayoutUnit RenderBlock::borderLeft() const
{
    if (style().writingMode() != LeftToRightWritingMode || !intrinsicBorderForFieldset())
        return RenderBox::borderLeft();
    return RenderBox::borderLeft() + intrinsicBorderForFieldset();
}

LayoutUnit RenderBlock::borderBottom() const
{
    if (style().writingMode() != BottomToTopWritingMode || !intrinsicBorderForFieldset())
        return RenderBox::borderBottom();
    return RenderBox::borderBottom() + intrinsicBorderForFieldset();
}

LayoutUnit RenderBlock::borderRight() const
{
    if (style().writingMode() != RightToLeftWritingMode || !intrinsicBorderForFieldset())
        return RenderBox::borderRight();
    return RenderBox::borderRight() + intrinsicBorderForFieldset();
}

LayoutUnit RenderBlock::borderBefore() const
{
    return RenderBox::borderBefore() + intrinsicBorderForFieldset();
}

bool RenderBlock::computePreferredWidthsForExcludedChildren(LayoutUnit& minWidth, LayoutUnit& maxWidth) const
{
    if (!isFieldset())
        return false;
    
    auto* legend = findFieldsetLegend();
    if (!legend)
        return false;
    
    legend->setIsExcludedFromNormalLayout(true);

    computeChildPreferredLogicalWidths(*legend, minWidth, maxWidth);
    
    // These are going to be added in later, so we subtract them out to reflect the
    // fact that the legend is outside the scrollable area.
    auto scrollbarWidth = intrinsicScrollbarLogicalWidth();
    minWidth -= scrollbarWidth;
    maxWidth -= scrollbarWidth;
    
    const auto& childStyle = legend->style();
    auto startMarginLength = childStyle.marginStartUsing(&style());
    auto endMarginLength = childStyle.marginEndUsing(&style());
    LayoutUnit margin;
    LayoutUnit marginStart;
    LayoutUnit marginEnd;
    if (startMarginLength.isFixed())
        marginStart += startMarginLength.value();
    if (endMarginLength.isFixed())
        marginEnd += endMarginLength.value();
    margin = marginStart + marginEnd;
    
    minWidth += margin;
    maxWidth += margin;

    return true;
}

LayoutUnit RenderBlock::adjustBorderBoxLogicalHeightForBoxSizing(LayoutUnit height) const
{
    // FIXME: We're doing this to match other browsers even though it's questionable.
    // Shouldn't height:100px mean the fieldset content gets 100px of height even if the
    // resulting fieldset becomes much taller because of the legend?
    LayoutUnit bordersPlusPadding = borderAndPaddingLogicalHeight();
    if (style().boxSizing() == CONTENT_BOX)
        return height + bordersPlusPadding - intrinsicBorderForFieldset();
    return std::max(height, bordersPlusPadding);
}

LayoutUnit RenderBlock::adjustContentBoxLogicalHeightForBoxSizing(std::optional<LayoutUnit> height) const
{
    // FIXME: We're doing this to match other browsers even though it's questionable.
    // Shouldn't height:100px mean the fieldset content gets 100px of height even if the
    // resulting fieldset becomes much taller because of the legend?
    if (!height)
        return 0;
    LayoutUnit result = height.value();
    if (style().boxSizing() == BORDER_BOX)
        result -= borderAndPaddingLogicalHeight();
    else
        result -= intrinsicBorderForFieldset();
    return std::max(LayoutUnit(), result);
}


    
} // namespace WebCore
