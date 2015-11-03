#ifndef _DARK_CPP__NET_TCP_BASE_CLIENT__HPP_
#define _DARK_CPP__NET_TCP_BASE_CLIENT__HPP_

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "tcp.hpp"

namespace dark
{
	namespace net
	{
		class tcp_base_client
		{
		private:
			boost::asio::io_service ios_;
			boost::asio::ip::tcp::socket socket_;
			std::string server_;
			
			unsigned short port_;
			std::size_t buffer_size_;
		public:
			tcp_base_client(const std::string server,unsigned short port,std::size_t buffer_size = 10240)
				:server_(server),port_(port)
				,socket_(ios_)
				,buffer_size_(buffer_size)
			{
				boost::asio::ip::address address = boost::asio::ip::address::from_string(server);
				boost::asio::ip::tcp::endpoint endpoint(address,port);

				socket_.connect(endpoint);

				
				unsigned int count = boost::thread::hardware_concurrency() * 2  + 2;
				for(unsigned int i = 0 ; i < count ; ++i)
				{
					post_read();
				}
			}
			inline unsigned short port()const
			{
				return port_;
			}
			inline std::size_t buffer_size()const
			{
				return buffer_size_;
			}
			inline std::string server()const
			{
				return server_;
			}
			inline void run()
			{
				ios_.run();
			}
		protected:
			void post_read()
			{
				buffer_ptr_t buffer = boost::make_shared<std::vector<char>>(buffer_size(),0);
				socket_.async_read_some(boost::asio::buffer(*buffer)
					,boost::bind(&tcp_base_client::read_handler
						,this
						,boost::asio::placeholders::error
						,buffer
						,boost::asio::placeholders::bytes_transferred
						)
					);
			}
			void read_handler(const boost::system::error_code& e,buffer_ptr_t buffer,std::size_t bytes)
			{
				//close
				if(e)
				{
					if(socket_.is_open())
					{
						closed();
						socket_.close();
					}
					return;
				}
				post_read();

				buffer->resize(bytes);
				readed(buffer);
			}
			void write_handler(const boost::system::error_code& e,buffer_ptr_t buffer,std::size_t bytes)
			{
				writed(e,buffer->data(),buffer->size());
			}
			
		public:
			void write(const char* data,std::size_t bytes)
			{
				buffer_ptr_t buffer = boost::make_shared<std::vector<char>>(bytes,0);
				std::copy(data,data+bytes,buffer->begin());
				socket_.async_write_some(boost::asio::buffer(*buffer)
					,boost::bind(&tcp_base_client::write_handler
						,this
						,boost::asio::placeholders::error
						,buffer
						,boost::asio::placeholders::bytes_transferred
						)
					);
			}
		protected:
			virtual void closed()=0;
			virtual void readed(buffer_ptr_t buffer)=0;
			virtual void writed(const boost::system::error_code& e,const char* data,std::size_t bytes)
			{
			}
		};
	};
};

#endif	//_DARK_CPP__NET_TCP_BASE_CLIENT__HPP_