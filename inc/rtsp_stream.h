#pragma once

#include "common.h"

class RtspStream
{
public:
	RtspStream();
	~RtspStream();

	int AvInit(int picWidth, int picHeight, std::string g_outFile,uint32_t bitRate,uint32_t gopSize, uint16_t frameRate);
	
	void YuvDataInit();
	
	int YuvDataToRtsp(void *dataBuf, uint32_t size, uint32_t seq);
	
	void BgrDataInint();
	int BgrDataToRtsp(void *dataBuf, uint32_t size, uint32_t seq);

	int FlushEncoder();

private:
    AVFormatContext* g_fmtCtx;
    AVCodecContext* g_codecCtx;
    AVStream* g_avStream;
    const AVCodec* g_codec;
    AVPacket* g_pkt;
	AVFrame* g_yuvFrame;
	uint8_t* g_yuvBuf;
	AVFrame* g_rgbFrame;
	uint8_t* g_brgBuf;
	int g_yuvSize;
	int g_rgbSize;
	struct SwsContext* g_imgCtx;
	bool g_bgrToRtspFlag;
	bool g_yuvToRtspFlag;
};
