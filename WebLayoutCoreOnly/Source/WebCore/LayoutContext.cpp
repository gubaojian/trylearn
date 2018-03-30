/*
 * Copyright (C) 2017 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "LayoutContext.h"

#include "LayoutState.h"
#include "RenderElement.h"

#include <wtf/SetForScope.h>
#include <wtf/SystemTracing.h>

namespace WebCore {

static bool isObjectAncestorContainerOf(RenderElement& ancestor, RenderElement& descendant)
{
    for (auto* renderer = &descendant; renderer; renderer = renderer->container()) {
        if (renderer == &ancestor)
            return true;
    }
    return false;
}

#ifndef NDEBUG
class RenderTreeNeedsLayoutChecker {
public :
    RenderTreeNeedsLayoutChecker(const RenderElement& layoutRoot)
        : m_layoutRoot(layoutRoot)
    {
    }

    ~RenderTreeNeedsLayoutChecker()
    {
        auto reportNeedsLayoutError = [] (const RenderObject& renderer) {
            WTFReportError(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, "post-layout: dirty renderer(s)");
            renderer.showRenderTreeForThis();
            ASSERT_NOT_REACHED();
        };

        if (m_layoutRoot.needsLayout()) {
            reportNeedsLayoutError(m_layoutRoot);
            return;
        }

        for (auto* descendant = m_layoutRoot.firstChild(); descendant; descendant = descendant->nextInPreOrder(&m_layoutRoot)) {
            if (!descendant->needsLayout())
                continue;
            
            reportNeedsLayoutError(*descendant);
            return;
        }
    }

private:
    const RenderElement& m_layoutRoot;
};
#endif

class LayoutScope {
public:
    LayoutScope(LayoutContext& layoutContext)
        : m_nestedState(layoutContext.m_layoutNestedState, layoutContext.m_layoutNestedState == LayoutContext::LayoutNestedState::NotInLayout ? LayoutContext::LayoutNestedState::NotNested : LayoutContext::LayoutNestedState::Nested)
        , m_schedulingIsEnabled(layoutContext.m_layoutSchedulingIsEnabled, false)
    {
        //m_view.setInProgrammaticScroll(true);
    }
        
    ~LayoutScope()
    {
        //m_view.setInProgrammaticScroll(m_inProgrammaticScroll);
    }
        
private:
    SetForScope<LayoutContext::LayoutNestedState> m_nestedState;
    SetForScope<bool> m_schedulingIsEnabled;
    bool m_inProgrammaticScroll { false };
};

LayoutContext::LayoutContext()
{
}

void LayoutContext::layout()
{
    /**
    ASSERT(!view().isPainting());
    ASSERT(frame().view() == &view());
    ASSERT(frame().document());
    ASSERT(frame().document()->pageCacheState() == Document::NotInPageCache);
    if (!canPerformLayout()) {
        LOG(Layout, "  is not allowed, bailing");
        return;
    }

    Ref<FrameView> protectView(view());
    LayoutScope layoutScope(*this);
    TraceScope tracingScope(LayoutStart, LayoutEnd);
    InspectorInstrumentationCookie inspectorLayoutScope(InspectorInstrumentation::willLayout(view().frame()));
    AnimationUpdateBlock animationUpdateBlock(&view().frame().animation());
    WeakPtr<RenderElement> layoutRoot;
    
    m_layoutTimer.stop();
    m_delayedLayout = false;
    m_setNeedsLayoutWasDeferred = false;

#if !LOG_DISABLED
    if (m_firstLayout && !frame().ownerElement())
        LOG(Layout, "FrameView %p elapsed time before first layout: %.3fs\n", this, document()->timeSinceDocumentCreation().value());
#endif
#if PLATFORM(IOS)
    if (view().updateFixedPositionLayoutRect() && subtreeLayoutRoot())
        convertSubtreeLayoutToFullLayout();
#endif
    if (handleLayoutWithFrameFlatteningIfNeeded())
        return;

    {
        SetForScope<LayoutPhase> layoutPhase(m_layoutPhase, LayoutPhase::InPreLayout);

        // If this is a new top-level layout and there are any remaining tasks from the previous layout, finish them now.
        if (!isLayoutNested() && m_asynchronousTasksTimer.isActive() && !view().isInChildFrameWithFrameFlattening())
            runAsynchronousTasks();

        updateStyleForLayout();
        if (view().hasOneRef())
            return;

        view().autoSizeIfEnabled();
        if (!renderView())
            return;

        layoutRoot = makeWeakPtr(subtreeLayoutRoot() ? subtreeLayoutRoot() : renderView());
        m_needsFullRepaint = is<RenderView>(layoutRoot.get()) && (m_firstLayout || renderView()->printing());
        view().willDoLayout(layoutRoot);
        m_firstLayout = false;
    }
    {
        SetForScope<LayoutPhase> layoutPhase(m_layoutPhase, LayoutPhase::InRenderTreeLayout);
        NoEventDispatchAssertion::InMainThread noEventDispatchAssertion;
        SubtreeLayoutStateMaintainer subtreeLayoutStateMaintainer(subtreeLayoutRoot());
        RenderView::RepaintRegionAccumulator repaintRegionAccumulator(renderView());
#ifndef NDEBUG
        RenderTreeNeedsLayoutChecker checker(*layoutRoot);
#endif
        layoutRoot->layout();
        ++m_layoutCount;
#if ENABLE(TEXT_AUTOSIZING)
        applyTextSizingIfNeeded(*layoutRoot.get());
#endif
        clearSubtreeLayoutRoot();
    }
    {
        SetForScope<LayoutPhase> layoutPhase(m_layoutPhase, LayoutPhase::InViewSizeAdjust);
        if (is<RenderView>(layoutRoot.get()) && !renderView()->printing()) {
            // This is to protect m_needsFullRepaint's value when layout() is getting re-entered through adjustViewSize().
            SetForScope<bool> needsFullRepaint(m_needsFullRepaint);
            view().adjustViewSize();
            // FIXME: Firing media query callbacks synchronously on nested frames could produced a detached FrameView here by
            // navigating away from the current document (see webkit.org/b/173329).
            if (view().hasOneRef())
                return;
        }
    }
    {
        SetForScope<LayoutPhase> layoutPhase(m_layoutPhase, LayoutPhase::InPostLayout);
        if (m_needsFullRepaint)
            renderView()->repaintRootContents();
        ASSERT(!layoutRoot->needsLayout());
        view().didLayout(layoutRoot);
        runOrScheduleAsynchronousTasks();
    }
    InspectorInstrumentation::didLayout(inspectorLayoutScope, *layoutRoot);
    DebugPageOverlays::didLayout(view().frame());
     */
}

void LayoutContext::reset()
{
    m_layoutPhase = LayoutPhase::OutsideLayout;
    clearSubtreeLayoutRoot();
    m_layoutCount = 0;
    m_layoutSchedulingIsEnabled = true;
    m_delayedLayout = false;
    m_firstLayout = true;
    m_needsFullRepaint = true;
}

bool LayoutContext::needsLayout() const
{
    /**
    // This can return true in cases where the document does not have a body yet.
    // Document::shouldScheduleLayout takes care of preventing us from scheduling
    // layout in that case.
    auto* renderView = this->renderView();
    return isLayoutPending()
        || (renderView && renderView->needsLayout())
        || subtreeLayoutRoot()
        || (m_disableSetNeedsLayoutCount && m_setNeedsLayoutWasDeferred);
        */
   return false;
}

void LayoutContext::setNeedsLayout()
{
    if (m_disableSetNeedsLayoutCount) {
        m_setNeedsLayoutWasDeferred = true;
        return;
    }
}

void LayoutContext::enableSetNeedsLayout()
{
    ASSERT(m_disableSetNeedsLayoutCount);
    if (!--m_disableSetNeedsLayoutCount)
        m_setNeedsLayoutWasDeferred = false; // FIXME: Find a way to make the deferred layout actually happen.
}

void LayoutContext::disableSetNeedsLayout()
{
    ++m_disableSetNeedsLayoutCount;
}

void LayoutContext::scheduleLayout()
{
    // FIXME: We should assert the page is not in the page cache, but that is causing
    // too many false assertions. See <rdar://problem/7218118>.

    if (subtreeLayoutRoot())
        convertSubtreeLayoutToFullLayout();
    if (!isLayoutSchedulingEnabled())
        return;
    if (!needsLayout())
        return;
}

void LayoutContext::unscheduleLayout()
{

    m_delayedLayout = false;
}

void LayoutContext::scheduleSubtreeLayout(RenderElement& layoutRoot)
{

}

void LayoutContext::layoutTimerFired()
{
    layout();
}

void LayoutContext::convertSubtreeLayoutToFullLayout()
{
    ASSERT(subtreeLayoutRoot());
    subtreeLayoutRoot()->markContainingBlocksForLayout(ScheduleRelayout::No);
    clearSubtreeLayoutRoot();
}

void LayoutContext::setSubtreeLayoutRoot(RenderElement& layoutRoot)
{
    m_subtreeLayoutRoot = makeWeakPtr(layoutRoot);
}

bool LayoutContext::canPerformLayout() const
{
    if (isInRenderTreeLayout())
        return false;

    if (layoutDisallowed())
        return false;


    if (!subtreeLayoutRoot())
        return false;

    return true;
}

#if ENABLE(TEXT_AUTOSIZING)
void LayoutContext::applyTextSizingIfNeeded(RenderElement& layoutRoot)
{
    auto& settings = layoutRoot.settings();
    if (!settings.textAutosizingEnabled() || renderView()->printing())
        return;
    auto minimumZoomFontSize = settings.minimumZoomFontSize();
    if (!minimumZoomFontSize)
        return;
    auto textAutosizingWidth = layoutRoot.page().textAutosizingWidth();
    if (auto overrideWidth = settings.textAutosizingWindowSizeOverride().width())
        textAutosizingWidth = overrideWidth;
    if (!textAutosizingWidth)
        return;
    layoutRoot.adjustComputedFontSizesOnBlocks(minimumZoomFontSize, textAutosizingWidth);
    if (!layoutRoot.needsLayout())
        return;
    LOG(TextAutosizing, "Text Autosizing: minimumZoomFontSize=%.2f textAutosizingWidth=%.2f", minimumZoomFontSize, textAutosizingWidth);
    layoutRoot.layout();
}
#endif

void LayoutContext::updateStyleForLayout()
{
}

bool LayoutContext::handleLayoutWithFrameFlatteningIfNeeded()
{
    return true;
}

void LayoutContext::startLayoutAtMainFrameViewIfNeeded()
{

}

LayoutSize LayoutContext::layoutDelta() const
{
    if (auto* layoutState = this->layoutState())
        return layoutState->layoutDelta();
    return { };
}
    
void LayoutContext::addLayoutDelta(const LayoutSize& delta)
{
    if (auto* layoutState = this->layoutState())
        layoutState->addLayoutDelta(delta);
}
    
#if !ASSERT_DISABLED
bool LayoutContext::layoutDeltaMatches(const LayoutSize& delta)
{
    if (auto* layoutState = this->layoutState())
        return layoutState->layoutDeltaMatches(delta);
    return false;
}
#endif

LayoutState* LayoutContext::layoutState() const
{
    if (m_layoutStateStack.isEmpty())
        return nullptr;
    return m_layoutStateStack.last().get();
}

void LayoutContext::pushLayoutState(RenderElement& root)
{
    ASSERT(!m_paintOffsetCacheDisableCount);
    ASSERT(!layoutState());

    m_layoutStateStack.append(std::make_unique<LayoutState>(root));
}

bool LayoutContext::pushLayoutStateForPaginationIfNeeded(RenderBlockFlow& layoutRoot)
{
    if (layoutState())
        return false;
    LayoutUnit unit;
    //m_layoutStateStack.append(std::make_unique<LayoutState>((RenderBox&)layoutRoot, LayoutState::IsPaginated::Yes, unit, false));
    return true;
}
    
bool LayoutContext::pushLayoutState(RenderBox& renderer, const LayoutSize& offset, LayoutUnit pageHeight, bool pageHeightChanged)
{
    // We push LayoutState even if layoutState is disabled because it stores layoutDelta too.
    auto* layoutState = this->layoutState();

    return false;
}
    
void LayoutContext::popLayoutState()
{
    m_layoutStateStack.removeLast();
}
    
#ifndef NDEBUG
void LayoutContext::checkLayoutState()
{
    ASSERT(layoutDeltaMatches(LayoutSize()));
    ASSERT(!m_paintOffsetCacheDisableCount);
}
#endif




} // namespace WebCore
