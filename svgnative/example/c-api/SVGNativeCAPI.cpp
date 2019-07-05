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
    std::shared_ptr<void>  renderer;
    SVGNative::SVGDocument  doc;
} RendererHive;

RendererHive*  hive;

/* ------------------------------------------------------- */

void* createHive()
{
    hive = (RendererHive*)malloc(sizeof(RendererHive));

    hive->version = 0x00009000;
    hive->render_options = new std::map<std::string, DataInMap>;

    return (void*)hive;
}

void deleteHive()
{
    if ( !hive )
        return;

    if ( hive->renderer )
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
        hive->renderer = std::make_shared<SVGNative::SkiaSVGRenderer>();; 
        break;
#if 0
    case RENDER_COREGRAPHICS:
        hive->renderer = std::make_shared<SVGNative::CGSVGRenderer>();; 
        break;
#endif
    case RENDER_CAIRO:
        hive->renderer = std::make_shared<SVGNative::CairoSVGRenderer>();; 
        break;
    case RENDER_QT:
        hive->renderer = std::make_shared<SVGNative::QtSVGRenderer>();; 
        break;
    default:
        hive->renderer = std::make_shared<SVGNative::StringSVGRenderer>();; 
    };
    return 0;
}

render_t getRendererType()
{
    if ( !hive )
        return RENDER_NONE;

    size_t hc = typeid( hive->renderer ).hash_code();
    if (hc == typeid( SVGNative::StringSVGRenderer ).hash_code())
        return RENDER_STRING;
    if (hc == typeid( SVGNative::SkiaSVGRenderer ).hash_code())
        return RENDER_SKIA;
#if 0
    if (hc == typeid( SVGNative::CGSVGRenderer ).hash_code())
        return RENDER_COREGRAPHICS;
#endif
    if (hc == typeid( SVGNative::CairoSVGRenderer ).hash_code())
        return RENDER_CAIRO;
    if (hc == typeid( SVGNative::QtSVGRenderer ).hash_code())
        return RENDER_QT;

    return RENDER_NONE;
}

int deleteRenderer()
{
    if ( !hive )
        return -1;

    hive->renderer.reset();

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
        (static_pointer_cast<SVGNative::SkiaSVGRenderer>(hive->renderer))->SetSkCanvas( (SkCanvas*)output ); 
        break;
#if 0
    case RENDER_COREGRAPHICS:
        (static_pointer_cast<SVGNative::CGSVGRenderer>(hive->renderer))->SetGraphicsContext( (CGContextRef)output ); 
        break;
#endif
    case RENDER_CAIRO:
        (static_pointer_cast<SVGNative::CairoSVGRenderer>(hive->renderer))->SetCairo( (cairo_t*)output ); 
        break;
    case RENDER_QT:
        (static_pointer_cast<SVGNative::QtSVGRenderer>(hive->renderer))->SetQPainter( (QPainter*)output ); 
        break;
    };
}

int createSvgDocument(unsigned char* buff)
{
    if ( !hive || !hive->renderer )
        return -1;

    hive->doc = SVGNative::SVGDocument::CreateSVGDocument(buff, static_cast<SVGNative::SVGRenderer*>(hive->renderer));
}

int deleteSvgDocument()
{
    if ( !hive || !hive->doc )
        return -1;

    hive->doc.reset();
}
