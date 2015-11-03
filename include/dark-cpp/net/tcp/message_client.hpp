#ifndef _DARK_CPP__NET_TCP_MESSAGE_CLIENT__HPP_
#define _DARK_CPP__NET_TCP_MESSAGE_CLIENT__HPP_

#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>

#include <list>

#include "base_client.hpp"

#include "message.hpp"

namespace dark
{
	namespace net
	{
		class tcp_message_client:public tcp_base_client
		{
		public:
			tcp_message_client(const std::string server,unsigned short port,std::size_t buffer_size = 10240)
				:tcp_base_client(server,port,buffer_size + sizeof(message_header))
			{
				id_ = 0;

				handlers_ = boost::make_shared<message_handlers_t>();
			}

		private:
			std::size_t id_;
			boost::unordered_set<std::size_t> used_id_;
		public:
			message_ptr_t new_message()
			{
				message_ptr_t rs = boost::make_shared<message>(new_id());

				return rs;
			}
		protected:
			std::size_t new_id()
			{
				std::size_t id;
				while(true)
				{
					id = id_++;
					if(used_id_.find(id) == used_id_.end())
					{
						used_id_.insert(id);
						break;
					}
				}
				return id;
			}
		public:
			void write_message(message_ptr_t message)
			{
				if(!message)
				{
					return;
				}

				std::size_t total = 0;
				BOOST_FOREACH(buffer_ptr_t node,message->datas())
				{
					total += node->size();
				}


				std::size_t size = buffer_size();
				std::size_t header_size = sizeof(message_header);
				std::size_t data_size = size - header_size;



				std::size_t order = 0;
				message_header header;
				header.id = message->id();
				header.size = (total + data_size -1) / data_size ;
				
				std::size_t n = 0;
				std::size_t pos = 0;

				message_data_iterator_t iter = message->datas().begin();
				std::size_t pos_data = 0;
				std::size_t need = data_size;
				for(std::size_t i=0;i<header.size;++i)
				{
					
					if(total < data_size)
					{
						need = total;
					}
					
					buffer_ptr_t buffer = boost::make_shared<std::vector<char>>(need + header_size,0);
				
					header.order = order++;
					std::copy((char*)&header,(char*)&header + header_size,buffer->begin());
					pos = header_size;
					
					pos_data = format_message(buffer,pos,
						iter,pos_data,
						need);

					//write one
					write(buffer);
					total -= need;
				}

				//erase id
				delete_message(message);
			}
			void delete_message(message_ptr_t message)
			{
				BOOST_AUTO(id,message->id());
				delete_message(id);
			}
			void delete_message(std::size_t id)
			{
				BOOST_AUTO(find,used_id_.find(id));
				if(find != used_id_.end())
				{
					used_id_.erase(find);
				}
			}
		protected:
			std::size_t format_message(buffer_ptr_t buffer,std::size_t pos,
				message_data_iterator_t& iter,std::size_t pos_data,
				std::size_t need)
			{
				while(need > 0)
				{
					std::size_t size = (*iter)->size();
					size -= pos_data;
					if(size <= need)
					{
						std::copy((*iter)->begin() + pos_data,(*iter)->end(),buffer->begin() + pos);

						pos_data = 0;
						++iter;

						pos += size;
						need -= size;
					}
					else
					{
						std::copy((*iter)->begin() + pos_data,(*iter)->begin() + pos_data + need,buffer->begin() + pos);
						pos_data += need;

						need = 0;
					}
				}
				return pos_data;
			}
		


		private:
			message_handlers_ptr_t handlers_;
		protected:
			virtual void readed(buffer_ptr_t buffer)
			{
				push_data(buffer);
			}
			void push_data(buffer_ptr_t buffer)
			{
				//insert
				message_header* header = (message_header*)buffer->data();
				std::size_t id = header->id;

				BOOST_AUTO(find_handler,handlers_->find(id));
				message_handler_ptr_t handler;
				if(find_handler == handlers_->end())
				{
					handler = boost::make_shared<message_handler>(header->size);
					(*handlers_)[id] = handler;
				}
				else
				{
					handler = find_handler->second; 
				}

				handler->insert(header->order,buffer);

				
				//check complete
				if(*handler)
				{
					handler->begin();
					message_readed(handler);

					//erase handler
					BOOST_AUTO(find_handler,handlers_->find(id));
					if(find_handler == handlers_->end())
					{
						handlers_->erase(find_handler);
					}

					delete_message(id);
				}
			}
			virtual void message_readed(message_handler_ptr_t handler)=0;
			virtual void closed()
			{
				used_id_.clear();
				handlers_ = boost::make_shared<message_handlers_t>();
			}
		};
	};
};

#endif	//_DARK_CPP__NET_TCP_MESSAGE_CLIENT__HPP_