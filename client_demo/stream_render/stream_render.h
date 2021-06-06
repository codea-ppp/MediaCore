#ifndef STREAM_RENDER_H_
#define STREAM_RENDER_H_

#include <memory>
#include <stdint.h>

class stream_render_impl;

class stream_render
{
public:
	void triggering(uint32_t tid, uint32_t ssrc, uint32_t ip, uint16_t send_port);
	void stop();

	stream_render(const std::string& video_name, const int w, const int h);
	~stream_render();

private:
	std::shared_ptr<stream_render_impl> impl;
};

#endif 

