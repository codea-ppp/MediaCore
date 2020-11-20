#include "stream_render.h"
#include "stream_render_impl.h"

void stream_render::triggering(uint32_t tid, uint32_t ssrc, uint32_t ip, uint16_t send_port)
{
	impl->triggering(tid, ssrc, ip, send_port);
}

void stream_render::stop()
{
	impl->stop();
}

stream_render::stream_render(const std::string& video_name, const int w, const int h)
{
	impl = std::make_shared<stream_render_impl>(video_name, w, h);
}

stream_render::~stream_render()
{
}
