#ifndef _DARK_CPP__NET_TCP_COMPLETE_SERVER__HPP_
#define _DARK_CPP__NET_TCP_COMPLETE_SERVER__HPP_

#include "server.hpp"
#include "complete.hpp"

namespace dark
{
	namespace net
	{
		
		class tcp_complete_server:public tcp_server
		{
		public:
			tcp_complete_server(const unsigned short port = 1102,std::size_t buffer_size = 10240)
				:tcp_server(port)
			{
				buffer_size += sizeof(complete_header);
			}

			virtual void readed(dark::net::tcp_socket_t s,const char* data,std::size_t bytes)
			{

			}

			void complete_write(tcp_socket_t s,const char* data,std::size_t bytes)
			{
			}

			virtual void complete_readed(dark::net::tcp_socket_t s,const char* data,std::size_t bytes)=0;
		};
	};
};

#endif	//_DARK_CPP__NET_TCP_COMPLETE_SERVER__HPP_