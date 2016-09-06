#ifndef _DARK_CPP__UTILS_ARRAY__HPP_
#define _DARK_CPP__UTILS_ARRAY__HPP_
#include <boost/smart_ptr.hpp>
namespace dark
{
	namespace utils
	{
		template<typename T>
		class array_sapt
		{
		protected:
			boost::shared_array<T> _ptr;
			std::size_t _size;

		public:
			array_sapt()
			{
				_size = 0;
			}
			array_sapt(const array_sapt& copy)
			{
				_ptr = copy._ptr;
				_size = copy._size;
			}
			array_sapt& operator=(const array_sapt& copy)
			{
				_ptr = copy._ptr;
				_size = copy._size;
				return *this;
			}

			array_sapt(boost::shared_array<T> ptr,std::size_t size)
			{
				_size = size;
				_ptr = ptr;
			}
			template<class Y>
			array_sapt(Y* ptr,std::size_t size):_ptr(ptr)
			{
				_size = size;
			}
			template<class Y,class D>
			array_sapt(Y* ptr,D d,std::size_t size):_ptr(ptr,d)
			{
				_size = size;
			}

			inline std::size_t size()const
			{
				return _size;
			}
			inline operator T*()const
			{
				return _ptr.get();
			}
			inline T* get()const
			{
				return _ptr.get();
			}
			inline boost::shared_array<T> get_sapt()const
			{
				return _ptr;
			}

			inline operator bool()const
			{
				return _ptr;
			}
			inline void reset()
			{
				_ptr.reset();
				_size = 0;
			}

		};
	};
};
#endif //_DARK_CPP__UTILS_ARRAY__HPP_