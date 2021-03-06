/*
 * Copyright (C) 2006 Apple Inc.  All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008, 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (C) 2009 Brent Fulgham <bfulgham@webkit.org>
 * Copyright (C) 2010, 2011 Igalia S.L.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2012, Intel Corporation
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
#include "CairoOperations.h"

#if USE(CAIRO)

#include "DrawErrorUnderline.h"
#include "FloatConversion.h"
#include "FloatRect.h"
#include "GraphicsContext.h"
#include "GraphicsContextPlatformPrivateCairo.h"
#include "Image.h"
#include "Path.h"
#include "PlatformContextCairo.h"
#include "PlatformPathCairo.h"
#include <algorithm>
#include <cairo.h>

namespace WebCore {
namespace Cairo {

static inline void fillRectWithColor(cairo_t* cr, const FloatRect& rect, const Color& color)
{
    if (!color.isVisible() && cairo_get_operator(cr) == CAIRO_OPERATOR_OVER)
        return;

    setSourceRGBAFromColor(cr, color);
    cairo_rectangle(cr, rect.x(), rect.y(), rect.width(), rect.height());
    cairo_fill(cr);
}

enum PathDrawingStyle {
    Fill = 1,
    Stroke = 2,
    FillAndStroke = Fill + Stroke
};

static inline void drawPathShadow(PlatformContextCairo& platformContext, const GraphicsContextState& contextState, GraphicsContext& targetContext, PathDrawingStyle drawingStyle)
{
    ShadowBlur& shadow = platformContext.shadowBlur();
    if (shadow.type() == ShadowBlur::NoShadow)
        return;

    // Calculate the extents of the rendered solid paths.
    cairo_t* cairoContext = platformContext.cr();
    std::unique_ptr<cairo_path_t, void(*)(cairo_path_t*)> path(cairo_copy_path(cairoContext), [](cairo_path_t* path) {
        cairo_path_destroy(path);
    });

    FloatRect solidFigureExtents;
    double x0 = 0;
    double x1 = 0;
    double y0 = 0;
    double y1 = 0;
    if (drawingStyle & Stroke) {
        cairo_stroke_extents(cairoContext, &x0, &y0, &x1, &y1);
        solidFigureExtents = FloatRect(x0, y0, x1 - x0, y1 - y0);
    }
    if (drawingStyle & Fill) {
        cairo_fill_extents(cairoContext, &x0, &y0, &x1, &y1);
        FloatRect fillExtents(x0, y0, x1 - x0, y1 - y0);
        solidFigureExtents.unite(fillExtents);
    }

    GraphicsContext* shadowContext = shadow.beginShadowLayer(targetContext, solidFigureExtents);
    if (!shadowContext)
        return;

    cairo_t* cairoShadowContext = shadowContext->platformContext()->cr();

    // It's important to copy the context properties to the new shadow
    // context to preserve things such as the fill rule and stroke width.
    copyContextProperties(cairoContext, cairoShadowContext);

    if (drawingStyle & Fill) {
        cairo_save(cairoShadowContext);
        cairo_append_path(cairoShadowContext, path.get());
        shadowContext->platformContext()->prepareForFilling(contextState, PlatformContextCairo::NoAdjustment);
        cairo_fill(cairoShadowContext);
        cairo_restore(cairoShadowContext);
    }

    if (drawingStyle & Stroke) {
        cairo_append_path(cairoShadowContext, path.get());
        shadowContext->platformContext()->prepareForStroking(contextState, PlatformContextCairo::DoNotPreserveAlpha);
        cairo_stroke(cairoShadowContext);
    }

    // The original path may still be hanging around on the context and endShadowLayer
    // will take care of properly creating a path to draw the result shadow. We remove the path
    // temporarily and then restore it.
    // See: https://bugs.webkit.org/show_bug.cgi?id=108897
    cairo_new_path(cairoContext);
    shadow.endShadowLayer(targetContext);
    cairo_append_path(cairoContext, path.get());
}

static inline void fillCurrentCairoPath(PlatformContextCairo& platformContext, const GraphicsContextState& contextState)
{
    cairo_t* cr = platformContext.cr();
    cairo_save(cr);

    platformContext.prepareForFilling(contextState, PlatformContextCairo::AdjustPatternForGlobalAlpha);
    cairo_fill(cr);

    cairo_restore(cr);
}

static inline void adjustFocusRingColor(Color& color)
{
#if !PLATFORM(GTK)
    // Force the alpha to 50%. This matches what the Mac does with outline rings.
    color = Color(makeRGBA(color.red(), color.green(), color.blue(), 127));
#else
    UNUSED_PARAM(color);
#endif
}

static inline void adjustFocusRingLineWidth(float& width)
{
#if PLATFORM(GTK)
    width = 2;
#else
    UNUSED_PARAM(width);
#endif
}

static inline StrokeStyle focusRingStrokeStyle()
{
#if PLATFORM(GTK)
    return DottedStroke;
#else
    return SolidStroke;
#endif
}

static bool mustUseShadowBlur(PlatformContextCairo& platformContext, const GraphicsContextState& state)
{
    // We can't avoid ShadowBlur if the shadow has blur.
    if (state.shadowColor.isVisible() && state.shadowBlur)
        return true;

    // We can avoid ShadowBlur and optimize, since we're not drawing on a
    // canvas and box shadows are affected by the transformation matrix.
    if (!state.shadowsIgnoreTransforms)
        return false;

    // We can avoid ShadowBlur, since there are no transformations to apply to the canvas.
    if (State::getCTM(platformContext).isIdentity())
        return false;

    // Otherwise, no chance avoiding ShadowBlur.
    return true;
}

static void drawGlyphsToContext(cairo_t* context, cairo_scaled_font_t* scaledFont, double syntheticBoldOffset, const Vector<cairo_glyph_t>& glyphs)
{
    cairo_matrix_t originalTransform;
    if (syntheticBoldOffset)
        cairo_get_matrix(context, &originalTransform);

    cairo_set_scaled_font(context, scaledFont);
    cairo_show_glyphs(context, glyphs.data(), glyphs.size());

    if (syntheticBoldOffset) {
        cairo_translate(context, syntheticBoldOffset, 0);
        cairo_show_glyphs(context, glyphs.data(), glyphs.size());

        cairo_set_matrix(context, &originalTransform);
    }
}

static void drawGlyphsShadow(PlatformContextCairo& platformContext, const GraphicsContextState& state, const FloatPoint& point, cairo_scaled_font_t* scaledFont, double syntheticBoldOffset, const Vector<cairo_glyph_t>& glyphs, GraphicsContext& targetContext)
{
    ShadowBlur& shadow = platformContext.shadowBlur();

    if (!(state.textDrawingMode & TextModeFill) || shadow.type() == ShadowBlur::NoShadow)
        return;

    if (!mustUseShadowBlur(platformContext, state)) {
        // Optimize non-blurry shadows, by just drawing text without the ShadowBlur.
        cairo_t* context = platformContext.cr();
        cairo_save(context);

        FloatSize shadowOffset(state.shadowOffset);
        cairo_translate(context, shadowOffset.width(), shadowOffset.height());
        setSourceRGBAFromColor(context, state.shadowColor);
        drawGlyphsToContext(context, scaledFont, syntheticBoldOffset, glyphs);

        cairo_restore(context);
        return;
    }

    cairo_text_extents_t extents;
    cairo_scaled_font_glyph_extents(scaledFont, glyphs.data(), glyphs.size(), &extents);
    FloatRect fontExtentsRect(point.x() + extents.x_bearing, point.y() + extents.y_bearing, extents.width, extents.height);

    if (GraphicsContext* shadowContext = shadow.beginShadowLayer(targetContext, fontExtentsRect)) {
        drawGlyphsToContext(shadowContext->platformContext()->cr(), scaledFont, syntheticBoldOffset, glyphs);
        shadow.endShadowLayer(targetContext);
    }
}

static bool cairoSurfaceHasAlpha(cairo_surface_t* surface)
{
    return cairo_surface_get_content(surface) != CAIRO_CONTENT_COLOR;
}

// FIXME: Fix GraphicsContext::computeLineBoundsAndAntialiasingModeForText()
// to be a static public function that operates on CTM and strokeThickness
// arguments instead of using an underlying GraphicsContext object.
FloatRect computeLineBoundsAndAntialiasingModeForText(PlatformContextCairo& platformContext, const FloatPoint& point, float width, bool printing, Color& color, float strokeThickness)
{
    FloatPoint origin = point;
    float thickness = std::max(strokeThickness, 0.5f);
    if (printing)
        return FloatRect(origin, FloatSize(width, thickness));

    AffineTransform transform = Cairo::State::getCTM(platformContext);
    // Just compute scale in x dimension, assuming x and y scales are equal.
    float scale = transform.b() ? sqrtf(transform.a() * transform.a() + transform.b() * transform.b()) : transform.a();
    if (scale < 1.0) {
        // This code always draws a line that is at least one-pixel line high,
        // which tends to visually overwhelm text at small scales. To counter this
        // effect, an alpha is applied to the underline color when text is at small scales.
        static const float minimumUnderlineAlpha = 0.4f;
        float shade = scale > minimumUnderlineAlpha ? scale : minimumUnderlineAlpha;
        color = color.colorWithAlphaMultipliedBy(shade);
    }

    FloatPoint devicePoint = transform.mapPoint(point);
    // Visual overflow might occur here due to integral roundf/ceilf. visualOverflowForDecorations adjusts the overflow value for underline decoration.
    FloatPoint deviceOrigin = FloatPoint(roundf(devicePoint.x()), ceilf(devicePoint.y()));
    if (auto inverse = transform.inverse())
        origin = inverse.value().mapPoint(deviceOrigin);
    return FloatRect(origin, FloatSize(width, thickness));
};

// FIXME: Replace once GraphicsContext::dashedLineCornerWidthForStrokeWidth()
// is refactored as a static public function.
static float dashedLineCornerWidthForStrokeWidth(float strokeWidth, const GraphicsContextState& state)
{
    float thickness = state.strokeThickness;
    return state.strokeStyle == DottedStroke ? thickness : std::min(2.0f * thickness, std::max(thickness, strokeWidth / 3.0f));
}

// FIXME: Replace once GraphicsContext::dashedLinePatternWidthForStrokeWidth()
// is refactored as a static public function.
static float dashedLinePatternWidthForStrokeWidth(float strokeWidth, const GraphicsContextState& state)
{
    float thickness = state.strokeThickness;
    return state.strokeStyle == DottedStroke ? thickness : std::min(3.0f * thickness, std::max(thickness, strokeWidth / 3.0f));
}

// FIXME: Replace once GraphicsContext::dashedLinePatternOffsetForPatternAndStrokeWidth()
// is refactored as a static public function.
static float dashedLinePatternOffsetForPatternAndStrokeWidth(float patternWidth, float strokeWidth)
{
    // Pattern starts with full fill and ends with the empty fill.
    // 1. Let's start with the empty phase after the corner.
    // 2. Check if we've got odd or even number of patterns and whether they fully cover the line.
    // 3. In case of even number of patterns and/or remainder, move the pattern start position
    // so that the pattern is balanced between the corners.
    float patternOffset = patternWidth;
    int numberOfSegments = std::floor(strokeWidth / patternWidth);
    bool oddNumberOfSegments = numberOfSegments % 2;
    float remainder = strokeWidth - (numberOfSegments * patternWidth);
    if (oddNumberOfSegments && remainder)
        patternOffset -= remainder / 2.0f;
    else if (!oddNumberOfSegments) {
        if (remainder)
            patternOffset += patternOffset - (patternWidth + remainder) / 2.0f;
        else
            patternOffset += patternWidth / 2.0f;
    }

    return patternOffset;
}

// FIXME: Replace once GraphicsContext::centerLineAndCutOffCorners()
// is refactored as a static public function.
static Vector<FloatPoint> centerLineAndCutOffCorners(bool isVerticalLine, float cornerWidth, FloatPoint point1, FloatPoint point2)
{
    // Center line and cut off corners for pattern painting.
    if (isVerticalLine) {
        float centerOffset = (point2.x() - point1.x()) / 2.0f;
        point1.move(centerOffset, cornerWidth);
        point2.move(-centerOffset, -cornerWidth);
    } else {
        float centerOffset = (point2.y() - point1.y()) / 2.0f;
        point1.move(cornerWidth, centerOffset);
        point2.move(-cornerWidth, -centerOffset);
    }

    return { point1, point2 };
}

namespace State {

void setStrokeThickness(PlatformContextCairo& platformContext, float strokeThickness)
{
    cairo_set_line_width(platformContext.cr(), strokeThickness);
}

void setStrokeStyle(PlatformContextCairo& platformContext, StrokeStyle strokeStyle)
{
    static const double dashPattern[] = { 5.0, 5.0 };
    static const double dotPattern[] = { 1.0, 1.0 };

    cairo_t* cr = platformContext.cr();
    switch (strokeStyle) {
    case NoStroke:
        // FIXME: is it the right way to emulate NoStroke?
        cairo_set_line_width(cr, 0);
        break;
    case SolidStroke:
    case DoubleStroke:
    case WavyStroke:
        // FIXME: https://bugs.webkit.org/show_bug.cgi?id=94110 - Needs platform support.
        cairo_set_dash(cr, 0, 0, 0);
        break;
    case DottedStroke:
        cairo_set_dash(cr, dotPattern, 2, 0);
        break;
    case DashedStroke:
        cairo_set_dash(cr, dashPattern, 2, 0);
        break;
    }
}

void setGlobalAlpha(PlatformContextCairo& platformContext, float alpha)
{
    platformContext.setGlobalAlpha(alpha);
}

void setCompositeOperation(PlatformContextCairo& platformContext, CompositeOperator compositeOperation, BlendMode blendMode)
{
    cairo_set_operator(platformContext.cr(), toCairoOperator(compositeOperation, blendMode));
}

void setShouldAntialias(PlatformContextCairo& platformContext, bool enable)
{
    // When true, use the default Cairo backend antialias mode (usually this
    // enables standard 'grayscale' antialiasing); false to explicitly disable
    // antialiasing.
    cairo_set_antialias(platformContext.cr(), enable ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);
}

void setImageInterpolationQuality(PlatformContextCairo& platformContext, InterpolationQuality quality)
{
    platformContext.setImageInterpolationQuality(quality);
}

void setCTM(PlatformContextCairo& platformContext, const AffineTransform& transform)
{
    const cairo_matrix_t matrix = toCairoMatrix(transform);
    cairo_set_matrix(platformContext.cr(), &matrix);

    if (auto* graphicsContextPrivate = platformContext.graphicsContextPrivate())
        graphicsContextPrivate->setCTM(transform);
}

AffineTransform getCTM(PlatformContextCairo& platformContext)
{
    cairo_matrix_t m;
    cairo_get_matrix(platformContext.cr(), &m);
    return AffineTransform(m.xx, m.yx, m.xy, m.yy, m.x0, m.y0);
}

void setShadowValues(PlatformContextCairo& platformContext, const FloatSize& radius, const FloatSize& offset, const Color& color, bool ignoreTransforms)
{
    // Cairo doesn't support shadows natively, they are drawn manually in the draw* functions using ShadowBlur.
    platformContext.shadowBlur().setShadowValues(radius, offset, color, ignoreTransforms);
}

void clearShadow(PlatformContextCairo& platformContext)
{
    platformContext.shadowBlur().clear();
}

IntRect getClipBounds(PlatformContextCairo& platformContext)
{
    double x1, x2, y1, y2;
    cairo_clip_extents(platformContext.cr(), &x1, &y1, &x2, &y2);
    return enclosingIntRect(FloatRect(x1, y1, x2 - x1, y2 - y1));
}

FloatRect roundToDevicePixels(PlatformContextCairo& platformContext, const FloatRect& rect)
{
    FloatRect result;
    double x = rect.x();
    double y = rect.y();

    cairo_t* cr = platformContext.cr();
    cairo_user_to_device(cr, &x, &y);
    x = round(x);
    y = round(y);
    cairo_device_to_user(cr, &x, &y);
    result.setX(narrowPrecisionToFloat(x));
    result.setY(narrowPrecisionToFloat(y));

    // We must ensure width and height are at least 1 (or -1) when
    // we're given float values in the range between 0 and 1 (or -1 and 0).
    double width = rect.width();
    double height = rect.height();
    cairo_user_to_device_distance(cr, &width, &height);
    if (width > -1 && width < 0)
        width = -1;
    else if (width > 0 && width < 1)
        width = 1;
    else
        width = round(width);
    if (height > -1 && height < 0)
        height = -1;
    else if (height > 0 && height < 1)
        height = 1;
    else
        height = round(height);
    cairo_device_to_user_distance(cr, &width, &height);
    result.setWidth(narrowPrecisionToFloat(width));
    result.setHeight(narrowPrecisionToFloat(height));

    return result;
}

bool isAcceleratedContext(PlatformContextCairo& platformContext)
{
    return cairo_surface_get_type(cairo_get_target(platformContext.cr())) == CAIRO_SURFACE_TYPE_GL;
}

} // namespace State

void setLineCap(PlatformContextCairo& platformContext, LineCap lineCap)
{
    cairo_line_cap_t cairoCap;
    switch (lineCap) {
    case ButtCap:
        cairoCap = CAIRO_LINE_CAP_BUTT;
        break;
    case RoundCap:
        cairoCap = CAIRO_LINE_CAP_ROUND;
        break;
    case SquareCap:
        cairoCap = CAIRO_LINE_CAP_SQUARE;
        break;
    }
    cairo_set_line_cap(platformContext.cr(), cairoCap);
}

void setLineDash(PlatformContextCairo& platformContext, const DashArray& dashes, float dashOffset)
{
    if (std::all_of(dashes.begin(), dashes.end(), [](auto& dash) { return !dash; }))
        cairo_set_dash(platformContext.cr(), 0, 0, 0);
    else
        cairo_set_dash(platformContext.cr(), dashes.data(), dashes.size(), dashOffset);
}

void setLineJoin(PlatformContextCairo& platformContext, LineJoin lineJoin)
{
    cairo_line_join_t cairoJoin;
    switch (lineJoin) {
    case MiterJoin:
        cairoJoin = CAIRO_LINE_JOIN_MITER;
        break;
    case RoundJoin:
        cairoJoin = CAIRO_LINE_JOIN_ROUND;
        break;
    case BevelJoin:
        cairoJoin = CAIRO_LINE_JOIN_BEVEL;
        break;
    }
    cairo_set_line_join(platformContext.cr(), cairoJoin);
}

void setMiterLimit(PlatformContextCairo& platformContext, float miterLimit)
{
    cairo_set_miter_limit(platformContext.cr(), miterLimit);
}

void fillRect(PlatformContextCairo& platformContext, const FloatRect& rect, const GraphicsContextState& contextState, GraphicsContext& targetContext)
{
    cairo_t* cr = platformContext.cr();

    cairo_rectangle(cr, rect.x(), rect.y(), rect.width(), rect.height());
    drawPathShadow(platformContext, contextState, targetContext, Fill);
    fillCurrentCairoPath(platformContext, contextState);
}

void fillRect(PlatformContextCairo& platformContext, const FloatRect& rect, const Color& color, bool hasShadow, GraphicsContext& targetContext)
{
    if (hasShadow)
        platformContext.shadowBlur().drawRectShadow(targetContext, FloatRoundedRect(rect));

    fillRectWithColor(platformContext.cr(), rect, color);
}

void fillRect(PlatformContextCairo& platformContext, const FloatRect& rect, cairo_pattern_t* platformPattern)
{
    cairo_t* cr = platformContext.cr();

    cairo_set_source(cr, platformPattern);
    cairo_rectangle(cr, rect.x(), rect.y(), rect.width(), rect.height());
    cairo_fill(cr);
}

void fillRoundedRect(PlatformContextCairo& platformContext, const FloatRoundedRect& rect, const Color& color, bool hasShadow, GraphicsContext& targetContext)
{
    if (hasShadow)
        platformContext.shadowBlur().drawRectShadow(targetContext, rect);

    cairo_t* cr = platformContext.cr();
    cairo_save(cr);

    Path path;
    path.addRoundedRect(rect);
    appendWebCorePathToCairoContext(cr, path);
    setSourceRGBAFromColor(cr, color);
    cairo_fill(cr);

    cairo_restore(cr);
}

void fillRectWithRoundedHole(PlatformContextCairo& platformContext, const FloatRect& rect, const FloatRoundedRect& roundedHoleRect, const GraphicsContextState& contextState, GraphicsContext& targetContext)
{
    // FIXME: this should leverage the specified color.

    if (mustUseShadowBlur(platformContext, contextState))
        platformContext.shadowBlur().drawInsetShadow(targetContext, rect, roundedHoleRect);

    Path path;
    path.addRect(rect);
    if (!roundedHoleRect.radii().isZero())
        path.addRoundedRect(roundedHoleRect);
    else
        path.addRect(roundedHoleRect.rect());

    cairo_t* cr = platformContext.cr();

    cairo_save(cr);
    setPathOnCairoContext(platformContext.cr(), path.platformPath()->context());
    fillCurrentCairoPath(platformContext, contextState);
    cairo_restore(cr);
}

void fillPath(PlatformContextCairo& platformContext, const Path& path, const GraphicsContextState& contextState, GraphicsContext& targetContext)
{
    cairo_t* cr = platformContext.cr();

    setPathOnCairoContext(cr, path.platformPath()->context());
    drawPathShadow(platformContext, contextState, targetContext, Fill);
    fillCurrentCairoPath(platformContext, contextState);
}

void strokeRect(PlatformContextCairo& platformContext, const FloatRect& rect, float lineWidth, const GraphicsContextState& contextState, GraphicsContext& targetContext)
{
    cairo_t* cr = platformContext.cr();
    cairo_save(cr);

    cairo_rectangle(cr, rect.x(), rect.y(), rect.width(), rect.height());
    cairo_set_line_width(cr, lineWidth);
    drawPathShadow(platformContext, contextState, targetContext, Stroke);
    platformContext.prepareForStroking(contextState);
    cairo_stroke(cr);

    cairo_restore(cr);
}

void strokePath(PlatformContextCairo& platformContext, const Path& path, const GraphicsContextState& contextState, GraphicsContext& targetContext)
{
    cairo_t* cr = platformContext.cr();

    setPathOnCairoContext(cr, path.platformPath()->context());
    drawPathShadow(platformContext, contextState, targetContext, Stroke);
    platformContext.prepareForStroking(contextState);
    cairo_stroke(cr);
}

void clearRect(PlatformContextCairo& platformContext, const FloatRect& rect)
{
    cairo_t* cr = platformContext.cr();

    cairo_save(cr);
    cairo_rectangle(cr, rect.x(), rect.y(), rect.width(), rect.height());
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_fill(cr);
    cairo_restore(cr);
}

void drawGlyphs(PlatformContextCairo& platformContext, const GraphicsContextState& state, const FloatPoint& point, cairo_scaled_font_t* scaledFont, double syntheticBoldOffset, const Vector<cairo_glyph_t>& glyphs, float xOffset, GraphicsContext& targetContext)
{
    drawGlyphsShadow(platformContext, state, point, scaledFont, syntheticBoldOffset, glyphs, targetContext);

    cairo_t* cr = platformContext.cr();
    cairo_save(cr);

    if (state.textDrawingMode & TextModeFill) {
        platformContext.prepareForFilling(state, PlatformContextCairo::AdjustPatternForGlobalAlpha);
        drawGlyphsToContext(cr, scaledFont, syntheticBoldOffset, glyphs);
    }

    // Prevent running into a long computation within cairo. If the stroke width is
    // twice the size of the width of the text we will not ask cairo to stroke
    // the text as even one single stroke would cover the full wdth of the text.
    //  See https://bugs.webkit.org/show_bug.cgi?id=33759.
    if (state.textDrawingMode & TextModeStroke && state.strokeThickness < 2 * xOffset) {
        platformContext.prepareForStroking(state);
        cairo_set_line_width(cr, state.strokeThickness);

        // This may disturb the CTM, but we are going to call cairo_restore soon after.
        cairo_set_scaled_font(cr, scaledFont);
        cairo_glyph_path(cr, glyphs.data(), glyphs.size());
        cairo_stroke(cr);
    }

    cairo_restore(cr);
}

void drawNativeImage(PlatformContextCairo& platformContext, cairo_surface_t* surface, const FloatRect& destRect, const FloatRect& srcRect, CompositeOperator compositeOperator, BlendMode blendMode, ImageOrientation orientation, GraphicsContext& targetContext)
{
    platformContext.save();

    // Set the compositing operation.
    if (compositeOperator == CompositeSourceOver && blendMode == BlendModeNormal && !cairoSurfaceHasAlpha(surface))
        Cairo::State::setCompositeOperation(platformContext, CompositeCopy, BlendModeNormal);
    else
        Cairo::State::setCompositeOperation(platformContext, compositeOperator, blendMode);

    FloatRect dst = destRect;
    if (orientation != DefaultImageOrientation) {
        // ImageOrientation expects the origin to be at (0, 0).
        Cairo::translate(platformContext, dst.x(), dst.y());
        dst.setLocation(FloatPoint());
        Cairo::concatCTM(platformContext, orientation.transformFromDefault(dst.size()));
        if (orientation.usesWidthAsHeight()) {
            // The destination rectangle will have its width and height already reversed for the orientation of
            // the image, as it was needed for page layout, so we need to reverse it back here.
            dst = FloatRect(dst.x(), dst.y(), dst.height(), dst.width());
        }
    }

    platformContext.drawSurfaceToContext(surface, dst, srcRect, targetContext);
    platformContext.restore();
}

void drawPattern(PlatformContextCairo& platformContext, cairo_surface_t* surface, const IntSize& size, const FloatRect& destRect, const FloatRect& tileRect, const AffineTransform& patternTransform, const FloatPoint& phase, CompositeOperator compositeOperator, BlendMode blendMode)
{
    // FIXME: Investigate why the size has to be passed in as an IntRect.
    drawPatternToCairoContext(platformContext.cr(), surface, size, tileRect, patternTransform, phase, toCairoOperator(compositeOperator, blendMode), destRect);
}

void drawRect(PlatformContextCairo& platformContext, const FloatRect& rect, float borderThickness, const GraphicsContextState& state)
{
    // FIXME: how should borderThickness be used?
    UNUSED_PARAM(borderThickness);

    cairo_t* cr = platformContext.cr();
    cairo_save(cr);

    fillRectWithColor(cr, rect, state.fillColor);

    if (state.strokeStyle != NoStroke) {
        setSourceRGBAFromColor(cr, state.strokeColor);
        FloatRect r(rect);
        r.inflate(-.5f);
        cairo_rectangle(cr, r.x(), r.y(), r.width(), r.height());
        cairo_set_line_width(cr, 1.0); // borderThickness?
        cairo_stroke(cr);
    }

    cairo_restore(cr);
}

void drawLine(PlatformContextCairo& platformContext, const FloatPoint& point1, const FloatPoint& point2, const GraphicsContextState& state)
{
    bool isVerticalLine = (point1.x() + state.strokeThickness == point2.x());
    float strokeWidth = isVerticalLine ? point2.y() - point1.y() : point2.x() - point1.x();
    if (!state.strokeThickness || !strokeWidth)
        return;

    cairo_t* cairoContext = platformContext.cr();
    float cornerWidth = 0;
    bool drawsDashedLine = state.strokeStyle == DottedStroke || state.strokeStyle == DashedStroke;

    if (drawsDashedLine) {
        cairo_save(cairoContext);
        // Figure out end points to ensure we always paint corners.
        cornerWidth = dashedLineCornerWidthForStrokeWidth(strokeWidth, state);
        if (isVerticalLine) {
            fillRectWithColor(cairoContext, FloatRect(point1.x(), point1.y(), state.strokeThickness, cornerWidth), state.strokeColor);
            fillRectWithColor(cairoContext, FloatRect(point1.x(), point2.y() - cornerWidth, state.strokeThickness, cornerWidth), state.strokeColor);
        } else {
            fillRectWithColor(cairoContext, FloatRect(point1.x(), point1.y(), cornerWidth, state.strokeThickness), state.strokeColor);
            fillRectWithColor(cairoContext, FloatRect(point2.x() - cornerWidth, point1.y(), cornerWidth, state.strokeThickness), state.strokeColor);
        }
        strokeWidth -= 2 * cornerWidth;
        float patternWidth = dashedLinePatternWidthForStrokeWidth(strokeWidth, state);
        // Check if corner drawing sufficiently covers the line.
        if (strokeWidth <= patternWidth + 1) {
            cairo_restore(cairoContext);
            return;
        }

        float patternOffset = dashedLinePatternOffsetForPatternAndStrokeWidth(patternWidth, strokeWidth);
        const double dashedLine[2] = { static_cast<double>(patternWidth), static_cast<double>(patternWidth) };
        cairo_set_dash(cairoContext, dashedLine, 2, patternOffset);
    } else {
        setSourceRGBAFromColor(cairoContext, state.strokeColor);
        if (state.strokeThickness < 1)
            cairo_set_line_width(cairoContext, 1);
    }

    auto centeredPoints = centerLineAndCutOffCorners(isVerticalLine, cornerWidth, point1, point2);
    auto p1 = centeredPoints[0];
    auto p2 = centeredPoints[1];

    if (state.shouldAntialias)
        cairo_set_antialias(cairoContext, CAIRO_ANTIALIAS_NONE);

    cairo_new_path(cairoContext);
    cairo_move_to(cairoContext, p1.x(), p1.y());
    cairo_line_to(cairoContext, p2.x(), p2.y());
    cairo_stroke(cairoContext);
    if (drawsDashedLine)
        cairo_restore(cairoContext);

    if (state.shouldAntialias)
        cairo_set_antialias(cairoContext, CAIRO_ANTIALIAS_DEFAULT);
}

void drawLinesForText(PlatformContextCairo& platformContext, const FloatPoint& point, const DashArray& widths, bool printing, bool doubleUnderlines, const Color& color, float strokeThickness)
{
    Color modifiedColor = color;
    FloatRect bounds = computeLineBoundsAndAntialiasingModeForText(platformContext, point, widths.last(), printing, modifiedColor, strokeThickness);

    Vector<FloatRect, 4> dashBounds;
    ASSERT(!(widths.size() % 2));
    dashBounds.reserveInitialCapacity(dashBounds.size() / 2);
    for (size_t i = 0; i < widths.size(); i += 2)
        dashBounds.append(FloatRect(FloatPoint(bounds.x() + widths[i], bounds.y()), FloatSize(widths[i+1] - widths[i], bounds.height())));

    if (doubleUnderlines) {
        // The space between double underlines is equal to the height of the underline
        for (size_t i = 0; i < widths.size(); i += 2)
            dashBounds.append(FloatRect(FloatPoint(bounds.x() + widths[i], bounds.y() + 2 * bounds.height()), FloatSize(widths[i+1] - widths[i], bounds.height())));
    }

    cairo_t* cr = platformContext.cr();
    cairo_save(cr);

    for (auto& dash : dashBounds)
        fillRectWithColor(cr, dash, modifiedColor);

    cairo_restore(cr);
}

void drawLineForDocumentMarker(PlatformContextCairo& platformContext, const FloatPoint& origin, float width, GraphicsContext::DocumentMarkerLineStyle style)
{
    if (style != GraphicsContext::DocumentMarkerSpellingLineStyle
        && style != GraphicsContext::DocumentMarkerGrammarLineStyle)
        return;

    cairo_t* cr = platformContext.cr();
    cairo_save(cr);

    if (style == GraphicsContext::DocumentMarkerSpellingLineStyle)
        cairo_set_source_rgb(cr, 1, 0, 0);
    else if (style == GraphicsContext::DocumentMarkerGrammarLineStyle)
        cairo_set_source_rgb(cr, 0, 1, 0);

    drawErrorUnderline(cr, origin.x(), origin.y(), width, cMisspellingLineThickness);
    cairo_restore(cr);
}

void drawEllipse(PlatformContextCairo& platformContext, const FloatRect& rect, const GraphicsContextState& state)
{
    cairo_t* cr = platformContext.cr();

    cairo_save(cr);
    float yRadius = .5 * rect.height();
    float xRadius = .5 * rect.width();
    cairo_translate(cr, rect.x() + xRadius, rect.y() + yRadius);
    cairo_scale(cr, xRadius, yRadius);
    cairo_arc(cr, 0., 0., 1., 0., 2 * piFloat);
    cairo_restore(cr);

    if (state.fillColor.isVisible()) {
        setSourceRGBAFromColor(cr, state.fillColor);
        cairo_fill_preserve(cr);
    }

    if (state.strokeStyle != NoStroke) {
        setSourceRGBAFromColor(cr, state.strokeColor);
        cairo_set_line_width(cr, state.strokeThickness);
        cairo_stroke(cr);
    } else
        cairo_new_path(cr);
}

void drawFocusRing(PlatformContextCairo& platformContext, const Path& path, float width, const Color& color)
{
    // FIXME: We should draw paths that describe a rectangle with rounded corners
    // so as to be consistent with how we draw rectangular focus rings.
    Color ringColor = color;
    adjustFocusRingColor(ringColor);
    adjustFocusRingLineWidth(width);

    cairo_t* cr = platformContext.cr();
    cairo_save(cr);

    cairo_push_group(cr);
    appendWebCorePathToCairoContext(cr, path);
    setSourceRGBAFromColor(cr, ringColor);
    cairo_set_line_width(cr, width);
    Cairo::State::setStrokeStyle(platformContext, focusRingStrokeStyle());
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_stroke_preserve(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
    cairo_fill(cr);

    cairo_pop_group_to_source(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_paint(cr);

    cairo_restore(cr);
}

void drawFocusRing(PlatformContextCairo& platformContext, const Vector<FloatRect>& rects, float width, const Color& color)
{
    Path path;
#if PLATFORM(GTK)
    for (const auto& rect : rects)
        path.addRect(rect);
#else
    unsigned rectCount = rects.size();
    int radius = (width - 1) / 2;
    Path subPath;
    for (unsigned i = 0; i < rectCount; ++i) {
        if (i > 0)
            subPath.clear();
        subPath.addRoundedRect(rects[i], FloatSize(radius, radius));
        path.addPath(subPath, AffineTransform());
    }
#endif

    drawFocusRing(platformContext, path, width, color);
}

void save(PlatformContextCairo& platformContext)
{
    platformContext.save();

    if (auto* graphicsContextPrivate = platformContext.graphicsContextPrivate())
        graphicsContextPrivate->save();
}

void restore(PlatformContextCairo& platformContext)
{
    platformContext.restore();

    if (auto* graphicsContextPrivate = platformContext.graphicsContextPrivate())
        graphicsContextPrivate->restore();
}

void translate(PlatformContextCairo& platformContext, float x, float y)
{
    cairo_translate(platformContext.cr(), x, y);

    if (auto* graphicsContextPrivate = platformContext.graphicsContextPrivate())
        graphicsContextPrivate->translate(x, y);
}

void rotate(PlatformContextCairo& platformContext, float angleInRadians)
{
    cairo_rotate(platformContext.cr(), angleInRadians);

    if (auto* graphicsContextPrivate = platformContext.graphicsContextPrivate())
        graphicsContextPrivate->rotate(angleInRadians);
}

void scale(PlatformContextCairo& platformContext, const FloatSize& size)
{
    cairo_scale(platformContext.cr(), size.width(), size.height());

    if (auto* graphicsContextPrivate = platformContext.graphicsContextPrivate())
        graphicsContextPrivate->scale(size);
}

void concatCTM(PlatformContextCairo& platformContext, const AffineTransform& transform)
{
    const cairo_matrix_t matrix = toCairoMatrix(transform);
    cairo_transform(platformContext.cr(), &matrix);

    if (auto* graphicsContextPrivate = platformContext.graphicsContextPrivate())
        graphicsContextPrivate->concatCTM(transform);
}

void beginTransparencyLayer(PlatformContextCairo& platformContext, float opacity)
{
    cairo_push_group(platformContext.cr());
    platformContext.layers().append(opacity);
}

void endTransparencyLayer(PlatformContextCairo& platformContext)
{
    cairo_t* cr = platformContext.cr();
    cairo_pop_group_to_source(cr);
    cairo_paint_with_alpha(cr, platformContext.layers().takeLast());
}

void clip(PlatformContextCairo& platformContext, const FloatRect& rect)
{
    cairo_t* cr = platformContext.cr();
    cairo_rectangle(cr, rect.x(), rect.y(), rect.width(), rect.height());
    cairo_fill_rule_t savedFillRule = cairo_get_fill_rule(cr);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING);
    // The rectangular clip function is traditionally not expected to
    // antialias. If we don't force antialiased clipping here,
    // edge fringe artifacts may occur at the layer edges
    // when a transformation is applied to the GraphicsContext
    // while drawing the transformed layer.
    cairo_antialias_t savedAntialiasRule = cairo_get_antialias(cr);
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
    cairo_clip(cr);
    cairo_set_fill_rule(cr, savedFillRule);
    cairo_set_antialias(cr, savedAntialiasRule);

    if (auto* graphicsContextPrivate = platformContext.graphicsContextPrivate())
        graphicsContextPrivate->clip(rect);
}

void clipOut(PlatformContextCairo& platformContext, const FloatRect& rect)
{
    cairo_t* cr = platformContext.cr();
    double x1, y1, x2, y2;
    cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
    cairo_rectangle(cr, x1, y1, x2 - x1, y2 - y1);
    cairo_rectangle(cr, rect.x(), rect.y(), rect.width(), rect.height());
    cairo_fill_rule_t savedFillRule = cairo_get_fill_rule(cr);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_clip(cr);
    cairo_set_fill_rule(cr, savedFillRule);
}

void clipOut(PlatformContextCairo& platformContext, const Path& path)
{
    cairo_t* cr = platformContext.cr();
    double x1, y1, x2, y2;
    cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
    cairo_rectangle(cr, x1, y1, x2 - x1, y2 - y1);
    appendWebCorePathToCairoContext(cr, path);

    cairo_fill_rule_t savedFillRule = cairo_get_fill_rule(cr);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_clip(cr);
    cairo_set_fill_rule(cr, savedFillRule);
}

void clipPath(PlatformContextCairo& platformContext, const Path& path, WindRule clipRule)
{
    cairo_t* cr = platformContext.cr();

    if (!path.isNull())
        setPathOnCairoContext(cr, path.platformPath()->context());

    cairo_fill_rule_t savedFillRule = cairo_get_fill_rule(cr);
    cairo_set_fill_rule(cr, clipRule == RULE_EVENODD ? CAIRO_FILL_RULE_EVEN_ODD : CAIRO_FILL_RULE_WINDING);
    cairo_clip(cr);
    cairo_set_fill_rule(cr, savedFillRule);

    if (auto* graphicsContextPrivate = platformContext.graphicsContextPrivate())
        graphicsContextPrivate->clip(path);
}

void clipToImageBuffer(PlatformContextCairo& platformContext, Image& image, const FloatRect& destRect)
{
    RefPtr<cairo_surface_t> surface = image.nativeImageForCurrentFrame();
    if (surface)
        platformContext.pushImageMask(surface.get(), destRect);
}

} // namespace Cairo
} // namespace WebCore

#endif // USE(CAIRO)
