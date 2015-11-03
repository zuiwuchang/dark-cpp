#ifndef _DARK_CPP__NET_TCP_CLIENT__HPP_
#define _DARK_CPP__NET_TCP_CLIENT__HPP_


#include "base_client.hpp"

namespace dark
{
	namespace net
	{
		class tcp_client:public tcp_base_client
		{
		
		public:
			tcp_client(std::string server,unsigned short port,std::size_t buffer_size = 10240)
				:tcp_base_client(server,port,buffer_size)
			{
			}
			virtual void readed(buffer_ptr_t buffer)
			{
				readed(buffer->data(),buffer->size());
			}
		protected:
			virtual void readed(const char* data,std::size_t bytes)=0;
		};
	};
};

#endif	//_DARK_CPP__NET_TCP_CLIENT__HPP_