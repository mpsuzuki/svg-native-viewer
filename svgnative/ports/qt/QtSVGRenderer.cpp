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

#include "QtSVGRenderer.h"
#include "base64.h"
#include "Config.h"
#include <math.h>

namespace SVGNative
{
QtSVGPath::QtSVGPath()
{
}

QtSVGPath::~QtSVGPath()
{
}

void QtSVGPath::Rect(float x, float y, float width, float height)
{
    mPath.addRect((qreal)x, (qreal)y, (qreal)width, (qreal)height);
}

void QtSVGPath::RoundedRect(float x, float y, float width, float height, float cornerRadius)
{
    mPath.addRoundedRect((qreal)x, (qreal)y, (qreal)width, (qreal)height,
                         (qreal)cornerRadius, (qreal)cornerRadius, Qt::AbsoluteSize);
}

void QtSVGPath::Ellipse(float cx, float cy, float rx, float ry) {
    mPath.addEllipse((qreal)(cx - rx), (qreal)(cy - ry), (qreal)(rx * 2), (qreal)(ry * 2));
}

void QtSVGPath::MoveTo(float x, float y)
{
    mPath.moveTo((qreal)x, (qreal)y);
    mCurrentX = x;
    mCurrentY = y;
}

void QtSVGPath::LineTo(float x, float y)
{
    mPath.lineTo((qreal)x, (qreal)y);
    mCurrentX = x;
    mCurrentY = y;
}

void QtSVGPath::CurveTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    mPath.cubicTo((qreal)x1, (qreal)y1, (qreal)x2, (qreal)y2, (qreal)x3, (qreal)y3);
    mCurrentX = x3;
    mCurrentY = y3;
}

void QtSVGPath::CurveToV(float x2, float y2, float x3, float y3)
{
    mPath.cubicTo((qreal)mCurrentX, (qreal)mCurrentY, (qreal)x2, (qreal)y2, (qreal)x3, (qreal)y3);
    mCurrentX = x3;
    mCurrentY = y3;
}

void QtSVGPath::ClosePath() {
    mPath.closeSubpath();
}

QtSVGTransform::QtSVGTransform(float a, float b, float c, float d, float tx, float ty) {
    mTransform = QTransform((qreal)a, (qreal)b, (qreal)c, (qreal)d, (qreal)tx, (qreal)ty);
}

void QtSVGTransform::Set(float a, float b, float c, float d, float tx, float ty) {
    mTransform.setMatrix((qreal)a,  (qreal)b,  0,
                         (qreal)c,  (qreal)d,  0,
                         (qreal)tx, (qreal)ty, 1.0);
}

void QtSVGTransform::Rotate(float degree) {
    mTransform.rotate((qreal)degree, Qt::ZAxis);
}

void QtSVGTransform::Translate(float tx, float ty) {
    mTransform.translate((qreal)tx, (qreal)ty);
}

void QtSVGTransform::Scale(float sx, float sy) {
    mTransform.scale((qreal)sx, (qreal)sy);
}

void QtSVGTransform::Concat(const Transform& other) {
    mTransform *= (static_cast<const QtSVGTransform&>(other)).mTransform;
}

QtSVGImageData::QtSVGImageData(const std::string& base64, ImageEncoding encoding)
{
    QByteArray q_blob = QByteArray::fromBase64(base64.c_str());

    switch (encoding) {
    case ImageEncoding::kJPEG:
        if (mImageData.loadFromData(q_blob, "JPG"))
            return;
        break;
    case ImageEncoding::kPNG:
        if (mImageData.loadFromData(q_blob, "PNG"))
            return;
        break;
    }
    throw ("image is broken, or not PNG or JPEG\n");
}

QtSVGImageData::~QtSVGImageData()
{
}

float QtSVGImageData::Width() const
{
    return static_cast<float>( mImageData.width() );
}

float QtSVGImageData::Height() const
{
    return static_cast<float>( mImageData.height() );
}

QtSVGRenderer::QtSVGRenderer()
{
}

QtSVGRenderer::~QtSVGRenderer()
{
}

void QtSVGRenderer::Save(const GraphicStyle& graphicStyle)
{
    SVG_ASSERT( mQPainter );
    mQPainter->save();

    if (graphicStyle.transform)
        mQPainter->setTransform( (static_cast<QtSVGTransform*>(graphicStyle.transform.get())->mTransform), true /* combined */);

    if (graphicStyle.clippingPath && graphicStyle.clippingPath->path)
    {
        QPainterPath clippingPath = static_cast<QtSVGPath*>(graphicStyle.clippingPath->path.get())->mPath;

        if (graphicStyle.clippingPath->transform)
        {
            QTransform xformForClippingPath = static_cast<const QtSVGTransform*>(graphicStyle.clippingPath->transform.get())->mTransform;
            clippingPath = xformForClippingPath.map(clippingPath);
        }
        
        mQPainter->setClipPath( clippingPath, Qt::ReplaceClip );
    }

    alphas.push_back( graphicStyle.opacity );
}

void QtSVGRenderer::Restore()
{
    SVG_ASSERT( mQPainter );
    alphas.pop_back();
    mQPainter->restore();
}

inline double getAlphaProduct(std::list<double> alphas)
{
    double prod = 1.0;

    for (double a : alphas)
        prod = prod * a;

    return prod;
}

void QtSVGRenderer::DrawPath(
    const Path& path, const GraphicStyle& graphicStyle, const FillStyle& fillStyle, const StrokeStyle& strokeStyle)
{
    SVG_ASSERT(mQPainter);
    Save(graphicStyle);

    double alpha = getAlphaProduct( alphas );
    QPainterPath qPath = (static_cast<const QtSVGPath&>(path)).mPath;

    if (fillStyle.hasFill)
    {
        QBrush qBrush;

        if (fillStyle.paint.type() == typeid(Gradient)) {
            const auto& gradient = boost::get<Gradient>(fillStyle.paint);
            QGradient qGradient;

            if (gradient.type == GradientType::kLinearGradient)
            {
                qGradient = QLinearGradient( (qreal)gradient.x1, (qreal)gradient.y1,
                                             (qreal)gradient.x2, (qreal)gradient.y2 );
            }
            else if (gradient.type == GradientType::kRadialGradient)
            {
                qGradient = QRadialGradient( (qreal)gradient.cx, (qreal)gradient.cy, (qreal)gradient.r,
                                             (qreal)gradient.fx, (qreal)gradient.fy, (qreal)0 );
            }

            for (const auto& stop : gradient.colorStops)
            {
                const auto& stopOffset = stop.first;
                const auto& stopColor  = stop.second;
                QColor qColor;
                qColor.setRgbF( stopColor[0], stopColor[1], stopColor[2], stopColor[3] * fillStyle.fillOpacity * alpha );
                qGradient.setColorAt((qreal)stopOffset, qColor);
            }
            qBrush = QBrush( qGradient );

        } else {
            qBrush.setStyle(Qt::SolidPattern);

            const auto& color = boost::get<Color>(fillStyle.paint);
            QColor qColor;
            qColor.setRgbF( color[0], color[1], color[2], color[3] * fillStyle.fillOpacity * alpha );
            qBrush.setColor( qColor );
        }
  

        mQPainter->setBrush(qBrush);

        switch (fillStyle.fillRule) {
        case WindingRule::kEvenOdd:
            qPath.setFillRule(Qt::OddEvenFill);
            break;
        case WindingRule::kNonZero:
        default:
            qPath.setFillRule(Qt::WindingFill);
        }

        mQPainter->fillPath(qPath, qBrush);
    }
    if (strokeStyle.hasStroke && strokeStyle.lineWidth > 0)
    {
        const auto& color = boost::get<Color>(strokeStyle.paint);
        QPen qPen;
        QColor qColor;
        qColor.setRgbF( color[0], color[1], color[2], color[3] * strokeStyle.strokeOpacity * alpha );
        qPen.setColor( qColor );
        qPen.setWidthF( (qreal)strokeStyle.lineWidth );

        switch (strokeStyle.lineCap) {
        case LineCap::kRound:
            qPen.setCapStyle(Qt::RoundCap);
            break;
        case LineCap::kSquare:
            qPen.setCapStyle(Qt::SquareCap);
            break;
        case LineCap::kButt:
        default:
            qPen.setCapStyle(Qt::FlatCap);
        }

        switch (strokeStyle.lineJoin) {
	case LineJoin::kRound:
            qPen.setJoinStyle(Qt::RoundJoin);
            break;
	case LineJoin::kBevel:
            qPen.setJoinStyle(Qt::BevelJoin);
            break;
	case LineJoin::kMiter:
	default:
            qPen.setJoinStyle(Qt::MiterJoin);
        }

        if (!strokeStyle.dashArray.empty()) {
            QVector<qreal> qDashes;
            qDashes.reserve( strokeStyle.dashArray.size() );
            for (size_t i = 0; i < strokeStyle.dashArray.size(); i ++ )
                qDashes.push_back( (qreal)strokeStyle.dashArray[i] / (qreal)strokeStyle.lineWidth  );
            qPen.setDashPattern(qDashes);
            qPen.setDashOffset((qreal)strokeStyle.dashOffset / (qreal)strokeStyle.lineWidth );
        }

        mQPainter->strokePath(qPath, qPen);
    }
    Restore();
}

void QtSVGRenderer::DrawImage(
    const ImageData& image, const GraphicStyle& graphicStyle, const Rect& clipArea, const Rect& fillArea)
{
    double alpha = getAlphaProduct( alphas );

    SVG_ASSERT(mQPainter);
    Save(graphicStyle);
    mQPainter->setClipRect((int)clipArea.x, (int)clipArea.y, (int)clipArea.width, (int)clipArea.height, Qt::ReplaceClip);

    mQPainter->setOpacity(alpha);
    QImage mImageData = (static_cast<const QtSVGImageData&>(image)).mImageData.scaled(fillArea.width, fillArea.height);
    mQPainter->drawImage((int)fillArea.x, (int)fillArea.y,
                         mImageData,
                         0, 0, (int)fillArea.width, (int)fillArea.height,
                         Qt::AutoColor);

    Restore();
}

void QtSVGRenderer::SetQPainter(QPainter* qPainter)
{
    SVG_ASSERT(qPainter);
    mQPainter = qPainter;
}

} // namespace SVGNative
