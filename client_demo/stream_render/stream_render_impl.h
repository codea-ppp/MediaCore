#ifndef STREAM_RENDER_IMPL_H_
#define STREAM_RENDER_IMPL_H_

#include <string>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "connection.h"

class stream_render_impl
{
public:
	void triggering(uint32_t tid, uint32_t ssrc, uint32_t ip, uint16_t send_port);

	stream_render_impl(const std::string& video_name, const int w, const int h);
	~stream_render_impl();

private:
	void _rendering(const connection conn);

private:
	std::string _video_name;

    SDL_Window*		_sdl_window;
    SDL_Renderer*	_sdl_render;
    SDL_Texture*	_sdl_texture;
	SDL_Rect		_sdl_rect;
};

#endif 
