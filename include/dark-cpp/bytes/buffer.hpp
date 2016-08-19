#ifndef _DARK_CPP__BYTES_BUFFER__HPP_
#define _DARK_CPP__BYTES_BUFFER__HPP_

#include <boost/smart_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/foreach.hpp>

#include <list>

namespace dark
{
	namespace bytes
	{
		/*
		class buffer_exception
			: public std::exception
		{
		public:
			buffer_exception()
			{
			}
			buffer_exception(const char* const& str)
				:exception(str)
			{
			}
			buffer_exception(const char* const& str,int n)
				:exception(str,n)
			{
			}
		};
		*/

		//一個類似 golang bytes.Buffer 的 io 緩衝區
		class buffer_t
			: boost::noncopyable
		{
		protected:
			class fragmentation_t
			{
			protected:
				//分片數據
				boost::shared_array<char> _data;
				//分片容量
				std::size_t _capacity;
				//有效數據 偏移
				std::size_t _offset;
				//有效數據 大小
				std::size_t _size;
			public:
				std::size_t size() const
				{
					return _size;
				}
				fragmentation_t(int capacity)
				{
					_capacity = capacity;
					_data = boost::shared_array<char>(new char[capacity]);

					_offset = _size = 0;
				}
				//返回 空閒 容量
				std::size_t get_free()
				{
					return _capacity - _offset - _size;
				}
				//寫入數據 返回實際寫入量
				std::size_t write(const char* bytes,std::size_t n)
				{
					std::size_t free = get_free();
					std::size_t need = n;
					if(need > free)
					{
						need = free;
					}
					memcpy(_data.get() + _offset + _size,bytes,need);
					_size += need;

					return need;
				}
				//讀取 數據 返回實際讀取 量
				//被讀取的 數據 將被 移除 緩衝區
				std::size_t read(char* bytes,std::size_t n)
				{
					std::size_t need = n;
					if(need > _size)
					{
						need = _size;
					}

					memcpy(bytes,_data.get() + _offset,need);
					_size -= need;
					_offset += need;

					return need;
				}

				//只 copy 數據 不 刪除緩衝區
				std::size_t copy_to(char* bytes,std::size_t n)
				{
					std::size_t need = n;
					if (n > _size)
					{
						need = _size;
					}
					memcpy(bytes,_data.get() + _offset,need);

					return need;
				}
				//跳過n字節 copy
				std::size_t copy_to(std::size_t skip,char* bytes,std::size_t n)
				{
					if(skip >= _size)
					{
						return 0;
					}
					std::size_t offset = _offset + skip;
					std::size_t size = _size - skip;

					std::size_t need = n;
					if (need > size)
					{
						need = size;
					}
					memcpy(bytes,_data.get() + offset,need);

					return need;
				}
			};
			typedef boost::shared_ptr<fragmentation_t> fragmentation_spt;

			//分片 大小
			int _capacity;
			//是否 啟用 線程 同步
			boost::shared_ptr<boost::mutex> _mutex;
			//分片 緩存
			std::list<fragmentation_spt> _fragmentations;

		public:
			buffer_t(int capacity = 1024/*分塊長度*/,bool sync = false)
			{
				_capacity = capacity;
				if(sync)
				{
					_mutex = boost::make_shared<boost::mutex>();
				}
			}

			//清空緩存 刪除所有待讀數據
			inline void reset()
			{
				_fragmentations.clear();
			}
			//返回 流中 待讀字節數
			std::size_t size()
			{
				if(_mutex)
				{
					boost::mutex::scoped_lock lock(*_mutex);
					return unlock_size();
				}
				return unlock_size();
			}
		protected:
			std::size_t unlock_size()
			{
				std::size_t sum = 0;
				BOOST_FOREACH(fragmentation_spt fragmentation,_fragmentations)
				{
					sum += fragmentation->size();
				}
				return sum;
			}

		public:
			//將緩衝區 copy 到指定內存 返回實際 copy數據長
			//被copy的數據 不會從 緩衝區中 刪除
			//如果 n > 緩衝區 數據 將 只copy 緩衝區 否則 copy n 字節數據
			std::size_t copy_to(char* bytes,std::size_t n)
			{
				if(_mutex)
				{
					boost::mutex::scoped_lock lock(*_mutex);
					return unlock_copy_to(bytes,n);
				}
				return unlock_copy_to(bytes,n);
			}
			std::size_t copy_to(std::size_t skip,char* bytes,std::size_t n)
			{
				if(_mutex)
				{
					boost::mutex::scoped_lock lock(*_mutex);
					return unlock_copy_to(skip,bytes,n);
				}
				return unlock_copy_to(skip,bytes,n);
			}
		protected:
			std::size_t unlock_copy_to(char* bytes,std::size_t n)
			{
				std::size_t sum = 0;
				BOOST_FOREACH(fragmentation_spt fragmentation,_fragmentations)
				{
					std::size_t count = fragmentation->copy_to(bytes,n);
					n -= count;
					bytes += count;
					sum += count;

					if(n < 1)
					{
						break;
					}
				}
				return sum;
			}
			std::size_t unlock_copy_to(std::size_t skip,char* bytes,std::size_t n)
			{
				std::size_t sum = 0;
				BOOST_FOREACH(fragmentation_spt fragmentation,_fragmentations)
				{
					std::size_t need_skip = skip;
					if(need_skip)
					{
						std::size_t size = fragmentation->size();
						if(need_skip > size)
						{
							need_skip = size;
						}
						skip -= need_skip;
					}

					std::size_t count = fragmentation->copy_to(need_skip,bytes,n);
					n -= count;
					bytes += count;
					sum += count;

					if(n < 1)
					{
						break;
					}
				}
				return sum;
			}

		public:
			std::size_t write(const char* bytes,std::size_t n)
			{
				if(_mutex)
				{
					boost::mutex::scoped_lock lock(*_mutex);
					return unlock_write(bytes,n);
				}
				return unlock_write(bytes,n);
			}
		protected:
			std::size_t unlock_write(const char* bytes,std::size_t n)
			{
				std::size_t sum = 0;
				while (n)
				{
					fragmentation_spt fragmentation = get_write_fragmentation();
					int count = fragmentation->write(bytes,n);
					n -= count;
					bytes += count;
					sum += count;
				}

				return sum;
			}
			//返回 可寫的 分片
			fragmentation_spt get_write_fragmentation()
			{
				if (!_fragmentations.empty())
				{
					fragmentation_spt fragmentation = _fragmentations.back();
					if(fragmentation->get_free())
					{
						return fragmentation;
					}
				}
				fragmentation_spt fragmentation = boost::make_shared<fragmentation_t>(_capacity);
				_fragmentations.push_back(fragmentation);
				return fragmentation;
			}
		public:
			std::size_t read(char* bytes,std::size_t n)
			{
				if(_mutex)
				{
					boost::mutex::scoped_lock lock(*_mutex);
					return unlock_read(bytes,n);
				}
				return unlock_read(bytes,n);
			}
		protected:
			std::size_t unlock_read(char* bytes,std::size_t n)
			{
				std::size_t sum = 0;

				while(n)
				{
					fragmentation_spt fragmentation = get_read_fragmentation();
					std::size_t count = fragmentation->read(bytes,n);
					n -= count;
					bytes += count;
					sum += count;
				}
				remove_no_read_fragmentation();

				return sum;
			}
			//返回 可讀的 分片
			fragmentation_spt get_read_fragmentation()
			{
				while(!_fragmentations.empty())
				{
					fragmentation_spt fragmentation = _fragmentations.front();
					if (fragmentation->size() < 1)
					{
						_fragmentations.pop_front();
						continue;
					}
					return fragmentation;
				}

				return fragmentation_spt();
			}
			//刪除 無數據可讀的 分片
			void remove_no_read_fragmentation()
			{
				while(!_fragmentations.empty())
				{
					fragmentation_spt fragmentation = _fragmentations.front();
					if(fragmentation->size())
					{
						break;
					}
					_fragmentations.pop_front();
				}
			}
		};
		typedef boost::shared_ptr<buffer_t> buffer_spt;
	};
};
#endif	//_DARK_CPP__BYTES_BUFFER__HPP_