//lua c api 的封裝
#ifndef _DARK_CPP__LUA_CONTEXT__HPP_
#define _DARK_CPP__LUA_CONTEXT__HPP_

#include <lua/lua.hpp>

#include <string>

#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace dark
{
	namespace lua
	{
		typedef boost::shared_ptr<lua_State> l_spt;

		//lua 的運行 環境
		class context_t
		{
		protected:
			l_spt _l;
			static void D(lua_State* l)
			{

			}
		public:
			//構造 copy 返回 指針
			context_t(bool openlibs = true)
			{
				lua_State* l = luaL_newstate();
				if(l)
				{
					_l = l_spt(l,lua_close);
					if(openlibs)
					{
						luaL_openlibs(l);
					}
				}
			}
			context_t(lua_State* l,bool close = true/*是否在析構時 釋放lua環境*/)
			{
				if(close)
				{
					_l = l_spt(l,lua_close);
				}
				else
				{
					_l = l_spt(l,D);
				}
			}
			context_t(l_spt l)
			{
				_l = l;
			}
			context_t(const context_t& copy)
			{
				_l = copy._l;
			}
			inline context_t& operator=(const context_t& copy)
			{
				_l = copy._l;
				return *this;
			}
			inline operator bool()const
			{
				return _l;
			}
			inline operator l_spt()const
			{
				return _l;
			}
			inline operator lua_State*()const
			{
				return _l.get();
			}
			inline l_spt get_spt() const
			{
				return _l;
			}
			inline lua_State* get()const
			{
				return _l.get();
			}

			inline void reset()
			{
				_l.reset();
			}
		public:
			/******	擴展 api	******/
			//將 當前 棧 打印到標準輸出
			void dump_stdout()
			{
				int n = lua_gettop(_l.get());
				printf("ctx: top=%d, stack=[",n);
				for(int i=0;i<n;++i)
				{
					if(i)
					{
						printf(",");
					}
					dump_stdout(i+1);
				}
				puts("]");
			}
		protected:
			void dump_stdout(int n)
			{
				lua_State* l = _l.get();

				switch(lua_type(l,n))
				{
				case LUA_TNONE :
					std::cout<<"none";
					break;
				case LUA_TNIL:
					std::cout<<"nil";
					break;
				case LUA_TBOOLEAN:
					if(lua_toboolean(l,n))
					{
						std::cout<<"true";
					}
					else
					{
						std::cout<<"false";
					}
					break;
				case LUA_TLIGHTUSERDATA:
					std::cout<<"LUA_TLIGHTUSERDATA";
					break;

				case LUA_TNUMBER:
					lua_pushvalue(l,n);
					std::cout<<lua_tostring(l,-1);
					lua_pop(l,1);
					break;
				case LUA_TSTRING:
					std::cout<<"\""<<lua_tostring(l,n)<<"\"";
					break;
				case LUA_TTABLE:
					{
						std::cout<<"{";
						lua_pushnil(l);
						bool first = true;
						while(lua_next(l,n))
						{
							//將 名稱 複製一份 以免將 index 轉換為 string
							lua_pushvalue(l, -2);
							if(first)
							{
								first = false;
							}
							else
							{
								std::cout<<",";
							}
							bool is_str = lua_type(l,-1) == LUA_TSTRING;
							if(is_str)
							{
								std::cout<<"\"";
							}
							std::cout<<lua_tostring(l,-1);
							if(is_str)
							{
								std::cout<<"\"";
							}
							std::cout<<":";

							dump_stdout(-2);
						
							lua_pop(l, 2);
						}
						std::cout<<"}";
					}
					break;
				case LUA_TFUNCTION:
					std::cout<<"LUA_TFUNCTION";
					break;
				case LUA_TUSERDATA:
					std::cout<<"LUA_TUSERDATA";
					break;
				case LUA_TTHREAD:
					std::cout<<"LUA_TTHREAD";
					break;
				case LUA_NUMTAGS:
					std::cout<<"LUA_NUMTAGS";
					break;
				default:
					std::cout<<"unknow";
					break;
				}
			}


		public:
			/******	lua c api 綁定	******/
	
			/***	棧 操作	***/
			//出棧
			inline void pop(int n=1)
			{
				lua_pop(_l.get(),n);
			}

			//入棧
			inline void push_boolean(int b)
			{
				lua_pushboolean(_l.get(),b);
			}
			inline void push_boolean(bool b)
			{
				if(b)
				{
					lua_pushboolean(_l.get(),1);
				}
				else
				{
					lua_pushboolean(_l.get(),0);
				}
				
			}
			inline void push(lua_CFunction fn, int n)
			{
				lua_pushcclosure(_l.get(),fn,n);
			}
			inline void push(lua_CFunction fn)
			{ 
				lua_pushcfunction(_l.get(),fn);
			}
			inline void push_globaltable()
			{
				lua_pushglobaltable(_l.get());
			}
			inline void push_integer(lua_Integer n)
			{ 
				lua_pushinteger(_l.get(),n);
			}
			inline void push_lightuserdata (void *p)
			{
				lua_pushlightuserdata(_l.get(),p);
			}
			inline void push_nil()
			{
				lua_pushnil(_l.get());
			}
			inline void push_number(lua_Number n)
			{
				lua_pushnumber(_l.get(),n);
			}
			inline void push(const char *s)
			{
				lua_pushstring(_l.get(),s);
			}
			inline void push(const char *s,std::size_t len)
			{
				lua_pushlstring(_l.get(),s,len);
			}
			inline void push(const std::string& s)
			{
				lua_pushlstring(_l.get(),s.data(),s.size());
			}
			inline void push_thread()
			{
				lua_pushthread(_l.get());
				
			}
			//複製 一份數據副本 並入棧
			inline void push_value(std::size_t idx)
			{
				lua_pushvalue(_l.get(),idx);
			}
			
			//獲取 全局變量/nil 到棧頂 返回 其lua型別
			inline int get_global(const char* name)
			{
				return lua_getglobal(_l.get(),name);
			}
			inline int get_global(const std::string& name)
			{
				return lua_getglobal(_l.get(),name.c_str());
			}
			//將 棧頂 設置為 全局變量 並出棧
			inline void set_global(const char* name)
			{
				lua_setglobal(_l.get(),name);
			}
			inline void set_global(const std::string& name)
			{
				lua_getglobal(_l.get(),name.c_str());
			}
			
			//check
			inline bool check_stack(int sz)
			{
				return lua_checkstack(_l.get(),sz) != 0;
			}
			inline void l_check_stack(int sz, const char *msg=NULL)
			{
				luaL_checkstack(_l.get(),sz,msg);
			}

			/******	返回數據	******/
			inline bool is_function(int idx)const
			{
				return lua_isfunction(_l.get(),idx);
			}
			inline bool is_table(int idx)const
			{
				return lua_istable(_l.get(),idx);
			}
			inline bool is_lightuserdata(int idx)const
			{
				return lua_islightuserdata(_l.get(),idx);
			}
			inline bool is_nil(int idx)const
			{
				return lua_isnil(_l.get(),idx);
			}
			inline bool is_boolean(int idx)const
			{
				return lua_isboolean(_l.get(),idx);
			}
			inline bool is_thread(int idx)const
			{
				return lua_isthread(_l.get(),idx);
			}
			inline bool is_none(int idx)const
			{
				return lua_isnone(_l.get(),idx);
			}

			inline bool to_boolean(int idx)
			{
				return lua_toboolean(_l.get(),idx) != 0;
			}
			inline lua_Integer to_integer(int idx)
			{
				return lua_tointeger(_l.get(),idx);
			}
			inline lua_Number to_tonumber(int idx)
			{
				return lua_tonumber(_l.get(),idx);
			}
			inline std::string to_string(int idx,size_t *len = NULL)
			{
				return lua_tolstring(_l.get(),idx,len);
			}
			inline const char* to_c_string(int idx,size_t *len = NULL)
			{
				return lua_tolstring(_l.get(),idx,len);
			}
			inline void* to_userdata(int idx)
			{
				return lua_touserdata(_l.get(),idx);
			}
			inline lua_State* to_thread(int idx)
			{
				return lua_tothread(_l.get(),idx);
			}
			inline lua_CFunction to_cfunction(int idx)
			{
				return lua_tocfunction(_l.get(),idx);
			}

			/***	table 操作	***/

			//創建 一個 table 並入棧
			//narr 預計 將使用的 數組大小 的參考值
			//nrec 預計 將使用的 hash表大小 的參考值
			inline void createtable(int narr=0, int nrec=0)
			{
				lua_createtable(_l.get(),narr,nrec);
			}

			//將 棧頂 設置到 table 的 key 並出棧
			inline void put_prop_string(int idx/*table 位置*/,const char* key)
			{
				lua_setfield(_l.get(),idx,key);
			}
			inline void set_prop_string(int idx/*table 位置*/,const std::string& key)
			{
				lua_setfield(_l.get(),idx,key.c_str());
			}
			//將 棧頂 設置到 table 的 指定位置 並出棧
			inline void set_prop_index(int idx/*table 位置*/,int arr_idx)
			{
				lua_seti(_l.get(),idx,arr_idx);
			}
			//在table中 查找key 返回到 棧頂 不存在返回 nil
			inline void get_prop_string(int idx/*table 位置*/,const std::string& key)
			{
				lua_getfield(_l.get(),idx,key.c_str());
			}
			//在table中 查找arr_idx 返回到 棧頂 不存在返回 nil
			inline void get_prop_index(int idx/*table 位置*/,int arr_idx)
			{
				lua_geti(_l.get(),idx,arr_idx);
			}



			/***	熱更新 操作	***/
			//加載 字符串 作為lua腳本 並作為匿名函數 返回
			inline bool l_loadstring(const char* s)
			{
				return luaL_loadstring(_l.get(),s) == LUA_OK;
			}
			inline bool l_loadstring(const std::string& s)
			{
				return luaL_loadstring(_l.get(),s.c_str()) == LUA_OK;
			}
			//加載 文件 作為lua腳本 並作為匿名函數 返回
			inline int l_loadfile(const char* f)
			{
				return luaL_loadfile(_l.get(),f);
			}
			inline int l_loadfile(const std::string& f)
			{
				return luaL_loadfile(_l.get(),f.c_str());
			}

			/***	調用 lua	***/
			/***	先 入棧函數 之後 參數 從左到右入棧 	***/
			inline void call(int nargs	/*參數數量*/ , int nresults/*返回值數量*/)
			{
				lua_call(_l.get(),nargs,nresults);
			}
			inline bool pcall(int nargs	/*參數數量*/ , int nresults/*返回值數量*/,int errfunc = NULL/*棧中的一個 錯誤回調函數*/)
			{
				return lua_pcall(_l.get(),nargs,nresults,errfunc) == LUA_OK;
			}
		};

		//lua 的 scoped 運行 環境 析構時不會 調用 lua_close
		//方便 用在 lua c擴展函數 中
		class scoped_context_t:public context_t,boost::noncopyable
		{
		public:
			scoped_context_t(lua_State* l)
				:context_t(l,false)
			{

			}
			scoped_context_t(const l_spt& l)
				:context_t(l.get(),false)
			{

			}
			scoped_context_t(const context_t& l)
				:context_t(l.get(),false)
			{

			}
		};

		//在 析構時 執行 lua_pop 操作
		class scoped_pop_t
			: boost::noncopyable
		{
		protected:
			lua_State* _l;
			int _n;
		public:
			scoped_pop_t(lua_State* l,int n)
			{
				_l = l;
				_n = n;
			}
			scoped_pop_t(context_t l,int n)
			{
				_l = l.get();
				_n = n;
			}
			~scoped_pop_t()
			{
				if(_n && _l)
				{
					lua_pop(_l,_n);
				}
			}

			scoped_pop_t& operator+=(int n)
			{
				_n += n;
				return *this;
			}
			void operator++()
			{
				++_n;
			}
			scoped_pop_t& operator-=(int n)
			{
				_n += n;
				return *this;
			}
			void operator--()
			{
				--_n;
			}

		};
	};
};
#endif	//_DARK_CPP__LUA_CONTEXT__HPP_