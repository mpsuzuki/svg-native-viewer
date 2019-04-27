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

#include "SVGDocument.h"
#include <list>
#include "CairoSVGRenderer.h"
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

#include <fstream>
#include <iostream>
#include <string>
#include <cctype>

int main(int argc, char* const argv[])
{
    if (argc != 3)
    {
        std::cout << "Incorrect number of arguments." << std::endl;
        return 0;
    }

    std::string svgInput{};
    std::ifstream input(argv[1]);
    if (!input)
    {
        std::cout << "Error! Could not open input file." << std::endl;
        exit(EXIT_FAILURE);
    }
    for (std::string line; std::getline(input, line);)
        svgInput.append(line);
    input.close();

    auto renderer = std::make_shared<SVGNative::CairoSVGRenderer>();

    auto doc = std::unique_ptr<SVGNative::SVGDocument>(SVGNative::SVGDocument::CreateSVGDocument(svgInput.c_str(), renderer));

    cairo_rectangle_t docExtents = { 0, 0, 0, 0 };
    docExtents.width = doc->Width();
    docExtents.height = doc->Height();

    std::string outPath = argv[2];
    std::string suffix = outPath.substr(outPath.rfind('.') + 1).c_str();
    std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

    cairo_surface_t* cairoSurface;
#if CAIRO_HAS_SVG_SURFACE
    if (suffix == "svg")
    {
        cairoSurface = cairo_svg_surface_create(argv[2], doc->Width(), doc->Height());
        cairo_svg_surface_set_document_unit(cairoSurface, CAIRO_SVG_UNIT_PX);
    }
    else
#endif
        cairoSurface = cairo_recording_surface_create( CAIRO_CONTENT_COLOR_ALPHA, &docExtents );

    auto cairoContext = cairo_create( cairoSurface );

    renderer->SetCairo( cairoContext );
    doc->Render();

#if CAIRO_HAS_SVG_SURFACE
    if (suffix == "svg")
    {
        cairo_show_page( cairoContext );
        cairo_destroy( cairoContext );
        cairo_surface_destroy( cairoSurface );
        // auto cairo_surface_svg = cairo_svg_surface_create(argv[2], doc->Width(), doc->Height());
        // auto cairo_svg = cairo_create( cairo_surface_svg );
        // cairo_set_source_surface( cairo_svg, cairoSurface, 0, 0 );
        // cairo_paint( cairo_svg );
        // cairo_destroy( cairo_svg );
        // cairo_surface_destroy( cairo_svg );
    }
    else
#endif
#if CAIRO_HAS_XML_SURFACE
    if (suffix == "xml")
    {
        auto cairo_dev = cairo_xml_create(argv[2]);
        cairo_xml_for_recording_surface(cairo_dev, cairoSurface);
    }
    else
#endif
#ifndef CAIRO_HAS_SCRIPT_SURFACE
        cairo_surface_write_to_png( cairoSurface, argv[2] );   
#else
    if (suffix == "png")
        cairo_surface_write_to_png( cairoSurface, argv[2] );   
    else
    {
        auto cairo_dev = cairo_script_create(argv[2]);
        cairo_script_from_recording_surface(cairo_dev, cairoSurface);
    }
#endif
    
    return 0;
}
