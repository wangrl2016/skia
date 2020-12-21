//
// Created by wangrl on 2020/12/9.
//

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

void PhotoEditor::onPaint(SkSurface* surface) {
    auto canvas = surface->getCanvas();

    // Clear background
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    // Draw a rectangle with red paint.
    SkRect rect = SkRect::MakeXYWH(10, 10, 128, 128);
    canvas->drawRect(rect, paint);

    // Set up a linear gradient and draw a circle.
    {
        SkPoint linearPoints[] = {{0,   0},
                                  {300, 300}};
        SkColor linearColors[] = {
                SK_ColorGREEN, SK_ColorBLACK};

        paint.setShader(SkGradientShader::MakeLinear(linearPoints, linearColors, nullptr, 2,
                                                     SkTileMode::kMirror));
        paint.setAntiAlias(true);

        canvas->drawCircle(200, 200, 64, paint);

        // Detach shader.
        paint.setShader(nullptr);
    }

    // Draw a message with a nice black paint.
    SkFont font;
    font.setSubpixel(true);
    font.setSize(20);
    paint.setColor(SK_ColorBLACK);

    canvas->save();
    static const char message[] = "Hello World";

    // Translate and rotate.
    canvas->translate(300, 300);
    mRotationAngle += 0.2f;
    if (mRotationAngle > 360) {
        mRotationAngle -= 360;
    }
    canvas->rotate(mRotationAngle);

    // Draw the text.
    canvas->drawSimpleText(message, strlen(message), SkTextEncoding::kUTF8, 0, 0, font, paint);

    canvas->restore();
}

void PhotoEditor::onIdle() {
    // Just re-paint continuously.
    mWindow->inval();
}

void PhotoEditor::onBackendCreated() {
    mWindow->show();
    mWindow->inval();
}
