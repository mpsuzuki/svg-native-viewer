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

typedef struct SVGNative_HiveRec_ {
    unsigned long  version; 
    std::map<std::string, DataInMap>  mRenderOptions;
    render_t   mRendererType;
    std::shared_ptr<SVGNative::SVGRenderer>  mRenderer;
    std::unique_ptr<SVGNative::SVGDocument>  mDocument;
} SVGNative_HiveRec;

SVGNative_Hive svgnative_hive_create()
{
    SVGNative_Hive hive = new SVGNative_HiveRec;
    hive->version = 0x00009000;
    hive->mRendererType = RENDER_NONE;
    return hive;
}

void svgnative_hive_destroy( SVGNative_Hive hive )
{
    if (!hive)
        return;
    hive->mRenderer.reset();
    hive->mDocument.reset();
    delete hive;
}

int svgnative_hive_install_renderer( SVGNative_Hive hive,
                                     render_t rendererType )
{
    if (!hive)
        return -1;

    switch(rendererType) {
#ifdef SVGViewer_StringSVGRenderer_h
    case RENDER_STRING:
        hive->mRenderer = std::make_shared<SVGNative::StringSVGRenderer>();
        hive->mRendererType = RENDER_STRING;
        return 0;
#endif

#ifdef SVGViewer_SkiaSVGRenderer_h
    case RENDER_SKIA:
        hive->mRenderer = std::make_shared<SVGNative::SkiaSVGRenderer>();
        hive->mRendererType = RENDER_SKIA;
        return 0;
#endif

#ifdef SVGViewer_CGSVGRenderer_h
    case RENDER_COREGRAPHICS:
        hive->mRenderer = std::make_shared<SVGNative::CGSVGRenderer>();
        hive->mRendererType = RENDER_COREGRAPHICS;
        return 0;
#endif

#ifdef SVGViewer_CairoSVGRenderer_h
    case RENDER_CAIRO:
        hive->mRenderer = std::make_shared<SVGNative::CairoSVGRenderer>();
        hive->mRendererType = RENDER_CAIRO;
        return 0;
#endif

#ifdef SVGViewer_QtSVGRenderer_h
    case RENDER_QT:
        hive->mRenderer = std::make_shared<SVGNative::QtSVGRenderer>();
        hive->mRendererType = RENDER_QT;
        return 0;
#endif

    default:
        hive->mRenderer.reset();
        hive->mRendererType = RENDER_NONE;
        return -1;
    }
}

render_t svgnative_hive_get_renderer_type( SVGNative_Hive hive )
{
    if (!hive)
        return RENDER_NONE;
    return hive->mRendererType;
}


int svgnative_hive_install_output( SVGNative_Hive hive,
                                   void* output )
{
    switch( svgnative_hive_get_renderer_type(hive) ) {
#ifdef SVGViewer_SkiaSVGRenderer_h
    case RENDER_SKIA:
         (std::dynamic_pointer_cast<SVGNative::SkiaSVGRenderer>(hive->mRenderer))->SetSkCanvas( (SkCanvas*)output ); 
         return 0;
#endif

#ifdef SVGViewer_CGSVGRenderer_h
    case RENDER_COREGRAPHICS:
        (std::dynamic_pointer_cast<SVGNative::CGSVGRenderer>(hive->mRenderer))->SetGraphicsContext( (CGContextRef)output ); 
        return 0;
#endif

#ifdef SVGViewer_CairoSVGRenderer_h
    case RENDER_CAIRO:
        (std::dynamic_pointer_cast<SVGNative::CairoSVGRenderer>(hive->mRenderer))->SetCairo( (cairo_t*)output ); 
        return 0;
#endif

#ifdef SVGViewer_QtSVGRenderer_h
    case RENDER_QT:
        (std::dynamic_pointer_cast<SVGNative::QtSVGRenderer>(hive->mRenderer))->SetQPainter( (QPainter*)output ); 
        return 0;
#endif

    default:
        return -1;
    }
}

int svgnative_hive_install_document_from_buffer( SVGNative_Hive hive,
                                                 char* buff )
{
    /* we cannot create SVGDocument without SVGRenderer */
    if (1 > svgnative_hive_get_renderer_type(hive))
        return -1;

    hive->mDocument = std::unique_ptr<SVGNative::SVGDocument>( SVGNative::SVGDocument::CreateSVGDocument(buff, hive->mRenderer) );
    return 0;
}

int svgnative_hive_render_installed_document( SVGNative_Hive hive )
{
    if (!hive || !hive->mDocument)
        return -1;
    hive->mDocument->Render();
    return 0;
}

long svgnative_hive_get_width_from_installed_document( SVGNative_Hive hive )
{
    if (!hive || !hive->mDocument)
        return -1;
    return hive->mDocument->Width();
}

long svgnative_hive_get_height_from_installed_document( SVGNative_Hive hive )
{
    if (!hive || !hive->mDocument)
        return -1;
    return hive->mDocument->Height();
}
