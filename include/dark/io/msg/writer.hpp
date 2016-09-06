#ifndef _DARK_CPP__IO_MSG_WRITER__HPP_
#define _DARK_CPP__IO_MSG_WRITER__HPP_

#include "message.hpp"
#include "../../bytes/buffer.hpp"

namespace dark
{
	namespace io
	{
		namespace msg
		{
			class writer_t
			{
			protected:
				//數據流 緩衝區
				dark::bytes::buffer_spt _buffer;
				
				//是否 啟用 線程 同步
				boost::shared_ptr<boost::mutex> _mutex;
			public:
				writer_t(int capacity = 1024,bool sync = false)
				{
					_buffer = boost::make_shared<dark::bytes::buffer_t>(capacity);
					if(sync)
					{
						_mutex = boost::make_shared<boost::mutex>();
					}
				}
				writer_t(const writer_t& copy)
				{
					_buffer = copy._buffer;
					_mutex = copy._mutex;
				}
				writer_t& operator=(const writer_t& copy)
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
				inline std::size_t write(const std::string& str)
				{
					return write(str.data(),str.size());
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
				message_t create_message(std::size_t id)
				{
					if(_mutex)
					{
						boost::mutex::scoped_lock lock(*_mutex);
						return unlock_create_message(id);
					}
					return unlock_create_message(id);
				}
			protected:
				message_t unlock_create_message(std::size_t id)
				{
					std::size_t sum = _buffer->size();
					std::size_t size = sum + sizeof(header_t);
					dark::utils::array_sapt<char> data(new char[size],size);
					message_t message(data);

					header_t* header = message.header();
					header->flag = DARK_IO_MSG_HEADER_FLAG;
					header->id = id;
					header->size = sum;

					dark::utils::slice_t<char> body = message.body();
					_buffer->read(body,body.size());

					return message;
				}
			};
		};
	};
};


#endif	//_DARK_CPP__IO_MSG_WRITER__HPP_
