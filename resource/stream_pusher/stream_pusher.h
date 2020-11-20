#ifndef STREAM_PUSHER_H_
#define STREAM_PUSHER_H_

#include <string>
#include <chrono>
#include <atomic>
#include <memory>
#include <stdint.h>
#include "connection.h"

class stream_pusher_impl;

class stream_pusher
{
public:
	int set_video(uint32_t tid, uint32_t ssrc, const std::string& video_name);
	int tell_me_size(uint32_t& w, uint32_t& h);
	int listening(uint16_t send_port);
	bool is_expires();
	void stop();

	stream_pusher();
	~stream_pusher();
	
private:
	std::shared_ptr<stream_pusher_impl> impl;
};

#endif 
