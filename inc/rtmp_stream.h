#pragma once
#include "ffmpeg_rtmp.h"
#include "common.h"

using namespace std;

class RtmpStream
{
private:

    SwsContext *g_swsctx;
    AVFormatContext *g_ofmt_ctx;
    const AVCodec *g_out_codec;
    AVStream *g_out_stream;
    AVCodecContext *g_out_codec_ctx;
    AVFrame *g_push_frame;
    AVFrame *g_yuv_frame;
public:
    RtmpStream();
    ~RtmpStream();
    int InitStream(double width,double height,int fps,int bitrate,string codec_profile,string server);
    void WriteFrame(const uint8_t *src,int width, int height);
    int FlushEncoder();
};
