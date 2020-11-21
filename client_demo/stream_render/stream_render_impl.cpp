#include <zlog.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "message_headers.h"
#include "stream_render_impl.h"
#include "threadpool_instance.h"

void stream_render_impl::triggering(uint32_t tid, uint32_t ssrc, uint32_t ip, uint16_t send_port)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_addr.s_addr	= ip;
	addr.sin_family			= AF_INET;
	addr.sin_port			= send_port;

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		dzlog_error("trigger connect failed, errno: %d", errno);
		return;
	}

	std::shared_ptr<media_core_message::client_stream_trigger_message> mess = std::make_shared<media_core_message::client_stream_trigger_message>();
	mess->full_data_direct(tid, ssrc);

	connection conn(sockfd, ip, send_port);
	conn.send_message(mess);

	_status = 1;

	threadpool_instance::get_instance()->schedule(std::bind(&stream_render_impl::_rendering, this, conn));
}

void stream_render_impl::stop()
{
	_status = 0;
}

stream_render_impl::stream_render_impl(const std::string& video_name, const int w, const int h)
{
	_status = 0;

    _sdl_window	= SDL_CreateWindow(video_name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	if (_sdl_window == nullptr)
	{
		dzlog_error("failed to create sdl window");
		return;
	}

    SDL_GL_SetSwapInterval(1);

    _sdl_render	= SDL_CreateRenderer(_sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	if (_sdl_render == nullptr)
	{
		dzlog_error("failed to create sdl render");
		SDL_DestroyWindow(_sdl_window);
		return;
	}

    _sdl_texture = SDL_CreateTexture(_sdl_render, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (_sdl_texture == nullptr)
	{
		dzlog_error("failed to create sdl texture");
		SDL_DestroyRenderer(_sdl_render);
		SDL_DestroyWindow(_sdl_window);
		return;
	}

	_video_name = video_name;
	_sdl_rect.w = w; _sdl_rect.h = h;
	_sdl_rect.x = 0;
	_sdl_rect.y = 0;
}

stream_render_impl::~stream_render_impl()
{
	clear();
}

void stream_render_impl::_rendering(const connection conn)
{
	if (!_status)
		return;

	SDL_PollEvent(&_sdl_event);
	switch(_sdl_event.type)
	{
		case SDL_QUIT:
			clear();
			return;

		default: break;
	}

	std::shared_ptr<media_core_message::message> mess;
	if (conn.give_message(mess))
	{
		dzlog_error("failed to get stream from %s:%d", conn.show_ip(), conn.show_port());
		return;
	}

	uint32_t type = mess->tell_me_type();
	if (type != MSGTYPE_STREAMFRAME)
	{
		dzlog_error("recv wrong message type %d", type);
		return;
	}

	dzlog_info("recving frame");

	uint32_t tid; 
	uint8_t* YUV[3]			= { 0, 0, 0 }; 
	uint32_t YUV_length[3]	= { 0, 0, 0 };

	std::shared_ptr<media_core_message::stream_message> stream_message = 
		std::dynamic_pointer_cast<media_core_message::stream_message>(mess);

	stream_message->give_me_data(tid, YUV, YUV_length);

	if (SDL_UpdateYUVTexture(_sdl_texture, &_sdl_rect, 
				YUV[0], YUV_length[0], 
				YUV[1], YUV_length[1], 
				YUV[2], YUV_length[2]))
	{
		dzlog_error("failed to update sdl texture");
	}
	else
	{
		SDL_RenderClear(_sdl_render);
		SDL_RenderCopy(_sdl_render, _sdl_texture, 0, 0);
		SDL_RenderPresent(_sdl_render);
	}

	threadpool_instance::get_instance()->schedule(std::bind(&stream_render_impl::_rendering, this, conn));
}

void stream_render_impl::clear()
{
    SDL_DestroyTexture(_sdl_texture);
    SDL_DestroyRenderer(_sdl_render);
    SDL_DestroyWindow(_sdl_window);
}
