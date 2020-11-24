//
// Created by wangrl on 2020/9/9.
//

#include "include/core/SkGraphics.h"
#include "include/core/SkTime.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skresources/include/SkResources.h"
#include "src/utils/SkOSPath.h"

#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"

#include "include/gpu/GrContextOptions.h"
#include "tools/render/Engine.h"
#include "tools/render/AnimationComponent.h"
#include "tools/render/RenderComponent.h"
#include "tools/render/FFmpegComponent.h"
#include "tools/render/OutputConfigComponent.h"

static DEFINE_string2(input, i, "", "skottie animation to render");
static DEFINE_string2(output, o, "", "mp4 file to create");
static DEFINE_string2(assetPath, a, "", "path to assets needed for json file");
static DEFINE_int_2(fps, f, 25, "fps");
static DEFINE_bool2(verbose, v, false, "verbose mode");
static DEFINE_bool2(loop, l, false, "loop mode for profiling");
static DEFINE_bool2(gpu, g, false, "use GPU for rendering");


int main(int argc, char** argv) {
    SkGraphics::Init();

    CommandLineFlags::SetUsage("Converts skottie to a mp4");
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_input.count() == 0) {
        SkDebugf("-i input_file.json argument required\n");
        return -1;
    }

    SkString assetPath;
    if (FLAGS_assetPath.count() > 0) {
        assetPath.set(FLAGS_assetPath[0]);
    } else {
        assetPath = SkOSPath::Dirname(FLAGS_input[0]);
    }
    SkDebugf("assetPath %s\n", assetPath.c_str());

    auto animation = skottie::Animation::Builder()
            .setResourceProvider(skresources::FileResourceProvider::Make(assetPath))
            .makeFromFile(FLAGS_input[0]);
    if (!animation) {
        SkDebugf("failed to load %s\n", FLAGS_input[0]);
        return -1;
    }

    SkISize dim = animation->size().toRound();
    double duration = animation->duration();
    int fps = SkTPin(FLAGS_fps, 1, 240);

    const double frame_duration = 1.0 / fps;

    if (FLAGS_verbose) {
        SkDebugf("Size %dx%d duration %g, fps %d, frame_duration %g\n",
                 dim.width(), dim.height(), duration, fps, frame_duration);
    }

    auto& engine = render::Engine::instance();
    auto& registry = engine.registry();
    auto entity = registry.create();
    registry.emplace<render::AnimationComponent>(entity, animation);
    registry.emplace<render::RenderComponent>(entity);
    registry.emplace<render::FFmpegComponent>(entity);
    registry.emplace<render::OutputConfigComponent>(entity);

    auto view = registry.view<render::AnimationComponent,
            render::RenderComponent>();
    for (auto entity : view) {
        auto& anim = view.get<render::AnimationComponent>(entity);
        anim.mFps = fps;
        SkDebugf("Animation duration %lfs\n", anim.getAnimation()->duration());
    }

    // 引擎初始化
    engine.init();

    // 如果没有停止就继续更新
    while (!render::AnimationComponent::finished()) {
        engine.update();
    }

    // 引擎销毁
    engine.destroy();

    return 0;
}
