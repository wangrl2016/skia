//
// Created by wangrl on 2020/12/21.
//

#pragma once

#include "modules/skresources/include/SkResources.h"
#include "tools/sk_app/Window.h"
#include "tools/sk_app/Application.h"

class PhotoEditor : public sk_app::Application, sk_app::Window::Layer {
public:
    PhotoEditor(int argc, char** argv, void* platformData);

    ~PhotoEditor() override;

    void onIdle() override;

    void onBackendCreated() override;

    void onPaint(SkSurface*) override;

private:
    sk_app::Window* mWindow;
    sk_app::Window::BackendType mBackendType;

    SkScalar mRotationAngle;

    sk_sp<skresources::ResourceProvider> mResourceProvider;

    sk_sp<skresources::ImageAsset> mSource;
};
