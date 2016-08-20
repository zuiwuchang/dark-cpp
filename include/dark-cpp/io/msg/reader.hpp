#ifndef _DARK_CPP__IO_MSG_READEr__HPP_
#define _DARK_CPP__IO_MSG_READEr__HPP_

#include "message.hpp"
#include "../../bytes/buffer.hpp"


namespace dark
{
	namespace io
	{
		namespace msg
		{
			class reader_t
			{
			protected:
				//數據流 緩衝區
				dark::bytes::buffer_spt _buffer;
				
				//是否 啟用 線程 同步
				boost::shared_ptr<boost::mutex> _mutex;
			public:
				reader_t(int capacity = 1024,bool sync = false)
				{
					_buffer = boost::make_shared<dark::bytes::buffer_t>(capacity);
					if(sync)
					{
						_mutex = boost::make_shared<boost::mutex>();
					}
				}
				reader_t(const reader_t& copy)
				{
					_buffer = copy._buffer;
					_mutex = copy._mutex;
				}
				reader_t& operator=(const reader_t& copy)
				{
					_buffer = copy._buffer;
					_mutex = copy._mutex;
					return *this;
				}

				//清空緩存 刪除所有待讀數據
				inline void reset()
				{
					_buffer->reset();
				}

				//寫入 數據
				inline std::size_t write(const dark::utils::slice_t<char>& slice)
				{
					return write(slice,slice.size());
				}
				std::size_t write(const char* bytes,std::size_t n)
				{
					if(!n)
					{
						return 0;
					}
					if(_mutex)
					{
						boost::mutex::scoped_lock lock(*_mutex);
						return unlock_write(bytes,n);
					}
					return unlock_write(bytes,n);
				}
			protected:
				inline std::size_t unlock_write(const char* bytes,std::size_t n)
				{
					return _buffer->write(bytes,n);
				}

			public:
				message_t get_message()
				{
					if(_mutex)
					{
						boost::mutex::scoped_lock lock(*_mutex);
						return unlock_get_message();
					}
					return unlock_get_message();
				}
			protected:
				message_t unlock_get_message()
				{
					std::size_t buf_size = _buffer->size();
					if(buf_size < sizeof(header_t))
					{
						return message_t();
					}
					std::size_t size = 0;
					_buffer->copy_to(DARK_IO_MSG_HEADER_SIZE_OFFSET,(char*)&size,sizeof(size));
					size += sizeof(header_t);
					if(buf_size < size)
					{
						return message_t();
					}

					dark::utils::array_sapt<char> data(new char[size],size);
					_buffer->read(data,data.size());
					return message_t(data);
				}
			};
		};
	};
};


#endif	//_DARK_CPP__IO_MSG_READEr__HPP_
