#ifndef _DARK_CPP__NET_TCP_BASE_SERVER__HPP_
#define _DARK_CPP__NET_TCP_BASE_SERVER__HPP_

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "tcp.hpp"

namespace dark
{
	namespace net
	{
		
		class tcp_base_server
		{
		private:
			boost::asio::io_service ios_;
			boost::asio::ip::tcp::acceptor acceptor_;
			
			unsigned short port_;
			std::size_t buffer_size_;
		public:
			tcp_base_server(unsigned short port = 1102,std::size_t buffer_size = 10240)
				:acceptor_(ios_,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
				,port_(port)
				,buffer_size_(buffer_size)
			{
				unsigned int count = boost::thread::hardware_concurrency() * 2  + 2;

				//accept
				for(unsigned int i = 0 ; i < count ; ++i)
				{
					post_accept();
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
			inline void run()
			{
				ios_.run();
			}
			inline void stop()
			{
				ios_.stop();
			}
		protected:
			void post_accept()
			{
				tcp_socket_t s = boost::make_shared<boost::asio::ip::tcp::socket>(ios_);

				acceptor_.async_accept(*s
					,boost::bind(&tcp_base_server::accept_handler
						,this
						,boost::asio::placeholders::error
						,s
						)
					);
			}
			void post_read(tcp_socket_t s)
			{
				buffer_ptr_t buffer = boost::make_shared<std::vector<char>>(buffer_size(),0);
				s->async_read_some(boost::asio::buffer(*buffer)
					,boost::bind(&tcp_base_server::read_handler
						,this
						,boost::asio::placeholders::error
						,s
						,buffer
						,boost::asio::placeholders::bytes_transferred
						)
					);
			}

			void accept_handler(const boost::system::error_code& e,tcp_socket_t s)
			{
				post_accept();

				accepted(e,s);
				
				if(e)
				{
					return;
				}

				post_read(s);
			}
			void read_handler(const boost::system::error_code& e,tcp_socket_t s,buffer_ptr_t buffer,std::size_t bytes)
			{
				//close
				if(e)
				{
					closed(s);
					return;
				}
				post_read(s);

				buffer->resize(bytes);
				readed(s,buffer);
			}
			void write_handler(const boost::system::error_code& e,tcp_socket_t s,buffer_ptr_t buffer,std::size_t bytes)
			{
				writed(e,s,buffer->data(),buffer->size());
			}
			
		public:
			void write(tcp_socket_t s,buffer_ptr_t buffer)
			{
				s->async_write_some(boost::asio::buffer(*buffer)
					,boost::bind(&tcp_base_server::write_handler
						,this
						,boost::asio::placeholders::error
						,s
						,buffer
						,boost::asio::placeholders::bytes_transferred
						)
					);
			}
			void write(tcp_socket_t s,const char* data,std::size_t bytes)
			{
				buffer_ptr_t buffer = boost::make_shared<std::vector<char>>(bytes,0);
				std::copy(data,data+bytes,buffer->begin());
				write(s,buffer);
			}
		protected:
			virtual void accepted(const boost::system::error_code& e,tcp_socket_t s)=0;
			virtual void closed(tcp_socket_t s)=0;
			virtual void readed(tcp_socket_t s,buffer_ptr_t buffer)=0;
			virtual void writed(const boost::system::error_code& e,tcp_socket_t s,const char* data,std::size_t bytes)
			{
			}
		};
	};
};

#endif	//_DARK_CPP__NET_TCP_BASE_SERVER__HPP_