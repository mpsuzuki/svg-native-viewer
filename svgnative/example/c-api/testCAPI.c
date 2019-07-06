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
    SVGNative_Hive hive;
    char*  svgBuff;
    cairo_rectangle_t docExtents = { 0, 0, 0, 0 };
    cairo_surface_t* cairoSurface;
    cairo_t* cairoContext;

    svgBuff = loadSvgFile(argv[1]);

    hive = svgnative_hive_create();
    svgnative_hive_install_renderer( hive, RENDER_CAIRO );
    svgnative_hive_install_document_from_buffer( hive, svgBuff );    
    free(svgBuff);

    docExtents.width = svgnative_hive_get_width_from_installed_document( hive );
    docExtents.height = svgnative_hive_get_height_from_installed_document( hive ); 

    cairoSurface = cairo_recording_surface_create( CAIRO_CONTENT_COLOR_ALPHA, &docExtents );
    cairoContext = cairo_create( cairoSurface );

    svgnative_hive_install_output( hive, (void*)cairoContext );
    svgnative_hive_render_installed_document( hive );

    cairo_destroy( cairoContext );
    cairo_surface_flush( cairoSurface );
    cairo_surface_write_to_png( cairoSurface, argv[2] );
    cairo_surface_finish( cairoSurface );
    cairo_surface_destroy( cairoSurface );

    svgnative_hive_destroy( hive );
}
