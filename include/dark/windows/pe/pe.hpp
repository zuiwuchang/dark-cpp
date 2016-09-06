#ifndef _DARK_CPP__WINDOWS_PE_PE__HPP_
#define _DARK_CPP__WINDOWS_PE_PE__HPP_

#include "32.hpp"

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/foreach.hpp>

#include <fstream>
#include <vector>
namespace dark
{
	namespace windows
	{
		typedef boost::shared_ptr<IMAGE_EXPORT_DIRECTORY> export_directory_ptr_t;
		typedef boost::shared_ptr<IMAGE_SECTION_HEADER> section_header_ptr_t;

		typedef struct _IMAGE_EXPORT_FUNCTION_
		{
			DWORD Index;
			DWORD Rva;
			DWORD Offset;
			std::string Name;
		}IMAGE_EXPORT_FUNCTION,*PIMAGE_EXPORT_FUNCTION;
		typedef boost::shared_ptr<IMAGE_EXPORT_FUNCTION>  image_export_function_ptr_t;
		typedef boost::shared_ptr<std::vector<image_export_function_ptr_t>> image_export_functions_ptr_t;

		class pe 
			: boost::noncopyable
		{
		protected:
			IMAGE_DOS_HEADER _dos_header;
			std::fstream _file;
			bool _ok;

			IMAGE_NT_HEADERS _nt_header;
			std::vector<section_header_ptr_t> _section_headers;

			//導出模塊名
			std::string _name;

			//導出表
			export_directory_ptr_t _export_directory_ptr;
			//導出函數
			image_export_functions_ptr_t _image_export_functions_ptr;
		public:
			pe(const std::string path)
			{
				_ok = false;
				_file.open(path,std::ios::binary|std::ios::in);
				if(!_file.is_open())
				{
					return;
				}

				_file.read((char*)&_dos_header,sizeof(_dos_header));
				if(_dos_header.e_magic != DARK_CPP_WINDOWS_PE_MAGIC_MZ)
				{
					return;
				}
				_file.seekg(_dos_header.e_lfanew,std::ios::beg);
				_file.read((char*)&_nt_header,sizeof(_nt_header));
				if(_nt_header.Signature != DARK_CPP_WINDOWS_PE_MAGIC_PE)
				{
					return;
				}

				for(int i=0;i<_nt_header.FileHeader.NumberOfSections;++i)
				{
					section_header_ptr_t tmp = boost::make_shared<IMAGE_SECTION_HEADER>();
					memset((char*)tmp.get(),0,sizeof(IMAGE_SECTION_HEADER));
					_file.read((char*)tmp.get(),sizeof(IMAGE_SECTION_HEADER));
					_section_headers.push_back(tmp);
				}

				_ok = true;
			}
			operator bool()
			{
				return _ok;
			}

			export_directory_ptr_t get_export_directory()
			{
				if(!_export_directory_ptr)
				{
					DWORD rva = _nt_header.OptionalHeader.DataDirectory[0].VirtualAddress;
					//DWORD size = _nt_header.OptionalHeader.DataDirectory[0].Size;
					
					DWORD foa = rva_to_foa(rva);
					if(foa != 0)
					{
						_file.seekg(foa,std::ios::beg);

						_export_directory_ptr = boost::make_shared<IMAGE_EXPORT_DIRECTORY>();
						_file.read((char*)_export_directory_ptr.get(),sizeof(IMAGE_EXPORT_DIRECTORY));
					}
				}
				return _export_directory_ptr;
			}
			DWORD rva_to_foa(DWORD rva)const
			{
				section_header_ptr_t find = find_directory_by_rva(rva);
				if(!find)
				{
					return 0;
				}
				DWORD offset = rva - find->VirtualAddress;
				return find->PointerToRawData + offset;
			}
			section_header_ptr_t find_directory_by_rva(DWORD rva)const
			{

				BOOST_FOREACH(section_header_ptr_t node,_section_headers)
				{
					if(node->VirtualAddress > rva)
					{
						continue;
					}
					if(node->VirtualAddress + node->Misc.VirtualSize <= rva)
					{
						continue;
					}

					return node;
				}
				return section_header_ptr_t();
			}
			image_export_functions_ptr_t get_image_export_functions()
			{
				if(!_image_export_functions_ptr)
				{
					export_directory_ptr_t export_directory = get_export_directory();
					if(!export_directory)
					{
						return _image_export_functions_ptr;
					}
					
					DWORD foa_name = rva_to_foa(export_directory->Name);
					if(!foa_name)
					{
						return _image_export_functions_ptr;
					}
					DWORD foa_name_i = rva_to_foa(export_directory->AddressOfNameOrdinals);
					if(!foa_name_i)
					{
						return _image_export_functions_ptr;
					}

					DWORD foa_func = rva_to_foa(export_directory->AddressOfFunctions);
					if(!foa_func)
					{
						return _image_export_functions_ptr;
					}

					_image_export_functions_ptr = boost::make_shared<std::vector<image_export_function_ptr_t>>();
					

					//獲取 函數 地址
					for(DWORD i=0;i<export_directory->NumberOfFunctions;++i)
					{
						DWORD rva = 0;
						_file.seekg(foa_func + i * sizeof(DWORD),std::ios::beg);
						_file.read((char*)&rva,sizeof(DWORD));
						if(rva == 0)
						{
							continue;
						}


						image_export_function_ptr_t tmp = boost::make_shared<IMAGE_EXPORT_FUNCTION>();
						tmp->Index = i + export_directory->Base;

						tmp->Rva = rva;
						tmp->Offset = rva_to_foa(rva);
						_image_export_functions_ptr->push_back(tmp);
					}

					//獲取 函數名
					_file.seekg(foa_name_i,std::ios::beg);
					std::vector<WORD> indexs;
					for(DWORD i=0;i<export_directory->NumberOfNames;++i)
					{
						WORD tmp = 0;
						_file.read((char*)&tmp,sizeof(WORD));
						indexs.push_back(tmp);
					}

					_file.seekg(foa_name,std::ios::beg);
					std::string name = read_name();
					BOOST_FOREACH(WORD index,indexs)
					{
						name = read_name();
						BOOST_FOREACH(image_export_function_ptr_t func,*_image_export_functions_ptr)
						{
							if(func->Index == index + export_directory->Base)
							{
								func->Name = name;
							}
						}
					}
				}
				return _image_export_functions_ptr;
			}
			std::string get_name()
			{
				if(_name.empty())
				{
					export_directory_ptr_t export_directory = get_export_directory();
					if(!export_directory)
					{
						return _name;
					}
					
					DWORD foa_name = rva_to_foa(export_directory->Name);
					if(!foa_name)
					{
						return _name;
					}
					_file.seekg(foa_name,std::ios::beg);
					_name = read_name();
				}
				return _name;
			}
		protected:
			std::string read_name()
			{
				std::string name;
				while(true)
				{
					char c = 0;
					_file.get(c);
					if(c == 0)
					{
						break;
					}
					else
					{
						name.push_back(c);
					}
				}
				return name;
			}
		};
	};
};

#endif	//_DARK_CPP__WINDOWS_PE_PE__HPP_