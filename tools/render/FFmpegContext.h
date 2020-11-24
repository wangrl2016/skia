//
// Created by wangrl on 2020/11/24.
//

#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

namespace render {

    // a wrapper around a single output AVStream
    struct OutputStream {
        AVStream* stream;
        AVCodecContext* codecContext;
        AVCodec* codec;

        // pts of the next frame/sample that will be generated
        int64_t nextPts;

        AVFrame* frame;
        AVFrame* tempFrame;

        // video rescale
        mutable SwsContext* swsContext;
        // audio resample
        SwrContext* swrContext;

        static void close(OutputStream& outputStream);
    };

    struct FFmpegContext {

        // Output media file name.
        std::string output;

        // Video configure width and height.
        int width = 0, height = 0;

        // Whether output have audio or not.
        bool haveAudio = false;

        // Whether output have video or not.
        bool haveVideo = false;

        // Audio input format context.
        AVFormatContext* inputAudioFormatContext = nullptr;
        // Audio input codec context.
        AVCodecContext* inputAudioCodecContext = nullptr;

        // The output media file configure parameters.
        // Output media format context.
        std::unique_ptr<AVFormatContext> outputFormatContext;
        // Output format.
        AVOutputFormat* outputFormat;
        mutable OutputStream outputAudioStream;
        mutable OutputStream outputVideoStream;

        AVDictionary* options = nullptr;
        AVAudioFifo* audioFifo = nullptr;

        bool isInited = false;
    };
}   // namespace render

