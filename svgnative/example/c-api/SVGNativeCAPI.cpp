#include "SVGDocument.h"
#include <list>
#include <map>
#include <fstream>
#include <iostream>
#include <string>
#include <cctype>

#include "SVGNativeCAPI.h"

#ifdef USE_TEXT
#include "StringSVGRenderer.h"
#endif

#ifdef USE_SKIA
#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkCanvas.h"
#include "SkiaSVGRenderer.h"
#endif

#ifdef USE_CAIRO
#include "cairo.h"
#include "CairoSVGRenderer.h"
#endif

#ifdef USE_QT
#include <QPicture>
#include <QPainter>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include "QtSVGRenderer.h"
#endif

typedef struct SVGNative_HiveRec_ {
    unsigned long  version; 
    std::map<std::string, void*>  mRenderOptions;
    render_t   mRendererType;
    std::shared_ptr<SVGNative::SVGRenderer>  mRenderer;
    std::unique_ptr<SVGNative::SVGDocument>  mDocument;

    bool hasInternalOutput;
    /* -- because SkSurface is always created in std::shared_ptr,
          we cannot case them into unified void pointer
     */
#if USE_SKIA
    sk_sp<SkSurface> mSkiaSurface;
    SkCanvas* mSkiaCanvas;
#else
    void* mSkiaSurface;
    void* mSkiaCanvas;
#endif

#if USE_CAIRO
    cairo_surface_t* mCairoSurface;
    cairo_t* mCairo;
#else
    void* mCairoSurface;
    void* mCairo;
#endif

#if USE_QT
    QImage mQImage;
    QPainter* mQPainter;
#else
    void* mQImage;
    void* mQPainter;
#endif

} SVGNative_HiveRec;

SVGNative_Hive svgnative_hive_create()
{
    SVGNative_Hive hive = new SVGNative_HiveRec();
    hive->version = 0x00009000;
    hive->mRendererType = RENDER_NONE;
    hive->hasInternalOutput = FALSE;
    return hive;
}

void svgnative_hive_destroy( SVGNative_Hive hive )
{
    if (!hive)
        return;
    hive->mRenderer.reset();
    hive->mDocument.reset();
    switch (svgnative_hive_get_renderer_type( hive )) {
#ifdef USE_SKIA
    case RENDER_SKIA:
        /* SkCanvas is owned by source surface, it should not be destroyed externally */
        if (hive->mSkiaSurface)
            hive->mSkiaSurface.reset();
        break;
#endif

#ifdef USE_COREGRAPHICS
    case RENDER_COREGRAPHICS:
        hive->mRenderer = std::make_shared<SVGNative::CGSVGRenderer>();
        hive->mRendererType = RENDER_COREGRAPHICS;
        break;
#endif

#ifdef USE_CAIRO
    case RENDER_CAIRO:
        /* in proper usage, cairo_t* pointer should be destroyed before this */
        if (hive->mCairo)
            cairo_destroy( hive->mCairo );

        if (hive->mCairoSurface)
        {
            cairo_surface_finish( hive->mCairoSurface );
            cairo_surface_destroy( hive->mCairoSurface );
        }
        break;
#endif

#ifdef USE_QT
    case RENDER_QT:
        delete hive->mQPainter;
        hive->mQImage.~QImage();
        break;
#endif

    default:
        break;
    };

    delete hive;
}

int svgnative_hive_install_renderer( SVGNative_Hive hive,
                                     render_t rendererType )
{
    if (!hive)
        return -1;

    switch(rendererType) {
#ifdef USE_STRING
    case RENDER_STRING:
        hive->mRenderer = std::make_shared<SVGNative::StringSVGRenderer>();
        hive->mRendererType = RENDER_STRING;
        return 0;
#endif

#ifdef USE_SKIA
    case RENDER_SKIA:
        hive->mRenderer = std::make_shared<SVGNative::SkiaSVGRenderer>();
        hive->mRendererType = RENDER_SKIA;
        return 0;
#endif

#ifdef USE_COREGRAPHICS
    case RENDER_COREGRAPHICS:
        hive->mRenderer = std::make_shared<SVGNative::CGSVGRenderer>();
        hive->mRendererType = RENDER_COREGRAPHICS;
        return 0;
#endif

#ifdef USE_CAIRO
    case RENDER_CAIRO:
        hive->mRenderer = std::make_shared<SVGNative::CairoSVGRenderer>();
        hive->mRendererType = RENDER_CAIRO;
        return 0;
#endif

#ifdef USE_QT
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

int svgnative_hive_import_output( SVGNative_Hive hive,
                                   void* output )
{
    switch( svgnative_hive_get_renderer_type(hive) ) {
#ifdef USE_SKIA
    case RENDER_SKIA:
         (std::dynamic_pointer_cast<SVGNative::SkiaSVGRenderer>(hive->mRenderer))->SetSkCanvas( (SkCanvas*)output ); 
         return 0;
#endif

#ifdef USE_COREGRAPHICS
    case RENDER_COREGRAPHICS:
        (std::dynamic_pointer_cast<SVGNative::CGSVGRenderer>(hive->mRenderer))->SetGraphicsContext( (CGContextRef)output ); 
        return 0;
#endif

#ifdef USE_CAIRO
    case RENDER_CAIRO:
        (std::dynamic_pointer_cast<SVGNative::CairoSVGRenderer>(hive->mRenderer))->SetCairo( (cairo_t*)output ); 
        return 0;
#endif

#ifdef USE_QT
    case RENDER_QT:
        (std::dynamic_pointer_cast<SVGNative::QtSVGRenderer>(hive->mRenderer))->SetQPainter( (QPainter*)output ); 
        return 0;
#endif

    default:
        return -1;
    }
}

int svgnative_hive_install_output( SVGNative_Hive hive )
{
    std::int32_t w = hive->mDocument->Width();
    std::int32_t h = hive->mDocument->Height();

    switch( svgnative_hive_get_renderer_type(hive) ) {
#ifdef USE_SKIA
    case RENDER_SKIA:
        hive->mSkiaSurface = SkSurface::MakeRasterN32Premul(w, h);
        hive->mSkiaCanvas = hive->mSkiaSurface->getCanvas();
        hive->hasInternalOutput = TRUE;
        return svgnative_hive_import_output(hive, hive->mSkiaCanvas);
#endif

#ifdef USE_COREGRAPHICS
    case RENDER_COREGRAPHICS:
        break;
#endif

#ifdef USE_CAIRO
    case RENDER_CAIRO:
        {
            cairo_rectangle_t docExtents = { 0, 0, 0, 0 };
            docExtents.width = w;
            docExtents.height = h;

            hive->mCairoSurface = cairo_recording_surface_create( CAIRO_CONTENT_COLOR_ALPHA, &docExtents );
            hive->mCairo = cairo_create( hive->mCairoSurface );
        }
        hive->hasInternalOutput = TRUE;
        return svgnative_hive_import_output(hive, hive->mCairo);
#endif

#ifdef USE_QT
    case RENDER_QT:
        hive->mQImage = QImage(w, h, QImage::Format_ARGB32);
        hive->mQPainter = new QPainter;
        hive->mQPainter->begin( &(hive->mQImage) );
        hive->hasInternalOutput = TRUE;
        return svgnative_hive_import_output(hive, hive->mQPainter);
#endif

    default:
        return -1;
    }
    return -1;
}

int svgnative_hive_import_document_from_buffer( SVGNative_Hive hive,
                                                 char* buff )
{
    /* we cannot create SVGDocument without SVGRenderer */
    if (1 > svgnative_hive_get_renderer_type(hive))
        return -1;

    hive->mDocument = std::unique_ptr<SVGNative::SVGDocument>( SVGNative::SVGDocument::CreateSVGDocument(buff, hive->mRenderer) );
    return 0;
}

int svgnative_hive_render_current_document( SVGNative_Hive hive )
{
    if (!hive || !hive->mDocument)
        return -1;
    hive->mDocument->Render();
    return 0;
}

long svgnative_hive_get_width_from_current_document( SVGNative_Hive hive )
{
    if (!hive || !hive->mDocument)
        return -1;
    return hive->mDocument->Width();
}

long svgnative_hive_get_height_from_current_document( SVGNative_Hive hive )
{
    if (!hive || !hive->mDocument)
        return -1;
    return hive->mDocument->Height();
}

#ifdef USE_CAIRO
typedef struct buff_list_t_ {
    std::list<std::pair<size_t, unsigned char*>>  buffs;
    size_t                                        total_size;
} buff_list_t;

cairo_status_t write_on_buff( void* closure,
                              const unsigned char* data,
                              unsigned int length )
{
    buff_list_t*  buff_list = (buff_list_t*)closure;

    unsigned char* chunk = (unsigned char*)malloc( length );
    if (!chunk)
        return CAIRO_STATUS_WRITE_ERROR;
    memcpy( chunk, data, length );
    buff_list->buffs.emplace_back( std::pair<size_t, unsigned char*>( length, chunk ) );
    buff_list->total_size += length;
    return CAIRO_STATUS_SUCCESS;
}

unsigned char* join_buff_list( buff_list_t* buff_list )
{
    unsigned char* joined_buff = (unsigned char*)malloc( buff_list->total_size );
    if (!joined_buff)
        return NULL;

    unsigned char* cur = joined_buff;
    for (auto itr = buff_list->buffs.begin(); itr != buff_list->buffs.end(); ++itr) {
        memcpy( cur, itr->second, itr->first );
        cur += itr->first;
        free( itr->second );
        itr->second = nullptr;
    };

    return (unsigned char*)joined_buff;
}
#endif

int svgnative_hive_get_rendered_png( SVGNative_Hive  hive,
                                     unsigned char** buff,
                                     size_t*         size )
{
    if ( !hive || !hive->hasInternalOutput )
        return -1;

    switch (svgnative_hive_get_renderer_type( hive )) {
    case RENDER_SKIA:
#ifdef USE_SKIA
        {
            auto skImage = hive->mSkiaSurface->makeImageSnapshot();
            sk_sp<SkData> pngData(skImage->encodeToData(SkEncodedImageFormat::kPNG, 100));
            *buff = (unsigned char*)malloc( pngData->size() );
            memcpy( (void*)*buff, (void*)pngData->data(), pngData->size() );
            *size = pngData->size();
        }
#endif
        break;

    case RENDER_COREGRAPHICS:
#ifdef USE_COREGRAPHICS
#endif
        break;

    case RENDER_CAIRO:
#ifdef USE_CAIRO
        {
            buff_list_t* buff_list = new buff_list_t;
            cairo_surface_write_to_png_stream( hive->mCairoSurface, write_on_buff, (void*)buff_list );
            *size = buff_list->total_size;
            *buff = join_buff_list( buff_list );
            delete buff_list;
        }
#endif
        break;

    case RENDER_QT:
#ifdef USE_QT
        {
            QByteArray qByteArray;
            QBuffer qBuffer(&qByteArray);
            qBuffer.open(QIODevice::WriteOnly);
            hive->mQImage.save(&qBuffer, "PNG");
            *size = qByteArray.size();
            *buff = (unsigned char*)malloc( *size );
            memcpy( (void*)*buff, (void*)qByteArray.data(), *size );
            qBuffer.close();
            qBuffer.~QBuffer(); /* why needed? valgrind finds leaks if we skip this */
            // qByteArray.~QByteArray();
        }
#endif
        break;

    default:
        break;
    };
    return 0;
}
