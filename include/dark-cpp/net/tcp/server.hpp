#ifndef _DARK_CPP__NET_TCP_SERVER__HPP_
#define _DARK_CPP__NET_TCP_SERVER__HPP_

#include "base_server.hpp"

namespace dark
{
	namespace net
	{
		
		class tcp_server:public tcp_base_server
		{
		public:
			tcp_server(unsigned short port = 1102,std::size_t buffer_size = 10240)
				:tcp_base_server(port,buffer_size)
			{
			}
			
			virtual void readed(tcp_socket_t s,buffer_ptr_t buffer)
			{
				readed(s,buffer->data(),buffer->size());
			}
		protected:
			virtual void readed(tcp_socket_t s,const char* data,std::size_t bytes)=0;
		};
	};
};

#endif	//_DARK_CPP__NET_TCP_SERVER__HPP_