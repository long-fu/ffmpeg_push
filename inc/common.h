#pragma once
#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
#include <memory>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if 0
#include <opencv2/opencv.hpp>
#endif

extern"C" {
	#include <libavutil/mathematics.h>
	#include <libavutil/time.h>
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
	#include "libavutil/imgutils.h"
	#include "libavutil/opt.h"
};

#define INFO_LOG(fmt, args...) fprintf(stdout, "[INFO]  " fmt "\n", ##args)
#define WARN_LOG(fmt, args...) fprintf(stdout, "[WARN]  " fmt "\n", ##args)
#define ERROR_LOG(fmt, args...) fprintf(stderr, "[ERROR]   " fmt "\n", ##args)