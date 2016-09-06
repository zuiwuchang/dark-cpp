#ifndef _DARK_CPP__DIRECTX_TEXTURE__HPP_
#define _DARK_CPP__DIRECTX_TEXTURE__HPP_

#include <boost/smart_ptr.hpp>

namespace dark
{
	namespace directx
	{
		typedef boost::shared_ptr<IDirect3DTexture9> texture_ptr_t;
		class texture
		{
		public:
			static void d(IDirect3DTexture9 *ptr)
			{
				ptr->Release();
			}
			static texture_ptr_t create_from_file(IDirect3DDevice9* device,const std::wstring& file)
			{
				texture_ptr_t ptr;

				IDirect3DTexture9* texture = NULL;
				D3DXCreateTextureFromFile(device,file.c_str(),&texture);
				if(texture)
				{
					ptr = texture_ptr_t(texture,texture::d);
				}
				return ptr;
			}
		};
	};
};

#endif	//_DARK_CPP__DIRECTX_TEXTURE__HPP_