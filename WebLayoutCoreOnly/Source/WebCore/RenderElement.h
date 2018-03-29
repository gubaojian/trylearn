/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2006, 2007, 2009, 2013 Apple Inc. All rights reserved.
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

#pragma once

#include "LengthFunctions.h"
#include "RenderObject.h"
#include "RenderPtr.h"

namespace WebCore {

class ControlStates;
class RenderBlock;

class RenderElement : public RenderObject {
    WTF_MAKE_ISO_ALLOCATED(RenderElement);
public:
    virtual ~RenderElement();

    enum RendererCreationType { CreateAllRenderers, OnlyCreateBlockAndFlexboxRenderers };
    static RenderPtr<RenderElement> createFor(Element&, RenderStyle&&, RendererCreationType = CreateAllRenderers);

    bool hasInitializedStyle() const { return m_hasInitializedStyle; }

    const RenderStyle& style() const { return m_style; }
    const RenderStyle& firstLineStyle() const;

    // FIXME: Style shouldn't be mutated.
    RenderStyle& mutableStyle() { return m_style; }

    void initializeStyle();

    // Calling with minimalStyleDifference > StyleDifferenceEqual indicates that
    // out-of-band state (e.g. animations) requires that styleDidChange processing
    // continue even if the style isn't different from the current style.
    void setStyle(RenderStyle&&, StyleDifference minimalStyleDifference = StyleDifferenceEqual);

    // The pseudo element style can be cached or uncached.  Use the cached method if the pseudo element doesn't respect
    // any pseudo classes (and therefore has no concept of changing state).
    const RenderStyle* getCachedPseudoStyle(PseudoId, const RenderStyle* parentStyle = nullptr) const;
    std::unique_ptr<RenderStyle> getUncachedPseudoStyle(const PseudoStyleRequest&, const RenderStyle* parentStyle = nullptr, const RenderStyle* ownStyle = nullptr) const;

    // This is null for anonymous renderers.
    //Element* element() const { return downcast<Element>(RenderObject::node()); }
    //Element* nonPseudoElement() const { return downcast<Element>(RenderObject::nonPseudoNode()); }
    //Element* generatingElement() const;

    RenderObject* firstChild() const { return m_firstChild; }
    RenderObject* lastChild() const { return m_lastChild; }

    bool canContainFixedPositionObjects() const;
    bool canContainAbsolutelyPositionedObjects() const;

    Color selectionColor(int colorProperty) const;
    std::unique_ptr<RenderStyle> selectionPseudoStyle() const;

    // Obtains the selection colors that should be used when painting a selection.
    Color selectionBackgroundColor() const;
    Color selectionForegroundColor() const;
    Color selectionEmphasisMarkColor() const;

    // FIXME: Make these standalone and move to relevant files.
    bool isRenderElement() const;
    bool isBoxModelObject() const;
    bool isRenderBlock() const;
    bool isRenderBlockFlow() const;
    bool isRenderReplaced() const;
    bool isRenderInline() const;

    virtual bool isChildAllowed(const RenderObject&, const RenderStyle&) const { return true; }
    virtual void addChild(RenderPtr<RenderObject>, RenderObject* beforeChild = nullptr);
    virtual void addChildIgnoringContinuation(RenderPtr<RenderObject> newChild, RenderObject* beforeChild = nullptr) { addChild(WTFMove(newChild), beforeChild); }
    virtual RenderPtr<RenderObject> takeChild(RenderObject&) WARN_UNUSED_RETURN;
    void removeAndDestroyChild(RenderObject&);

    // The following functions are used when the render tree hierarchy changes to make sure layers get
    // properly added and removed. Since containership can be implemented by any subclass, and since a hierarchy
    // can contain a mixture of boxes and other object types, these functions need to be in the base class.
    void addLayers(RenderLayer* parentLayer);
    void removeLayers(RenderLayer* parentLayer);
    void moveLayers(RenderLayer* oldParent, RenderLayer* newParent);
    RenderLayer* findNextLayer(RenderLayer* parentLayer, RenderObject* startPoint, bool checkParent = true);

    void insertChildInternal(RenderPtr<RenderObject>, RenderObject* beforeChild);
    RenderPtr<RenderObject> takeChildInternal(RenderObject&) WARN_UNUSED_RETURN;

    virtual RenderElement* hoverAncestor() const;

    virtual void dirtyLinesFromChangedChild(RenderObject&) { }

    bool ancestorLineBoxDirty() const { return m_ancestorLineBoxDirty; }
    void setAncestorLineBoxDirty(bool f = true);

    void setChildNeedsLayout(MarkingBehavior = MarkContainingBlockChain);
    void clearChildNeedsLayout();
    void setNeedsPositionedMovementLayout(const RenderStyle* oldStyle);
    void setNeedsSimplifiedNormalFlowLayout();

    virtual void paint(PaintInfo&, const LayoutPoint&) = 0;

    // inline-block elements paint all phases atomically. This function ensures that. Certain other elements
    // (grid items, flex items) require this behavior as well, and this function exists as a helper for them.
    // It is expected that the caller will call this function independent of the value of paintInfo.phase.
    void paintAsInlineBlock(PaintInfo&, const LayoutPoint&);

    // Recursive function that computes the size and position of this object and all its descendants.
    virtual void layout();

    /* This function performs a layout only if one is needed. */
    void layoutIfNeeded() { if (needsLayout()) layout(); }

    // Updates only the local style ptr of the object. Does not update the state of the object,
    // and so only should be called when the style is known not to have changed (or from setStyle).
    void setStyleInternal(RenderStyle&& style) { m_style = WTFMove(style); }

    // Repaint only if our old bounds and new bounds are different. The caller may pass in newBounds and newOutlineBox if they are known.
    bool repaintAfterLayoutIfNeeded(const RenderElement* repaintContainer, const LayoutRect& oldBounds, const LayoutRect& oldOutlineBox, const LayoutRect* newBoundsPtr = nullptr, const LayoutRect* newOutlineBoxPtr = nullptr);

    bool borderImageIsLoadedAndCanBeRendered() const;
    bool mayCauseRepaintInsideViewport(const IntRect* visibleRect = nullptr) const;
    bool isVisibleInDocumentRect(const IntRect& documentRect) const;

    // Returns true if this renderer requires a new stacking context.
    static bool createsGroupForStyle(const RenderStyle&);
    bool createsGroup() const { return createsGroupForStyle(style()); }

    bool isTransparent() const { return style().opacity() < 1.0f; }
    float opacity() const { return style().opacity(); }

    bool visibleToHitTesting() const { return style().visibility() == VISIBLE && style().pointerEvents() != PE_NONE; }

    bool hasBackground() const { return style().hasBackground(); }
    //bool hasMask() const { return style().hasMask(); }
    bool hasClip() const { return isOutOfFlowPositioned() && style().hasClip(); }
    bool hasClipOrOverflowClip() const { return hasClip() || hasOverflowClip(); }
    //bool hasClipPath() const { return style().clipPath(); }
    bool hasHiddenBackface() const { return style().backfaceVisibility() == BackfaceVisibilityHidden; }
    bool hasOutlineAnnotation() const;
    bool hasOutline() const { return style().hasOutline() || hasOutlineAnnotation(); }
    bool hasSelfPaintingLayer() const;

    bool checkForRepaintDuringLayout() const;

    // anchorRect() is conceptually similar to absoluteBoundingBoxRect(), but is intended for scrolling to an anchor.
    // For inline renderers, this gets the logical top left of the first leaf child and the logical bottom right of the
    // last leaf child, converts them to absolute coordinates, and makes a box out of them.
    LayoutRect absoluteAnchorRect(bool* insideFixed = nullptr) const;

    //bool hasFilter() const { return style().hasFilter(); }
    bool hasBackdropFilter() const
    {
#if ENABLE(FILTERS_LEVEL_2)
        return style().hasBackdropFilter();
#else
        return false;
#endif
    }


#if ENABLE(CSS_COMPOSITING)
    bool hasBlendMode() const { return style().hasBlendMode(); }
#else
    bool hasBlendMode() const { return false; }
#endif

    //bool hasShapeOutside() const { return style().shapeOutside(); }

    void registerForVisibleInViewportCallback();
    void unregisterForVisibleInViewportCallback();

    VisibleInViewportState visibleInViewportState() const { return static_cast<VisibleInViewportState>(m_visibleInViewportState); }
    void setVisibleInViewportState(VisibleInViewportState);
    virtual void visibleInViewportStateChanged();

    bool hasPausedImageAnimations() const { return m_hasPausedImageAnimations; }
    void setHasPausedImageAnimations(bool b) { m_hasPausedImageAnimations = b; }

    void setRenderBoxNeedsLazyRepaint(bool b) { m_renderBoxNeedsLazyRepaint = b; }
    bool renderBoxNeedsLazyRepaint() const { return m_renderBoxNeedsLazyRepaint; }

    bool hasCounterNodeMap() const { return m_hasCounterNodeMap; }
    void setHasCounterNodeMap(bool f) { m_hasCounterNodeMap = f; }

    const RenderElement* enclosingRendererWithTextDecoration(TextDecoration, bool firstLine) const;

    bool childRequiresTable(const RenderObject& child) const;

#if ENABLE(TEXT_AUTOSIZING)
    void adjustComputedFontSizesOnBlocks(float size, float visibleWidth);
    WEBCORE_EXPORT void resetTextAutosizing();
#endif
    RenderBlock* containingBlockForFixedPosition() const;
    RenderBlock* containingBlockForAbsolutePosition() const;

    //RespectImageOrientationEnum shouldRespectImageOrientation() const;

    void removeFromRenderFragmentedFlow();
    virtual void resetEnclosingFragmentedFlowAndChildInfoIncludingDescendants(RenderFragmentedFlow*);

    // Called before anonymousChild.setStyle(). Override to set custom styles for
    // the child.
    virtual void updateAnonymousChildStyle(const RenderObject&, RenderStyle&) const { };

    void removeAnonymousWrappersForInlinesIfNecessary();

    bool hasContinuationChainNode() const { return m_hasContinuationChainNode; }
    bool isContinuation() const { return m_isContinuation; }
    void setIsContinuation() { m_isContinuation = true; }
    bool isFirstLetter() const { return m_isFirstLetter; }
    void setIsFirstLetter() { m_isFirstLetter = true; }

    void destroyLeftoverChildren();

protected:
    enum BaseTypeFlag {
        RenderElementFlag  = 1 << 0,
        RenderBoxModelObjectFlag    = 1 << 1,
        RenderInlineFlag            = 1 << 2,
        RenderReplacedFlag          = 1 << 3,
        RenderBlockFlag             = 1 << 4,
        RenderBlockFlowFlag         = 1 << 5,
    };
    
    typedef unsigned BaseTypeFlags;

    RenderElement(RenderStyle&&, BaseTypeFlags);

    bool layerCreationAllowedForSubtree() const;

    enum StylePropagationType { PropagateToAllChildren, PropagateToBlockChildrenOnly };
    void propagateStyleToAnonymousChildren(StylePropagationType);

    LayoutUnit valueForLength(const Length&, LayoutUnit maximumValue) const;
    LayoutUnit minimumValueForLength(const Length&, LayoutUnit maximumValue) const;

    void setFirstChild(RenderObject* child) { m_firstChild = child; }
    void setLastChild(RenderObject* child) { m_lastChild = child; }

    virtual void styleWillChange(StyleDifference, const RenderStyle& newStyle);
    virtual void styleDidChange(StyleDifference, const RenderStyle* oldStyle);

    void insertedIntoTree() override;
    void willBeRemovedFromTree() override;
    void willBeDestroyed() override;

    void setRenderInlineAlwaysCreatesLineBoxes(bool b) { m_renderInlineAlwaysCreatesLineBoxes = b; }
    bool renderInlineAlwaysCreatesLineBoxes() const { return m_renderInlineAlwaysCreatesLineBoxes; }

    void setHasContinuationChainNode(bool b) { m_hasContinuationChainNode = b; }

    void setRenderBlockHasMarginBeforeQuirk(bool b) { m_renderBlockHasMarginBeforeQuirk = b; }
    void setRenderBlockHasMarginAfterQuirk(bool b) { m_renderBlockHasMarginAfterQuirk = b; }
    void setRenderBlockShouldForceRelayoutChildren(bool b) { m_renderBlockShouldForceRelayoutChildren = b; }
    bool renderBlockHasMarginBeforeQuirk() const { return m_renderBlockHasMarginBeforeQuirk; }
    bool renderBlockHasMarginAfterQuirk() const { return m_renderBlockHasMarginAfterQuirk; }
    bool renderBlockShouldForceRelayoutChildren() const { return m_renderBlockShouldForceRelayoutChildren; }

    void setRenderBlockFlowLineLayoutPath(unsigned u) { m_renderBlockFlowLineLayoutPath = u; }
    void setRenderBlockFlowHasMarkupTruncation(bool b) { m_renderBlockFlowHasMarkupTruncation = b; }
    unsigned renderBlockFlowLineLayoutPath() const { return m_renderBlockFlowLineLayoutPath; }
    bool renderBlockFlowHasMarkupTruncation() const { return m_renderBlockFlowHasMarkupTruncation; }

    void paintFocusRing(PaintInfo&, const RenderStyle&, const Vector<LayoutRect>& focusRingRects);
    void paintOutline(PaintInfo&, const LayoutRect&);
    void updateOutlineAutoAncestor(bool hasOutlineAuto);

    void removeFromRenderFragmentedFlowIncludingDescendants(bool shouldUpdateState);
    void adjustFragmentedFlowStateOnContainingBlockChangeIfNeeded();
    
    bool noLongerAffectsParentBlock() const { return s_noLongerAffectsParentBlock; }
    bool isVisibleInViewport() const;

private:
    void node() const = delete;
    void nonPseudoNode() const = delete;
    void generatingNode() const = delete;
    void isText() const = delete;
    //void isRenderElement() const = delete;

    RenderObject* firstChildSlow() const final { return firstChild(); }
    RenderObject* lastChildSlow() const final { return lastChild(); }

    // Called when an object that was floating or positioned becomes a normal flow object
    // again.  We have to make sure the render tree updates as needed to accommodate the new
    // normal flow object.
    void handleDynamicFloatPositionChange();

    bool shouldRepaintForStyleDifference(StyleDifference) const;
    bool hasImmediateNonWhitespaceTextChildOrBorderOrOutline() const;


    StyleDifference adjustStyleDifference(StyleDifference, unsigned contextSensitiveProperties) const;
    std::unique_ptr<RenderStyle> computeFirstLineStyle() const;
    void invalidateCachedFirstLineStyle();

    bool canDestroyDecodedData() { return !isVisibleInViewport(); }
    //VisibleInViewportState imageFrameAvailable(CachedImage&, ImageAnimatingState, const IntRect* changeRect) final;
    //void didRemoveCachedImageClient(CachedImage&) final;

    bool getLeadingCorner(FloatPoint& output, bool& insideFixed) const;
    bool getTrailingCorner(FloatPoint& output, bool& insideFixed) const;

    void clearSubtreeLayoutRootIfNeeded() const;
    
    bool shouldWillChangeCreateStackingContext() const;
    void issueRepaintForOutlineAuto(float outlineSize);

    unsigned m_baseTypeFlags : 6;
    unsigned m_ancestorLineBoxDirty : 1;
    unsigned m_hasInitializedStyle : 1;

    unsigned m_renderInlineAlwaysCreatesLineBoxes : 1;
    unsigned m_renderBoxNeedsLazyRepaint : 1;
    unsigned m_hasPausedImageAnimations : 1;
    unsigned m_hasCounterNodeMap : 1;
    unsigned m_hasContinuationChainNode : 1;
    unsigned m_isContinuation : 1;
    unsigned m_isFirstLetter : 1;
    mutable unsigned m_hasValidCachedFirstLineStyle : 1;

    unsigned m_renderBlockHasMarginBeforeQuirk : 1;
    unsigned m_renderBlockHasMarginAfterQuirk : 1;
    unsigned m_renderBlockShouldForceRelayoutChildren : 1;
    unsigned m_renderBlockFlowHasMarkupTruncation : 1;
    unsigned m_renderBlockFlowLineLayoutPath : 2;

    unsigned m_isRegisteredForVisibleInViewportCallback : 1;
    unsigned m_visibleInViewportState : 2;

    RenderObject* m_firstChild;
    RenderObject* m_lastChild;

    RenderStyle m_style;

    // FIXME: Get rid of this hack.
    // Store state between styleWillChange and styleDidChange
    static bool s_affectsParentBlock;
    static bool s_noLongerAffectsParentBlock;
};

inline void RenderElement::setAncestorLineBoxDirty(bool f)
{
    m_ancestorLineBoxDirty = f;
    if (m_ancestorLineBoxDirty)
        setNeedsLayout();
}

inline void RenderElement::setChildNeedsLayout(MarkingBehavior markParents)
{
    ASSERT(!isSetNeedsLayoutForbidden());
    if (normalChildNeedsLayout())
        return;
    setNormalChildNeedsLayoutBit(true);
    if (markParents == MarkContainingBlockChain)
        markContainingBlocksForLayout();
}

inline LayoutUnit RenderElement::valueForLength(const Length& length, LayoutUnit maximumValue) const
{
    return WebCore::valueForLength(length, maximumValue);
}

inline LayoutUnit RenderElement::minimumValueForLength(const Length& length, LayoutUnit maximumValue) const
{
    return WebCore::minimumValueForLength(length, maximumValue);
}

inline bool RenderElement::isRenderElement() const
{
    return m_baseTypeFlags & RenderElementFlag;
}

inline bool RenderElement::isBoxModelObject() const
{
    return m_baseTypeFlags & RenderBoxModelObjectFlag;
}

inline bool RenderElement::isRenderBlock() const
{
    return m_baseTypeFlags & RenderBlockFlag;
}

inline bool RenderElement::isRenderBlockFlow() const
{
    return m_baseTypeFlags & RenderBlockFlowFlag;
}

inline bool RenderElement::isRenderReplaced() const
{
    return m_baseTypeFlags & RenderReplacedFlag;
}

inline bool RenderElement::isRenderInline() const
{
    return m_baseTypeFlags & RenderInlineFlag;
}

inline bool RenderElement::canContainFixedPositionObjects() const
{
    return isRenderView()
        || (hasTransform() && isRenderBlock())
        || isSVGForeignObject()
        || isOutOfFlowRenderFragmentedFlow();
}

inline bool RenderElement::canContainAbsolutelyPositionedObjects() const
{
    return style().position() != StaticPosition
        || (isRenderBlock() && hasTransformRelatedProperty())
        || isSVGForeignObject()
        || isRenderView();
}

inline bool RenderElement::createsGroupForStyle(const RenderStyle& style)
{
    return style.opacity() < 1.0f  || style.hasBlendMode();
}


inline bool RenderObject::isBoxModelObject() const
{
    return is<RenderElement>(*this) && downcast<RenderElement>(*this).isBoxModelObject();
}

inline bool RenderObject::isRenderBlock() const
{
    return is<RenderElement>(*this) && downcast<RenderElement>(*this).isRenderBlock();
}

inline bool RenderObject::isRenderBlockFlow() const
{
    return is<RenderElement>(*this) && downcast<RenderElement>(*this).isRenderBlockFlow();
}

inline bool RenderObject::isRenderReplaced() const
{
    return is<RenderElement>(*this) && downcast<RenderElement>(*this).isRenderReplaced();
}

inline bool RenderObject::isRenderInline() const
{
    return is<RenderElement>(*this) && downcast<RenderElement>(*this).isRenderInline();
}

inline const RenderStyle& RenderObject::style() const
{
    if (isText())
        return m_parent->style();
    return downcast<RenderElement>(*this).style();
}

inline const RenderStyle& RenderObject::firstLineStyle() const
{
    if (isText())
        return m_parent->firstLineStyle();
    return downcast<RenderElement>(*this).firstLineStyle();
}



inline int adjustForAbsoluteZoom(int value, const RenderElement& renderer)
{
    return adjustForAbsoluteZoom(value, renderer.style());
}

inline LayoutUnit adjustLayoutUnitForAbsoluteZoom(LayoutUnit value, const RenderElement& renderer)
{
    return adjustLayoutUnitForAbsoluteZoom(value, renderer.style());
}

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_RENDER_OBJECT(RenderElement, isRenderElement())
