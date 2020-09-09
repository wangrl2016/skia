//
// Created by wangrl on 2020/9/9.
//

#include "include/core/SkGraphics.h"
#include "tools/flags/CommandLineFlags.h"
int main(int argc, char* argv[]) {
    SkGraphics::Init();

    CommandLineFlags::SetUsage("Convert skottie to a movie");
    CommandLineFlags::Parse(argc, argv);

    return 0;
}