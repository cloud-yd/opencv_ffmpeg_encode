#ifndef _AV_H264_H_
#define _AV_H264_H_

/***********************************************************
**	Author:yangdong
**	Data:2020-4-30
**  Mail:yangdongaln@163.com
**	Explain:a h264 codec
***********************************************************/

#include <opencv2/opencv.hpp>

#define __STDC_CONSTANT_MACROS 
#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
};
#endif

typedef struct AvH264EncConfig_T {

	int width = 320;
	int height = 240;
	int frame_rate = 25;
	int64_t bit_rate = 320000;
	int gop_size = 250;
	int max_b_frames = 0;
}AvH264EncConfig;

class AvH264 {

public:
	AvH264();
	int open(AvH264EncConfig h264_config);
	cv::Mat GetMat();
	AVPacket *encode(cv::Mat mat);
	void close();
private:
	FILE* fp_file;
	const char* filename;
	cv::VideoCapture cap;
	cv::Mat frame;
	AVCodec *cdc_;
	AVCodecContext *cdc_ctx_;
	AVFrame *avf_;
	AVPacket *avp_;
	int frame_size_;
	int pts_;
};

#endif