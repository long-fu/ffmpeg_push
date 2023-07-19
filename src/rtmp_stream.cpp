
#include "rtmp_stream.h"

using namespace std;

RtmpStream::RtmpStream()
{
}

int RtmpStream::InitStream(double width, double height, int fps, int bitrate, string codec_profile, string server)
{
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif
    avformat_network_init();

    const char *output = server.c_str();

    // initialize_avformat_context(g_ofmt_ctx, "flv");
    int ret = avformat_alloc_output_context2(&g_ofmt_ctx, nullptr, "flv", nullptr);
    if (ret < 0)
    {
        std::cout << "Could not allocate output format context!" << std::endl;
        exit(1);
    }

    // initialize_io_context(g_ofmt_ctx, output);

    if (!(g_ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open2(&g_ofmt_ctx->pb, output, AVIO_FLAG_WRITE, nullptr, nullptr);
        if (ret < 0)
        {
            std::cout << "Could not open output IO context!" << std::endl;
            exit(1);
        }
    }

    g_out_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    g_out_stream = avformat_new_stream(g_ofmt_ctx, g_out_codec);
    g_out_codec_ctx = avcodec_alloc_context3(g_out_codec);

    // set_codec_params(g_ofmt_ctx, g_out_codec_ctx, width, height, fps, bitrate);
    const AVRational dst_fps = {fps, 1};

    g_out_codec_ctx->codec_tag = 0;
    g_out_codec_ctx->codec_id = AV_CODEC_ID_H264;
    g_out_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    g_out_codec_ctx->width = width;
    g_out_codec_ctx->height = height;
    g_out_codec_ctx->gop_size = 12;
    g_out_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    g_out_codec_ctx->framerate = dst_fps;
    g_out_codec_ctx->time_base = av_inv_q(dst_fps);
    g_out_codec_ctx->bit_rate = bitrate;
    // 去掉B帧只留下 I帧和P帧
    g_out_codec_ctx->max_b_frames = 0;

    // if (fctx->oformat->flags & AVFMT_GLOBALHEADER)
    // {
    //   codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    // }

    // AV_CODEC_FLAG_GLOBAL_HEADER  -- 将全局头文件放在引渡文件中，而不是每个关键帧中。
    // AV_CODEC_FLAG_LOW_DELAY      -- 较低延迟
    g_out_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER | AV_CODEC_FLAG_LOW_DELAY;

    // codec_ctx->me_range

    g_ofmt_ctx->max_interleave_delta = 1000;

    // initialize_codec_stream(g_out_stream, g_out_codec_ctx, g_out_codec, codec_profile);
    ret = avcodec_parameters_from_context(g_out_stream->codecpar, g_out_codec_ctx);
    if (ret < 0)
    {
        ERROR_LOG("Could not initialize stream codec parameters!");
        exit(1);
    }

    AVDictionary *codec_options = nullptr;
    // 质量
    av_dict_set(&codec_options, "profile", codec_profile.c_str(), 0);
    // 质量
    av_dict_set(&codec_options, "preset", "superfast", 0);
    // 实时推流，零延迟
    av_dict_set(&codec_options, "tune", "zerolatency", 0);

    // open video encoder
    ret = avcodec_open2(g_out_codec_ctx, g_out_codec, &codec_options);
    if (ret < 0)
    {
        // std::cout << "Could not open video encoder!" << std::endl;
        ERROR_LOG("Could not open video encoder!");
        exit(1);
    }

    g_out_stream->codecpar->extradata = g_out_codec_ctx->extradata;
    g_out_stream->codecpar->extradata_size = g_out_codec_ctx->extradata_size;

    av_dump_format(g_ofmt_ctx, 0, output, 1);

    // g_swsctx = initialize_sample_scaler(g_out_codec_ctx, width, height);
    g_swsctx = sws_getContext(width, height, AV_PIX_FMT_NV12,
                              width, height, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                              nullptr, nullptr, nullptr);
    if (!g_swsctx)
    {
        ERROR_LOG("Could not initialize sample scaler!");
        exit(1);
    }

    // intput
    // g_yuv_frame = allocate_frame_buffer(AV_PIX_FMT_NV12, width, height);
    {
        g_yuv_frame = av_frame_alloc();
        int i = av_image_get_buffer_size(AV_PIX_FMT_NV12, width, height, 1);
        uint8_t *framebuf = new uint8_t[i];

        av_image_fill_arrays(g_yuv_frame->data, g_yuv_frame->linesize, framebuf, AV_PIX_FMT_NV12, width, height, 1);
        g_yuv_frame->width = width;
        g_yuv_frame->height = height;
        g_yuv_frame->format = static_cast<int>(AV_PIX_FMT_NV12);
    }

    // out
    // g_push_frame = allocate_frame_buffer(g_out_codec_ctx->pix_fmt, width, height);
    {
        g_push_frame = av_frame_alloc();
        int i = av_image_get_buffer_size(g_out_codec_ctx->pix_fmt, width, height, 1);
        uint8_t *framebuf = new uint8_t[i];

        av_image_fill_arrays(g_push_frame->data, g_push_frame->linesize, framebuf, g_out_codec_ctx->pix_fmt, width, height, 1);
        g_push_frame->width = width;
        g_push_frame->height = height;
        g_push_frame->format = static_cast<int>(g_out_codec_ctx->pix_fmt);
    }

    ret = avformat_write_header(g_ofmt_ctx, nullptr);
    if (ret < 0)
    {
        ERROR_LOG("Could not write header!");
        exit(1);
    }
    return ret;
}

void RtmpStream::WriteFrame(const uint8_t *src, int width, int height)
{

    av_image_fill_arrays(g_yuv_frame->data, g_yuv_frame->linesize, src, AV_PIX_FMT_NV12, width, height, 1);

    int ret = sws_scale(g_swsctx, g_yuv_frame->data, g_yuv_frame->linesize, 0, height, g_push_frame->data, g_push_frame->linesize);

    if (ret < 0)
    {
        ERROR_LOG("Could not initialize stream codec parameters!");
        exit(1);
    }

    g_push_frame->pts += av_rescale_q(1, g_out_codec_ctx->time_base, g_out_stream->time_base);

    AVPacket pkt = {0};
    av_new_packet(&pkt, 0);

    ret = avcodec_send_frame(g_out_codec_ctx, g_push_frame);
    if (ret < 0)
    {
        ERROR_LOG("Error sending frame to codec context!");
        exit(1);
    }

    ret = avcodec_receive_packet(g_out_codec_ctx, &pkt);
    if (ret < 0)
    {
        ERROR_LOG("Error receiving packet from codec context!");
        exit(1);
    }

    av_interleaved_write_frame(g_ofmt_ctx, &pkt);
    av_packet_unref(&pkt);
}

int RtmpStream::FlushEncoder()
{
    av_write_trailer(g_ofmt_ctx);
    sws_freeContext(g_swsctx);
    av_frame_free(&g_push_frame);
    av_frame_free(&g_yuv_frame);
    avcodec_close(g_out_codec_ctx);
    avio_close(g_ofmt_ctx->pb);
    avformat_free_context(g_ofmt_ctx);
    return 0;
}

RtmpStream::~RtmpStream()
{
}
