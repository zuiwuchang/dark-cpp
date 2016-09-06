#ifndef _DARK_CPP__IO_MSG_MESSAGE__HPP_
#define _DARK_CPP__IO_MSG_MESSAGE__HPP_

#include <boost/smart_ptr.hpp>
#include "../../utils/slice.hpp"
#include "../../utils/array.hpp"

namespace dark
{
	namespace io
	{
		namespace msg
		{

#define DARK_IO_MSG_HEADER_FLAG			0x0000044E
#define DARK_IO_MSG_HEADER_SIZE_OFFSET	8

			struct header_t
			{
				std::size_t flag;
				std::size_t id;
				std::size_t size;
				header_t():flag(0),id(0),size(0)
				{
				}
			};
			typedef dark::utils::slice_t<char> body_t;
			typedef dark::utils::array_sapt<char> data_t;
			class message_t
			{
			protected:
				data_t _data;
			public:
				message_t()
				{
				}
				message_t(data_t data)
				{
					_data = data;
				}
				message_t(const message_t& copy)
				{
					_data = copy._data;
				}
				message_t& operator=(const message_t& copy)
				{
					_data = copy._data;
					return *this;
				}

				inline operator bool()const
				{
					return _data;
				}

				inline header_t* header()const
				{
					if(_data)
					{
						return (header_t*)_data.get();
					}
					return NULL;
				}
				inline body_t body()const
				{
					if(_data)
					{
						return body_t(_data.get() + sizeof(header_t),header()->size);
					}
					return body_t();
				}
				inline data_t data()const
				{
					return _data;
				}
			};
		};
	};
};


#endif	//_DARK_CPP__IO_MSG_MESSAGE__HPP_
