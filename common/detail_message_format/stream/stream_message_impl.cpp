#include "stream_message_impl.h"

namespace media_core_message
{
	int stream_message_impl::send_data_to(int sockfd)
	{
		if (sockfd < 0)
		{
			dzlog_error("sockfd < 0");
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		if (_y_length == 0 && _u_length == 0 && _v_length == 0)
		{
			clear();

			dzlog_error("send a frame all 0");
			return ERR_FRAME_ALL_0;
		}

		uint32_t head_buffer[7];
		head_buffer[0] = MSGTYPE_STREAMFRAME;
		head_buffer[1] = 16 + _y_length + _u_length + _v_length;
		head_buffer[2] = _tid;
		head_buffer[3] = _height;
		head_buffer[4] = _y_length;
		head_buffer[5] = _u_length;
		head_buffer[6] = _v_length;

		int once = 0;
		
		for (unsigned int i = 0; i < 28; i += once)
		{
			once = send(sockfd, (uint8_t*)head_buffer + i, 28 - i, 0);
			if (once == -1)
			{
				clear();

				dzlog_error("send failed: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		once = 0;

		uint64_t _y_data_length = _y_length * _height;
		for (unsigned int i = 0; i < _y_data_length; i += once)
		{
			once = send(sockfd, _y + i, _y_data_length - i, 0);
			if (once == -1)
			{
				clear();

				dzlog_error("send failed: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		once = 0;

		uint64_t _u_data_length = _u_length * _height / 2;
		for (unsigned int i = 0; i < _u_data_length; i += once)
		{
			once = send(sockfd, _u + i, _u_data_length - i, 0);
			if (once == -1)
			{
				clear();

				dzlog_error("send failed: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		once = 0;
		
		uint64_t _v_data_length = _v_length * _height / 2;
		for (unsigned int i = 0; i < _v_data_length; i += once)
		{
			once = send(sockfd, _v + i, _v_data_length - i, 0);
			if (once == -1)
			{
				clear();

				dzlog_error("send failed: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		return 0;
	}

	int stream_message_impl::full_data_remote(int sockfd, uint32_t tid)
	{
		if (sockfd < 0)
		{
			dzlog_error("socket < 0");
			return ERR_SOCKET_FD_NEGATIVE;
		}

		clear();

		uint32_t YUV_length_buffer[4] = { 0 };

		unsigned int once = recv(sockfd, (uint8_t*)YUV_length_buffer, 16, MSG_WAITALL);
		if (once != 16)
		{
			dzlog_error("recv failed: %d", errno);
			return ERR_NEED_2_CLOSE_SOCKET;
		}

		_height		= YUV_length_buffer[0];
		_y_length	= YUV_length_buffer[1];
		_u_length	= YUV_length_buffer[2];
		_v_length	= YUV_length_buffer[3];

		once = 0;

		// 视频的分量可以为 0 
		if (_y_length != 0)
		{
			uint64_t _y_data_length = _y_length * _height;
			_y = new uint8_t[_y_data_length];
			once = recv(sockfd, _y, _y_data_length, MSG_WAITALL);
			if (once != _y_data_length)
			{
				clear();

				dzlog_error("recv failed: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		if (_u_length != 0)
		{
			uint64_t _u_data_length = _u_length * _height / 2;
			_u = new uint8_t[_u_data_length];
			once = recv(sockfd, _u, _u_data_length, MSG_WAITALL);
			if (once != _u_data_length)
			{
				clear();

				dzlog_error("recv failed: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		if (_v_length != 0)
		{
			uint64_t _v_data_length = _v_length * _height / 2;
			_v = new uint8_t[_v_data_length];
			once = recv(sockfd, _v, _v_data_length, MSG_WAITALL);
			if (once != _v_data_length)
			{
				clear();

				dzlog_error("recv failed: %d", errno);
				return ERR_NEED_2_CLOSE_SOCKET;
			}
		}

		_tid = tid;

//		dzlog_info("get YUV(%d, %d, %d)", _y_length, _u_length, _v_length);
		return 0;
	}

	int stream_message_impl::full_data_direct(uint32_t tid, uint8_t* YUV[3], uint32_t YUV_length[3], uint32_t height)
	{
		clear();

		_y_length = YUV_length[0];
		_u_length = YUV_length[1];
		_v_length = YUV_length[2];

		_height = height;
		_tid	= tid;

//		dzlog_info("set YUV(%d, %d, %d)", _y_length, _u_length, _v_length);

		if (YUV[0] != nullptr)
		{
			if (!_y_length)
			{
				clear();

				dzlog_error("_y_length == 0");
				return ERR_LENGTH_EQU_0_BUT_PTR_NOT;
			}

			uint64_t _y_data_length = _y_length * height;
			_y = new uint8_t[_y_data_length];
			memcpy(_y, YUV[0], _y_data_length);
		}

		if (YUV[1] != nullptr)
		{
			if (!_u_length)
			{
				clear();

				dzlog_error("_u_length == 0");
				return ERR_LENGTH_EQU_0_BUT_PTR_NOT;
			}

			uint64_t _u_data_length = _u_length * height / 2;
			_u = new uint8_t[_u_data_length];
			memcpy(_u, YUV[1], _u_data_length);
		}

		if (YUV[2] != nullptr)
		{
			if (!_v_length)
			{
				clear();
				
				dzlog_error("_v_length == 0");
				return ERR_LENGTH_EQU_0_BUT_PTR_NOT;
			}

			uint64_t _v_data_length = _v_length * height / 2;
			_v = new uint8_t[_v_data_length];
			memcpy(_v, YUV[2], _v_data_length);
		}

		return 0;
	}

	int stream_message_impl::give_me_data(uint32_t& tid, uint8_t* must_be_nullptr[3], uint32_t YUV_length[3])
	{
		if (must_be_nullptr[0] != nullptr)
		{
			dzlog_error("must_be_nullptr[0] != nullptr");
			return ERR_PTR_IS_NOT_NULLPTR;
		}

		if (must_be_nullptr[1] != nullptr)
		{
			dzlog_error("must_be_nullptr[1] != nullptr");
			return ERR_PTR_IS_NOT_NULLPTR;
		}

		if (must_be_nullptr[2] != nullptr)
		{
			dzlog_error("must_be_nullptr[2] != nullptr");
			return ERR_PTR_IS_NOT_NULLPTR;
		}

		YUV_length[0] = _y_length;
		YUV_length[1] = _u_length;
		YUV_length[2] = _v_length;

		tid = _tid;

		must_be_nullptr[0] = _y;
		must_be_nullptr[1] = _u;
		must_be_nullptr[2] = _v;

		return 0;
	}

	void stream_message_impl::print_data()
	{
		dzlog_info("tid: %d, (y, u, v) = (%d, %d, %d)", _tid, _y_length, _u_length, _v_length);
		dzlog_info("%s", (char*)_y);
		dzlog_info("%s", (char*)_u);
		dzlog_info("%s", (char*)_v);
	}

	void stream_message_impl::init()
	{
		_y = _u = _v = nullptr;
		_y_length = _u_length = _v_length = 0;
		_tid = 0;
	}

	void stream_message_impl::clear()
	{
		if (_y != nullptr) delete[] _y;
		if (_u != nullptr) delete[] _u;
		if (_v != nullptr) delete[] _v;

		_y = _u = _v = nullptr;
		_y_length = _u_length = _v_length = 0;
		_tid = 0;
	}

	stream_message_impl::stream_message_impl()
	{
		init();
	}

	stream_message_impl::~stream_message_impl()
	{
		clear();
	}
}
