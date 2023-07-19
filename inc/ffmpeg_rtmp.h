#pragma once

#include <iostream>
#include <vector>
#include "common.h"

using namespace std;

void initialize_avformat_context(AVFormatContext *&fctx, const char *format_name);
void initialize_io_context(AVFormatContext *&fctx, const char *output);
void set_codec_params(AVFormatContext *&fctx, AVCodecContext *&codec_ctx, double width, double height, int fps, int bitrate);
void initialize_codec_stream(AVStream *&stream, AVCodecContext *&codec_ctx, const AVCodec *&codec, std::string codec_profile);
AVFrame *allocate_frame_buffer(AVPixelFormat pix_fmt, double width, double height);
void write_frame(AVCodecContext *codec_ctx, AVFormatContext *fmt_ctx, AVFrame *frame);
SwsContext *initialize_sample_scaler(AVCodecContext *codec_ctx, double width, double height);
