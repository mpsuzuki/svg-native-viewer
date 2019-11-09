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

#ifndef SVGViewer_QtSVGRenderer_h
#define SVGViewer_QtSVGRenderer_h

#include <list>
#include "SVGRenderer.h"

#include <QVector>
#include <QPainterPath>
#include <QTransform>
#include <QImage>
#include <QPainter>

namespace SVGNative
{

class QtSVGPath final : public Path
{
public:
    QtSVGPath();
    ~QtSVGPath();

    void Rect(float x, float y, float width, float height) override;
    void RoundedRect(float x, float y, float width, float height, float cornerRadius) override;
    void Ellipse(float cx, float cy, float rx, float ry) override;

    void MoveTo(float x, float y) override;
    void LineTo(float x, float y) override;
    void CurveTo(float x1, float y1, float x2, float y2, float x3, float y3) override;
    void CurveToV(float x2, float y2, float x3, float y3) override;
    void ClosePath() override;

    QPainterPath mPath;

private:
    float mCurrentX{};
    float mCurrentY{};
};

class QtSVGTransform final : public Transform
{
public:
    QtSVGTransform(float a, float b, float c, float d, float tx, float ty);

    void Set(float a, float b, float c, float d, float tx, float ty) override;
    void Rotate(float r) override;
    void Translate(float tx, float ty) override;
    void Scale(float sx, float sy) override;
    void Concat(const Transform& other) override;

    QTransform mTransform;
};

class QtSVGImageData final : public ImageData
{
public:
    QtSVGImageData(const std::string& base64, ImageEncoding encoding);
    ~QtSVGImageData();

    float Width() const override;

    float Height() const override;


    QImage mImageData;
};

class QtSVGRenderer final : public SVGRenderer
{
public:
    QtSVGRenderer();
    ~QtSVGRenderer();

    std::unique_ptr<ImageData> CreateImageData(const std::string& base64, ImageEncoding encoding) override
    {
        return std::unique_ptr<QtSVGImageData>(new QtSVGImageData(base64, encoding));
    }

    std::unique_ptr<Path> CreatePath() override
    {
        return std::unique_ptr<QtSVGPath>(new QtSVGPath);
    }

    std::unique_ptr<Transform> CreateTransform(
        float a = 1.0, float b = 0.0, float c = 0.0, float d = 1.0, float tx = 0.0, float ty = 0.0) override
    {
        return std::unique_ptr<QtSVGTransform>(new QtSVGTransform(a, b, c, d, tx, ty));
    }

    void Save(const GraphicStyle& graphicStyle) override;
    void Restore() override;

    void DrawPath(const Path& path, const GraphicStyle& graphicStyle, const FillStyle& fillStyle, const StrokeStyle& strokeStyle) override;
    void DrawImage(const ImageData& image, const GraphicStyle& graphicStyle, const Rect& clipArea, const Rect& fillArea) override;

    void SetQPainter(QPainter* qpainter);

private:
    std::list<double> alphas;
    QPainter *mQPainter;
};

} // namespace SVGNative

#endif // SVGViewer_QtSVGRenderer_h
