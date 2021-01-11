//
// Created by wangrl on 20-6-2.
//

#include <jni.h>
#include <string>
#include "entt/entt.hpp"
#include <android/log.h>
#include <android/asset_manager.h>
#include <unistd.h>

#define TAG "Renderer"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavfilter/avfilter.h"
};

#include "include/core/SkTypes.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "src/utils/SkOSPath.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skresources/include/SkResources.h"

#include "bfx/Engine.h"
#include "bfx/Utils/EngineUtils.h"

entt::entity animationEntity;

float nativeRendererPercent = 0.0f;

const std::string RendererClassStr = "com/chuangkit/videorenderer/NativeRendererInterface";
const std::string rendererPercentStr = "rendererPercent";

enum RendererStatus {
    SUCCESS = 0,
    FAILED,
    BREAK,
    ERROR
};

extern "C"
JNIEXPORT jint JNICALL
Java_com_chuangkit_videorenderer_NativeRendererInterface_rendererVideo(JNIEnv *env, jobject thiz,
                                                                       jstring input_json_file,
                                                                       jstring output_video_file) {
    // TODO: implement rendererVideo()
    const char *inputJsonFile = (env)->GetStringUTFChars(input_json_file, nullptr);
    int inputLength = (env)->GetStringUTFLength(input_json_file);
    std::string jsonFile = std::string(inputJsonFile, inputLength);

    const char *outputVideoFile = (env)->GetStringUTFChars(output_video_file, nullptr);
    int outputLength = (env)->GetStringUTFLength(output_video_file);
    std::string outputFile = std::string(outputVideoFile, outputLength);

    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", jsonFile.c_str());
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", outputFile.c_str());


    std::string assetPath = jsonFile.substr(0, jsonFile.find_last_of('/'));
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", assetPath.c_str());

    // audio文件所在位置
    std::string audioAssets = assetPath + "/audios/music_0.mp3";


    auto animation = skottie::Animation::Builder()
            .setResourceProvider(
                    skresources::FileResourceProvider::Make(SkString(assetPath.c_str())))
            .makeFromFile(jsonFile.c_str());

    if (animation == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "construct animation failed");
        return RendererStatus::FAILED;
    }

    SkISize dim = animation->size().toRound();
    float totalCount = animation->duration() * animation->fps();

    __android_log_print(ANDROID_LOG_INFO, TAG, "animation width %d height %d", dim.width(),
                        dim.height());

    auto &bfx = bfx::Engine::instance();

    // load first then init
    animationEntity = bfx::EngineUtils::Animation::load(jsonFile);
    bfx.init(dim.width(), dim.height());

    bfx::EngineUtils::FFmpeg::configOutputFilePath(animationEntity, outputFile);

    bfx::AudioInput *audioInput = new bfx::AudioInput(audioAssets, 0.0f, 0.0f);
    if (access(audioAssets.c_str(), F_OK) != -1) {
        bfx::EngineUtils::FFmpeg::configAudioInput(animationEntity, audioInput);
    }

    int nativeCount = 0;
    bool userBreak = false;
    while (!bfx::EngineUtils::Animation::finished()) {

        if (bfx::EngineUtils::Animation::isUserBreak(animationEntity)) {
            userBreak = true;
            break;
        }

        nativeCount++;
        __android_log_print(ANDROID_LOG_INFO, TAG, "renderer frame %d", nativeCount);
        nativeRendererPercent = ((float) nativeCount) / totalCount;


        // 1. 获取Class类型对象
        jclass RendererClass = env->FindClass(RendererClassStr.c_str());
        if (nullptr == RendererClass) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "can not found %s class",
                                RendererClassStr.c_str());
        }

        // 2. 获取属性ID值
        jfieldID rendererPercentID = env->GetStaticFieldID(RendererClass,
                                                           rendererPercentStr.c_str(), "F");
        if (nullptr == rendererPercentID) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "can not found %s attribute",
                                rendererPercentStr.c_str());
        }

        // 3. 获取静态属性值
        // jfloat rendererPercentValue = env->GetStaticFloatField(RendererClass,
        //                                                        rendererPercentID);

        // 4. 将计算出来的百分比进行赋值
        jfloat rendererPercentValue = nativeRendererPercent;

        bfx.step();

        // 5. 设置静态属性值
        env->SetStaticFloatField(RendererClass, rendererPercentID, rendererPercentValue);

        // 6. 删除本地局部引用表，基本数据类型不需要
        env->DeleteLocalRef(RendererClass);
    }

    delete audioInput;
    bfx.destroy();

    nativeRendererPercent = 0;

    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", "renderer end");

    if (userBreak) {
        return RendererStatus::BREAK;
    } else {
        return RendererStatus::SUCCESS;
    }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_chuangkit_videorenderer_NativeRendererInterface_breakRenderer(JNIEnv *env, jobject thiz) {
    bfx::EngineUtils::Animation::setUserBreak(animationEntity, true);
    return JNI_TRUE;
}