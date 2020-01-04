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

#ifndef SVGViewer_GDIPlusSVGRenderer_h
#define SVGViewer_GDIPlusSVGRenderer_h

#include "SVGRenderer.h"
#include <gdiplus.h>

namespace SVGNative
{
class SVG_IMP_EXP GDIPlusSVGRenderer final : public SVGRenderer
{
public:
    GDIPlusSVGRenderer();

    virtual ~GDIPlusSVGRenderer() 
    { 
    }

    std::unique_ptr<ImageData> CreateImageData(const std::string& base64, ImageEncoding encoding) override;

    std::unique_ptr<Path> CreatePath() override;

    std::unique_ptr<Transform> CreateTransform(
        float a = 1.0, float b = 0.0, float c = 0.0, float d = 1.0, float tx = 0.0, float ty = 0.0) override;

    void Save(const GraphicStyle& graphicStyle) override;
    void Restore() override;

    void DrawPath(const Path& path, const GraphicStyle& graphicStyle, const FillStyle& fillStyle, const StrokeStyle& strokeStyle) override;
    void DrawImage(const ImageData& image, const GraphicStyle& graphicStyle, const Rect& clipArea, const Rect& fillArea) override;

    void SetGraphicsContext(Gdiplus::Graphics* inContext)
    {
        mContext = inContext;
    }

    void ReleaseGraphicsContext()
    {
        mContext = nullptr;
    }

private:
    std::unique_ptr<Gdiplus::Brush> CreateGradientBrush(const Gradient& gradient, float opacity);

    Gdiplus::Graphics* mContext{};
    std::vector<Gdiplus::GraphicsState> mStateStack;
};

} // namespace SVGNative

#endif // SVGViewer_GDIPlusSVGRenderer_h
