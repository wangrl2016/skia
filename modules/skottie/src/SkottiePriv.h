/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottiePriv_DEFINED
#define SkottiePriv_DEFINED

#include "modules/skottie/include/Skottie.h"

#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTHash.h"
#include "include/utils/SkCustomTypeface.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/sksg/include/SkSGScene.h"
#include "src/utils/SkUTF.h"

#include <vector>

class SkFontMgr;

namespace skjson {
class ArrayValue;
class ObjectValue;
class Value;
} // namespace skjson

namespace sksg {
class Color;
class Path;
class RenderNode;
class Transform;
} // namespace sksg

namespace skottie {
namespace internal {

// Close-enough to AE.
static constexpr float kBlurSizeToSigma = 0.3f;

class TransformAdapter2D;
class TransformAdapter3D;

using AnimatorScope = std::vector<sk_sp<Animator>>;

class AnimationBuilder final : public SkNoncopyable {
public:
    AnimationBuilder(sk_sp<ResourceProvider>, sk_sp<SkFontMgr>,
                     sk_sp<Logger>, sk_sp<MarkerObserver>, sk_sp<PrecompInterceptor>,
                     Animation::Builder::Stats*, const SkSize& comp_size,
                     float duration, float framerate, uint32_t flags);

    struct AnimationInfo {
        std::unique_ptr<sksg::Scene> fScene;
        AnimatorScope                fAnimators;
    };

    AnimationInfo parse(const skjson::ObjectValue&);


    void log(Logger::Level, const skjson::Value*, const char fmt[], ...) const;

    sk_sp<sksg::Transform> attachMatrix2D(const skjson::ObjectValue&, sk_sp<sksg::Transform>,
                                          bool auto_orient = false) const;
    sk_sp<sksg::Transform> attachMatrix3D(const skjson::ObjectValue&, sk_sp<sksg::Transform>,
                                          bool auto_orient = false) const;

    sk_sp<sksg::RenderNode> attachOpacity(const skjson::ObjectValue&,
                                          sk_sp<sksg::RenderNode>) const;
    sk_sp<sksg::Path> attachPath(const skjson::Value&) const;

    bool hasNontrivialBlending() const { return fHasNontrivialBlending; }

    class AutoScope final {
    public:
        explicit AutoScope(const AnimationBuilder* builder) : AutoScope(builder, AnimatorScope()) {}

        AutoScope(const AnimationBuilder* builder, AnimatorScope&& scope)
            : fBuilder(builder)
            , fCurrentScope(std::move(scope))
            , fPrevScope(fBuilder->fCurrentAnimatorScope) {
            fBuilder->fCurrentAnimatorScope = &fCurrentScope;
        }

        AnimatorScope release() {
            fBuilder->fCurrentAnimatorScope = fPrevScope;
            SkDEBUGCODE(fBuilder = nullptr);

            return std::move(fCurrentScope);
        }

        ~AutoScope() { SkASSERT(!fBuilder); }

    private:
        const AnimationBuilder* fBuilder;
        AnimatorScope           fCurrentScope;
        AnimatorScope*          fPrevScope;
    };

    template <typename T>
    void attachDiscardableAdapter(sk_sp<T> adapter) const {
        if (adapter->isStatic()) {
            // Fire off a synthetic tick to force a single SG sync before discarding.
            adapter->seek(0);
        } else {
            fCurrentAnimatorScope->push_back(std::move(adapter));
        }
    }

    template <typename T, typename... Args>
    auto attachDiscardableAdapter(Args&&... args) const ->
        typename std::decay<decltype(T::Make(std::forward<Args>(args)...)->node())>::type
    {
        using NodeType =
        typename std::decay<decltype(T::Make(std::forward<Args>(args)...)->node())>::type;

        NodeType node;
        if (auto adapter = T::Make(std::forward<Args>(args)...)) {
            node = adapter->node();
            this->attachDiscardableAdapter(std::move(adapter));
        }
        return node;
    }

    bool dispatchColorProperty(const sk_sp<sksg::Color>&) const;
    bool dispatchOpacityProperty(const sk_sp<sksg::OpacityEffect>&) const;
    bool dispatchTransformProperty(const sk_sp<TransformAdapter2D>&) const;

private:
    friend class CompositionBuilder;
    friend class LayerBuilder;

    struct FootageAssetInfo;
    struct LayerInfo;

    void parseAssets(const skjson::ArrayValue*);


    sk_sp<sksg::RenderNode> attachBlendMode(const skjson::ObjectValue&,
                                            sk_sp<sksg::RenderNode>) const;

    const FootageAssetInfo* loadFootageAsset(const skjson::ObjectValue&) const;
    sk_sp<sksg::RenderNode> attachFootageAsset(const skjson::ObjectValue&, LayerInfo*) const;

    sk_sp<sksg::RenderNode> attachFootageLayer(const skjson::ObjectValue&, LayerInfo*) const;


    sk_sp<ResourceProvider>    fResourceProvider;
    sk_sp<Logger>              fLogger;
    sk_sp<MarkerObserver>      fMarkerObserver;
    sk_sp<PrecompInterceptor>  fPrecompInterceptor;
    Animation::Builder::Stats* fStats;
    const SkSize               fCompSize;
    const float                fDuration,
                               fFrameRate;
    const uint32_t             fFlags;
    mutable AnimatorScope*     fCurrentAnimatorScope;
    mutable const char*        fPropertyObserverContext;
    mutable bool               fHasNontrivialBlending : 1;

    struct LayerInfo {
        SkSize      fSize;
        const float fInPoint,
                    fOutPoint;
    };

    struct AssetInfo {
        const skjson::ObjectValue* fAsset;
        mutable bool               fIsAttaching; // Used for cycle detection
    };

    struct FootageAssetInfo {
        sk_sp<ImageAsset> fAsset;
        SkISize           fSize;
    };

    class ScopedAssetRef {
    public:
        ScopedAssetRef(const AnimationBuilder* abuilder, const skjson::ObjectValue& jlayer);

        ~ScopedAssetRef() {
            if (fInfo) {
                fInfo->fIsAttaching = false;
            }
        }

        operator bool() const { return !!fInfo; }

        const skjson::ObjectValue& operator*() const { return *fInfo->fAsset; }

    private:
        const AssetInfo* fInfo = nullptr;
    };

    SkTHashMap<SkString, AssetInfo>                fAssets;
    mutable SkTHashMap<SkString, FootageAssetInfo> fImageAssetCache;

    using INHERITED = SkNoncopyable;
};

} // namespace internal
} // namespace skottie

#endif // SkottiePriv_DEFINED
