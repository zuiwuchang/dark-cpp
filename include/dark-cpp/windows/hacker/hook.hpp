#ifndef _DARK_CPP__WINDOWS_HACKER_HOOK__HPP_
#define _DARK_CPP__WINDOWS_HACKER_HOOK__HPP_

#include <boost/smart_ptr.hpp>

namespace dark
{
	namespace windows
	{
		namespace hacker
		{
			class hook_code_t
			{
			public:
				//hook 位置
				DWORD address;

				//新函數 jmp 到原函數位置
				DWORD jump;
	
				//原函數被替換字節長度
				DWORD size;

				//原函數被替換代碼
				boost::shared_array<BYTE> code;

				hook_code_t()
				{
					address	=	jump	=	size	=	0;
				}
				hook_code_t(const hook_code_t& copy)
				{
					address	=	copy.address;
					jump	=	copy.jump;
					size	=	copy.size;
					code	=	copy.code;
				}
				hook_code_t& operator=(const hook_code_t& copy)
				{
					address	=	copy.address;
					jump	=	copy.jump;
					size	=	copy.size;
					code	=	copy.code;
					return *this;
				}
			};
			typedef boost::shared_ptr<hook_code_t> hook_code_ptr_t;

			class hook
			{
			public:
				inline static hook_code_ptr_t inline_hook(const DWORD address,		//被hook位置
					const BYTE* code,	//hook 代碼
					DWORD size	//hook代碼長度
					)
				{
					return inline_hook((PVOID)address,code,size);
				}
				static hook_code_ptr_t inline_hook(const PVOID address,		//被hook位置
					const BYTE* code,	//hook 代碼
					DWORD size	//hook代碼長度
					)
				{
					if(!address
						|| !code
						|| !size
						)
					{
						return hook_code_ptr_t();
					}
					MEMORY_BASIC_INFORMATION mbi_thunk;  
					//查詢頁信息  
					VirtualQuery(address, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));
	
					//改變頁保護屬性為讀寫
					if(!VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize,PAGE_EXECUTE_READWRITE, &mbi_thunk.Protect))
					{
						return hook_code_ptr_t();
					}
					//保存被hook代碼
					hook_code_ptr_t code_ptr = boost::make_shared<hook_code_t>();
					code_ptr->code	=	boost::shared_array<BYTE>(new BYTE[size]);
					code_ptr->size	=	size;
					code_ptr->address	=	(DWORD)address;
					memcpy(code_ptr->code.get(),address,size);

					//寫入hook代碼
					memcpy(address,code,size);

					//恢復頁保護屬性
					VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize, mbi_thunk.Protect, NULL);
					return code_ptr;
				}
				
				//恢復hook代碼
				static bool un_inline_hook(hook_code_ptr_t code_ptr)
				{
					if(!code_ptr
						)
					{
						return false;
					}
					PVOID address = (PVOID)code_ptr->address;

					MEMORY_BASIC_INFORMATION mbi_thunk;  
					//查詢頁信息  
					VirtualQuery(address, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));
	
					//改變頁保護屬性為讀寫
					if(!VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize,PAGE_EXECUTE_READWRITE, &mbi_thunk.Protect))
					{
						return false;
					}

					//恢復hook代碼
					memcpy((void*)code_ptr->address,code_ptr->code.get(),code_ptr->size);

					//恢復頁保護屬性
					VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize, mbi_thunk.Protect, NULL);
					return true;
				}


				inline static hook_code_ptr_t inline_hook(const DWORD address,		//被hook位置
					const DWORD myaddress,	//自定義函數 位置
					DWORD size				//hook代碼長度
					)
				{
					return inline_hook((PVOID)address,(PVOID)myaddress,size);
				}
				static hook_code_ptr_t inline_hook(const PVOID address,		//被hook位置
					const PVOID myaddress,	///自定義函數 位置
					DWORD size				//hook代碼長度
					)
				{
					if(!address
						|| !myaddress
						|| !size
						)
					{
						return hook_code_ptr_t();
					}
					MEMORY_BASIC_INFORMATION mbi_thunk;  
					//查詢頁信息  
					VirtualQuery(address, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));
	
					//改變頁保護屬性為讀寫
					if(!VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize,PAGE_EXECUTE_READWRITE, &mbi_thunk.Protect))
					{
						return false;
					}

					hook_code_ptr_t code_ptr = boost::make_shared<hook_code_t>();
					//新函數跳轉回原函數地址
					code_ptr->jump	=	(DWORD)address + size;

					//保存被hook代碼
					code_ptr->code	=	boost::shared_array<BYTE>(new BYTE[size]);
					code_ptr->size	=	size;
					code_ptr->address	=	(DWORD)address;
					memcpy(code_ptr->code.get(),address,size);

					//hook code
					__asm
					{
						push ebx;

						mov eax,myaddress;
						sub eax,address;
						sub eax,5;

						mov ebx,address;
						mov byte ptr [ebx],0xE9;
						mov [ebx+1],eax;

						pop ebx;
					}

					//恢復頁保護屬性
					VirtualProtect(mbi_thunk.BaseAddress,mbi_thunk.RegionSize, mbi_thunk.Protect, NULL);
					return code_ptr;
				}
			};
		};
	};
};
#endif	//_DARK_CPP__WINDOWS_HACKER_HOOK__HPP_