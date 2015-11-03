#ifndef _DARK_CPP__NET_TCP_COMPLETE_SERVER__HPP_
#define _DARK_CPP__NET_TCP_COMPLETE_SERVER__HPP_

#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <boost/typeof/typeof.hpp>

#include "base_server.hpp"
#include "message.hpp"

namespace dark
{
	namespace net
	{
		
		class tcp_message_server:public tcp_base_server
		{
		public:
			tcp_message_server(const unsigned short port = 1102,std::size_t buffer_size = 10240)
				:tcp_base_server(port,buffer_size + sizeof(message_header))
			{
			}

		protected:
		private:			
			boost::unordered_map<tcp_socket_t,message_handlers_ptr_t> socket_handlers_;
		protected:
			virtual void readed(tcp_socket_t s,buffer_ptr_t buffer)
			{
				push_data(s,buffer);
			}
			void push_data(tcp_socket_t s,buffer_ptr_t buffer)
			{
				BOOST_AUTO(find,socket_handlers_.find(s));
				if(find == socket_handlers_.end())
				{
					//not should happen
					return;
				}

				//insert
				message_header* header = (message_header*)buffer->data();
				std::size_t id = header->id;

				message_handlers_ptr_t handlers = find->second;
				BOOST_AUTO(find_handler,handlers->find(id));
				message_handler_ptr_t handler;
				if(find_handler == handlers->end())
				{
					handler = boost::make_shared<message_handler>(header->size);
					(*handlers)[id] = handler;
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
					message_readed(s,handler);

					//erase handler
					BOOST_AUTO(find_handler,handlers->find(id));
					if(find_handler == handlers->end())
					{
						handlers->erase(find_handler);
					}

					delete_message(id);
				}
			}
		protected:
			virtual void accepted(const boost::system::error_code& e,dark::net::tcp_socket_t s)
			{
				socket_handlers_[s] = boost::make_shared<message_handlers_t>();
			}
			virtual void closed(dark::net::tcp_socket_t s)
			{
				BOOST_AUTO(find,socket_handlers_.find(s));
				if(find != socket_handlers_.end())
				{
					message_handlers_ptr_t handlers = find->second;
					typedef std::pair<std::size_t,message_handler_ptr_t> node_t;
					BOOST_FOREACH(const node_t& node,*handlers)
					{
						std::size_t id = node.first;
						free_id(id);
					}

					socket_handlers_.erase(find);
				}
			}
		protected:
			virtual void message_readed(tcp_socket_t s,message_handler_ptr_t handler)=0;
		
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
			void free_id(std::size_t id)
			{
				BOOST_AUTO(find,used_id_.find(id));
				if(find != used_id_.end())
				{
					used_id_.erase(find);
				}
			}
		public:
			void write_message(tcp_socket_t s,message_ptr_t message)
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
					write(s,buffer);

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
			inline void delete_message(std::size_t id)
			{
				free_id(id);
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
		};
	};
};

#endif	//_DARK_CPP__NET_TCP_COMPLETE_SERVER__HPP_