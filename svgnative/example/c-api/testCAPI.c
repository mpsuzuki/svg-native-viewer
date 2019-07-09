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
    unsigned char* buff;
    size_t size;
    FILE* pngFile;

    svgBuff = loadSvgFile(argv[1]);

    hive = svgnative_hive_create();
    svgnative_hive_install_renderer( hive, RENDER_CAIRO );
    svgnative_hive_import_document_from_buffer( hive, svgBuff );    
    free(svgBuff);

    svgnative_hive_install_output( hive );
    svgnative_hive_render_current_document( hive );
    svgnative_hive_get_rendered_png( hive, &buff, &size);

    pngFile = fopen(argv[2], "wb+");
    fwrite( buff, 1, size, pngFile );
    fclose( pngFile );
    free( buff );

    svgnative_hive_destroy( hive );
}
