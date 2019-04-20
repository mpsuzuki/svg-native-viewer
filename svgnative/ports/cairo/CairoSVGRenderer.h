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

#ifndef SVGViewer_CairoSVGRenderer_h
#define SVGViewer_CairoSVGRenderer_h

#include "SVGRenderer.h"
#include "cairo.h"

// struct SkRect;
// class SkCanvas;
// class SkImage;

namespace SVGNative
{
class CairoSVGPath final : public Path
{
public:
    CairoSVGPath();

    void Rect(float x, float y, float width, float height) override;
    void RoundedRect(float x, float y, float width, float height, float cornerRadius) override;
    void Ellipse(float cx, float cy, float rx, float ry) override;

    void MoveTo(float x, float y) override;
    void LineTo(float x, float y) override;
    void CurveTo(float x1, float y1, float x2, float y2, float x3, float y3) override;
    void CurveToV(float x2, float y2, float x3, float y3) override;
    void ClosePath() override;

    // Native SVG's path object could be a collection of multiple paths.
    // Skia's rect, round-rect, ellipse APIs can add new path object to existing path object,
    // but Cairo's has no such API to add existing path object.
    // thus we cannot use cairo_path_t object as the internal data, we have to use opaque cairo_t object,
    // to hold a surface.
    cairo_t* mPath;

private:
    float mCurrentX{};
    float mCurrentY{};
};

class CairoSVGTransform final : public Transform
{
public:
    CairoSVGTransform(float a, float b, float c, float d, float tx, float ty);

    void Set(float a, float b, float c, float d, float tx, float ty) override;
    void Rotate(float r) override;
    void Translate(float tx, float ty) override;
    void Scale(float sx, float sy) override;
    void Concat(const Transform& other) override;

    cairo_matrix_t* mMatrix;
};

class CairoSVGImageData final : public ImageData
{
public:
    CairoSVGImageData(const std::string& base64, ImageEncoding encoding);

    float Width() const override;

    float Height() const override;

    // Cairo has no special data type to pass a raster data to the renderer.
    // cairo_surface_t is a dual-use type, for renderin destination, and
    // for a raster data to be rendered.
    cairo_surface_t* mImageData;
};

class CairoSVGRenderer final : public SVGRenderer
{
public:
    CairoSVGRenderer();

    std::unique_ptr<ImageData> CreateImageData(const std::string& base64, ImageEncoding encoding) override { return std::make_unique<CairoSVGImageData>(base64, encoding); }

    std::unique_ptr<Path> CreatePath() override { return std::make_unique<CairoSVGPath>(); }

    std::unique_ptr<Transform> CreateTransform(
        float a = 1.0, float b = 0.0, float c = 0.0, float d = 1.0, float tx = 0.0, float ty = 0.0) override
    {
        return std::make_unique<CairoSVGTransform>(a, b, c, d, tx, ty);
    }

    void Save(const GraphicStyle& graphicStyle) override;
    void Restore() override;

    void DrawPath(const Path& path, const GraphicStyle& graphicStyle, const FillStyle& fillStyle, const StrokeStyle& strokeStyle) override;
    void DrawImage(const ImageData& image, const GraphicStyle& graphicStyle, const Rect& clipArea, const Rect& fillArea) override;

    void SetCairoSurface(cairo_surface_t* surface);

private:
    // following to the terminology of Skia, but Cairo does not use the term "canvas".
    cairo_surface_t* mCanvas;
};

} // namespace SVGNative

#endif // SVGViewer_CairoSVGRenderer_h
