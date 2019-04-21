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
// #include "SkCanvas.h"
// #include "SkData.h"
// #include "SkGradientShader.h"
// #include "SkImage.h"
// #include "SkPoint.h"
// #include "SkRect.h"
// #include "SkRRect.h"
// #include "SkShader.h"
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
    std::string imageString = base64_decode(base64);

    mImageData.encoding = encoding;
    mImageData.blob_size = imageString.size() + 1;

    switch (encoding) {
    case kJPEG:
        if (CAIRO_STATUS_SUCCESS !=_cairo_image_info_get_jpeg_info(&img_info, imageString.data(), imageString.size())) {
            return;
            mImageData.width = img_info.width;
            mImageData.height = img_info.height;
        };
    case kPNG:
        if (CAIRO_STATUS_SUCCESS !=_cairo_image_info_get_png_info(&img_info, imageString.data(), imageString.size())) {
            return;
        };
    default:
            return;
    };
 
    mImageData.width = img_info.width;
    mImageData.height = img_info.height;
    mImageData.blob = new unsigned char[mImageData.blob_size];
    memcpy(mImageData.blob, imageString.data(), imageString.size());
}

float CairoSVGImageData::Width() const
{
    if (!mImageData)
        return 0;
    return static_cast<float>(mImageData.width);
}

float SkiaSVGImageData::Height() const
{
    if (!mImageData)
        return 0;
    return static_cast<float>(mImageData.height);
}

CairoSVGRenderer::CairoSVGRenderer()
{
}

void CairoSVGRenderer::Save(const GraphicStyle& graphicStyle)
{
    SVG_ASSERT(mCanvas);
    cairo_save(mCanvas);
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
    SVG_ASSERT(mCanvas);
    cairo_restore(mCanvas);
}

inline void CreateSkPaint(const Paint& paint, float opacity, SkPaint& skPaint)
{
    if (paint.type() == typeid(Color))
    {
        const auto& color = boost::get<Color>(paint);
        skPaint.setColor(SkColorSetARGB(static_cast<uint8_t>(opacity * color[3] * 255), static_cast<uint8_t>(color[0] * 255),
            static_cast<uint8_t>(color[1] * 255), static_cast<uint8_t>(color[2] * 255)));
    }
    else if (paint.type() == typeid(Gradient))
    {
        const auto& gradient = boost::get<Gradient>(paint);
        std::vector<SkColor> colors;
        std::vector<SkScalar> pos;
        for (const auto& stop : gradient.colorStops)
        {
            pos.push_back(stop.first);
            const auto& stopColor = stop.second;
            colors.push_back(SkColorSetARGB(static_cast<uint8_t>(opacity * stopColor[3] * 255), static_cast<uint8_t>(stopColor[0] * 255),
                static_cast<uint8_t>(stopColor[1] * 255), static_cast<uint8_t>(stopColor[2] * 255)));
        }
        SkShader::TileMode mode;
        switch (gradient.method)
        {
        case SpreadMethod::kReflect:
            mode = SkShader::TileMode::kMirror_TileMode;
            break;
        case SpreadMethod::kRepeat:
            mode = SkShader::TileMode::kRepeat_TileMode;
            break;
        case SpreadMethod::kPad:
        default:
            mode = SkShader::TileMode::kClamp_TileMode;
            break;
        }
        SkMatrix* matrix{};
        if (gradient.transform)
            matrix = &(static_cast<SkiaSVGTransform*>(gradient.transform.get())->mMatrix);
        if (gradient.type == GradientType::kLinearGradient)
        {
            SkPoint points[2] = {SkPoint::Make(gradient.x1, gradient.y1), SkPoint::Make(gradient.x2, gradient.y2)};
            skPaint.setShader(
                SkGradientShader::MakeLinear(points, colors.data(), pos.data(), static_cast<int>(colors.size()), mode, 0, matrix));
        }
        else if (gradient.type == GradientType::kRadialGradient)
        {
            skPaint.setShader(
                SkGradientShader::MakeTwoPointConical(SkPoint::Make(gradient.fx, gradient.fy), 0, SkPoint::Make(gradient.cx, gradient.cy),
                    gradient.r, colors.data(), pos.data(), static_cast<int>(colors.size()), mode, 0, matrix));
        }
    }
}

void CairoSVGRenderer::DrawPath(
    const Path& path, const GraphicStyle& graphicStyle, const FillStyle& fillStyle, const StrokeStyle& strokeStyle)
{
    SVG_ASSERT(mCanvas);
    Save(graphicStyle);
    if (fillStyle.hasFill)
    {
        cairo_set_source_rgba(mCanvas, r, g, b, a);
        cairo_set_fill_rule (mCanvas, fr);

        cairo_new_path (mCanvas);
        cairo_append_path (mCanvas, static_cast<const CairoSVGPath&>(path).mPath->path);
        cairo_fill (mCanvas);
    }
    if (strokeStyle.hasStroke)
    {
        cairo_set_source_rgba(mCanvas, r, g, b, a);
        cairo_set_line_width(mCanvas, lw);
        cairo_set_line_cap(mCanvas, lc);
        cairo_set_line_join(mCanvas, lj);

        cairo_new_path (mCanvas);
        cairo_append_path (mCanvas, static_cast<const CairoSVGPath&>(path).mPath->path);
        cairo_stroke (mCanvas);
    }
    Restore();
}

void SkiaSVGRenderer::DrawImage(
    const ImageData& image, const GraphicStyle& graphicStyle, const Rect& clipArea, const Rect& fillArea)
{
    SVG_ASSERT(mCanvas);
    Save(graphicStyle);
    mCanvas->clipRect({clipArea.x, clipArea.y, clipArea.x + clipArea.width, clipArea.y + clipArea.height}, SkClipOp::kIntersect);
    mCanvas->drawImageRect(static_cast<const SkiaSVGImageData&>(image).mImageData,
        {fillArea.x, fillArea.y, fillArea.x + fillArea.width, fillArea.y + fillArea.height}, nullptr);
    Restore();
}

void SkiaSVGRenderer::SetSkCanvas(SkCanvas* canvas)
{
    SVG_ASSERT(canvas);
    mCanvas = canvas;
}

} // namespace SVGNative
