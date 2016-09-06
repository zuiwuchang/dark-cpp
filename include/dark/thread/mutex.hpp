#ifndef _DARK_CPP__THREAD_MUTEX__HPP_
#define _DARK_CPP__THREAD_MUTEX__HPP_

#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

namespace dark
{
	namespace thread
	{
		typedef boost::shared_ptr<boost::mutex> mutex_spt;
		class scoped_mutex_lock
			:boost::noncopyable
		{
		protected:
			mutex_spt _mutex;
		public:
			scoped_mutex_lock(mutex_spt mutex)
			{
				if(mutex)
				{
					_mutex = mutex;
					_mutex->lock();
				}
			}
			virtual ~scoped_mutex_lock()
			{
				if(_mutex)
				{
					_mutex->unlock();
				}
			}
		};

	};
};
#endif