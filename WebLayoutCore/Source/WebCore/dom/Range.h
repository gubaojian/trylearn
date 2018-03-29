/*
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Gunnstein Lye (gunnstein@netcom.no)
 * (C) 2000 Frederik Holljen (frederik.holljen@hig.no)
 * (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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

#pragma once

#include "FloatRect.h"
#include "IntRect.h"
#include "RangeBoundaryPoint.h"
#include <wtf/Forward.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>

namespace WebCore {

class ContainerNode;
class DOMRect;
class DOMRectList;
class Document;
class DocumentFragment;
class FloatQuad;
class Node;
class NodeWithIndex;
class RenderText;
class SelectionRect;
class Text;
class VisiblePosition;

class Range : public RefCounted<Range> {
public:
     static Ref<Range> create(Document&);
     static Ref<Range> create(Document&, RefPtr<Node>&& startContainer, int startOffset, RefPtr<Node>&& endContainer, int endOffset);
     static Ref<Range> create(Document&, const Position&, const Position&);
     static Ref<Range> create(Document&, const VisiblePosition&, const VisiblePosition&);
     ~Range();

    Document& ownerDocument() const { return m_ownerDocument; }

    Node& startContainer() const { ASSERT(m_start.container()); return *m_start.container(); }
    unsigned startOffset() const { return m_start.offset(); }
    Node& endContainer() const { ASSERT(m_end.container()); return *m_end.container(); }
    unsigned endOffset() const { return m_end.offset(); }
    bool collapsed() const { return m_start == m_end; }

    Node* commonAncestorContainer() const { return commonAncestorContainer(&startContainer(), &endContainer()); }
     static Node* commonAncestorContainer(Node* containerA, Node* containerB);
     ExceptionOr<void> setStart(Ref<Node>&& container, unsigned offset);
     ExceptionOr<void> setEnd(Ref<Node>&& container, unsigned offset);
     void collapse(bool toStart);
     ExceptionOr<bool> isPointInRange(Node& refNode, unsigned offset);
     ExceptionOr<short> comparePoint(Node& refNode, unsigned offset) const;
    enum CompareResults { NODE_BEFORE, NODE_AFTER, NODE_BEFORE_AND_AFTER, NODE_INSIDE };
     ExceptionOr<CompareResults> compareNode(Node& refNode) const;
    enum CompareHow { START_TO_START, START_TO_END, END_TO_END, END_TO_START };
     ExceptionOr<short> compareBoundaryPoints(CompareHow, const Range& sourceRange) const;
     ExceptionOr<short> compareBoundaryPointsForBindings(unsigned short compareHow, const Range& sourceRange) const;
    static ExceptionOr<short> compareBoundaryPoints(Node* containerA, unsigned offsetA, Node* containerB, unsigned offsetB);
    static ExceptionOr<short> compareBoundaryPoints(const RangeBoundaryPoint& boundaryA, const RangeBoundaryPoint& boundaryB);
     bool boundaryPointsValid() const;
     ExceptionOr<bool> intersectsNode(Node& refNode) const;
     ExceptionOr<void> deleteContents();
     ExceptionOr<Ref<DocumentFragment>> extractContents();
     ExceptionOr<Ref<DocumentFragment>> cloneContents();
     ExceptionOr<void> insertNode(Ref<Node>&&);
     String toString() const;

    String toHTML() const;
     String text() const;

     ExceptionOr<Ref<DocumentFragment>> createContextualFragment(const String& html);

     void detach();
     Ref<Range> cloneRange() const;

     ExceptionOr<void> setStartAfter(Node&);
     ExceptionOr<void> setEndBefore(Node&);
     ExceptionOr<void> setEndAfter(Node&);
     ExceptionOr<void> selectNode(Node&);
     ExceptionOr<void> selectNodeContents(Node&);
     ExceptionOr<void> surroundContents(Node&);
     ExceptionOr<void> setStartBefore(Node&);

    const Position startPosition() const { return m_start.toPosition(); }
    const Position endPosition() const { return m_end.toPosition(); }
     ExceptionOr<void> setStart(const Position&);
     ExceptionOr<void> setEnd(const Position&);

     Node* firstNode() const;
     Node* pastLastNode() const;

    ShadowRoot* shadowRoot() const;

    enum RangeInFixedPosition {
        NotFixedPosition,
        PartiallyFixedPosition,
        EntirelyFixedPosition
    };

    // Not transform-friendly
    enum class RespectClippingForTextRects { No, Yes };
     void absoluteTextRects(Vector<IntRect>&, bool useSelectionHeight = false, RangeInFixedPosition* = nullptr, RespectClippingForTextRects = RespectClippingForTextRects::No) const;
     IntRect absoluteBoundingBox() const;

    // Transform-friendly
     void absoluteTextQuads(Vector<FloatQuad>&, bool useSelectionHeight = false, RangeInFixedPosition* = nullptr) const;
     FloatRect absoluteBoundingRect(RespectClippingForTextRects = RespectClippingForTextRects::No) const;
#if PLATFORM(IOS)
     void collectSelectionRects(Vector<SelectionRect>&) const;
     int collectSelectionRectsWithoutUnionInteriorLines(Vector<SelectionRect>&) const;
#endif

    void nodeChildrenChanged(ContainerNode&);
    void nodeChildrenWillBeRemoved(ContainerNode&);
    void nodeWillBeRemoved(Node&);

    void textInserted(Node*, unsigned offset, unsigned length);
    void textRemoved(Node*, unsigned offset, unsigned length);
    void textNodesMerged(NodeWithIndex& oldNode, unsigned offset);
    void textNodeSplit(Text* oldNode);

    // Expand range to a unit (word or sentence or block or document) boundary.
    // Please refer to https://bugs.webkit.org/show_bug.cgi?id=27632 comment #5 
    // for details.
     ExceptionOr<void> expand(const String&);

    Ref<DOMRectList> getClientRects() const;
    Ref<DOMRect> getBoundingClientRect() const;

#if ENABLE(TREE_DEBUGGING)
    void formatForDebugger(char* buffer, unsigned length) const;
#endif

     bool contains(const Range&) const;
    bool contains(const VisiblePosition&) const;

    enum ActionType { Delete, Extract, Clone };

private:
    explicit Range(Document&);
    Range(Document&, Node* startContainer, int startOffset, Node* endContainer, int endOffset);

    void setDocument(Document&);
    ExceptionOr<Node*> checkNodeWOffset(Node&, unsigned offset) const;
    ExceptionOr<RefPtr<DocumentFragment>> processContents(ActionType);

    enum class CoordinateSpace { Absolute, Client };
    Vector<FloatRect> borderAndTextRects(CoordinateSpace, RespectClippingForTextRects = RespectClippingForTextRects::No) const;
    FloatRect boundingRect(CoordinateSpace, RespectClippingForTextRects = RespectClippingForTextRects::No) const;

    Vector<FloatRect> absoluteRectsForRangeInText(Node*, RenderText&, bool useSelectionHeight, bool& isFixed, RespectClippingForTextRects) const;

    Ref<Document> m_ownerDocument;
    RangeBoundaryPoint m_start;
    RangeBoundaryPoint m_end;
};

 Ref<Range> rangeOfContents(Node&);

 bool areRangesEqual(const Range*, const Range*);
bool rangesOverlap(const Range*, const Range*);

inline bool documentOrderComparator(const Node* a, const Node* b)
{
    return Range::compareBoundaryPoints(const_cast<Node*>(a), 0, const_cast<Node*>(b), 0).releaseReturnValue() < 0;
}
    
WTF::TextStream& operator<<(WTF::TextStream&, const RangeBoundaryPoint&);
WTF::TextStream& operator<<(WTF::TextStream&, const Range&);

} // namespace

#if ENABLE(TREE_DEBUGGING)
// Outside the WebCore namespace for ease of invocation from the debugger.
void showTree(const WebCore::Range*);
#endif
