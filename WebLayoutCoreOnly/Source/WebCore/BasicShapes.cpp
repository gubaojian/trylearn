/*
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include "BasicShapes.h"

#include "BasicShapeFunctions.h"
#include "CalculationValue.h"
#include "FloatRect.h"
#include "FloatRoundedRect.h"
#include "LengthFunctions.h"
#include "Path.h"
//#include "RenderBox.h"

#include <wtf/NeverDestroyed.h>
#include <wtf/TinyLRUCache.h>

namespace WebCore {

void BasicShapeCenterCoordinate::updateComputedLength()
{
    if (m_direction == TopLeft) {
        m_computedLength = m_length.isUndefined() ? Length(0, Fixed) : m_length;
        return;
    }

    if (m_length.isUndefined()) {
        m_computedLength = Length(100, Percent);
        return;
    }
    
    m_computedLength = convertTo100PercentMinusLength(m_length);
}



struct EllipsePathPolicy : public TinyLRUCachePolicy<FloatRect, Path> {
public:
    static bool isKeyNull(const FloatRect& rect) { return rect.isEmpty(); }

    static Path createValueForKey(const FloatRect& rect)
    {
        Path path;
        path.addEllipse(rect);
        return path;
    }
};

struct RoundedRectPathPolicy : public TinyLRUCachePolicy<FloatRoundedRect, Path> {
public:
    static bool isKeyNull(const FloatRoundedRect& rect) { return rect.isEmpty(); }

    static Path createValueForKey(const FloatRoundedRect& rect)
    {
        Path path;
        path.addRoundedRect(rect);
        return path;
    }
};

struct PolygonPathPolicy : public TinyLRUCachePolicy<Vector<FloatPoint>, Path> {
public:
    static bool isKeyNull(const Vector<FloatPoint>& points) { return !points.size(); }

    static Path createValueForKey(const Vector<FloatPoint>& points) { return Path::polygonPathFromPoints(points); }
};



static const Path& cachedEllipsePath(const FloatRect& rect)
{
    static NeverDestroyed<TinyLRUCache<FloatRect, Path, 4, EllipsePathPolicy>> cache;
    return cache.get().get(rect);
}

static const Path& cachedRoundedRectPath(const FloatRoundedRect& rect)
{
    static NeverDestroyed<TinyLRUCache<FloatRoundedRect, Path, 4, RoundedRectPathPolicy>> cache;
    return cache.get().get(rect);
}

static const Path& cachedPolygonPath(const Vector<FloatPoint>& points)
{
    static NeverDestroyed<TinyLRUCache<Vector<FloatPoint>, Path, 4, PolygonPathPolicy>> cache;
    return cache.get().get(points);
}




}
