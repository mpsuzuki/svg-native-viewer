/*
Copyright 2019 Adobe. All rights reserved.
This file is licensed to you under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License. You may obtain a copy
of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS
OF ANY KIND, either express or implied. See the License for the specific language
governing permissions and limitations under the License.
*/

#include "CairoSVGRenderer.h"
#include "base64.h"
#include "Config.h"
#include "cairo.h"
#include <math.h>

namespace SVGNative
{
CairoSVGPath::CairoSVGPath() {}

void CairoSVGPath::Rect(float x, float y, float width, float height)
{
    cairo_new_sub_path (mPath.cr);
    cairo_rectangle (mPath.cr, x, y, width, height);
    cairo_close_path (mPath.cr);
    mPath.path = cairo_copy_path (mPath.cr);
}

void CairoSVGPath::RoundedRect(float x, float y, float width, float height, float cornerRadius)
{
    // Cairo does not provide single API to draw "rounded rect". See
    // https://www.cairographics.org/samples/rounded_rectangle/

    double    aspect = 1.0;
    double    radius = cornerRadius / aspect;
    double    degrees = M_PI / 180.0;

    cairo_new_sub_path (mPath.cr);
    cairo_arc (mPath.cr, x + width - radius, y + radius,          radius, -90 * degrees,   0 * degrees);
    cairo_arc (mPath.cr, x + width - radius, y + height - radius, radius,   0 * degrees,  90 * degrees);
    cairo_arc (mPath.cr, x + radius,         y + height - radius, radius,  90 * degrees, 180 * degrees);
    cairo_arc (mPath.cr, x + radius,         y + radius,          radius, 180 * degrees, 270 * degrees);
    cairo_close_path (mPath.cr);
    mPath.path = cairo_copy_path (mPath.cr);
}

void CairoSVGPath::Ellipse(float cx, float cy, float rx, float ry) {
    // Cairo does not provide single API to draw "ellipse". See
    // https://cairographics.org/cookbook/ellipses/

    cairo_matrix_t  save_matrix;
    cairo_get_matrix (mPath.cr, &save_matrix); 

    cairo_translate (mPath.cr, cx, cy);
    cairo_scale (mPath.cr, rx, ry);
    cairo_arc (mPath.cr, 0, 0, 1, 0, 2 * M_PI);

    cairo_set_matrix (mPath.cr, &save_matrix); 
    mPath.path = cairo_copy_path (mPath.cr);
}

void CairoSVGPath::MoveTo(float x, float y)
{
    cairo_move_to (mPath.cr, x, y);
    mCurrentX = x;
    mCurrentY = y;
}

void CairoSVGPath::LineTo(float x, float y)
{
    cairo_line_to (mPath.cr, x, y);
    mCurrentX = x;
    mCurrentY = y;
    mPath.path = cairo_copy_path (mPath.cr);
}

void CairoSVGPath::CurveTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    cairo_curve_to (mPath.cr, x1, y1, x2, y2, x3, y3);
    mCurrentX = x3;
    mCurrentY = y3;
    mPath.path = cairo_copy_path (mPath.cr);
}

void CairoSVGPath::CurveToV(float x2, float y2, float x3, float y3)
{
    cairo_curve_to (mPath.cr, mCurrentX, mCurrentY, x2, y2, x3, y3);
    mCurrentX = x3;
    mCurrentY = y3;
    mPath.path = cairo_copy_path (mPath.cr);
}

void CairoSVGPath::ClosePath() {
    cairo_close_path (cr);
    mPath.path = cairo_copy_path (mPath.cr);
}

CairoSVGTransform::CairoSVGTransform(float a, float b, float c, float d, float tx, float ty) {
    cairo_matrix_init(&mMatrix, a, b, c, d, tx, ty);
}

void CairoSVGTransform::Set(float a, float b, float c, float d, float tx, float ty) {
    // see CGSVGRenderer.cpp,
    // internal of CGSVGTransform::CGSVGTransform() and CGSVGTransform::Set() are same.
    cairo_matrix_init(&mMatrix, a, b, c, d, tx, ty);
}

void CairoSVGTransform::Rotate(float degree) {
    cairo_matrix_rotate(&mMatrix, degree * M_PI / 180.0 );
}

void CairoSVGTransform::Translate(float tx, float ty) {
    cairo_matrix_translate(&mMatrix, tx, ty);
}

void CairoSVGTransform::Scale(float sx, float sy) {
    cairo_matrix_scale(&mMatrix, sx, sy);
}

void CairoSVGTransform::Concat(const Transform& other) {
    cairo_matrix_t  result;
    cairo_matrix_multiply(&result, &mMatrix, &(static_cast<const CairoSVGTransform&>(other).mMatrix));
    // Cairo has no API to copy a matrix to another
    cairo_matrix_init(&mMatrix, result.xx, result.yx, result.xy, result.yy, result.x0, result.y0); 
}

CairoSVGImageData::CairoSVGImageData(const std::string& base64, ImageEncoding encoding)
{
    cairo_image_info_t  img_info = {0, 0, 0, 0};
    cairo_format_t      cr_fmt;
    char                mime_type[64]; // XXX: C++ has better technique?
    std::string imageString = base64_decode(base64);

    switch (encoding) {
    case kJPEG:
        if (CAIRO_STATUS_SUCCESS !=_cairo_image_info_get_jpeg_info(&img_info, imageString.data(), imageString.size())) {
            return;
        };
        strncpy(mime_type, CAIRO_MIME_TYPE_JPEG, sizeof(mime_type));
        break;
    case kPNG:
        if (CAIRO_STATUS_SUCCESS !=_cairo_image_info_get_png_info(&img_info, imageString.data(), imageString.size())) {
            return;
        };
        strncpy(mime_type, CAIRO_MIME_TYPE_PNG, sizeof(mime_type));
        break;
    default:
        return;
    };

    switch (img_info.num_components) {
    default:
        cr_fmt = CAIRO_FORMAT_ARGB32;
    };

    void* blob_data = malloc( imageString.size() );
    memcpy(blob_data, imageString.data(), imageString.size());

    mImageData = cairo_image_surface_create (cr_fmt, img_info.width, img_info.height);
    cairo_surface_set_mime_data (mImageData, mime_type, blob_data, imageString.size(), free, blob_data);
}

float CairoSVGImageData::Width() const
{
    if (!mImageData)
        return 0;
    return static_cast<float>( cairo_image_surface_get_width (mImageData) );
}

float CairoSVGImageData::Height() const
{
    if (!mImageData)
        return 0;
    return static_cast<float>( cairo_image_surface_get_height (mImageData) );
}

CairoSVGRenderer::CairoSVGRenderer()
{
}

void CairoSVGRenderer::Save(const GraphicStyle& graphicStyle)
{
    SVG_ASSERT(mSurface);
    cairo_save(mSurface);
    if (graphicStyle.opacity != 1.0)
        mCanvas->saveLayerAlpha(nullptr, graphicStyle.opacity);
    else
        mCanvas->save();
    if (graphicStyle.transform)
        mCanvas->concat(static_cast<SkiaSVGTransform*>(graphicStyle.transform.get())->mMatrix);
    if (graphicStyle.clippingPath && graphicStyle.clippingPath->path)
    {
        SkPath clippingPath(static_cast<const SkiaSVGPath*>(graphicStyle.clippingPath->path.get())->mPath);
        if (graphicStyle.clippingPath->transform)
        {
            const auto& matrix = static_cast<const SkiaSVGTransform*>(graphicStyle.clippingPath->transform.get())->mMatrix;
            clippingPath.transform(matrix);
        }
		clippingPath.setFillType(graphicStyle.clippingPath->clipRule == WindingRule::kNonZero ? SkPath::kWinding_FillType : SkPath::kEvenOdd_FillType);
        mCanvas->clipPath(clippingPath);
    }
}

void CairoSVGRenderer::Restore()
{
    SVG_ASSERT(mSurface);
    cairo_restore(mSurface);
}

inline void CreateCairoPattern(const Paint& paint, float opacity, cairo_pattern_t** pat)
{
    *pat = NULL;

    if (paint.type() != typeid(Gradient))
        return;

    const auto& gradient = boost::get<Gradient>(paint);

    // in Cairo, gradient type might be set before setting color stops. See
    // https://www.cairographics.org/samples/gradient/
    if (gradient.type == GradientType::kLinearGradient)
    {
        *pat = cairo_pattern_create_linear( gradient.x1, gradient.y1,
                                            gradient.x2, gradient.y2 );
    }
    else if (gradient.type == GradientType::kRadialGradient)
    {
        *pat = cairo_pattern_create_radial( gradient.fx, gradient.fy, 0,
                                            gradient.cx, gradient.cy, gradient.r );

    }
    else // unknown GradientType
        return;

    // set transform matrix
    if (gradient.transform)
        cairo_pattern_set_matrix ( *pat, &(static_cast<CairoSVGTransform*>(gradient.transform.get())->mMatrix) );

    // set "stop"s of gradient 
    for (const auto& stop : gradient.colorStops)
    {
        // here, ColorStop is a pair of offset (in float) and color
        const auto& stopOffset = stop.first;
        const auto& stopColor = stop.second;

        cairo_pattern_add_color_stop_rgba( *pat, stopOffset,
                                           static_cast<uint8_t>(stopColor[0] * 255),
                                           static_cast<uint8_t>(stopColor[1] * 255),
                                           static_cast<uint8_t>(stopColor[2] * 255),
                                           static_cast<uint8_t>(opacity * stopColor[3] * 255) );
    }

    // set the mode how to fill the wide area by a small pattern
    switch (gradient.method)
    {
    case SpreadMethod::kReflect:
        cairo_pattern_set_extend (*pat, CAIRO_EXTEND_REFLECT );
        break;
    case SpreadMethod::kRepeat:
        cairo_pattern_set_extend (*pat, CAIRO_EXTEND_REPEAT );
        break;
    case SpreadMethod::kPad:
        cairo_pattern_set_extend (*pat, CAIRO_EXTEND_PAD );
        break;
    default:
        cairo_pattern_set_extend (*pat, CAIRO_EXTEND_NONE );
        break;
    }
    return;
}

void CairoSVGRenderer::DrawPath(
    const Path& path, const GraphicStyle& graphicStyle, const FillStyle& fillStyle, const StrokeStyle& strokeStyle)
{
    SVG_ASSERT(mSurface);
    Save(graphicStyle);
    if (fillStyle.hasFill)
    {
        if (paint.type() == typeid(Gradient)) {
            cairo_pattern_t* pat;
            CreateCairoPattern(fillStyle.paint, fillStyle.fillOpacity, &pat);
            cairo_set_source(mSurface, pat);
        } else {
            cairo_set_source_rgba(mSurface, static_cast<uint8_t>(color[0] * 255),
                                            static_cast<uint8_t>(color[1] * 255),
                                            static_cast<uint8_t>(color[2] * 255),
                                            static_cast<uint8_t>(color[3] * 255));
        }

        // Skia backend does not handle fillStyle.Rule yet
        switch (fillStyle.Rule) {
        case WindingRule::kEvenOdd:
            cairo_set_fill_rule (mSurface, CAIRO_FILL_RULE_EVEN_ODD);
            break;
        case WindingRule::kNonZero:
        default:
            cairo_set_fill_rule (mSurface, CAIRO_FILL_RULE_WINDING);
        }

        cairo_new_path (mSurface);
        cairo_append_path (mSurface, static_cast<const CairoSVGPath&>(path).mPath->path);
        cairo_fill (mSurface);
    }
    if (strokeStyle.hasStroke)
    {
        cairo_set_source_rgba(mSurface, r, g, b, a);
        cairo_set_line_width(mSurface, lw);
        cairo_set_line_cap(mSurface, lc);
        cairo_set_line_join(mSurface, lj);

        cairo_new_path (mSurface);
        cairo_append_path (mSurface, static_cast<const CairoSVGPath&>(path).mPath->path);
        cairo_stroke (mSurface);
    }
    Restore();
}

void CairoSVGRenderer::DrawImage(
    const ImageData& image, const GraphicStyle& graphicStyle, const Rect& clipArea, const Rect& fillArea)
{
    SVG_ASSERT(mSurface);
    Save(graphicStyle);
    cairo_new_path (mSurface);
    cairo_rectangle (mSurface, clipArea.x, clipArea.y, clipArea.width, clipArea.height);

    const CairoSVGImageData cairoSvgImgData = static_cast<const CairoSVGImageData&>(image);

    mCanvas->drawImageRect(static_cast<const SkiaSVGImageData&>(image).mImageData,
        {fillArea.x, fillArea.y, fillArea.x + fillArea.width, fillArea.y + fillArea.height}, nullptr);
    cairo_translate (mSurface, fillArea.x, fillArea.y);
    cairo_scale (mSurface, fillArea.width / cairoSvgImgData.Width(), fillArea.height / cairoSvgImgData.Height() );
    cairo_set_source_surface (mSurface, cairoSvgImgData.mImageData, 0, 0);
    cairo_paint (mSurface);

    Restore();
}

void CairoSVGRenderer::SetCairoSurface(cairo_surface_t* cr)
{
    SVG_ASSERT(cr);
    mSurface = cr;
}

} // namespace SVGNative
