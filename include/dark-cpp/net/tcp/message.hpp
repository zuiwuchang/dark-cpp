#ifndef _DARK_CPP__NET_TCP_MESSAGE__HPP_
#define _DARK_CPP__NET_TCP_MESSAGE__HPP_


#include <boost/unordered_map.hpp>

#include <map>

#include "server.hpp"

namespace dark
{
	namespace net
	{
		struct message_header
		{
			std::size_t id;			//flag for uniqueness

			std::size_t order;		//n package
			std::size_t size;		//size package
		};



		typedef std::list<buffer_ptr_t>::const_iterator message_data_iterator_t;
		class message
		{
		private:
			std::size_t id_;
			std::list<buffer_ptr_t> datas_;
		public:
			message(std::size_t id)
			{
				id_ = id;
			}
			inline std::size_t id()const
			{
				return id_;
			}

			inline void push_data(buffer_ptr_t data)
			{
				if(data->empty())
				{
					return;
				}

				datas_.push_back(data);
			}
			inline void push_data(const char* data,std::size_t bytes)
			{
				if(bytes == 0)
				{
					return;
				}

				buffer_ptr_t buffer = boost::make_shared<std::vector<char>>(bytes,0);
				std::copy(data,data+bytes,buffer->begin());
				push_data(buffer);
			}
			const std::list<buffer_ptr_t>& datas()const
			{
				return datas_;
			}
		
			operator bool()const
			{
				return !datas_.empty();
			}
		};
		typedef boost::shared_ptr<message> message_ptr_t;


		
		class message_handler
		{
		private:
			std::size_t size_;
			std::map<std::size_t/*order*/,buffer_ptr_t/*data*/> datas_;
			std::map<std::size_t/*order*/,buffer_ptr_t/*data*/>::const_iterator pos_;
		public:
			message_handler(std::size_t size)
			{
				size_ = size;
			}
			inline std::size_t size()const
			{
				return size_;
			}
			inline void insert(std::size_t order,buffer_ptr_t data)
			{
				datas_[order] = data;
			}
			inline operator bool()const
			{
				return size_ == datas_.size();
			}
		
			inline bool empty() const
			{
				return datas_.empty();
			}
			inline void begin()
			{
				pos_ = datas_.begin();
			}
			inline bool next()
			{
				++pos_;
				return pos_ != datas_.end();
			}
			inline bool end()
			{
				return pos_ == datas_.end();
			}
			inline std::pair<char*,std::size_t> get()
			{
				return std::make_pair(pos_->second->data() + sizeof(message_header),pos_->second->size() - sizeof(message_header));
			}
		};
		typedef boost::shared_ptr<message_handler> message_handler_ptr_t;

		typedef boost::unordered_map<std::size_t/*id*/,message_handler_ptr_t/*handler*/> message_handlers_t;
		typedef boost::shared_ptr<message_handlers_t> message_handlers_ptr_t;
	};
};

#endif	//_DARK_CPP__NET_TCP_MESSAGE__HPP_