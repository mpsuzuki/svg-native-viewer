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

typedef struct RendererHive_
{
    unsigned long           version;
    std::map<std::string, DataInMap>*  render_options;
    render_t               renderType;
    std::unique_ptr<SVGNative::SVGDocument>        doc;
    std::shared_ptr<SVGNative::StringSVGRenderer>  renderString;
    std::shared_ptr<SVGNative::SkiaSVGRenderer>    renderSkia;
#if 0
    std::shared_ptr<SVGNative::CGSVGRenderer>      renderCoreGraphics;
#endif
    std::shared_ptr<SVGNative::CairoSVGRenderer>   renderCairo;
    std::shared_ptr<SVGNative::QtSVGRenderer>      renderQt;
} RendererHive;

RendererHive*  hive;

/* ------------------------------------------------------- */

void* createHive()
{
    hive = (RendererHive*)malloc(sizeof(RendererHive));

    hive->version = 0x00009000;
    hive->render_options = new std::map<std::string, DataInMap>;
    hive->renderType = RENDER_NONE;

    return (void*)hive;
}

void deleteHive()
{
    if ( !hive )
        return;

    if ( getRendererType() > 0 )
        deleteRenderer();

    delete hive->render_options;
    free( hive );
}

int createRenderer(render_t render_type)
{
    if ( !hive )
        return -1;

    switch (render_type) {
    case RENDER_NONE:
        return -1;

    case RENDER_SKIA:
        hive->renderSkia = std::make_shared<SVGNative::SkiaSVGRenderer>();; 
        hive->renderType = RENDER_SKIA;
        break;
#if 0
    case RENDER_COREGRAPHICS:
        hive->renderCoreGraphics = std::make_shared<SVGNative::CGSVGRenderer>();; 
        hive->renderType = RENDER_COREGRAPHICS;
        break;
#endif
    case RENDER_CAIRO:
        hive->renderCairo = std::make_shared<SVGNative::CairoSVGRenderer>();; 
        hive->renderType = RENDER_CAIRO;
        break;
    case RENDER_QT:
        hive->renderQt = std::make_shared<SVGNative::QtSVGRenderer>();; 
        hive->renderType = RENDER_QT;
        break;
    default:
        hive->renderString = std::make_shared<SVGNative::StringSVGRenderer>();; 
        hive->renderType = RENDER_STRING;
    };
    return 0;
}

render_t getRendererType()
{
    if ( !hive )
        return RENDER_NONE;

    return hive->renderType;
}

int deleteRenderer()
{
    if ( !hive )
        return -1;

    switch (getRendererType()) {
    case RENDER_STRING:
        hive->renderString.reset();
        break;
    case RENDER_SKIA:
        hive->renderSkia.reset();
        break;
#if 0
    case RENDER_COREGRAPHICS:
        hive->renderCoreGraphics.reset();
        break;
#endif
    case RENDER_CAIRO:
        hive->renderCairo.reset();
        break;
    case RENDER_QT:
        hive->renderQt.reset();
        break;
    };

    return 0;
}

int recreateRenderer()
{
    render_t r = getRendererType();
    if (1 > r)
        return -1;

    deleteRenderer();
    createRenderer(r);

    return 0;
}

int setOutputForRenderer(void* output)
{
    switch( getRendererType() ) {
    case RENDER_SKIA:
        hive->renderSkia->SetSkCanvas( (SkCanvas*)output ); 
        break;
#if 0
    case RENDER_COREGRAPHICS:
        hive->rendererCoreGraphics->SetGraphicsContext( (CGContextRef)output ); 
        break;
#endif
    case RENDER_CAIRO:
        hive->renderCairo->SetCairo( (cairo_t*)output ); 
        break;
    case RENDER_QT:
        hive->renderQt->SetQPainter( (QPainter*)output ); 
        break;
    };
}

int createSvgDocument(char* buff)
{
    render_t r = getRendererType();
    switch (r) {
    case RENDER_STRING:
        hive->doc = SVGNative::SVGDocument::CreateSVGDocument(buff, hive->renderString);
        break;
    case RENDER_SKIA:
        hive->doc = SVGNative::SVGDocument::CreateSVGDocument(buff, hive->renderSkia);
        break;
#if 0
    case RENDER_COREGRAPHICS:
        hive->doc = SVGNative::SVGDocument::CreateSVGDocument(buff, hive->renderCoreGraphics);
        break;
#endif
    case RENDER_CAIRO:
        hive->doc = SVGNative::SVGDocument::CreateSVGDocument(buff, hive->renderCairo);
        break;
    case RENDER_QT:
        hive->doc = SVGNative::SVGDocument::CreateSVGDocument(buff, hive->renderQt);
        break;
    default:
        return -1;
    }
}

int deleteSvgDocument()
{
    if ( !hive || !hive->doc )
        return -1;

    hive->doc.reset();
}
