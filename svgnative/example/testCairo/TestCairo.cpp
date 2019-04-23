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
#include "cairo-script.h"

#include <fstream>
#include <iostream>
#include <string>

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

    auto cairoSurface = cairo_recording_surface_create( CAIRO_CONTENT_COLOR_ALPHA, NULL );
    auto cairoContext = cairo_create( cairoSurface );

    renderer->SetCairo( cairoContext );
    doc->Render();

    auto cairo_dev = cairo_script_create(argv[2]);
    cairo_script_from_recording_surface(cairo_dev, cairoSurface);
    
    return 0;
}
