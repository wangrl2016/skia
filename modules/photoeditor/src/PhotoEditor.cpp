//
// Created by wangrl on 2020/12/9.
//

#include "include/core/SkGraphics.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
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
}

void PhotoEditor::onIdle() {

}

void PhotoEditor::onBackendCreated() {
    mWindow->show();
    mWindow->inval();
}
