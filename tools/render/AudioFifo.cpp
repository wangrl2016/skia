//
// Created by wangrl on 2020/11/24.
//

#include <cstdio>

#include "tools/render/AudioFifo.h"

namespace render {

    void AudioFifo::initPacket(AVPacket* packet) {
        av_init_packet(packet);
        // set the packet data and size so that it is recognized as being empty
        packet->data = nullptr;
        packet->size = 0;
    }

    int AudioFifo::initInputFrame(AVFrame** frame) {
        if (!(*frame = av_frame_alloc())) {
            printf("Could not allocate input frame");
            return AVERROR(ENOMEM);
        }
        return 0;
    }

    int
    AudioFifo::decodeAudioFrame(AVFrame* frame, AVFormatContext* inputFormatContext, AVCodecContext* inputCodecContext,
                                int* dataPresent, int* finished) {
        // packet used for temporary storage
        AVPacket inputPacket;
        int error;
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        initPacket(&inputPacket);
        // read one audio frame from the input file into a temporary packet
        if ((error = av_read_frame(inputFormatContext, &inputPacket)) < 0) {
            // if we are at the end of the file, flush the decoder below
            if (error == AVERROR_EOF)
                *finished = 1;
            else {
                printf("Could not read frame (error '%s')",
                         av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
                return error;
            }
        }
        // Send the audio frame stored in the temporary packet to the decoder.
        // The input audio stream decoder is used to do this.
        if ((error = avcodec_send_packet(inputCodecContext, &inputPacket)) < 0) {
            printf("Could not send packet for decoding (error '%s')",
                      av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            return error;
        }
        // receive one frame from the decoder
        error = avcodec_receive_frame(inputCodecContext, frame);
        // If the decoder asks for more data to be able to decode a frame,
        // return indicating that no data is present
        if (error == AVERROR(EAGAIN)) {
            error = 0;
            goto cleanup;
            // if the end of the input file is reached, stop decoding
        } else if (error == AVERROR_EOF) {
            *finished = 1;
            error = 0;
            goto cleanup;
        } else if (error < 0) {
            printf("Could not decode frame (error '%s')",
                      av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            goto cleanup;
            // default case: Return decoded data
        } else {
            *dataPresent = 1;
            goto cleanup;
        }
        cleanup:
        av_packet_unref(&inputPacket);
        return error;
    }

    int AudioFifo::initConvertedSamples(uint8_t*** convertedInputSamples, AVCodecContext* outputCodecContext,
                                        int frameSize) {
        int error;
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        // Allocate as many pointers as there are audio channels.
        // Each pointer will later point to the audio samples of the corresponding
        // channels (although it may be NULL for interleaved formats).
        if (!(*convertedInputSamples = (unsigned char**) calloc(outputCodecContext->channels,
                                                                sizeof(**convertedInputSamples)))) {
            printf("Could not allocate converted input sample pointers");
            return AVERROR(ENOMEM);
        }

        // Allocate memory for the samples of all channels in one consecutive
        // block for convenience.
        if ((error = av_samples_alloc(*convertedInputSamples, NULL,
                                      outputCodecContext->channels,
                                      frameSize,
                                      outputCodecContext->sample_fmt, 0)) < 0) {
            printf("Could not allocate converted input samples (error '%s')",
                      av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            av_freep(&(*convertedInputSamples)[0]);
            free(*convertedInputSamples);
            return error;
        }
        return 0;
    }

    int AudioFifo::convertSamples(const uint8_t** inputData, uint8_t** convertedData, const int frameSize,
                                  SwrContext* resampleContext) {
        int error;
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        // convert the samples using the resampler
        if ((error = swr_convert(resampleContext,
                                 convertedData, frameSize,
                                 inputData, frameSize)) < 0) {
            printf("Could not convert input samples (error '%s')",
                     av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            return error;
        }
        return 0;
    }

    int AudioFifo::addSamplesToFifo(AVAudioFifo* fifo, uint8_t** convertedInputSamples, const int frameSize) {
        int error;
        // Make the FIFO as large as it needs to be to hold both,
        // the old and the new samples.
        if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frameSize)) < 0) {
            printf("Could not reallocate FIFO");
            return error;
        }
        // store the new samples in the FIFO buffer
        if (av_audio_fifo_write(fifo, (void**) convertedInputSamples,
                                frameSize) < frameSize) {
            printf("Could not write data to FIFO");
            return AVERROR_EXIT;
        }
        return 0;
    }

    int AudioFifo::readDecodeConvertAndStore(AVAudioFifo* fifo, AVFormatContext* inputFormatContext,
                                             AVCodecContext* inputCodecContext, AVCodecContext* outputCodecContext,
                                             SwrContext* resamplerContext, int* finished) {
        // temporary storage of the input samples of the frame read from the file
        AVFrame* inputFrame = NULL;
        // temporary storage for the converted input samples
        uint8_t** convertedInputSamples = NULL;
        int dataPresent = 0;
        int ret = AVERROR_EXIT;
        // initialize temporary storage for one input frame
        if (initInputFrame(&inputFrame))
            goto cleanup;
        // decode one frame worth of audio samples
        if (decodeAudioFrame(inputFrame, inputFormatContext,
                             inputCodecContext, &dataPresent, finished))
            goto cleanup;

        // If we are at the end of the file and there are no more samples
        // in the decoder which are delayed, we are actually finished.
        // This must not be treated as an error.
        if (*finished) {
            ret = 0;
            goto cleanup;
        }

        // if there is decoded data, convert and store it
        if (dataPresent) {
            // initialize the temporary storage for the converted input samples
            if (initConvertedSamples(&convertedInputSamples, outputCodecContext,
                                     inputFrame->nb_samples))
                goto cleanup;
            // Convert the input samples to the desired output sample format.
            // This requires a temporary storage provided by convertedInputSamples
            if (convertSamples((const uint8_t**) inputFrame->extended_data, convertedInputSamples,
                               inputFrame->nb_samples, resamplerContext))
                goto cleanup;
            // add the converted input samples to the FIFO buffer for later processing
            if (addSamplesToFifo(fifo, convertedInputSamples,
                                 inputFrame->nb_samples))
                goto cleanup;
            ret = 0;
        }
        ret = 0;
        cleanup:
        if (convertedInputSamples) {
            av_freep(&convertedInputSamples[0]);
            free(convertedInputSamples);
        }
        av_frame_free(&inputFrame);
        return ret;
    }

    int AudioFifo::initOutputFrame(AVFrame** frame, AVCodecContext* outputCodecContext, int frameSize) {
        int error;
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        // create a new frame to store the audio samples
        if (!(*frame = av_frame_alloc())) {
            printf("Could not allocate output frame");
            return AVERROR_EXIT;
        }

        // Set the frame's parameters, especially its size and format.
        // av_frame_get_buffer needs this to allocate memory for the
        // audio samples of the frame.
        // Default channel layouts based on the number of channels
        // are assumed for simplicity.
        (*frame)->nb_samples = frameSize;
        (*frame)->channel_layout = outputCodecContext->channel_layout;
        (*frame)->format = outputCodecContext->sample_fmt;
        (*frame)->sample_rate = outputCodecContext->sample_rate;

        // Allocate the samples of the created frame. This call will make
        // sure that the audio frame can hold as many samples as specified.
        if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
            printf("Could not allocate output frame samples (error '%s')",
                      av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            av_frame_free(frame);
            return error;
        }
        return 0;
    }

    int AudioFifo::encodeAudioFrame(AVFrame* frame, AVFormatContext* outputFormatContext,
                                    AVCodecContext* outputCodecContext, int64_t* audioPts, int* dataPresent) {
        // packet used for temporary storage
        AVPacket outputPacket;
        int error;
        char errBuf[AV_ERROR_MAX_STRING_SIZE];
        initPacket(&outputPacket);
        // set a timestamp based on the sample rate for the container
        if (frame) {
            frame->pts = *audioPts;
            (*audioPts) += frame->nb_samples;
        }
        // Send the audio frame stored in the temporary packet to the encoder.
        // The output audio stream encoder is used to do this.
        error = avcodec_send_frame(outputCodecContext, frame);
        // the encoder signals that it has nothing more to encode
        if (error == AVERROR_EOF) {
            error = 0;
            goto cleanup;
        } else if (error < 0) {
            printf("Could not send packet for encoding (error '%s')",
                      av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            return error;
        }
        // receive one encoded frame from the encoder
        error = avcodec_receive_packet(outputCodecContext, &outputPacket);
        // If the encoder asks for more data to be able to provide an
        // encoded frame, return indicating that no data is present.
        if (error == AVERROR(EAGAIN)) {
            error = 0;
            goto cleanup;
            // If the last frame has been encoded, stop encoding
        } else if (error == AVERROR_EOF) {
            error = 0;
            goto cleanup;
        } else if (error < 0) {
            printf("Could not encode frame (error '%s')",
                      av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            goto cleanup;
            // default case: Return encoded data
        } else {
            *dataPresent = 1;
        }
        outputPacket.stream_index = 1;

        // write one audio frame from the temporary packet to the output file
        if (*dataPresent &&
            (error = av_interleaved_write_frame(outputFormatContext, &outputPacket)) < 0) {
            printf("Could not write frame (error '%s')",
                      av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, error));
            goto cleanup;
        }
        cleanup:
        av_packet_unref(&outputPacket);
        return error;
    }

    int AudioFifo::loadEncodeAndWrite(AVAudioFifo* fifo, AVFormatContext* outputFormatContext,
                                      AVCodecContext* outputCodecContext, int64_t* audioPts) {
        // temporary storage of the output samples of the frame written to the file
        AVFrame* outputFrame;
        // Use the maximum number of possible samples per frame.
        // If there is less than the maximum possible frame size in the FIFO
        // buffer use this number. Otherwise, use the maximum possible frame size.
        const int frameSize = FFMIN(av_audio_fifo_size(fifo),
                                    outputCodecContext->frame_size);
        int data_written;
        // initialize temporary storage for one output frame
        if (initOutputFrame(&outputFrame, outputCodecContext, frameSize))
            return AVERROR_EXIT;
        // Read as many samples from the FIFO buffer as required to fill the frame.
        // The samples are stored in the frame temporarily
        if (av_audio_fifo_read(fifo, (void**) outputFrame->data, frameSize) < frameSize) {
            printf("Could not read data from FIFO");
            av_frame_free(&outputFrame);
            return AVERROR_EXIT;
        }
        // encode one frame worth of audio samples
        if (encodeAudioFrame(outputFrame, outputFormatContext,
                             outputCodecContext, audioPts, &data_written)) {
            av_frame_free(&outputFrame);
            return AVERROR_EXIT;
        }
        av_frame_free(&outputFrame);
        return 0;
    }
}   // namespace render
