#ifndef _DARK_CPP__NET_NET__HPP_
#define _DARK_CPP__NET_NET__HPP_

#include <boost/smart_ptr.hpp>
#include <vector>

namespace dark
{
	namespace net
	{
		typedef boost::shared_ptr<std::vector<char>> buffer_ptr_t;
	};
};

#endif	//_DARK_CPP__NET_NET__HPP_