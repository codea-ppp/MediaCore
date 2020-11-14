#ifndef THREAD_POOL_INSTANCE_H_
#define THREAD_POOL_INSTANCE_H_

#include <thread>
#include <boost/threadpool.hpp>

class threadpool_instance
{
public:
	static boost::threadpool::pool* get_instance()
	{
		static boost::threadpool::pool tp(std::thread::hardware_concurrency());
		return &tp;
	}
};

#endif 
    
