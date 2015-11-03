#ifndef _DARK_CPP__NET_TCP_COMPLETE__HPP_
#define _DARK_CPP__NET_TCP_COMPLETE__HPP_


#include "server.hpp"

namespace dark
{
	namespace net
	{
		struct complete_header
		{
			std::size_t id;		//flag for uniqueness

			std::size_t order;	//n package
		};
	};
};

#endif	//_DARK_CPP__NET_TCP_COMPLETE__HPP_