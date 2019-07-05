#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "SVGNativeCAPI.h"

#include "cairo.h"

#if CAIRO_HAS_SCRIPT_SURFACE
#include "cairo-script.h"
#endif
#if CAIRO_HAS_SVG_SURFACE
#include "cairo-svg.h"
#endif
#if CAIRO_HAS_XML_SURFACE
#include "cairo-xml.h"
#endif

char* loadSvgFile(char* svgPath)
{
    FILE* svgFile;
    size_t svgLength;
    char*  svgBuff;

    svgFile = fopen(svgPath, "r");
    fseek(svgFile, 0L, SEEK_END); 
    svgLength = ftell(svgFile);

    svgBuff = (char*)malloc(svgLength + 1);

    fseek(svgFile, 0L, SEEK_SET);
    fread((void*)svgBuff, 1, svgLength, svgFile);

    fclose(svgFile);

    return svgBuff;
}

int main(int argc, char** argv)
{
    char*  svgBuff;
    cairo_rectangle_t docExtents = { 0, 0, 0, 0 };
    cairo_surface_t* cairoSurface;
    cairo_t* cairoContext;

    svgBuff = loadSvgFile(argv[1]);

    createHive();
    createRenderer( RENDER_CAIRO );
    createSvgDocument( svgBuff );    

    docExtents.width = getWidthFromSvgDocument();
    docExtents.height = getHeightFromSvgDocument();

    cairoSurface = cairo_recording_surface_create( CAIRO_CONTENT_COLOR_ALPHA, &docExtents );
    cairoContext = cairo_create( cairoSurface );

    setOutputForRenderer( (void*)cairoContext );

    renderSvgDocument();

    cairo_destroy( cairoContext );
    cairo_surface_write_to_png( cairoSurface, argv[2] );

    deleteSvgDocument();
    deleteRenderer();
    deleteHive();

    free(svgBuff);
}
