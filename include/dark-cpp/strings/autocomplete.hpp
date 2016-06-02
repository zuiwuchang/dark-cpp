#ifndef _DARK_CPP__STRINGS_AUTOCOMPLETE__HPP_
#define _DARK_CPP__STRINGS_AUTOCOMPLETE__HPP_

#include <boost/xpressive/xpressive.hpp>
#include <boost/foreach.hpp>
#include <boost/array.hpp>

#include <vector>
#include <string>
namespace dark
{
	namespace strings
	{
#define DARK_STRINGS_AUTOCOMPLETE_REGEX_FLAG_COUNT 15
		class autocomplete_t
		{
		protected:
			boost::array<std::string,DARK_STRINGS_AUTOCOMPLETE_REGEX_FLAG_COUNT> _filter_strs;
			boost::array<boost::xpressive::sregex,DARK_STRINGS_AUTOCOMPLETE_REGEX_FLAG_COUNT> _filter_regs;
		public:
			autocomplete_t()
			{
				_filter_strs[0] = std::string("\\\\");
				_filter_strs[1] = std::string("\\|");
				_filter_strs[2] = std::string("\\^");

				_filter_strs[3] = std::string("\\$");
				_filter_strs[4] = std::string("\\*");
				_filter_strs[5] = std::string("\\+");

				_filter_strs[6] = std::string("\\?");
				_filter_strs[7] = std::string("\\(");
				_filter_strs[8] = std::string("\\)");

				_filter_strs[9] = std::string("\\.");
				_filter_strs[10] = std::string("\\[");
				_filter_strs[11] = std::string("\\]");

				_filter_strs[12] = std::string("\\-");
				_filter_strs[13] = std::string("\\{");
				_filter_strs[14] = std::string("\\}");
				
				for(std::size_t i=0;i<_filter_strs.size();++i)
				{
					_filter_regs[i] = boost::xpressive::sregex::compile(_filter_strs[i]);
				}
			}
		
			void get_cmds(const std::string& cmd,const std::vector<std::string>& cmds,std::vector<std::string>& outs,bool icase = false)
			{
				std::string reg_str;
				replace_flags(cmd,reg_str);
				reg_str = "^" + reg_str + ".*";
				boost::xpressive::sregex reg;
				if(icase)
				{
					reg = boost::xpressive::sregex::compile(reg_str,boost::xpressive::icase);
				}
				else
				{
					reg = boost::xpressive::sregex::compile(reg_str);
				}
				BOOST_FOREACH(const std::string& str,cmds)
				{
					if(boost::xpressive::regex_match(str,reg))
					{
						outs.push_back(str);
					}
				}
			}
		protected:
			void replace_flags(const std::string& in,std::string& out)
			{
				out = in;
				for(std::size_t i=0;i<_filter_strs.size();++i)
				{
					out = boost::xpressive::regex_replace(out,_filter_regs[i],_filter_strs[i]);
				}
			}
		};
		class wautocomplete_t
		{
		protected:
			boost::array<std::wstring,DARK_STRINGS_AUTOCOMPLETE_REGEX_FLAG_COUNT> _filter_strs;
			boost::array<boost::xpressive::wsregex,DARK_STRINGS_AUTOCOMPLETE_REGEX_FLAG_COUNT> _filter_regs;
		public:
			wautocomplete_t()
			{
				_filter_strs[0] = std::wstring(L"\\\\");
				_filter_strs[1] = std::wstring(L"\\|");
				_filter_strs[2] = std::wstring(L"\\^");

				_filter_strs[3] = std::wstring(L"\\$");
				_filter_strs[4] = std::wstring(L"\\*");
				_filter_strs[5] = std::wstring(L"\\+");

				_filter_strs[6] = std::wstring(L"\\?");
				_filter_strs[7] = std::wstring(L"\\(");
				_filter_strs[8] = std::wstring(L"\\)");

				_filter_strs[9] = std::wstring(L"\\.");
				_filter_strs[10] = std::wstring(L"\\[");
				_filter_strs[11] = std::wstring(L"\\]");

				_filter_strs[12] = std::wstring(L"\\-");
				_filter_strs[13] = std::wstring(L"\\{");
				_filter_strs[14] = std::wstring(L"\\}");
				
				for(std::size_t i=0;i<_filter_strs.size();++i)
				{
					_filter_regs[i] = boost::xpressive::wsregex::compile(_filter_strs[i]);
				}
			}
		
			void get_cmds(const std::wstring& cmd,const std::vector<std::wstring>& cmds,std::vector<std::wstring>& outs,bool icase = false)
			{
				std::wstring reg_str;
				replace_flags(cmd,reg_str);
				reg_str = L"^" + reg_str + L".*";
				boost::xpressive::wsregex reg;
				if(icase)
				{
					reg = boost::xpressive::wsregex::compile(reg_str,boost::xpressive::icase);
				}
				else
				{
					reg = boost::xpressive::wsregex::compile(reg_str);
				}
				BOOST_FOREACH(const std::wstring& str,cmds)
				{
					if(boost::xpressive::regex_match(str,reg))
					{
						outs.push_back(str);
					}
				}
			}
		protected:
			void replace_flags(const std::wstring& in,std::wstring& out)
			{
				out = in;
				for(std::size_t i=0;i<_filter_strs.size();++i)
				{
					out = boost::xpressive::regex_replace(out,_filter_regs[i],_filter_strs[i]);
				}
			}
		};
	};
};

#endif