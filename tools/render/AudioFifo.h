//
// Created by wangrl on 2020/11/24.
//

#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

namespace render {

    struct AudioFifo {

        /// Initialize one data packet for reading or writing.
        ///
        /// @param packet Packet to be initialized
        ///
        static void initPacket(AVPacket* packet);

        /// Initialize one audio frame for reading from the input file.
        ///
        /// @param[out] frame Frame to be initialized
        /// @return Error code (0 if successful)
        ///
        static int initInputFrame(AVFrame** frame);

        /// Decode one audio frame from the input file.
        ///
        /// @param      frame                Audio frame to be decoded
        /// @param      inputFormatContext Format context of the input file
        /// @param      inputCodecContext  Codec context of the input file
        /// @param[out] dataPresent         Indicates whether data has been decoded
        /// @param[out] finished             Indicates whether the end of file has
        ///                                 been reached and all data has been
        ///                                 decoded. If this flag is false, there
        ///                                 is more data to be decoded, i.e., this
        ///                                 function has to be called again.
        /// @return Error code (0 if successful)
        ///
        static int decodeAudioFrame(AVFrame* frame,
                                    AVFormatContext* inputFormatContext,
                                    AVCodecContext* inputCodecContext,
                                    int* dataPresent, int* finished);

        /// Initialize a temporary storage for the specified number of audio samples.
        /// The conversion requires temporary storage due to the different format.
        /// The number of audio samples to be allocated is specified in frame_size.
        ///
        /// @param[out] convertedInputSamples Array of converted samples. The
        ///                                     dimensions are reference, channel
        ///                                     (for multi-channel audio), sample.
        /// @param      outputCodecContext    Codec context of the output file
        /// @param      frameSize              Number of samples to be converted in
        ///                                     each round
        /// @return Error code (0 if successful)
        ///
        static int initConvertedSamples(uint8_t*** convertedInputSamples,
                                        AVCodecContext* outputCodecContext,
                                        int frameSize);

        /// Convert the input audio samples into the output sample format.
        /// The conversion happens on a per-frame basis, the size of which is
        /// specified by frame_size.
        ///
        /// @param      inputData       Samples to be decoded. The dimensions are
        ///                             channel (for multi-channel audio), sample.
        /// @param[out] convertedData   Converted samples. The dimensions are channel
        ///                             (for multi-channel audio), sample.
        /// @param      frameSize       Number of samples to be converted
        /// @param      resampleContext Resample context for the conversion
        /// @return Error code (0 if successful)
        ///
        static int convertSamples(const uint8_t** inputData,
                                  uint8_t** convertedData, const int frameSize,
                                  SwrContext* resampleContext);

        /// Add converted input audio samples to the FIFO buffer for later processing.
        ///
        /// @param fifo                    Buffer to add the samples to
        /// @param convertedInputSamples Samples to be added. The dimensions are channel
        ///                                 (for multi-channel audio), sample.
        /// @param frameSize              Number of samples to be converted
        /// @return Error code (0 if successful)
        ///
        static int addSamplesToFifo(AVAudioFifo* fifo,
                                    uint8_t** convertedInputSamples,
                                    const int frameSize);

        /// Read one audio frame from the input file, decode, convert and store
        /// it in the FIFO buffer.
        ///
        /// @param      fifo                 Buffer used for temporary storage
        /// @param      inputFormatContext Format context of the input file
        /// @param      inputCodecContext  Codec context of the input file
        /// @param      outputCodecContext Codec context of the output file
        /// @param      resamplerContext    Resample context for the conversion
        /// @param[out] finished             Indicates whether the end of file has
        ///                                 been reached and all data has been
        ///                                 decoded. If this flag is false,
        ///                                 there is more data to be decoded,
        ///                                 i.e., this function has to be called again.
        /// @return Error code (0 if successful)
        ///
        static int readDecodeConvertAndStore(AVAudioFifo* fifo,
                                             AVFormatContext* inputFormatContext,
                                             AVCodecContext* inputCodecContext,
                                             AVCodecContext* outputCodecContext,
                                             SwrContext* resamplerContext,
                                             int* finished);

        /// Initialize one input frame for writing to the output file.
        /// The frame will be exactly frame_size samples large.
        ///
        /// @param[out] frame                Frame to be initialized
        /// @param      outputCodecContext Codec context of the output file
        /// @param      frameSize           Size of the frame
        /// @return Error code (0 if successful)
        ///
        static int initOutputFrame(AVFrame** frame,
                                   AVCodecContext* outputCodecContext,
                                   int frameSize);

        /// Encode one frame worth of audio to the output file.
        /// @param      frame                 Samples to be encoded
        /// @param      outputFormatContext Format context of the output file
        /// @param      outputCodecContext  Codec context of the output file
        /// @param[out] dataPresent          Indicates whether data has been
        ///                                 encoded
        /// @return Error code (0 if successful)
        ///
        static int encodeAudioFrame(AVFrame* frame,
                                    AVFormatContext* outputFormatContext,
                                    AVCodecContext* outputCodecContext,
                                    int64_t* audioPts,
                                    int* dataPresent);

        /// Load one audio frame from the FIFO buffer, encode and write it to the output file.
        ///
        /// @param fifo                  Buffer used for temporary storage
        /// @param outputFormatContext Format context of the output file
        /// @param outputCodecContext  Codec context of the output file
        /// @return Error code (0 if successful)
        ///
        static int loadEncodeAndWrite(AVAudioFifo* fifo,
                                      AVFormatContext* outputFormatContext,
                                      AVCodecContext* outputCodecContext, int64_t* audioPts);

    };
} // namespace render

