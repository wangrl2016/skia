//
// Created by wangrl on 2020/12/9.
//

#include <modules/sksg/include/SkSGRenderNode.h>
#include <modules/sksg/include/SkSGImage.h>
#include <modules/sksg/include/SkSGClipEffect.h>
#include <modules/sksg/include/SkSGRect.h>
#include <modules/sksg/include/SkSGPath.h>
#include <modules/sksg/include/SkSGMerge.h>
#include <modules/sksg/include/SkSGPaint.h>
#include <modules/sksg/include/SkSGRenderEffect.h>
#include <modules/sksg/include/SkSGDraw.h>
#include <src/utils/SkOSPath.h>
#include <modules/sksg/include/SkSGMaskEffect.h>
#include "include/core/SkGraphics.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkFont.h"
#include "include/effects/SkGradientShader.h"
#include "modules/photoeditor/include/PhotoEditor.h"

sk_app::Application* sk_app::Application::Create(int argc, char** argv, void* platformData) {
    return new PhotoEditor(argc, argv, platformData);
}

PhotoEditor::PhotoEditor(int argc, char** argv, void* platformData)
        : mBackendType(sk_app::Window::kNativeGL_BackendType), mRotationAngle(0) {
    SkDebugf("PhotoEditor::PhotoEditor\n");
    mResourceProvider = skresources::FileResourceProvider::Make(SkString("resources/"));

    SkGraphics::Init();

    mWindow = sk_app::Window::CreateNativeWindow(platformData);
    mWindow->setRequestedDisplayParams(sk_app::DisplayParams());

    // register callbacks
    mWindow->pushLayer(this);

    mWindow->attach(mBackendType);
}

PhotoEditor::~PhotoEditor() {
    mWindow->detach();
    delete mWindow;
}

struct MaskInfo {
    SkBlendMode       fBlendMode;      // used when masking with layers/blending
    sksg::Merge::Mode fMergeMode;      // used when clipping
    bool              fInvertGeometry;
};

const MaskInfo* GetMaskInfo(char mode) {
    static constexpr MaskInfo k_add_info =
            { SkBlendMode::kSrcOver   , sksg::Merge::Mode::kUnion     , false };
    static constexpr MaskInfo k_int_info =
            { SkBlendMode::kSrcIn     , sksg::Merge::Mode::kIntersect , false };
    static constexpr MaskInfo k_sub_info =
            { SkBlendMode::kDstOut    , sksg::Merge::Mode::kDifference, true  };
    static constexpr MaskInfo k_dif_info =
            { SkBlendMode::kXor       , sksg::Merge::Mode::kXOR       , false };

    switch (mode) {
        case 'a': return &k_add_info;
        case 'f': return &k_dif_info;
        case 'i': return &k_int_info;
        case 's': return &k_sub_info;
        default: break;
    }

    return nullptr;
}

using ScalarValue = SkScalar;
using   Vec2Value = SkV2;

sk_sp<sksg::RenderNode> AttachMask(sk_sp<sksg::RenderNode> childNode) {

    struct MaskRecord {
        sk_sp<sksg::Path> mask_path;    // for clipping and masking
        sksg::Merge::Mode merge_mode;   // for clipping
    };

    SkSTArray<4, MaskRecord, true> mask_stack;
    bool hasEffect;

    const auto mode = 'a';
    const auto* mask_info = GetMaskInfo(mode);

    SkPath path;
    path.moveTo(600.076, 302.457);
    path.lineTo(188.733, 302.457);
    path.lineTo(188.733, 718.639);
    path.lineTo(600.076, 718.639);
    path.lineTo(600.076, 302.457);

    sk_sp<sksg::Path> mask_path = sksg::Path::Make(path);

    auto mask_blend_mode = mask_info->fBlendMode;
    auto mask_merge_mode = mask_info->fMergeMode;
    auto mask_inverted   = false;

    if (mask_stack.empty()) {
        mask_blend_mode = SkBlendMode::kSrc;
        mask_merge_mode = sksg::Merge::Mode::kMerge;
        mask_inverted   = mask_inverted != mask_info->fInvertGeometry;
    }

    mask_path->setFillType(mask_inverted ? SkPathFillType::kInverseWinding
                                         : SkPathFillType::kWinding);

    sk_sp<sksg::PaintNode> fMaskPaint;
    SkBlendMode            fBlendMode = mask_blend_mode;

    sk_sp<sksg::BlurImageFilter> fMaskFilter; // optional "feather"

    Vec2Value   fFeather = {89,89};
    ScalarValue fOpacity = 100;

    fMaskPaint = sksg::Color::Make(SK_ColorBLACK);
    fMaskPaint->setAntiAlias(true);
    fMaskPaint->setBlendMode(fBlendMode);

    fMaskFilter = sksg::BlurImageFilter::Make();

    // Close enough to AE.
    static constexpr SkScalar kFeatherToSigma = 0.38f;
    fMaskFilter->setSigma({fFeather.x * kFeatherToSigma,
                           fFeather.y * kFeatherToSigma});

    sk_sp<sksg::RenderNode> mask = sksg::Draw::Make(std::move(mask_path), fMaskPaint);

    // Optional mask blur (feather).
    mask = sksg::ImageFilterEffect::Make(std::move(mask), fMaskFilter);

    childNode =  sksg::MaskEffect::Make(std::move(childNode), std::move(mask));;


    float w = 720, h = 1280;

    const auto info = SkImageInfo::MakeS32(int(w), int(h), kUnpremul_SkAlphaType);
    sk_sp<SkSurface> surf = SkSurface::MakeRaster(info);

    childNode->revalidate(nullptr, SkMatrix::I());
    childNode->render(surf->getCanvas(), nullptr);

    const auto filename = "final.png";
    SkFILEWStream stream(SkOSPath::Join("out", filename).c_str());

    auto image = surf->makeImageSnapshot()->encodeToData();

    stream.write(image->data(), image->size());

    return childNode;
}

void PhotoEditor::onPaint(SkSurface* surface) {

    const int w = 720, h = 1280;

    auto canvas = surface->getCanvas();

    // clear background
    canvas->clear(SK_ColorWHITE);

    // Potentially null.
    sk_sp<sksg::RenderNode> layer;

    auto image_node = sksg::Image::Make(nullptr);

    auto frame_data = mSource->getFrame(0);
    image_node->setImage(std::move(frame_data));

    layer = image_node;

    layer = sksg::ClipEffect::Make(std::move(layer),
                                   sksg::Rect::Make(SkRect::MakeWH(w, h)), true);

    // 添加蒙版
    layer = AttachMask(std::move(layer));

    // 显示
    layer->revalidate(nullptr, SkMatrix::I());
    layer->render(canvas);
}

//void PhotoEditor::onPaint(SkSurface* surface) {
//    auto canvas = surface->getCanvas();
//
//    // Clear background
//    canvas->clear(SK_ColorWHITE);
//
//    SkPaint paint;
//    paint.setColor(SK_ColorRED);
//
//    // Draw a rectangle with red paint.
//    SkRect rect = SkRect::MakeXYWH(10, 10, 128, 128);
//    canvas->drawRect(rect, paint);
//
//    // Set up a linear gradient and draw a circle.
//    {
//        SkPoint linearPoints[] = {{0,   0},
//                                  {300, 300}};
//        SkColor linearColors[] = {
//                SK_ColorGREEN, SK_ColorBLACK};
//
//        paint.setShader(SkGradientShader::MakeLinear(linearPoints, linearColors, nullptr, 2,
//                                                     SkTileMode::kMirror));
//        paint.setAntiAlias(true);
//
//        canvas->drawCircle(200, 200, 64, paint);
//
//        // Detach shader.
//        paint.setShader(nullptr);
//    }
//
//    // Draw a message with a nice black paint.
//    SkFont font;
//    font.setSubpixel(true);
//    font.setSize(20);
//    paint.setColor(SK_ColorBLACK);
//
//    canvas->save();
//    static const char message[] = "Hello World";
//
//    // Translate and rotate.
//    canvas->translate(300, 300);
//    mRotationAngle += 0.2f;
//    if (mRotationAngle > 360) {
//        mRotationAngle -= 360;
//    }
//    canvas->rotate(mRotationAngle);
//
//    // Draw the text.
//    canvas->drawSimpleText(message, strlen(message), SkTextEncoding::kUTF8, 0, 0, font, paint);
//
//    canvas->restore();
//}

void PhotoEditor::onIdle() {
    // Just re-paint continuously.
    // mWindow->inval();
}

void PhotoEditor::onBackendCreated() {
    SkDebugf("PhotoEditor::onBackendCreated\n");
    mWindow->show();
    mWindow->inval();

    const char* path = "skottie/mask/images";
    const char* name = "img_0.jpg";
    const char* id = "MASK_SOURCE";

    mSource = mResourceProvider->loadImageAsset(path, name, id);
}
