
#include "AvH264.h"
#include <iostream>



AvH264::AvH264() {

	cdc_ = NULL;
	cdc_ctx_ = NULL;
	avf_ = NULL;
	avp_ = NULL;
	filename = "test.h264";
}

int AvH264::open(AvH264EncConfig h264_config) 
{
	//open camera
	cap.open(0);
	//open out file
	errno_t err;
	err = fopen_s(&fp_file, filename, "wb");
	if (err)
	{
		printf("Could not open output file\n");
	}
	pts_ = 0;
	cdc_ = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!cdc_) {

		return -1;
	}
	cdc_ctx_ = avcodec_alloc_context3(cdc_);
	if (!cdc_ctx_) {

		return -1;
	}
	cdc_ctx_->bit_rate = h264_config.bit_rate;
	cdc_ctx_->width = h264_config.width;
	cdc_ctx_->height = h264_config.height;
	cdc_ctx_->time_base = { 1, h264_config.frame_rate };
	cdc_ctx_->framerate = { h264_config.frame_rate, 1 };
	cdc_ctx_->gop_size = h264_config.gop_size;
	cdc_ctx_->max_b_frames = h264_config.max_b_frames;
	cdc_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
	cdc_ctx_->codec_id = AV_CODEC_ID_H264;
	cdc_ctx_->codec_type = AVMEDIA_TYPE_VIDEO;
	//cdc_ctx_->qmin = 10;
	//cdc_ctx_->qmax = 51;
	//cdc_ctx_->qcompress = 0.6;
	AVDictionary *dict = 0;
	av_dict_set(&dict, "preset", "slow", 0);
	av_dict_set(&dict, "tune", "zerolatency", 0);
	av_dict_set(&dict, "profile", "main", 0);
	avf_ = av_frame_alloc();
	avp_ = av_packet_alloc();
	if (!avf_ || !avp_) {

		return -1;
	}
	frame_size_ = cdc_ctx_->width * cdc_ctx_->height;
	avf_->format = cdc_ctx_->pix_fmt;
	avf_->width = cdc_ctx_->width;
	avf_->height = cdc_ctx_->height;
	// alloc memory
	int r = av_frame_get_buffer(avf_, 0);
	if (r < 0) {

		return -1;
	}
	r = av_frame_make_writable(avf_);
	if (r < 0) {

		return -1;
	}
	return avcodec_open2(cdc_ctx_, cdc_, &dict);
}

cv::Mat AvH264::GetMat()
{
	cap >> frame;
	return frame;
}

void AvH264::close() {

	if (cdc_ctx_) 
		avcodec_free_context(&cdc_ctx_);
	if (avf_) 
		av_frame_free(&avf_);
	if (avp_) 
		av_packet_free(&avp_);
}

AVPacket *AvH264::encode(cv::Mat mat) {

	if (mat.empty()) 
		return NULL;
	cv::resize(mat, mat, cv::Size(cdc_ctx_->width, cdc_ctx_->height));
	cv::Mat yuv;
	cv::cvtColor(mat, yuv, cv::COLOR_BGR2YUV_I420);
	unsigned char *pdata = yuv.data;
	// fill yuv420
	// yyy yyy yyy yyy
	// uuu
	// vvv
	avf_->data[0] = pdata;
	avf_->data[1] = pdata + frame_size_;
	avf_->data[2] = pdata + frame_size_ * 5 / 4;
	avf_->pts = pts_++;
	int r = avcodec_send_frame(cdc_ctx_, avf_);
	while (r >= 0) {

		r = avcodec_receive_packet(cdc_ctx_, avp_);
		if (r == 0) {

			//avp_->stream_index = 0;
			//return avp_;
			fwrite(avp_->data, 1, avp_->size, fp_file);
			av_packet_unref(avp_);
		}
		if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {

			return NULL;
		}
	}
	return NULL;
}

int main()
{
	AvH264 h264;
	AvH264EncConfig conf;
	conf.bit_rate = 320000;
	conf.width = 640;
	conf.height = 480;
	conf.gop_size = 250;
	conf.max_b_frames = 0;
	conf.frame_rate = 25;
	h264.open(conf);
	while(true){

		// get mat
		cv::Mat f = h264.GetMat();
		// do encode
		AVPacket *pkt = h264.encode(f);
		// do pkt
	}
	h264.close();
}


