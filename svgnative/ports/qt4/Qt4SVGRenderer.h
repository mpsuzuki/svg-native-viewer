/*
Copyright 2019 suzuki toshiya <mpsuzuki@hiroshima-u.ac.jp>. All rights reserved.
This file is licensed to you under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License. You may obtain a copy
of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS
OF ANY KIND, either express or implied. See the License for the specific language
governing permissions and limitations under the License.
*/

#ifndef SVGViewer_Qt4SVGRenderer_h
#define SVGViewer_Qt4SVGRenderer_h

#include <list>
#include "SVGRenderer.h"

#include <QVector>
#include <QPainterPath>
#include <QTransform>
#include <QImage>
#include <QPainter>

namespace SVGNative
{

class Qt4SVGPath final : public Path
{
public:
    Qt4SVGPath();
    ~Qt4SVGPath();

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

class Qt4SVGTransform final : public Transform
{
public:
    Qt4SVGTransform(float a, float b, float c, float d, float tx, float ty);

    void Set(float a, float b, float c, float d, float tx, float ty) override;
    void Rotate(float r) override;
    void Translate(float tx, float ty) override;
    void Scale(float sx, float sy) override;
    void Concat(const Transform& other) override;

    QTransform mTransform;
};

class Qt4SVGImageData final : public ImageData
{
public:
    Qt4SVGImageData(const std::string& base64, ImageEncoding encoding);
    ~Qt4SVGImageData();

    float Width() const override;

    float Height() const override;


    QImage mImageData;
};

class Qt4SVGRenderer final : public SVGRenderer
{
public:
    Qt4SVGRenderer();
    ~Qt4SVGRenderer();

    std::unique_ptr<ImageData> CreateImageData(const std::string& base64, ImageEncoding encoding) override
    {
        return std::unique_ptr<Qt4SVGImageData>(new Qt4SVGImageData(base64, encoding));
    }

    std::unique_ptr<Path> CreatePath() override
    {
        return std::unique_ptr<Qt4SVGPath>(new Qt4SVGPath);
    }

    std::unique_ptr<Transform> CreateTransform(
        float a = 1.0, float b = 0.0, float c = 0.0, float d = 1.0, float tx = 0.0, float ty = 0.0) override
    {
        return std::unique_ptr<Qt4SVGTransform>(new Qt4SVGTransform(a, b, c, d, tx, ty));
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

#endif // SVGViewer_Qt4SVGRenderer_h
