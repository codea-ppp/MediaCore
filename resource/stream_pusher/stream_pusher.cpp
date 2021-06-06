#include "stream_pusher.h"
#include "stream_pusher_impl.h"

int stream_pusher::set_video(uint32_t tid, uint32_t ssrc, const std::string& video_name, void (*f)(uint32_t))
{
	return impl->set_video(tid, ssrc, video_name, f);
}

int stream_pusher::tell_me_size(uint32_t& w, uint32_t& h)
{
	return impl->tell_me_size(w, h);
}

int stream_pusher::listening(uint16_t& send_port)
{
	return impl->listening(send_port);
}

bool stream_pusher::is_expires()
{
	return impl->is_expires();
}

void stream_pusher::stop()
{
	impl->stop();
}

stream_pusher::stream_pusher()
{
	impl = std::make_shared<stream_pusher_impl>();
}

stream_pusher::~stream_pusher()
{
}
