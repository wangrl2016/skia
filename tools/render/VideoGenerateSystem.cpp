//
// Created by wangrl on 2020/9/18.
//

#include <stdint.h>
#include <include/core/SkImage.h>
#include <include/core/SkImageInfo.h>

extern "C" {
#include <libavutil/pixfmt.h>
#include <libavutil/log.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
}

#include "tools/render/AudioFifo.h"
#include "tools/render/Engine.h"
#include "tools/render/OutputConfigComponent.h"
#include "tools/render/FFmpegContext.h"
#include "tools/render/RenderComponent.h"
#include "tools/render/AnimationComponent.h"
#include "tools/render/VideoGenerateSystem.h"


namespace render {

    void VideoGenerateSystem::init() {
        return;
    }

    void VideoGenerateSystem::update() {
        auto& r = Engine::registry();

        // Skip if there were any errors when loading animation.
        auto componentsView = r.view<AnimationComponent, OutputConfigComponent, FFmpegContext, RenderComponent>();
        for (auto e: componentsView) {
            auto& res = componentsView.get<AnimationComponent>(e);
            if (res.mState == AnimationComponent::State::STOP ||
                res.mState == AnimationComponent::State::IDLE) {
                continue;
            }

            auto& context = componentsView.get<FFmpegContext>(e);
            auto& config = componentsView.get<OutputConfigComponent>(e);
            if (!context.isInited)
                context.isInited = initFFmpegContext(&context, config);

            sk_sp<SkImage> image = componentsView.get<RenderComponent>(e).mImage;

            SkColorType colorType = image->colorType();
            // pixel encode format
            AVPixelFormat avPixelFormat;
            switch (colorType) {
                case kBGRA_8888_SkColorType:
                    avPixelFormat = AV_PIX_FMT_BGRA;
                    break;
                case kRGBA_8888_SkColorType:
                    avPixelFormat = AV_PIX_FMT_RGBA;
                    break;
                default:
                    SkDebugf("Unsupported color type: %d", colorType);
                    return;
            }

            SkPixmap pixmap;
            if (image->peekPixels(&pixmap)) {
                // const uint32_t* pixels = pixmap.addr32();
                const uint8_t* pixels = reinterpret_cast<const uint8_t*>(pixmap.addr32());

                // convert to YUV420P
                const int lineSize[1] = {4 * context.outputVideoStream.codecContext->width};

                if (!context.outputVideoStream.swsContext) {
                    context.outputVideoStream.swsContext = sws_getContext(
                            context.outputVideoStream.codecContext->width,
                            context.outputVideoStream.codecContext->height,
                            avPixelFormat,
                            context.outputVideoStream.codecContext->width,
                            context.outputVideoStream.codecContext->height,
                            AV_PIX_FMT_YUV420P,
                            SWS_BICUBIC,
                            nullptr,
                            nullptr,
                            nullptr
                    );
                }

                sws_scale(context.outputVideoStream.swsContext,
                          &pixels,
                          lineSize,
                          0,
                          context.outputVideoStream.codecContext->height,
                          context.outputVideoStream.frame->data,
                          context.outputVideoStream.frame->linesize
                );

                // Write audio or video frame to the file.
                //
                // Every time the function handleFrame is called, write the rescale picture to mp4 file.
                // Sometime we need a video which has not audio.
                // Use the audio/video timestamp to decide write audio frame or video frame.
                writeVideoFrame(context.outputFormatContext.get(), &context.outputVideoStream);

                while (context.haveAudio &&
                       av_compare_ts(context.outputVideoStream.nextPts,
                                     context.outputVideoStream.codecContext->time_base,
                                     context.outputAudioStream.nextPts,
                                     context.outputAudioStream.codecContext->time_base) > 0 &&
                       (config.audioInput.duration <= 0 ||      // Duration <= 0 mean the system default.
                        context.outputAudioStream.nextPts * av_q2d(context.outputAudioStream.codecContext->time_base)
                        <= config.audioInput.duration)) {
                    // Sometimes the audio is stop before the video frame, then the audio pts is keep equal as before.
                    // The while loop can not quit as always the condition is true.
                    // So when the audio is finish, we break the loop.
                    if (writeAudioFrame(context.inputAudioFormatContext, context.outputFormatContext.get(),
                                        context.inputAudioCodecContext,
                                        &context.outputAudioStream, context.audioFifo) == 1) {
                        break;
                    }
                }
            }
        }
    }

    void VideoGenerateSystem::finalize() {
        // Write the trailer, if any. The trailer must be written before you
        // close the CodecContexts open when you wrote the header; otherwise
        // av_write_trailer() may try to use memory that was freed on
        // av_codec_close().
        auto& r = Engine::registry();
        auto componentsView = r.view<AnimationComponent, FFmpegContext, RenderComponent>();

        for (auto e : componentsView) {
            auto& res = componentsView.get<AnimationComponent>(e);
            if (res.mState == AnimationComponent::State::STOP || res.mState == AnimationComponent::State::IDLE)
                continue;

            auto& context = componentsView.get<FFmpegContext>(e);

            // If the user break the FFmpeg video process, we don't need to write the rest frame to file.
            // Because the user may delete our file immediately, cause our writing core dump.
            // When buffer is flushed the cycle will break.
            int ret;
            AVPacket pkt;
            do {
                // When the frame parameter is null the buffer will be flushed.
                // General buffer flushing at end of stream.
                avcodec_send_frame(context.outputVideoStream.codecContext, nullptr);
                ret = avcodec_receive_packet(context.outputVideoStream.codecContext, &pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
                    break;
                }
                writeFrame(context.outputFormatContext.get(), &context.outputVideoStream.codecContext->time_base,
                           context.outputVideoStream.stream, &pkt);
                av_packet_unref(&pkt);
            } while (ret == 0);

            if (context.isInited)
                av_write_trailer(context.outputFormatContext.get());

            if (context.haveAudio) {
                if (context.audioFifo)
                    av_audio_fifo_free(context.audioFifo);

                if (context.inputAudioCodecContext)
                    avcodec_free_context(&context.inputAudioCodecContext);
                if (context.inputAudioFormatContext)
                    avformat_close_input(&context.inputAudioFormatContext);
            }

            // close each codec
            // Todo: using scope guard instead.
            if (context.haveVideo)
                OutputStream::close(context.outputVideoStream);
            if (context.haveAudio)
                OutputStream::close(context.outputAudioStream);

            context.haveAudio = false;
            context.haveVideo = false;
            context.isInited = false;
        }
    }

    bool VideoGenerateSystem::initFFmpegContext(FFmpegContext* context, OutputConfigComponent& config) {
        static constexpr auto kBitRate = 3000000;
        static constexpr auto kFrameRate = 24; // 24 fps
        static constexpr auto kPixelFormat = AV_PIX_FMT_YUV420P;

        av_log_set_level(AV_LOG_ERROR);

        context->output = config.outputPath;
        if (context->output.empty()) context->output = "out/sink/video.mp4";

        SkDebugf("Output media file name %s", context->output.c_str());

        context->width = config.outputSize.width();
        context->height = config.outputSize.height();

        std::string audioPath = config.audioInput.filePath;
        SkDebugf("Input audio file path %s", audioPath.c_str());
        // init input audio FFmpeg parameter, using it parameter to init audio output
        if (audioPath.empty() ||
            (initInputConfig(audioPath.c_str(), &context->inputAudioFormatContext, &context->inputAudioCodecContext,
                             AVMEDIA_TYPE_AUDIO) < 0)) {
            // just print warn message, a video can has not audio
            context->haveAudio = false;
            SkDebugf("Init input audio FFmpeg parameters failed");
        } else {
            context->haveAudio = true;
            AudioInput& audioInput = config.audioInput;
            // Set the audio start time.
            av_seek_frame(context->inputAudioFormatContext, -1, audioInput.startTime * AV_TIME_BASE,
                          AVSEEK_FLAG_BACKWARD);
        }

        // Allocate the output media context.
        AVFormatContext* formatContext;
        avformat_alloc_output_context2(&formatContext, nullptr, nullptr, context->output.data());
        if (!formatContext) {
            SkDebugf("Could not deduce output format from file extension: using MPEG.");
            avformat_alloc_output_context2(&formatContext, nullptr, "mpeg", context->output.data());
        }

        assert(formatContext);
        context->outputFormatContext = std::unique_ptr<AVFormatContext>(formatContext);
        context->outputFormat = context->outputFormatContext->oformat;

        // Add the audio and video streams using the default format codecs
        // and initialize the codecs.
        if (context->outputFormat->video_codec != AV_CODEC_ID_NONE) {
            context->outputVideoStream = setup(context->outputFormatContext.get(),
                                               context->outputFormat->video_codec);

            auto& codecContext = context->outputVideoStream.codecContext;
            codecContext->codec_id = context->outputFormat->video_codec;
            codecContext->bit_rate = kBitRate;
            // Resolution must be a multiple of two.
            codecContext->width = context->width;
            codecContext->height = context->height;
            // timebase: This is the fundamental unit of time (in seconds) in terms
            // of which frame timestamps are represented. For fixed-fps content,
            // timebase should be 1/framerate and timestamp increments should be
            // identical to 1.
            context->outputVideoStream.stream->time_base = (AVRational) {1, kFrameRate};
            codecContext->time_base = context->outputVideoStream.stream->time_base;
            codecContext->gop_size = 12; // emit one intra frame every twelve frames at most
            codecContext->pix_fmt = kPixelFormat;
            if (codecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
                // Just for testing, we also add B-frames.
                codecContext->max_b_frames = 2;
            }
            if (codecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
                // Needed to avoid using macroblocks in which some coeffs overflow.
                // This does not happen with normal video, it just happens here as
                // the motion of the chroma plane does not match the luma plane.
                codecContext->mb_decision = 2;
            }
            if (codecContext->codec_id == AV_CODEC_ID_H264) {
                av_opt_set(codecContext->priv_data, "preset", "fast", 0);
            }
            context->haveVideo = true;
        }

        if (context->haveAudio && context->outputFormat->audio_codec != AV_CODEC_ID_NONE) {
            context->outputAudioStream = setup(context->outputFormatContext.get(),
                                               context->outputFormat->audio_codec);

            auto& codecContext = context->outputAudioStream.codecContext;
            auto& codec = context->outputAudioStream.codec;

            int i;

            codecContext->sample_fmt = codec->sample_fmts ?
                                       codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
            codecContext->bit_rate = 64000;
            codecContext->sample_rate = context->inputAudioCodecContext->sample_rate;
            if (codec->supported_samplerates) {
                codecContext->sample_rate = codec->supported_samplerates[0];
                for (i = 0; codec->supported_samplerates[i]; i++) {
                    if (codec->supported_samplerates[i] == 44100)
                        codecContext->sample_rate = 44100;
                }
            }

            codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
            if (codec->channel_layouts) {
                codecContext->channel_layout = codec->channel_layouts[0];
                for (i = 0; codec->channel_layouts[i]; i++) {
                    if (codec->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                        codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
                }
            }
            codecContext->channels = av_get_channel_layout_nb_channels(codecContext->channel_layout);
            context->outputAudioStream.stream->time_base = (AVRational) {1, codecContext->sample_rate};
        }

        // Now that all the parameters are set, we can open the audio and
        // video codecs and allocate the necessary encode buffers.
        if (context->haveVideo) {
            openVideo(&context->outputVideoStream, context->options);
        }

        if (context->haveAudio) {
            openAudio(&context->outputAudioStream, context->inputAudioCodecContext, context->options);
        }

        // Dump output media metadata.
        av_dump_format(context->outputFormatContext.get(), 0, context->output.data(), 1);

        int ret;
        char errorBuf[AV_ERROR_MAX_STRING_SIZE];
        // Open the output file, if needed.
        if (!(context->outputFormatContext->flags & AVFMT_NOFILE)) {
            ret = avio_open(&context->outputFormatContext->pb, context->output.data(), AVIO_FLAG_WRITE);
            if (ret < 0) {
                return false;
            }
        }

        // Initialize the FIFO buffer to store audio samples to be encoded.
        if (context->haveAudio && initAudioFifo(&context->audioFifo, context->outputAudioStream.codecContext)) {
            SkDebugf("Could not allocate audio FIFO");
        }

        // Write the stream header, if any.
        ret = avformat_write_header(context->outputFormatContext.get(), &context->options);
        if (ret < 0) {
            SkDebugf("Error occurred when opening output file: {}",
                     av_make_error_string(errorBuf, AV_ERROR_MAX_STRING_SIZE, ret));
            return false;
        }

        return true;
    }

    int VideoGenerateSystem::initInputConfig(const char* filename,
                                             AVFormatContext** inputFormatContext,
                                             AVCodecContext** inputCodecContext,
                                             AVMediaType type) {
        AVCodecContext* codecContext;
        AVCodec* iCodec;
        int error;
        char errBuf[AV_ERROR_MAX_STRING_SIZE];

        // open the input media file to read FFmpeg info
        if ((error = avformat_open_input(inputFormatContext, filename, nullptr, nullptr)) < 0) {
            SkDebugf("Could not open input file '%s' (error '%s')", filename,
                     av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            *inputFormatContext = nullptr;
            return error;
        }

        // get information on the input file (number of stream etc)
        if ((error = avformat_find_stream_info(*inputFormatContext, nullptr)) < 0) {
            SkDebugf("Could not find stream info (error '%s')",
                     av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            avformat_close_input(inputFormatContext);
            return error;
        }

        // -1 for auto selection
        int index = av_find_best_stream(*inputFormatContext, type, -1, -1, nullptr, 0);

        if (type == AVMEDIA_TYPE_AUDIO) {
            // find a decoder for the audio stream
            if (!(iCodec = avcodec_find_decoder((*inputFormatContext)->streams[index]->codecpar->codec_id))) {
                SkDebugf("Could not find input codec");
                avformat_close_input(inputFormatContext);
                return AVERROR_EXIT;
            }
        } else {
            SkDebugf("The input media type is not implemented");
            return AVERROR_UNKNOWN;
        }

        // allocate a new decoding context
        codecContext = avcodec_alloc_context3(iCodec);
        if (!codecContext) {
            SkDebugf("Could not allocate a decoding context");
            avformat_close_input(inputFormatContext);
            avcodec_free_context(&codecContext);
            return AVERROR(ENOMEM);
        }

        // initialize the stream parameters with demuxer information
        error = avcodec_parameters_to_context(codecContext, (*inputFormatContext)->streams[index]->codecpar);
        if (error < 0) {
            avformat_close_input(inputFormatContext);
            avcodec_free_context(&codecContext);
            return error;
        }

        // open the decoder for the audio stream to use it later
        if ((error = avcodec_open2(codecContext, iCodec, nullptr)) < 0) {
            SkDebugf("Could not open input codec (error '%s')",
                     av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            avcodec_free_context(&codecContext);
            avformat_close_input(inputFormatContext);
            return error;
        }

        // save the decode context for easier access later
        *inputCodecContext = codecContext;

        return EXIT_SUCCESS;
    }

    OutputStream VideoGenerateSystem::setup(AVFormatContext* outputFormatContext, AVCodecID codecId) {
        OutputStream stream = {};

        // find the encoder
        stream.codec = avcodec_find_encoder(codecId);
        if (!stream.codec) {
            SkDebugf("Could not find encoder for '{}'", avcodec_get_name(codecId));
        }

        stream.stream = avformat_new_stream(outputFormatContext, nullptr);
        if (!stream.stream) {
            SkDebugf("Could not allocate stream");
        }
        stream.stream->id = outputFormatContext->nb_streams - 1;

        stream.codecContext = avcodec_alloc_context3(stream.codec);

        if (!stream.codecContext) {
            SkDebugf("Could not alloc an encoding context");
        }

        // Some formats want stream headers to be separate.
        if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
            stream.codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        return stream;
    }

    int VideoGenerateSystem::openVideo(OutputStream* outputVideoStream, AVDictionary* optArg) {
        int ret;
        char errorBuf[AV_ERROR_MAX_STRING_SIZE];

        AVCodecContext* codecContext = outputVideoStream->codecContext;
        AVDictionary* opt = nullptr;
        av_dict_copy(&opt, optArg, 0);

        // open the codec
        ret = avcodec_open2(codecContext, outputVideoStream->codec, &opt);
        av_dict_free(&opt);
        if (ret < 0)
            SkDebugf("Could not open video codec: %s",
                     av_make_error_string(errorBuf, AV_ERROR_MAX_STRING_SIZE, ret));

        // allocate and init a re-usable frame
        outputVideoStream->frame = createFrame(codecContext->pix_fmt, codecContext->width, codecContext->height);
        if (!outputVideoStream->frame)
            SkDebugf("Could not allocate video frame");

        // If the output format is not YUV420P, then a temporary YUV420P
        // picture is needed too. It is then converted to the required
        // output format.
        outputVideoStream->tempFrame = nullptr;
        if (codecContext->pix_fmt != AV_PIX_FMT_YUV420P) {
            outputVideoStream->tempFrame = createFrame(AV_PIX_FMT_YUV420P, codecContext->width,
                                                       codecContext->height);
            if (!outputVideoStream->tempFrame) {
                SkDebugf("Could not allocate temporary picture");
            }
        }

        // copy the stream parameters to the muxer
        ret = avcodec_parameters_from_context(outputVideoStream->stream->codecpar, codecContext);
        if (ret < 0) {
            SkDebugf("Could not copy the stream parameters");
        }

        return ret;
    }

    int VideoGenerateSystem::openAudio(OutputStream* outputAudioStream, AVCodecContext* inputAudioContext,
                                       AVDictionary* optArg) {
        int nbSamples, ret;
        char errorBuf[AV_ERROR_MAX_STRING_SIZE];

        AVDictionary* opt = nullptr;
        av_dict_copy(&opt, optArg, 0);
        // open it
        auto codecContext = outputAudioStream->codecContext;
        ret = avcodec_open2(codecContext, outputAudioStream->codec, &opt);
        av_dict_free(&opt);
        if (ret < 0)
            SkDebugf("Could not open audio codec: {}",
                     av_make_error_string(errorBuf, AV_ERROR_MAX_STRING_SIZE, ret));

        if (codecContext->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
            nbSamples = 10000;
        else
            nbSamples = codecContext->frame_size;
        outputAudioStream->frame = createFrame(codecContext->sample_fmt, codecContext->channel_layout,
                                               codecContext->sample_rate, nbSamples);
        outputAudioStream->tempFrame = createFrame(AV_SAMPLE_FMT_S16, codecContext->channel_layout,
                                                   codecContext->sample_rate, nbSamples);
        // copy the stream parameters to the muxer
        ret = avcodec_parameters_from_context(outputAudioStream->stream->codecpar, codecContext);
        if (ret < 0) {
            SkDebugf("Could not copy the stream parameters");
        }
        // create resampler context
        outputAudioStream->swrContext = swr_alloc();
        if (!outputAudioStream->swrContext) {
            SkDebugf("Could not allocate resampler context");
        }

        // set options
        av_opt_set_int(outputAudioStream->swrContext, "in_channel_count", inputAudioContext->channels, 0);
        av_opt_set_int(outputAudioStream->swrContext, "in_sample_rate", inputAudioContext->sample_rate, 0);
        av_opt_set_sample_fmt(outputAudioStream->swrContext, "in_sample_fmt", inputAudioContext->sample_fmt, 0);

        av_opt_set_int(outputAudioStream->swrContext, "out_channel_count", codecContext->channels, 0);
        av_opt_set_int(outputAudioStream->swrContext, "out_sample_rate", codecContext->sample_rate, 0);
        av_opt_set_sample_fmt(outputAudioStream->swrContext, "out_sample_fmt", codecContext->sample_fmt, 0);

        // Perform a sanity check so that the number of converted samples is
        // not greater than the number of samples to be converted.
        // If the sample rates differ, this case has to be handled differently.

        // initialize the resampling context
        if (swr_init(outputAudioStream->swrContext) < 0) {
            SkDebugf("Failed to initialize the resampling context");
        }
        return 0;
    }

    AVFrame* VideoGenerateSystem::createFrame(AVPixelFormat pixelFormat, int width, int height) {
        AVFrame* picture = av_frame_alloc();
        if (!picture) { return nullptr; }

        picture->format = pixelFormat;
        picture->width = width;
        picture->height = height;

        // allocate the buffers for the frame data
        if (av_frame_get_buffer(picture, 32) < 0) {
            SkDebugf("Could not allocate frame data");
        }
        return picture;
    }

    AVFrame* VideoGenerateSystem::createFrame(AVSampleFormat sampleFormat, int64_t channelLayout, int sampleRate,
                                              int sampleCount) {
        AVFrame* frame = av_frame_alloc();
        if (!frame) {
            SkDebugf("Error allocating an audio frame");
        }

        assert(frame);

        frame->format = sampleFormat;
        frame->channel_layout = channelLayout;
        frame->sample_rate = sampleRate;
        frame->nb_samples = sampleCount;

        if (sampleCount) {
            if (av_frame_get_buffer(frame, 0) < 0) {
                SkDebugf("Error allocating an audio buffer");
            }
        }
        return frame;
    }

    int VideoGenerateSystem::initAudioFifo(AVAudioFifo** fifo, AVCodecContext* outputCodecContext) {
        // create the FIFO buffer based on the specified output sample format
        if (!(*fifo = av_audio_fifo_alloc(outputCodecContext->sample_fmt,
                                          outputCodecContext->channels, 1))) {
            SkDebugf("Could not allocate FIFO");
            return AVERROR(ENOMEM);
        }
        return 0;
    }

    int
    VideoGenerateSystem::writeFrame(AVFormatContext* outputFormatContext, const AVRational* timeBase,
                                    AVStream* stream,
                                    AVPacket* packet) {
        // rescale output packet timestamp values from codec to stream timebase
        av_packet_rescale_ts(packet, *timeBase, stream->time_base);
        packet->stream_index = stream->index;
        // write the compressed frame to the media file
        // logPacket(outputFormatContext, packet);
        return av_interleaved_write_frame(outputFormatContext, packet);
    }

    int
    VideoGenerateSystem::writeVideoFrame(AVFormatContext* outputFormatContext, OutputStream* outputVideoStream) {
        int ret;

        AVCodecContext* codecContext = outputVideoStream->codecContext;

        AVPacket pkt;
        av_init_packet(&pkt); // Fixme: missing av_packet_free()

        // 时间戳相关，修改nextPts的值
        outputVideoStream->frame->pts = outputVideoStream->nextPts++;

        // avcodec_encode_video2 already deprecated by FFmpeg
        // ret = avcodec_encode_video2(codecContext, &pkt, ost->frame, &got_packet);
        ret = avcodec_send_frame(codecContext, outputVideoStream->frame);
        if (ret < 0) {
            SkDebugf("Error sending a frame for encoding");
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(codecContext, &pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return ret;
            else if (ret < 0) {
                SkDebugf("Error during encoding");
                return ret;
            }
            writeFrame(outputFormatContext, &codecContext->time_base, outputVideoStream->stream, &pkt);
            av_packet_unref(&pkt);
        }

        return outputVideoStream->frame ? 0 : 1;
    }

    int VideoGenerateSystem::writeAudioFrame(AVFormatContext* inputFormatContext,
                                             AVFormatContext* outputFormatContext,
                                             AVCodecContext* codecContext,
                                             OutputStream* outputStream, AVAudioFifo* fifo) {
        // use the encoder's desired frame size for processing
        const int outputFrameSize = outputStream->codecContext->frame_size;
        int finished = 0;

        // Make sure that there is one frame worth of samples in the FIFO
        // buffer so that the encoder can do its work.
        // Since the decoder's and the encoder's frame size may differ, we
        // need to FIFO buffer to store as many frames worth of input samples
        // that they make up at least one frame worth of output samples.
        while (av_audio_fifo_size(fifo) < outputFrameSize) {
            // Decode one frame worth of audio samples, convert it to the
            // output sample format and put it into the FIFO buffer.
            if (AudioFifo::readDecodeConvertAndStore(fifo, inputFormatContext,
                                                     codecContext,
                                                     outputStream->codecContext,
                                                     outputStream->swrContext, &finished)) {
                SkDebugf("Read decode convert and store fail");
                break;
            }
            // If we are at the end of the input file, we continue
            // encoding the remaining audio samples to the output file
            if (finished)
                break;
        }
        // If we have enough samples for the encoder, we encode them.
        // At the end of the file, we pass the remaining samples to
        // the encoder.
        while (av_audio_fifo_size(fifo) >= outputFrameSize ||
               (finished && av_audio_fifo_size(fifo) > 0))
            // Take one frame worth of audio samples from the FIFO buffer,
            // encode it and write it to the output file.
            if (AudioFifo::loadEncodeAndWrite(fifo, outputFormatContext,
                                              outputStream->codecContext, &(outputStream->nextPts))) {
                SkDebugf("Load encode and write fail");
                break;
            }
        // If we are at the end of the input file and have encoded
        // all remaining samples, we can exit init filters this loop and finish.
        if (finished) {
            int dataWritten;
            // flush the encoder as it may have delayed frames
            do {
                dataWritten = 0;
                if (AudioFifo::encodeAudioFrame(nullptr, outputFormatContext,
                                                outputStream->codecContext, &(outputStream->nextPts),
                                                &dataWritten)) {
                }
            } while (dataWritten);
            return 1;
        }
        return 0;
    }

    void OutputStream::close(OutputStream& outputStream) {
        avcodec_free_context(&outputStream.codecContext);
        av_frame_free(&outputStream.frame);
        av_frame_free(&outputStream.tempFrame);
        sws_freeContext(outputStream.swsContext);
        swr_free(&outputStream.swrContext);
    }

}  // namespace render
