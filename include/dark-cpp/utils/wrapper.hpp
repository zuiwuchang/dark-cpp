#ifndef _DARK_CPP__UTILS_WRAPPER__HPP_
#define _DARK_CPP__UTILS_WRAPPER__HPP_
#include <boost/smart_ptr.hpp>
namespace dark
{
	namespace utils
	{
		//為boost::shared_ptr<T> 定義一個 包裝 
		//以創建 dll 模塊時 將 boost::shared_ptr 隱藏
		template<typename T>
		class spt_wrapper
		{
		protected:
			boost::shared_ptr<T> _spt;
		public:
			//構造
			spt_wrapper()
			{
			}
			spt_wrapper(const boost::shared_ptr<T>& copy)
			{
				_spt = copy;
			}
			spt_wrapper(const spt_wrapper& copy)
			{
				_spt = copy._spt;
			}
			spt_wrapper(T* p)
			{
				_spt = boost::shared_ptr<T>(p);
			}
			//copy
			spt_wrapper& operator=(const boost::shared_ptr<T>& copy)
			{
				_spt = copy;
				return *this;
			}
			spt_wrapper& operator=(const spt_wrapper& copy)
			{
				_spt = copy._spt;
				return *this;
			}
			spt_wrapper& operator=(T* p)
			{
				_spt = boost::shared_ptr<T>(p);
				return *this;
			}

			//解包 返回boost::shared_ptr<T>
			inline spt_wrapper unpack()const
			{
				return _spt;
			}
			inline operator spt_wrapper()const
			{
				return _spt;
			}
			
			inline T* get()const
			{
				return _spt.get();
			}
			inline T* operator ->()const
			{
				return _spt.get();
			}
			inline operator T*()const
			{
				return _spt.get();
			}

			//放棄 資源
			inline void reset()
			{
				_spt.reset();
			}
			inline bool empty()const
			{
				return _spt;
			}
			inline operator bool()const
			{
				return _spt;
			}
		};
	};
};
#endif	//_DARK_CPP__UTILS_WRAPPER__HPP_