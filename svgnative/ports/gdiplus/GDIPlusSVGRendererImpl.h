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

#ifndef SVGViewer_GDIPlusSVGRendererImpl_h
#define SVGViewer_GDIPlusSVGRendererImpl_h

#include "SVGRenderer.h"
#include <gdiplus.h>

namespace SVGNative
{
class GDIPlusSVGPath final : public Path
{
public:
    GDIPlusSVGPath();
    ~GDIPlusSVGPath();

    void Rect(float x, float y, float width, float height) override;
    void RoundedRect(float x, float y, float width, float height, float rx, float ry) override;
    void Ellipse(float cx, float cy, float rx, float ry) override;

    void MoveTo(float x, float y) override;
    void LineTo(float x, float y) override;
    void CurveTo(float x1, float y1, float x2, float y2, float x3, float y3) override;
    void CurveToV(float x2, float y2, float x3, float y3) override;
    void ClosePath() override;

    const Gdiplus::GraphicsPath& GetGraphicsPath() const;

private:
    Gdiplus::GraphicsPath mPath;
    float mCurrentX{};
    float mCurrentY{};
};

class GDIPlusSVGTransform final : public Transform
{
public:
    GDIPlusSVGTransform(float a, float b, float c, float d, float tx, float ty);

    void Set(float a, float b, float c, float d, float tx, float ty) override;
    void Rotate(float r) override;
    void Translate(float tx, float ty) override;
    void Scale(float sx, float sy) override;
    void Concat(const Transform& other) override;

    const Gdiplus::Matrix& GetMatrix() const;

private:
    Gdiplus::Matrix mTransform;
};

class GDIPlusSVGImageData final : public ImageData
{
public:
    GDIPlusSVGImageData(const std::string& base64, ImageEncoding encoding);
    ~GDIPlusSVGImageData();

    float Width() const override;
    float Height() const override;

    const std::unique_ptr<Gdiplus::Image>& GetImage() const;

private:
    std::unique_ptr<Gdiplus::Image> mImage;
};

} // namespace SVGNative

#endif // SVGViewer_GDIPlusSVGRendererImpl_h
