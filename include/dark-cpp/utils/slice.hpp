#ifndef _DARK_CPP__UTILS_SLICE__HPP_
#define _DARK_CPP__UTILS_SLICE__HPP_
#include <boost/smart_ptr.hpp>
namespace dark
{
	namespace utils
	{
		template<typename T>
		class slice_t
		{
		protected:
			T* _ptr;
			std::size_t _size;

		public:
			slice_t()
			{
				_ptr = NULL;
				_size = 0;
			}
			slice_t(T* ptr,std::size_t size)
			{
				if(size)
				{
					_ptr = ptr;
				}
				else
				{
					_ptr = NULL;
				}
				_size = size;
			}

			inline operator T*()const
			{
				return _ptr;
			}
			inline T* get()const
			{
				return _ptr;
			}
			inline std::size_t size()const
			{
				return _size;
			}
			inline operator bool()
			{
				return _size;
			}
		};
		
	};
};
#endif //_DARK_CPP__UTILS_SLICE__HPP_