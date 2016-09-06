#ifndef _DARK_CPP__POOL_POOL_CHUNK__HPP_
#define _DARK_CPP__POOL_POOL_CHUNK__HPP_
/*
	pool_chunk 是孤 實現的 一個 內存池 
	只可以從 內存池中 申請 固定大小的 資源 chunk
	pool_chunk 不可 申請 數組

	pool_chunk 是為 快速 申請釋放 大數據塊 設計 其使用 hashset deque 為每塊 chunk 標記
	對於每塊 chunk 將 多佔用 10～20字節 內存
	故對於 小塊 資源 使用 pool_chunk 可能會 佔用 過多內存
*/

#include <boost/noncopyable.hpp>
#include <boost/unordered_set.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <deque>

#include <dark/thread/mutex.hpp>

namespace dark
{
	namespace pool
	{
		class pool_chunk_t
			: boost::noncopyable
		{
		protected:
			typedef void element_type;
			//每塊 chunk 大小
			std::size_t _chunk_size;
			//可用 chunk
			std::deque<element_type*> _frees;

			//已分配 chunk
			boost::unordered_set<element_type*> _mallocs;
	
			dark::thread::mutex_spt _mutex;
	
			virtual element_type* malloc_from_os()
			{
				return malloc(_chunk_size);
			}
			virtual void free_to_os(element_type* chunk)
			{
				free(chunk);
			}
			void * malloc_from_pool()
			{
				if(_frees.empty())
				{
					element_type * ret = malloc_from_os();
					if(ret)
					{
						_mallocs.insert(ret);
					}
					return ret;
				}

				element_type * ret =	_frees.back();
				_mallocs.insert(ret);

				_frees.pop_back();
				return ret;
			}
			void free_to_pool(element_type * chunk)
			{
				BOOST_AUTO(find,_mallocs.find(chunk));
				if(find == _mallocs.end())
				{
					return;
				}
				_mallocs.erase(find);

				_frees.push_back(chunk);
			}
		public:
			pool_chunk_t(std::size_t chunk_size,dark::thread::mutex_spt mutex = dark::thread::mutex_spt())
				:_chunk_size(chunk_size),_mutex(mutex)
			{
			}
			virtual ~pool_chunk_t()
			{
				purge_memory();
			}
			//返回 內存池中 空閒內存
			inline std::size_t get_frees()
			{
				dark::thread::scoped_mutex_lock lock(_mutex);
				return _frees.size();
			} 
			//返回 內存池中 使用的 內存
			inline std::size_t get_mallocs()
			{
				dark::thread::scoped_mutex_lock lock(_mutex);
				return _mallocs.size();
			}
			//從內存池中 申請 chunk
			inline void * malloc_chunk()
			{
				dark::thread::scoped_mutex_lock lock(_mutex);

				return malloc_from_pool();
			}
			//歸還 chunk 到內存池
			inline void free_chunk(element_type * chunk)
			{
				dark::thread::scoped_mutex_lock lock(_mutex);

				free_to_pool(chunk);
			}
			
			//將 未使用 內存 還給 os
			void release_memory()
			{
				dark::thread::scoped_mutex_lock lock(_mutex);

				BOOST_FOREACH(element_type * chunk,_frees)
				{
					free_to_os(chunk);
				}
				_frees.clear();
			}
			//同上 但 保存 n個 chunk 在池中
			void release_memory(std::size_t n)
			{
				dark::thread::scoped_mutex_lock lock(_mutex);

				if(n >= _frees.size())
				{
					return;
				}
				for(std::size_t i=n;i<_frees.size();++i)
				{
					free_to_os(_frees[i]);
				}
				_frees.erase(_frees.begin()+n,_frees.end());
			}
			
			//將所有 內存 還給 os 
			//析構時  被 自動調用
			void purge_memory()
			{
				dark::thread::scoped_mutex_lock lock(_mutex);

				BOOST_FOREACH(element_type * chunk,_frees)
				{
					free_to_os(chunk);
				}
				_frees.clear();

				BOOST_FOREACH(element_type * chunk,_mallocs)
				{
					free_to_os(chunk);
				}
				_mallocs.clear();
			}

			//返回 指針 是否 來自內存池
			inline bool is_from(element_type *chunk)
			{
				dark::thread::scoped_mutex_lock lock(_mutex);

				return _mallocs.find(chunk) != _mallocs.end();
			}
		};
	};
};

#endif	//_DARK_CPP__POOL_POOL_CHUNK__HPP_

