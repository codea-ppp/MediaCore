#ifndef STREAM_PUSHER_IMPL_H_
#define STREAM_PUSHER_IMPL_H_

#include <string>
#include <chrono>
#include <atomic>
#include <stdint.h>
#include "connection.h"

extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

class stream_pusher_impl
{
public:
	int set_video(uint32_t tid, uint32_t ssrc, const std::string& video_name);
	int tell_me_size(uint32_t& w, uint32_t& h);
	int listening(uint16_t send_port);

	stream_pusher_impl();
	~stream_pusher_impl();

private:
	void waiting_trigger(const connection conn);
	void streaming(const connection conn);
	void clear();

private:
	std::chrono::time_point<std::chrono::steady_clock> _last_time_fresh;

	std::atomic_int		_status;
	std::string			_video_name;
	uint32_t	_h, _w;
	uint32_t	_tid;
	uint32_t	_ssrc;
	uint16_t	_port;
	double		_sleep_time;
	int			_video_stream_index;

	uint8_t*	_buffer;
	
	AVFormatContext*	_av_format_context;
	AVCodecContext*		_av_codec_context;
	AVCodec*			_av_codec;
	AVFrame*			_av_frame_raw;
	AVFrame*			_av_frame_yuv;
	AVPacket*			_av_packet;
	SwsContext*			_sws_context;
};

#endif 
