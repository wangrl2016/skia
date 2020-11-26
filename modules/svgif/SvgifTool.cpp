//
// Created by wangrl on 2020/11/26.
//

#include <cstdlib>
#include <iostream>

#include "include/core/SkStream.h"
#include "tools/flags/CommandLineFlags.h"
#include "modules/svgif/include/SVGDOM.h"

static DEFINE_string2(input, i, nullptr, "Input SVG file.");
static DEFINE_string2(output, o, nullptr, "Output GIF file.");

int main(int argc, char* argv[]) {

    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_input.isEmpty() || FLAGS_output.isEmpty()) {
        std::cerr << "Missing required 'input' and 'output' args.\n";
        return EXIT_FAILURE;
    }

    SkFILEStream in(FLAGS_input[0]);
    if (!in.isValid()) {
        std::cerr << "Could not open " << FLAGS_input[0] << "\n";
        return EXIT_FAILURE;
    }

    auto svg_dom = SVGDOM::Builder()
            .setFontManager(SkFontMgr::RefDefault())
            .make(in);

    if (!svg_dom) {
        std::cerr << "Could not parse " << FLAGS_input[0] << "\n";
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
