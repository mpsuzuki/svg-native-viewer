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
#include "CairoImageInfo.h"

namespace SVGNative
{
CairoSVGPath::CairoSVGPath()
{
    cairo_surface_t* sf = cairo_recording_surface_create ( CAIRO_CONTENT_COLOR_ALPHA, NULL );
    mPath = cairo_create (sf);
}

CairoSVGPath::~CairoSVGPath()
{
    cairo_surface_t* sf = cairo_get_target( mPath );

    cairo_destroy ( mPath );
    cairo_surface_finish ( sf );
    cairo_surface_destroy ( sf );
}

void CairoSVGPath::Rect(float x, float y, float width, float height)
{
    cairo_new_sub_path ( mPath );
    cairo_rectangle ( mPath, x, y, width, height );
    cairo_close_path ( mPath );
}

void CairoSVGPath::RoundedRect(float x, float y, float width, float height, float cornerRadius)
{
    // Cairo does not provide single API to draw "rounded rect". See
    // https://www.cairographics.org/samples/rounded_rectangle/

    double    aspect = 1.0;
    double    radius = cornerRadius / aspect;
    double    degrees = M_PI / 180.0;

    cairo_new_sub_path ( mPath );
    cairo_arc ( mPath, x + width - radius, y + radius,          radius, -90 * degrees,   0 * degrees);
    cairo_arc ( mPath, x + width - radius, y + height - radius, radius,   0 * degrees,  90 * degrees);
    cairo_arc ( mPath, x + radius,         y + height - radius, radius,  90 * degrees, 180 * degrees);
    cairo_arc ( mPath, x + radius,         y + radius,          radius, 180 * degrees, 270 * degrees);
    cairo_close_path ( mPath );
}

void CairoSVGPath::Ellipse(float cx, float cy, float rx, float ry) {
    // Cairo does not provide single API to draw "ellipse". See
    // https://cairographics.org/cookbook/ellipses/

    cairo_matrix_t  save_matrix;
    cairo_get_matrix ((cairo_t*)mPath, &save_matrix); 

    cairo_translate ((cairo_t*)mPath, cx, cy);
    cairo_scale ((cairo_t*)mPath, rx, ry);
    cairo_arc ((cairo_t*)mPath, 0, 0, 1, 0, 2 * M_PI);

    cairo_set_matrix ((cairo_t*)mPath, &save_matrix); 
}

void CairoSVGPath::MoveTo(float x, float y)
{
    cairo_move_to ((cairo_t*)mPath, x, y);
    mCurrentX = x;
    mCurrentY = y;
}

void CairoSVGPath::LineTo(float x, float y)
{
    cairo_line_to ((cairo_t*)mPath, x, y);
    mCurrentX = x;
    mCurrentY = y;
}

void CairoSVGPath::CurveTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    cairo_curve_to ((cairo_t*)mPath, x1, y1, x2, y2, x3, y3);
    mCurrentX = x3;
    mCurrentY = y3;
}

void CairoSVGPath::CurveToV(float x2, float y2, float x3, float y3)
{
    cairo_curve_to (mPath, mCurrentX, mCurrentY, x2, y2, x3, y3);
    mCurrentX = x3;
    mCurrentY = y3;
}

void CairoSVGPath::ClosePath() {
    cairo_close_path (mPath);
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
    case ImageEncoding::kJPEG:
        if (CAIRO_STATUS_SUCCESS !=_cairo_image_info_get_jpeg_info(&img_info, (unsigned char*)imageString.data(), imageString.size())) {
            return;
        };
        strncpy(mime_type, CAIRO_MIME_TYPE_JPEG, sizeof(mime_type));
        break;
    case ImageEncoding::kPNG:
        if (CAIRO_STATUS_SUCCESS !=_cairo_image_info_get_png_info(&img_info, (unsigned char*)imageString.data(), imageString.size())) {
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

    const unsigned char* blob_data = (const unsigned char*)malloc( imageString.size() );
    memcpy((void *)blob_data, imageString.data(), imageString.size());

    mImageData = cairo_image_surface_create (cr_fmt, img_info.width, img_info.height);
    cairo_surface_set_mime_data (mImageData, mime_type, blob_data, imageString.size(), free, (void*)blob_data);
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

CairoSVGRenderer::~CairoSVGRenderer()
{
}

inline cairo_path_t* getPathObjFromCairoSvgPath( const Path* path )
{
    cairo_t* cr = static_cast<const CairoSVGPath*>(path)->mPath;
    return cairo_copy_path( cr );
}

inline cairo_path_t* getTransformedClippingPath( const ClippingPath* clippingPath )
{
    cairo_path_t* path = getPathObjFromCairoSvgPath( clippingPath->path.get() );
    if (clippingPath->transform)
    {
        cairo_matrix_t matrix = static_cast<const CairoSVGTransform*>(clippingPath->transform.get())->mMatrix;
        cairo_path_t* pathTransformed = (cairo_path_t*)malloc( sizeof( cairo_path_t ) );
        pathTransformed->num_data = path->num_data;
        pathTransformed->status = path->status;
        pathTransformed->data = (cairo_path_data_t*)malloc( path->num_data * sizeof( cairo_path_data_t ) );
        for (int i = 0; i < pathTransformed->num_data; i += pathTransformed->data[i].header.length )
        {
            for (int j = 0; j < pathTransformed->data[i].header.length; j ++ )
            {
                double x = path->data[i+1+j].point.x;
                double y = path->data[i+1+j].point.y;
                cairo_matrix_transform_point( &matrix, &x, &y );
                pathTransformed->data[i+1+j].point.x = x;
                pathTransformed->data[i+1+j].point.y = y;
            }
        }
        cairo_path_destroy( path );
        return pathTransformed;
    } else
        return path;
}

void CairoSVGRenderer::Save(const GraphicStyle& graphicStyle)
{
    SVG_ASSERT( mCairo );
    cairo_save ( mCairo );

    if (graphicStyle.transform)
        cairo_transform( mCairo, &(static_cast<CairoSVGTransform*>(graphicStyle.transform.get())->mMatrix));

    if (graphicStyle.clippingPath && graphicStyle.clippingPath->path)
    {
        cairo_path_t* path = getTransformedClippingPath( graphicStyle.clippingPath.get() );
        cairo_append_path( mCairo, path );
        cairo_clip( mCairo );
        // FIXME: Cairo has no API to control winding/even-odd clipping. See
        // https://lists.cairographics.org/archives/cairo/2009-September/018104.html
    }

    alphas.push_back( graphicStyle.opacity );
}

void CairoSVGRenderer::Restore()
{
    SVG_ASSERT( mCairo );
    alphas.pop_back();
    cairo_restore( mCairo );
}

inline double getAlphaProduct(std::list<double> alphas)
{
    double prod = 1.0;

    for (double a : alphas)
        prod = prod * a;

    return prod;
}

inline void createCairoPattern(const Paint& paint, float opacity, cairo_pattern_t** pat)
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

inline void appendCairoSvgPath( cairo_t* mCairo, const Path& path )
{
    cairo_append_path (mCairo, getPathObjFromCairoSvgPath(&path) );
}

void CairoSVGRenderer::DrawPath(
    const Path& path, const GraphicStyle& graphicStyle, const FillStyle& fillStyle, const StrokeStyle& strokeStyle)
{
    SVG_ASSERT(mCairo);
    Save(graphicStyle);

    double alpha = getAlphaProduct( alphas );

    if (fillStyle.hasFill)
    {
        if (fillStyle.paint.type() == typeid(Gradient)) {
            cairo_pattern_t* pat;
            createCairoPattern(fillStyle.paint, fillStyle.fillOpacity * alpha, &pat);
            cairo_set_source(mCairo, pat);
        } else {
            const auto& color = boost::get<Color>(fillStyle.paint);
            cairo_set_source_rgba(mCairo,
                                  static_cast<uint8_t>(color[0] * 255),
                                  static_cast<uint8_t>(color[1] * 255),
                                  static_cast<uint8_t>(color[2] * 255),
                                  static_cast<uint8_t>(color[3] * 255 * fillStyle.fillOpacity * alpha ));
        }

        // Skia backend does not handle fillStyle.Rule yet
        switch (fillStyle.fillRule) {
        case WindingRule::kEvenOdd:
            cairo_set_fill_rule (mCairo, CAIRO_FILL_RULE_EVEN_ODD);
            break;
        case WindingRule::kNonZero:
        default:
            cairo_set_fill_rule (mCairo, CAIRO_FILL_RULE_WINDING);
        }

        cairo_new_path (mCairo);
        appendCairoSvgPath( mCairo, path );
        cairo_fill (mCairo);
    }
    if (strokeStyle.hasStroke)
    {
        const auto& color = boost::get<Color>(strokeStyle.paint);
        cairo_set_source_rgba(mCairo,
                              static_cast<uint8_t>(color[0] * 255),
                              static_cast<uint8_t>(color[1] * 255),
                              static_cast<uint8_t>(color[2] * 255),
                              static_cast<uint8_t>(color[3] * 255 * strokeStyle.strokeOpacity * alpha ));

        cairo_set_line_width(mCairo, strokeStyle.lineWidth);
        switch (strokeStyle.lineCap) {
        case LineCap::kRound:
            cairo_set_line_cap(mCairo, CAIRO_LINE_CAP_ROUND);
            break;
        case LineCap::kSquare:
            cairo_set_line_cap(mCairo, CAIRO_LINE_CAP_SQUARE);
            break;
        case LineCap::kButt:
        default:
            cairo_set_line_cap(mCairo, CAIRO_LINE_CAP_BUTT);
        }

        switch (strokeStyle.lineJoin) {
	case LineJoin::kRound:
	    cairo_set_line_join(mCairo, CAIRO_LINE_JOIN_ROUND);
            break;
	case LineJoin::kBevel:
	    cairo_set_line_join(mCairo, CAIRO_LINE_JOIN_BEVEL);
            break;
	case LineJoin::kMiter:
	default:
	    cairo_set_line_join(mCairo, CAIRO_LINE_JOIN_MITER);
        }

        cairo_new_path (mCairo);
        appendCairoSvgPath( mCairo, path );
        cairo_stroke (mCairo);
    }
    Restore();
}

void CairoSVGRenderer::DrawImage(
    const ImageData& image, const GraphicStyle& graphicStyle, const Rect& clipArea, const Rect& fillArea)
{
    double alpha = getAlphaProduct( alphas );

    SVG_ASSERT(mCairo);
    Save(graphicStyle);
    cairo_new_path (mCairo);
    cairo_rectangle (mCairo, clipArea.x, clipArea.y, clipArea.width, clipArea.height);

    const CairoSVGImageData cairoSvgImgData = static_cast<const CairoSVGImageData&>(image);

    cairo_translate (mCairo, fillArea.x, fillArea.y);
    cairo_scale (mCairo, fillArea.width / cairoSvgImgData.Width(), fillArea.height / cairoSvgImgData.Height() );
    cairo_set_source_surface (mCairo, cairoSvgImgData.mImageData, 0, 0);
    cairo_paint_with_alpha (mCairo, alpha);

    Restore();
}

void CairoSVGRenderer::SetCairo(cairo_t* cr)
{
    SVG_ASSERT(cr);
    mCairo = cr;
}

} // namespace SVGNative
