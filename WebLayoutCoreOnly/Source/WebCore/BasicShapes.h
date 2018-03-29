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

#pragma once

#include "Length.h"
#include "LengthSize.h"
#include "WindRule.h"
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/TypeCasts.h>
#include <wtf/Vector.h>

namespace WebCore {

class FloatRect;
class Path;
class RenderBox;

class BasicShape : public RefCounted<BasicShape> {
public:
    virtual ~BasicShape() = default;

    enum Type {
        BasicShapePolygonType,
        BasicShapePathType,
        BasicShapeCircleType,
        BasicShapeEllipseType,
        BasicShapeInsetType
    };

    virtual Type type() const = 0;

    virtual const Path& path(const FloatRect&) = 0;
    virtual WindRule windRule() const { return RULE_NONZERO; }

    virtual bool canBlend(const BasicShape&) const = 0;
    virtual Ref<BasicShape> blend(const BasicShape& from, double) const = 0;

    virtual bool operator==(const BasicShape&) const = 0;
};

class BasicShapeCenterCoordinate {
public:
    enum Direction {
        TopLeft,
        BottomRight
    };

    BasicShapeCenterCoordinate()
        : m_direction(TopLeft)
        , m_length(Undefined)
    {
        updateComputedLength();
    }

    BasicShapeCenterCoordinate(Direction direction, Length length)
        : m_direction(direction)
        , m_length(length)
    {
        updateComputedLength();
    }

    BasicShapeCenterCoordinate(const BasicShapeCenterCoordinate& other)
        : m_direction(other.direction())
        , m_length(other.length())
        , m_computedLength(other.m_computedLength)
    {
    }

    Direction direction() const { return m_direction; }
    const Length& length() const { return m_length; }
    const Length& computedLength() const { return m_computedLength; }

    BasicShapeCenterCoordinate blend(const BasicShapeCenterCoordinate& from, double progress) const
    {
        return BasicShapeCenterCoordinate(TopLeft, WebCore::blend(from.m_computedLength, m_computedLength, progress));
    }
    
    bool operator==(const BasicShapeCenterCoordinate& other) const
    {
        return m_direction == other.m_direction
            && m_length == other.m_length
            && m_computedLength == other.m_computedLength;
    }

private:
    void updateComputedLength();

    Direction m_direction;
    Length m_length;
    Length m_computedLength;
};





} // namespace WebCore

#define SPECIALIZE_TYPE_TRAITS_BASIC_SHAPE(ToValueTypeName, predicate) \


SPECIALIZE_TYPE_TRAITS_BASIC_SHAPE(BasicShapeCircle, BasicShape::BasicShapeCircleType)
SPECIALIZE_TYPE_TRAITS_BASIC_SHAPE(BasicShapeEllipse, BasicShape::BasicShapeEllipseType)
SPECIALIZE_TYPE_TRAITS_BASIC_SHAPE(BasicShapePolygon, BasicShape::BasicShapePolygonType)
SPECIALIZE_TYPE_TRAITS_BASIC_SHAPE(BasicShapePath, BasicShape::BasicShapePathType)
SPECIALIZE_TYPE_TRAITS_BASIC_SHAPE(BasicShapeInset, BasicShape::BasicShapeInsetType)
