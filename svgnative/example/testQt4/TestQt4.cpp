/*
Copyright 2019 suzuki toshiya <mpsuzuki@hiroshima-u.ac.jp>. All rights reserved.
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

#include "Qt4SVGRenderer.h"
#include <QPicture>

#include <fstream>
#include <iostream>
#include <string>
#include <cctype>

int main(int argc, char* const argv[])
{
    if (argc < 3)
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

    auto renderer = std::make_shared<SVGNative::Qt4SVGRenderer>();

    auto doc = std::unique_ptr<SVGNative::SVGDocument>(SVGNative::SVGDocument::CreateSVGDocument(svgInput.c_str(), renderer));

    QPainter qPainter;
    renderer->SetQPainter(&qPainter);

    for (int i = 2; i < argc; i++ )
    {
        std::string outPath = argv[i];
        std::string suffix = outPath.substr(outPath.rfind('.') + 1).c_str();
        std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

        if (suffix == "pic")
        {
            QPicture qPicture;
            qPainter.begin(&qPicture);
            doc->Render();
            qPainter.end();
            qPicture.save(QString(outPath.c_str()));
        }
        else
        {
            QImage qImage(doc->Width(), doc->Height(), QImage::Format_ARGB32);
            qPainter.begin(&qImage);
            doc->Render();
            qImage.save(QString(outPath.c_str()));
            qPainter.end();
        }
    }

    doc.reset();
    renderer.reset();

    return 0;
}
