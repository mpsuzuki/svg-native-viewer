#include "SVGDocument.h"
#include <list>
#include <map>
#include <fstream>
#include <iostream>
#include <string>
#include <cctype>
#include <typeinfo> // to use typeid

#include "SVGNativeCAPI.h"

#include "StringSVGRenderer.h"

#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkiaSVGRenderer.h"

#include "cairo.h"
#include "CairoSVGRenderer.h"

#include <QPicture>
#include "QtSVGRenderer.h"

typedef struct DataInMap_
{
    data_t  dataType;
    union {
        unsigned char*  ustr;

        int32_t   i32;
        uint32_t  ui32;
        double    dbl;

        int32_t   s2_i32[2];
        uint32_t  s2_ui32[2];
        double    s2_dbl[2];

        int32_t   s4_i32[4];
        uint32_t  s4_ui32[4];
        double    s4_dbl[4];

        int32_t   s6_i32[6];
        uint32_t  s6_ui32[6];
        double    s6_dbl[6];
    };
} DataInMap;

class RenderHive
{
public:
    RenderHive();
    ~RenderHive();

    unsigned long getVersion(){ return version; };
    render_t getRendererType(){ return mRendererType; };
    int installRenderer(render_t);
    int installOutput(void*);

    int installDocumentFromBuff(char*);
    int renderDocument();
    long getWidthFromDocument();
    long getHeightFromDocument();

private:
    unsigned long  version; 
    std::map<std::string, DataInMap>  mRenderOptions;
    render_t   mRendererType;
    std::shared_ptr<SVGNative::SVGRenderer>  mRenderer;
    std::unique_ptr<SVGNative::SVGDocument>  mDocument;
};


RenderHive::RenderHive()
{
    version = 0x00009000;
    mRendererType = RENDER_NONE;
}

RenderHive::~RenderHive()
{
    mRenderer.reset();
    mDocument.reset();
}

int RenderHive::installRenderer(render_t rendererType)
{
    switch(rendererType) {
#ifdef SVGViewer_StringSVGRenderer_h
    case RENDER_STRING:
        mRenderer = std::make_shared<SVGNative::StringSVGRenderer>();
        mRendererType = RENDER_STRING;
        return 0;
#endif

#ifdef SVGViewer_SkiaSVGRenderer_h
    case RENDER_SKIA:
        mRenderer = std::make_shared<SVGNative::SkiaSVGRenderer>();
        mRendererType = RENDER_SKIA;
        return 0;
#endif

#ifdef SVGViewer_CGSVGRenderer_h
    case RENDER_COREGRAPHICS:
        mRenderer = std::make_shared<SVGNative::CGSVGRenderer>();
        mRendererType = RENDER_COREGRAPHICS;
        return 0;
#endif

#ifdef SVGViewer_CairoSVGRenderer_h
    case RENDER_CAIRO:
        mRenderer = std::make_shared<SVGNative::CairoSVGRenderer>();
        mRendererType = RENDER_CAIRO;
        return 0;
#endif

#ifdef SVGViewer_QtSVGRenderer_h
    case RENDER_QT:
        mRenderer = std::make_shared<SVGNative::QtSVGRenderer>();
        mRendererType = RENDER_QT;
        return 0;
#endif

    default:
        mRendererType = RENDER_NONE;
        return -1;
    }
}

int RenderHive::installOutput(void* output)
{
    switch( getRendererType() ) {
#ifdef SVGViewer_SkiaSVGRenderer_h
    case RENDER_SKIA:
         (std::dynamic_pointer_cast<SVGNative::SkiaSVGRenderer>(mRenderer))->SetSkCanvas( (SkCanvas*)output ); 
         return 0;
#endif

#ifdef SVGViewer_CGSVGRenderer_h
    case RENDER_COREGRAPHICS:
        (std::dynamic_pointer_cast<SVGNative::CGSVGRenderer>(mRenderer))->SetGraphicsContext( (CGContextRef)output ); 
        return 0;
#endif

#ifdef SVGViewer_CairoSVGRenderer_h
    case RENDER_CAIRO:
        (std::dynamic_pointer_cast<SVGNative::CairoSVGRenderer>(mRenderer))->SetCairo( (cairo_t*)output ); 
        return 0;
#endif

#ifdef SVGViewer_QtSVGRenderer_h
    case RENDER_QT:
        (std::dynamic_pointer_cast<SVGNative::QtSVGRenderer>(mRenderer))->SetQPainter( (QPainter*)output ); 
        return 0;
#endif

    default:
        return -1;
    }
}

int RenderHive::installDocumentFromBuff(char* buff)
{
    if (1 > getRendererType())
        return -1;
    mDocument = std::unique_ptr<SVGNative::SVGDocument>( SVGNative::SVGDocument::CreateSVGDocument(buff, mRenderer) );
    return 0;
}

int RenderHive::renderDocument()
{
    if (!mDocument)
        return -1;
    mDocument->Render();
    return 0;
}

long RenderHive::getWidthFromDocument()
{
    return mDocument->Width();
}

long RenderHive::getHeightFromDocument()
{
    return mDocument->Height();
}


/* ------------------------------------------------------- */

std::vector<RenderHive*> hives;

int appendHive()
{
    hives.emplace_back( new RenderHive );
    return hives.size() - 1;
}

int removeHive(int i)
{
    if (hives.size() - 1 < i || hives[i])
        return -1;
    RenderHive* aHive = hives[i];

    hives.pop_back();
    delete aHive;
    return hives.size();
}

int installRendererToHive(int i, render_t r)
{
    if (hives.size() - 1 < i || hives[i])
        return -1;
    return hives[i]->installRenderer(r);
}

render_t getRendererTypeFromHive(int i)
{
    if (hives.size() - 1 < i || hives[i])
        return RENDER_NONE;
    return hives[i]->getRendererType();
}

int installOutputToHive(int i, void* o)
{
    if (hives.size() - 1 < i || hives[i])
        return -1;
    return hives[i]->installOutput(o);
}

int installDocumentToHive(int i, char* buff)
{
    if (hives.size() - 1 < i || hives[i])
        return -1;
    return hives[i]->installDocumentFromBuff(buff);
}

void renderDocumentInHive(int i)
{
    if (hives.size() - 1 < i || hives[i])
        return;
    hives[i]->renderDocument();
}

long getWidthFromDocumentInHive(int i)
{
    if (hives.size() - 1 < i || hives[i])
        return -1;
    return hives[i]->getWidthFromDocument();
}

long getHeightFromDocumentInHive(int i)
{
    if (hives.size() - 1 < i || hives[i])
        return -1;
    return hives[i]->getHeightFromDocument();
}
