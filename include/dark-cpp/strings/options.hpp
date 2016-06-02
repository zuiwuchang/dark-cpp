#ifndef _DARK_CPP__STRINGS_OPTIONS__HPP_
#define _DARK_CPP__STRINGS_OPTIONS__HPP_
#include <boost/smart_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <vector>
#include <string>
namespace dark
{
	namespace strings
	{
		template<class charT>
		class options_params
		{
		public:
			std::basic_string<charT> full_name;
			std::basic_string<charT> short_name;
			std::basic_string<charT> declaration;
			std::basic_string<charT> value;

			options_params(const std::basic_string<charT>& full_name,const std::basic_string<charT>& short_name,const std::basic_string<charT>& declaration,const std::basic_string<charT>& value)
			{
				this->full_name = full_name;
				this->short_name = short_name;
				this->declaration = declaration;
				this->value = value;
			}
			options_params(const options_params& copy)
			{
				full_name = copy.full_name;
				short_name = copy.short_name;
				declaration = copy.declaration;
				value = copy.value;
			}
			options_params& operator=(const options_params& copy)
			{
				full_name = copy.full_name;
				short_name = copy.short_name;
				declaration = copy.declaration;
				value = copy.value;
				return *this;
			}
		};

		template<class charT>
		class options_result
		{
		public:
			boost::unordered_map<std::basic_string<charT>/*full name*/,std::basic_string<charT>/*value*/> _params;
			typedef typename boost::unordered_map<std::basic_string<charT>/*full name*/,std::basic_string<charT>/*value*/>::iterator iterator_t;
			//返回 是否傳入了此 參數
			bool has_param(const std::basic_string<charT>& full_name)
			{
				iterator_t find = _params.find(full_name);
				return find != _params.end();
			}
			//返回 是否傳入了此 參數 如果傳入了 返回 參數值
			bool has_param(const std::basic_string<charT>& full_name,std::basic_string<charT>& out)
			{
				iterator_t find = _params.find(full_name);
				if(find == _params.end())
				{
					return false;
				}
				out = find->second;
				return true;
			}
			template<typename Target>
			bool has_param(const std::basic_string<charT>& full_name,Target& out)
			{
				iterator_t find = _params.find(full_name);
				if(find == _params.end())
				{
					return false;
				}
				try
				{
					out = boost::lexical_cast<Target>(find->second);
				}
				catch(const boost::bad_lexical_cast&)
				{

				}
				return true;
			}

		};
		typedef options_result<char> options_result_t;
		typedef options_result<wchar_t> woptions_result_t;

		template<class charT>
		class basic_options
		{
		public:
			typedef std::basic_string<charT> string;
			typedef boost::shared_ptr<options_params<charT>> options_params_ptr_t;
		protected:
			std::vector<options_params_ptr_t> _params;
		public:
			string _dec;
			basic_options(string dec = string())
			{
				_dec = dec;
			}
			//設置 參數
			inline basic_options& operator()(const string& full_name,const string& short_name,const string& declaration,const string& value)
			{
				options_params_ptr_t ptr = boost::make_shared<options_params<charT>>(full_name,short_name,declaration,value);
				_params.push_back(ptr);
				
				return *this;
			}
			inline basic_options& operator()(options_params_ptr_t ptr)
			{
				_params.push_back(ptr);
				
				return *this;
			}

			//生成 幫助信息
			void write_style(std::basic_ostream<charT>& ostream,boost::function<void(std::basic_ostream<charT>&,options_params_ptr_t node)> call)
			{
				BOOST_FOREACH(options_params_ptr_t node,_params)
				{
					call(ostream,node);
				}
			}
			virtual void write(std::basic_ostream<charT>& ostream) = 0;
		};
		class options_t 
			:public basic_options<char>
		{
		protected:
			boost::xpressive::sregex _token;
			boost::xpressive::sregex _flag_begin;
			boost::xpressive::sregex _flag_end;

		public:
			options_t()
			{
				_token = boost::xpressive::_s;
				_flag_begin = boost::xpressive::sregex::compile("^-{1,2}[a-zA-Z]+=\".+");
				_flag_end = boost::xpressive::sregex::compile(".+[^\\\\]\"\\s*$");
			}
		public:
			virtual void write(std::basic_ostream<char>& ostream)
			{
				if(!_dec.empty())
				{
					ostream<<_dec<<"\n";
				}
				BOOST_FOREACH(options_params_ptr_t node,_params)
				{
					ostream<<"--"<<node->full_name;
					if(node->short_name.empty())
					{
						ostream<<"\n";
					}
					else
					{
						ostream<<" -"<<node->short_name<<"\n";
					}
					if(!node->value.empty())
					{
						ostream<<"  --"<<node->full_name<<"="<<node->value<<"\n";
						if(!node->short_name.empty())
						{
							ostream<<"   -"<<node->short_name<<"="<<node->value<<"\n";
						}
					}
					ostream<<"        "<<node->declaration
						<<"\n";
				}
			}
			//分析參數
			void analyze(const std::string& str,options_result<char>& out)
			{
				//設置參數 默認值
				BOOST_FOREACH(options_t::options_params_ptr_t node,_params)
				{
					if(!node->value.empty())
					{
						out._params[node->full_name] = node->value;
					}
				}

				boost::xpressive::sregex_token_iterator iterator(str.begin(),str.end(),_token,-1);
				boost::xpressive::sregex_token_iterator end;
				
				//std::vector<std::string> strs;
				std::string param;
				bool cat = false;
				std::size_t pos = 0;
				while(iterator!=end)
				{
					string s = *iterator;
					if(s.empty())
					{
						if(pos < str.size())
						{
							s += str[pos++];
						}
					}
					else
					{
						pos += s.size();
						if(pos < str.size())
						{
							s += str[pos++];
						}
					}
					
					if(cat)
					{
						param += s;

						std::size_t size = s.size();
						if(boost::xpressive::regex_match(s,_flag_end))
						{
							boost::algorithm::trim(param);
							if(!s.empty())
							{
								//strs.push_back(param);
								set_param(param,out);
							}
							//param.clear();
							cat = false;
						}
					}
					else
					{
						if(boost::xpressive::regex_match(s,_flag_begin) &&
							!boost::xpressive::regex_match(s,_flag_end))
						{
							cat = true;
							param = s;
						}
						else
						{
							boost::algorithm::trim(s);
							if(!s.empty())
							{
								//strs.push_back(s);
								set_param(s,out);
							}
						}
					}
					++iterator;
				}
			}
		protected:
			void set_param(const std::string& param,options_result<char>& out)
			{
				BOOST_FOREACH(options_t::options_params_ptr_t node,_params)
				{
					if(param == "--" + node->full_name ||
						param == "--" + node->full_name + "=" ||
						param == "-" + node->short_name ||
						param == "-" + node->short_name + "=")
					{
						out._params[node->full_name] = "";
						return;
					}

					if(boost::xpressive::regex_match(param,boost::xpressive::sregex::compile("^--" + node->full_name + "=.+")))
					{
						std::size_t size = node->full_name.size() + 3;
						if(param[size] == '\"' )
						{
							out._params[node->full_name] = param.substr(size+1,param.size() - size -2);
						}
						else
						{
							out._params[node->full_name] = param.substr(size);
						}
						return;
					}

					if(boost::xpressive::regex_match(param,boost::xpressive::sregex::compile("^-" + node->short_name + "=.+")))
					{
						std::size_t size = node->short_name.size() + 2;
						if(param[size] == '\"' )
						{
							out._params[node->full_name] = param.substr(size+1,param.size() - size -2);
						}
						else
						{
							out._params[node->full_name] = param.substr(size);
						}
						return;
					}
				}
			}
		};
		class woptions_t 
			:public basic_options<wchar_t>
		{
		protected:
			boost::xpressive::wsregex _token;
			boost::xpressive::wsregex _flag_begin;
			boost::xpressive::wsregex _flag_end;
		public:
			woptions_t()
			{
				_token = boost::xpressive::_s;
				_flag_begin = boost::xpressive::wsregex::compile(L"^-{1,2}[a-zA-Z]+=\".+");
				_flag_end = boost::xpressive::wsregex::compile(L".+[^\\\\]\"\\s*$");
			}
			virtual void write(std::basic_ostream<wchar_t>& ostream)
			{
				if(!_dec.empty())
				{
					ostream<<_dec<<L"\n";
				}
				BOOST_FOREACH(options_params_ptr_t node,_params)
				{
					ostream<<L"--"<<node->full_name;
					if(node->short_name.empty())
					{
						ostream<<L"\n";
					}
					else
					{
						ostream<<L" -"<<node->short_name<<L"\n";
					}
					if(!node->value.empty())
					{
						ostream<<L"  --"<<node->full_name<<L"="<<node->value<<L"\n";
						if(!node->short_name.empty())
						{
							ostream<<L"   -"<<node->short_name<<L"="<<node->value<<L"\n";
						}
					}
					ostream<<L"        "<<node->declaration
						<<L"\n";
				}
			}
			//分析參數
			void analyze(const std::wstring& str,options_result<wchar_t>& out)
			{
				//設置參數 默認值
				BOOST_FOREACH(woptions_t::options_params_ptr_t node,_params)
				{
					if(!node->value.empty())
					{
						out._params[node->full_name] = node->value;
					}
				}

				boost::xpressive::wsregex_token_iterator iterator(str.begin(),str.end(),_token,-1);
				boost::xpressive::wsregex_token_iterator end;
				
				//std::vector<std::wstring> strs;
				std::wstring param;
				bool cat = false;
				std::size_t pos = 0;
				while(iterator!=end)
				{
					std::wstring s = *iterator;
					if(s.empty())
					{
						if(pos < str.size())
						{
							s += str[pos++];
						}
					}
					else
					{
						pos += s.size();
						if(pos < str.size())
						{
							s += str[pos++];
						}
					}
					
					if(cat)
					{
						param += s;

						std::size_t size = s.size();
						if(boost::xpressive::regex_match(s,_flag_end))
						{
							boost::algorithm::trim(param);
							if(!s.empty())
							{
								//strs.push_back(param);
								set_param(param,out);
							}
							//param.clear();
							cat = false;
						}
					}
					else
					{
						if(boost::xpressive::regex_match(s,_flag_begin) &&
							!boost::xpressive::regex_match(s,_flag_end))
						{
							cat = true;
							param = s;
						}
						else
						{
							boost::algorithm::trim(s);
							if(!s.empty())
							{
								//strs.push_back(s);
								set_param(s,out);
							}
						}
					}
					++iterator;
				}
			}
			protected:
			void set_param(const std::wstring& param,options_result<wchar_t>& out)
			{
				BOOST_FOREACH(woptions_t::options_params_ptr_t node,_params)
				{
					if(param == L"--" + node->full_name ||
						param == L"--" + node->full_name + L"=" ||
						param == L"-" + node->short_name ||
						param == L"-" + node->short_name + L"=")
					{
						out._params[node->full_name] = L"";
						return;
					}
	
					if(boost::xpressive::regex_match(param,boost::xpressive::wsregex::compile(L"^--" + node->full_name + L"=.+")))
					{
						std::size_t size = node->full_name.size() + 3;
						if(param[size] == L'\"' )
						{
							out._params[node->full_name] = param.substr(size+1,param.size() - size -2);
						}
						else
						{
							out._params[node->full_name] = param.substr(size);
						}
						return;
					}

					
					if(boost::xpressive::regex_match(param,boost::xpressive::wsregex::compile(L"^-" + node->short_name + L"=.+")))
					{
						std::size_t size = node->short_name.size() + 2;
						if(param[size] == L'\"' )
						{
							out._params[node->full_name] = param.substr(size+1,param.size() - size -2);
						}
						else
						{
							out._params[node->full_name] = param.substr(size);
						}
						return;
					}
				}
			}
		};
		
		
	};
};
#endif //_DARK_CPP__STRINGS_OPTIONS__HPP_