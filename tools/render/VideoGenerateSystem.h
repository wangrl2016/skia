//
// Created by wangrl on 2020/9/18.
//

#pragma once

#include "tools/render/System.h"
#include "tools/render/FFmpegContext.h"
#include "tools/render/OutputConfigComponent.h"

namespace render {

    class VideoGenerateSystem : public System {
    public:

        VideoGenerateSystem() = default;

        void init() override;

        void update() override;

        void finalize() override;


    private:

        /// Init FFmpeg context, such as AVFormatContext, AVCodecContext, to generate the media file.
        ///
        /// @param [out]context         FFmpeg context
        /// @param config               Output config
        /// @return true when success
        ///
        static bool initFFmpegContext(FFmpegContext* context, OutputConfigComponent& config);

        /// use input media file to init FFmpeg input configure, it will init AVFormatContext and
        /// AVCodecContext, any error will return.
        ///
        /// @param filename             The input file name
        /// @param [out]inputFormatContext      Input format context, include many media info
        /// @param [out]inputCodecContext       Input codec context, include decode info
        /// @param type                 The media type, video or audio
        /// @return < 0 if error, 0 success
        ///
        static int initInputConfig(const char* filename, AVFormatContext** inputFormatContext,
                                   AVCodecContext** inputCodecContext, AVMediaType type);


        /// Set up output video/audio stream parameter
        ///
        /// @param outputFormatContext Output media format context
        /// @param codecId Video or audio id of AVFormatContext
        ///
        static OutputStream setup(AVFormatContext* outputFormatContext, AVCodecID codecId);

        /// Init video stream
        ///
        /// @param outputVideoStream Collect of Output video info
        ///
        static int openVideo(OutputStream* outputVideoStream, AVDictionary* optArg);

        /// Init audio stream
        ///
        /// @param outputAudioStream Collect of Output video context
        /// @param inputAudioContext    Input audio codec context
        ///
        static int openAudio(OutputStream* outputAudioStream, AVCodecContext* inputAudioContext, AVDictionary* optArg);

        // create video frame
        static AVFrame* createFrame(AVPixelFormat pixelFormat, int width, int height);

        // create audio frame
        static AVFrame* createFrame(AVSampleFormat sampleFormat, int64_t channelLayout, int sampleRate,
                                    int sampleCount);

        /// Initialize a FIFO buffer for the audio samples to be encoded.
        ///
        /// @param[out] fifo                 Sample buffer
        /// @param      outputCodecContext Codec context of the output file
        /// @return Error code (0 if successful)
        ///
        static int initAudioFifo(AVAudioFifo** fifo, AVCodecContext* outputCodecContext);

        /// Write packet to media file.
        ///
        /// @param outputFormatContext    Output media file format context
        /// @param timeBase     timestamp
        /// @param stream       A/V stream
        /// @param packet   The packet write to the media file
        ///
        static int writeFrame(AVFormatContext* outputFormatContext, const AVRational* timeBase, AVStream* stream,
                              AVPacket* packet);


        /// Encode one video frame and send it to the muxer
        ///
        /// @return 1 when encoding is finished, 0 otherwise
        ///
        static int writeVideoFrame(AVFormatContext* outputFormatContext, OutputStream* outputVideoStream);

        /// Encode one audio frame and send it to the muxer
        ///
        /// @param inputFormatContext      Input audio format context
        /// @param outputFormatContext           Output media format context
        /// @param codecContext             Input audio codec context
        /// @param outputStream         Output audio Stream
        /// @param fifo                 Audio FIFO
        /// @return 1 when encoding is finished, 0 otherwise
        ///
        static int writeAudioFrame(AVFormatContext* inputFormatContext,
                                   AVFormatContext* outputFormatContext,
                                   AVCodecContext* codecContext,
                                   OutputStream* outputStream, AVAudioFifo* fifo);

    };

}
