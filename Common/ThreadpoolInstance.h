#ifndef THREAD_POOL_INSTANCE_H_
#define THREAD_POOL_INSTANCE_H_

#include <boost/threadpool.hpp>

class ThreadpoolInstance
{
public:
	static boost::threadpool::pool* GetInstance()
	{
		static boost::threadpool::pool tp(std::thread::hardware_concurrency());
		return &tp;
	}
};

#endif 
