#ifndef _DARK_CPP__JS_DUKTAPE_DUKTAPE__HPP_
#define _DARK_CPP__JS_DUKTAPE_DUKTAPE__HPP_
#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>
namespace dark
{
	namespace js
	{
		namespace duktape
		{
			class context
			{
			protected:
				boost::shared_ptr<duk_context> _ctx;
				static void _D(duk_context* ctx)
				{
					duk_destroy_heap(ctx);
				}
			public:
				context()
				{
				}
				context(duk_context* ctx)
				{
					_ctx = boost::shared_ptr<duk_context>(ctx,context::_D);
				}
				context(const context& copy)
				{
					_ctx = copy._ctx;
				}
				inline context& operator=(const context& copy)
				{
					_ctx = copy._ctx;
					return *this;
				}
				inline context& operator=(duk_context* ctx)
				{
					_ctx = boost::shared_ptr<duk_context>(ctx,context::_D);
					return *this;
				}
				inline operator duk_context*()const
				{
					if(_ctx)
					{
						return _ctx.get();
					}
					return NULL;
				}
				inline operator bool() const
				{
					return _ctx;
				}
				inline duk_context* get() const
				{
					if(_ctx)
					{
						return _ctx.get();
					}
					return NULL;
				}
				inline void reset()
				{
					_ctx.reset();
				}
			};
			class scoped_pop
				: boost::noncopyable
			{
			protected:
				int _n;
				duk_context* _ctx_ptr;
				context _ctx;
			public:
				scoped_pop(duk_context *ctx,int n)
				{
					_ctx_ptr = ctx;
					_ctx.reset();
					_n = n;
				}
				scoped_pop(context ctx,int n)
				{
					_ctx_ptr = NULL;
					_ctx = ctx;
					_n = n;
				}

				~scoped_pop()
				{
					reset();
				}
				inline void operator++()
				{
					++_n;
				}
				inline void operator--()
				{
					--_n;
				}
				//n==0 set pop 0 n>0 pop+ n<0 pop- 
				inline void modify(int n)
				{
					if(n == 0)
					{
						_n = 0;
						return;
					}
					_n +=n;
					if(_n<0)
					{
						_n = 0;
					}
				}
				//ÊÖ„Ó pop
				void reset()
				{
					if(_n == 0)
					{
						_ctx_ptr = NULL;
						if(_ctx)
						{
							_ctx.reset();
						}
						return;
					}
					if(_ctx_ptr == NULL)
					{
						_ctx_ptr = _ctx;
					}
					if(_n == 1)
					{
						duk_pop(_ctx_ptr);
					}
					else
					{
						duk_pop_n(_ctx_ptr,_n);
					}

					_n = 0;
					_ctx_ptr = NULL;
					if(_ctx)
					{
						_ctx.reset();
					}
				}
		
				operator bool()const
				{
					if(_ctx)
					{
						return true;
					}
					return _ctx_ptr != NULL;
				}
			};
		};
	};
};
#endif //_DARK_CPP__JS_DUKTAPE_DUKTAPE__HPP_