#ifndef _DARK_CPP__NET_TCP_TCP__HPP_
#define _DARK_CPP__NET_TCP_TCP__HPP_

#include <boost/smart_ptr.hpp>
#include<vector>

namespace dark
{
	namespace net
	{
		typedef boost::shared_ptr<std::vector<char>> buffer_ptr_t;
		typedef boost::shared_ptr<boost::asio::ip::tcp::socket> tcp_socket_t;
	};
};

#endif	//_DARK_CPP__NET_TCP_TCP__HPP_