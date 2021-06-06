#include <zlog.h>
#include <memory>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "message_headers.h"
#include "stream_pusher_err.h"
#include "stream_pusher_impl.h"
#include "threadpool_instance.h"

int stream_pusher_impl::set_video(uint32_t tid, uint32_t ssrc, const std::string& video_name, void (*streaming_end_callback)(uint32_t))
{
	int expect = STATUS_STANDBY;
	if (!std::atomic_compare_exchange_strong(&_status, &expect, STATUS_PENDING))
		return ERR_BUSY;

	if (avformat_open_input(&_av_format_context, video_name.c_str(), 0, 0))
	{
		dzlog_error("cannot open video %s", video_name.c_str());
		clear();
		return ERR_CANNOT_OPEN_VIDEO;
	}

	if (avformat_find_stream_info(_av_format_context, 0) < 0)
	{
		dzlog_error("cannot find video message from %s", video_name.c_str());
		clear();
		return ERR_VIDEO_FORMAT_INVAILD;
	}

	for (unsigned int i = 0; i < _av_format_context->nb_streams; i++)
	{
		if (_av_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) 
		{
			_video_stream_index = i;

			dzlog_info("find stream index %d", i);
			break;
		}
	}

	if (_video_stream_index == -1)
	{
		dzlog_error("cannot find stream index");
		clear();
		return ERR_NO_STREAM_INDEX;
	}

	_av_codec_context = avcodec_alloc_context3(_av_codec);
	if (_av_codec_context == nullptr)
	{
		dzlog_error("cannot alloc avcodec_context");
		clear();
		return ERR_AVCODEC_CONTEXT_ALLOC_FAILED;
	}

	_av_codec = avcodec_find_decoder(_av_format_context->streams[_video_stream_index]->codecpar->codec_id);
	if (_av_codec == nullptr) 
	{
		dzlog_error("cannot find codec");
		clear();
		return ERR_CANNOT_FIND_CODEC;
	}

	if (avcodec_parameters_to_context(_av_codec_context, _av_format_context->streams[_video_stream_index]->codecpar))
	{
		dzlog_error("failed to set para");
		clear();
		return ERR_FALIED_TO_SET_PARA;
	}

	AVDictionary* optionsDict = nullptr;
	if (avcodec_open2(_av_codec_context, _av_codec, &optionsDict) < 0)
	{
		dzlog_error("avcodec open failed");
		clear();
		return ERR_AVCODEC_OPEN_FAILED;
	}

	_av_frame_raw = av_frame_alloc();
	if (_av_frame_raw == nullptr)
	{
		dzlog_error("avframe alloc failed");
		clear();
		return ERR_AV_FRAME_ALLOC_FAILED;
	}

	_av_frame_yuv = av_frame_alloc();
	if (_av_frame_yuv == nullptr)
	{
		dzlog_error("avframe alloc failed");
		clear();
		return ERR_AV_FRAME_ALLOC_FAILED;
	}

    int	size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, _av_codec_context->width, _av_codec_context->height, 32);
    _buffer	 = (uint8_t*)av_malloc(size * sizeof(uint8_t));
	av_image_fill_arrays(_av_frame_yuv->data, _av_frame_yuv->linesize, _buffer, AV_PIX_FMT_YUV420P, _av_codec_context->width, _av_codec_context->height, 32);

	_av_packet = av_packet_alloc();
	if (_av_packet == nullptr)
	{
		dzlog_error("avpacket alloc failed");
		clear();
		return ERR_AV_PACKET_ALLOC_FAILED;
	}

	_sws_context = sws_getContext(_av_codec_context->width, _av_codec_context->height, _av_codec_context->pix_fmt, _av_codec_context->width, _av_codec_context->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0);
	if (_sws_context == nullptr)
	{
		dzlog_error("sws context alloc failed");
		return ERR_SWS_ALLOC_FAILED;
	}

	_streaming_end_callback = streaming_end_callback;
	_sleep_time				= (1.00 / av_q2d(_av_format_context->streams[_video_stream_index]->r_frame_rate) * 1000000 - 1000);
	_video_name				= video_name;
	_ssrc					= ssrc;
	_tid					= tid;
	_w						= _av_codec_context->width;
	_h						= _av_codec_context->height;

	return 0;
}

int stream_pusher_impl::tell_me_size(uint32_t& w, uint32_t& h)
{
	w = _w;
	h = _h;

	return 0;
}

int stream_pusher_impl::listening(uint16_t& send_port)
{
	int expect = STATUS_PENDING;
	if (!std::atomic_compare_exchange_strong(&_status, &expect, STATUS_LISTENING))
		return ERR_BUSY;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		dzlog_error("socket fd == -1, errno = %d", errno);
		clear();
		return ERR_NET_FAILED;
	}

	struct sockaddr_in my_sock, temp;
	memset(&my_sock, 0, sizeof(struct sockaddr_in));

	my_sock.sin_addr.s_addr	= INADDR_ANY;
	my_sock.sin_family		= AF_INET;
	my_sock.sin_port		= 0;

	if (bind(sockfd, (struct sockaddr*)&my_sock, sizeof(my_sock)))
	{
		dzlog_error("bind socket %d failed, errno: %d", sockfd, errno);
		clear();
		return ERR_NET_FAILED;
	}

	if (listen(sockfd, 1))
	{
		dzlog_error("listening to socked %d failed, errno: %d", sockfd, errno);
		clear();
		return ERR_NET_FAILED;
	}

	socklen_t len;
	if (getsockname(sockfd, (struct sockaddr*)&temp, &len))
	{
		dzlog_error("failed to get socked %d message, errno: %d", sockfd, errno);
		clear();
		return ERR_NET_FAILED;
	}
	
	_port = send_port = temp.sin_port;
	dzlog_info("alloc port %d", send_port);

	threadpool_instance::get_instance()->schedule(std::bind(&stream_pusher_impl::listening_part2, this, sockfd));
	return 0;
}

void stream_pusher_impl::listening_part2(int sockfd)
{
	struct sockaddr_in peer_sock;
	memset(&peer_sock, 0, sizeof(struct sockaddr_in));

	_last_time_fresh = std::chrono::steady_clock::now();

	socklen_t temp;
	int peer = accept(sockfd, (struct sockaddr*)&peer_sock, &temp);
	if (peer == -1)
	{
		dzlog_error("accept to socked %d failed, errno: %d", sockfd, errno);
		clear();
		return;
	}

	connection conn(peer, peer_sock.sin_addr.s_addr, peer_sock.sin_port);

	std::thread* p = new std::thread(std::bind(&stream_pusher_impl::waiting_trigger, this, conn));
	p->detach();
	delete p;
}

void stream_pusher_impl::waiting_trigger(const connection conn)
{
	int expect = STATUS_LISTENING;
	if (!std::atomic_compare_exchange_strong(&_status, &expect, STATUS_STREAMING))
		return;

	std::shared_ptr<media_core_message::message> mess;
	if (conn.give_message(mess))
	{
		dzlog_error("waiting ssrc %d failed for %s:%d", _ssrc, conn.show_ip(), conn.show_port());
		clear();
		return;
	}

	int type = mess->tell_me_type();
	if (type != MSGTYPE_CLIENTSTREAMTRIGGER)
	{
		dzlog_error("get wrong message %d", type);
		clear();
		return;
	}

	std::shared_ptr<media_core_message::client_stream_trigger_message> trigger_message =
		std::dynamic_pointer_cast<media_core_message::client_stream_trigger_message>(mess);

	uint32_t tid; 
	uint32_t ssrc;

	trigger_message->give_me_data(tid, ssrc);
	if (ssrc != _ssrc)
	{
		dzlog_error("recv wrong ssrc %d, should be %d", ssrc, _ssrc);
		clear();
		return;
	}

	dzlog_info("start to streaming for %d", ssrc);
	std::thread* p = new std::thread(&stream_pusher_impl::streaming, this, conn);
	p->detach();
	delete p;
}

void stream_pusher_impl::streaming(const connection conn)
{
	if (_status != STATUS_STREAMING)
	{
		clear();
		return;
	}

	std::shared_ptr<media_core_message::stream_message> mess = std::make_shared<media_core_message::stream_message>();

	while(av_read_frame(_av_format_context, _av_packet) >= 0)
	{
		if (_av_packet->stream_index == _video_stream_index) 
		{
			int ret = avcodec_send_packet(_av_codec_context, _av_packet);
			if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) 
				break;

			while (ret >= 0) 
			{
				ret = avcodec_receive_frame(_av_codec_context, _av_frame_raw);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) 
					break;

				sws_scale(_sws_context, _av_frame_raw->data, _av_frame_raw->linesize, 0, _av_codec_context->height, _av_frame_yuv->data, _av_frame_yuv->linesize);

				mess->full_data_direct(_tid, _av_frame_yuv->data, (uint32_t*)_av_frame_yuv->linesize, _av_codec_context->height);
				conn.send_message(mess);
				usleep(_sleep_time);
			}
		}

		av_packet_unref(_av_packet);
	}

	if (_streaming_end_callback != nullptr)
		_streaming_end_callback(_ssrc);
}

bool stream_pusher_impl::is_expires()
{
	if (_status == STATUS_STREAMING)
		return false;

	return (std::chrono::steady_clock::now() - _last_time_fresh) > std::chrono::seconds(300);
}

void stream_pusher_impl::stop()
{
	_status = STATUS_STOPING;
	dzlog_info("stoping stream %d", _ssrc);
}

stream_pusher_impl::stream_pusher_impl()
{
	_buffer				= nullptr;
	_av_format_context	= nullptr;
	_av_codec_context	= nullptr;
	_av_codec			= nullptr;
	_av_frame_raw		= nullptr;
	_av_frame_yuv		= nullptr;
	_av_packet			= nullptr;
	_sws_context		= nullptr;

	_h = _w = -1;
	_video_stream_index = -1;
	_status	= STATUS_STANDBY;
}

stream_pusher_impl::~stream_pusher_impl()
{
	clear();
}

void stream_pusher_impl::clear()
{
	if (_sws_context != nullptr)
	{
		sws_freeContext(_sws_context);
		_sws_context = nullptr;
	}

	if (_av_packet != nullptr)
	{
		av_packet_unref(_av_packet);
		_av_packet = nullptr;
	}

	if (_av_frame_yuv != nullptr)
	{
		av_free(_av_frame_yuv);
		_av_frame_yuv = nullptr;
	}

	if (_av_frame_raw != nullptr)
	{
		av_free(_av_frame_raw);
		_av_frame_raw = nullptr;
	}

	if (_av_codec_context != nullptr)
	{
		avcodec_close(_av_codec_context);
		_av_codec_context = nullptr;
	}

	if (_av_format_context != nullptr)
	{
		avformat_close_input(&_av_format_context);
		_av_format_context = nullptr;
	}

	if (_buffer == nullptr)
	{
		av_free(_buffer);
		_buffer = nullptr;
	}

	_status = STATUS_STANDBY;
}
