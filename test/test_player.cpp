#include <zlog.h>
#include <memory>
#include <iostream>
#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <SDL2/SDL_thread.h>
#include "message_headers.h"
#include "stream_pusher_err.h"
#include "stream_pusher_impl.h"
#include "threadpool_instance.h"

extern "C" {
#include<libavutil/avutil.h>
#include<libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#undef main
int main(int argc, char* argv[]) 
{
	dzlog_init("../zlog.config", "test");

    SDL_Window*		_sdl_window		= nullptr;
    SDL_Renderer*	_sdl_render		= nullptr;
    SDL_Texture*	_sdl_texture	= nullptr;
	SDL_Rect		_sdl_rect;

    SDL_Init(SDL_INIT_EVERYTHING);

	int w = 1280, h = 720;
    _sdl_window	= SDL_CreateWindow("test player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	if (_sdl_window == nullptr)
	{
		dzlog_error("failed to create sdl window");
		return -1; 
	}

    SDL_GL_SetSwapInterval(1);

	_sdl_render	= SDL_CreateRenderer(_sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	if (_sdl_render == nullptr)
	{
		dzlog_error("failed to create sdl render");
		SDL_DestroyWindow(_sdl_window);
		return -1;
	}

	_sdl_texture = SDL_CreateTexture(_sdl_render, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (_sdl_texture == nullptr)
	{
		dzlog_error("failed to create sdl texture");
		SDL_DestroyRenderer(_sdl_render);
		SDL_DestroyWindow(_sdl_window);
		return -1;
	}

	_sdl_rect.w = w; _sdl_rect.h = h;
	_sdl_rect.x = SDL_WINDOWPOS_UNDEFINED;
	_sdl_rect.y = SDL_WINDOWPOS_UNDEFINED;

	AVFormatContext*	_av_format_context	= nullptr;
	AVCodecContext*		_av_codec_context	= nullptr;
	AVCodec*			_av_codec			= nullptr;
	AVFrame*			_av_frame			= nullptr;
	AVPacket*			_av_packet			= nullptr;
	SwsContext*			_sws_context		= nullptr;

	int _video_stream_index;

	if (avformat_open_input(&_av_format_context, "/home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/resource/videos/test_video5.mp4", 0, 0))
		return ERR_CANNOT_OPEN_VIDEO;

	if (avformat_find_stream_info(_av_format_context, 0) < 0)
		return ERR_VIDEO_FORMAT_INVAILD;

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
		return ERR_NO_STREAM_INDEX;
	}

	_av_codec_context = avcodec_alloc_context3(_av_codec);
	if (_av_codec_context == nullptr)
	{
		dzlog_error("cannot alloc avcodec_context");
		return ERR_AVCODEC_CONTEXT_ALLOC_FAILED;
	}

	avcodec_parameters_to_context(_av_codec_context, _av_format_context->streams[_video_stream_index]->codecpar);

	_av_codec = avcodec_find_decoder(_av_format_context->streams[_video_stream_index]->codecpar->codec_id);
	if (_av_codec == nullptr) 
	{
		dzlog_error("cannot find codec");
		return ERR_CANNOT_FIND_CODEC;
	}

	AVDictionary* optionsDict = nullptr;
	if (avcodec_open2(_av_codec_context, _av_codec, &optionsDict) < 0)
	{
		dzlog_error("avcodec open failed");
		return ERR_AVCODEC_OPEN_FAILED;
	}

	_av_frame = av_frame_alloc();
	if (_av_frame == nullptr)
	{
		dzlog_error("avframe alloc failed");
		return ERR_AV_FRAME_ALLOC_FAILED;
	}

	_av_packet = av_packet_alloc();
	if (_av_packet == nullptr)
	{
		dzlog_error("avpacket alloc failed");
		return ERR_AV_PACKET_ALLOC_FAILED;
	}

	_sws_context = sws_getContext(
			_av_codec_context->width, _av_codec_context->height, 
			_av_codec_context->pix_fmt, 
			_av_codec_context->width, _av_codec_context->height, 
			AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	if (_sws_context == nullptr)
	{
		dzlog_error("sws context alloc failed");
		return -1;
	}

    AVFrame*	pict	= av_frame_alloc();
    int			size	= av_image_get_buffer_size(AV_PIX_FMT_YUV420P, _av_codec_context->width, _av_codec_context->height, 32);
    uint8_t*	buffer	= (uint8_t*)av_malloc(size * sizeof(uint8_t));
	av_image_fill_arrays(pict->data, pict->linesize, buffer, AV_PIX_FMT_YUV420P, _av_codec_context->width, _av_codec_context->height, 32);
	
	uint8_t* YUV[3] = { 0, 0, 0 };
	uint32_t YUV_length[3] = { 1280, 640, 640 };

	while(av_read_frame(_av_format_context, _av_packet) >= 0)
	{
		if (_av_packet->stream_index == _video_stream_index) 
		{
			int ret = avcodec_send_packet(_av_codec_context, _av_packet);
			if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) 
			{
				std::cout << "avcodec_send_packet: " << ret << std::endl;
				break;
			}

			while (ret  >= 0) 
			{
				ret = avcodec_receive_frame(_av_codec_context, _av_frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) 
					break;

				double fps = av_q2d(_av_format_context->streams[_video_stream_index]->r_frame_rate);
				double sleep_time = 1.0/(double)fps;

				sws_scale(_sws_context, _av_frame->data, _av_frame->linesize, 0, _av_codec_context->height, pict->data, pict->linesize);

				_sdl_rect.x = _sdl_rect.y = 0;
				_sdl_rect.w = _av_codec_context->width;
				_sdl_rect.h = _av_codec_context->height;

				YUV[0] = new uint8_t[YUV_length[0] * _av_codec_context->height];			memcpy(YUV[0], pict->data[0], YUV_length[0] * _av_codec_context->height);
				YUV[1] = new uint8_t[YUV_length[1] * _av_codec_context->height / 2];		memcpy(YUV[1], pict->data[1], YUV_length[1] * _av_codec_context->height / 2);
				YUV[2] = new uint8_t[YUV_length[2] * _av_codec_context->height / 2];		memcpy(YUV[2], pict->data[2], YUV_length[2] * _av_codec_context->height / 2);

				SDL_RenderClear(_sdl_render);

				if (-1 == SDL_UpdateYUVTexture(_sdl_texture, &_sdl_rect, 
						pict->data[0], YUV_length[0], 
						pict->data[1], YUV_length[1], 
						pict->data[2], YUV_length[2]))
				{
					dzlog_error("%s", SDL_GetError());
				}


				if (SDL_RenderCopy(_sdl_render, _sdl_texture, 0, 0) < 0)
				{
					dzlog_error("sdl render copy failed: %s", SDL_GetError());
				}

				SDL_RenderPresent(_sdl_render);

				delete[] YUV[0];
				delete[] YUV[1];
				delete[] YUV[2];

				usleep(sleep_time * 1000000 - 1000);
			}
		}

		av_packet_unref(_av_packet);
	}

    SDL_DestroyTexture(_sdl_texture);
    SDL_DestroyRenderer(_sdl_render);
    SDL_DestroyWindow(_sdl_window);

    SDL_Quit();
	return 0;
}
