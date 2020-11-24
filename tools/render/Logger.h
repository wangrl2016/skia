//
// Created by wangrl on 2020/11/24.
//

#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
}

namespace render {

    static void logPacket(const AVFormatContext* formatContext, const AVPacket* pkt) {
        AVRational* time_base = &formatContext->streams[pkt->stream_index]->time_base;

        char ptsBuf[AV_TS_MAX_STRING_SIZE];
        char ptsTimeBuf[AV_TS_MAX_STRING_SIZE];
        char dtsBuf[AV_TS_MAX_STRING_SIZE];
        char dtsTimeBuf[AV_TS_MAX_STRING_SIZE];
        char durationBuf[AV_TS_MAX_STRING_SIZE];
        char durationTimeBuf[AV_TS_MAX_STRING_SIZE];

        printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
               av_ts_make_string(ptsBuf, pkt->pts), av_ts_make_time_string(ptsTimeBuf, pkt->dts, time_base),
               av_ts_make_string(dtsBuf, pkt->dts), av_ts_make_time_string(dtsTimeBuf, pkt->dts, time_base),
               av_ts_make_string(durationBuf, pkt->duration),
               av_ts_make_time_string(durationTimeBuf, pkt->duration, time_base),
               pkt->stream_index);
    }
}
