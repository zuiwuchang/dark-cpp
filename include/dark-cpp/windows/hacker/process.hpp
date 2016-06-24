#ifndef _DARK_CPP__WINDOWS_HACKER_PROCESS__HPP_
#define _DARK_CPP__WINDOWS_HACKER_PROCESS__HPP_
namespace dark
{
	namespace windows
	{
		namespace hacker
		{
			class util
			{
			public:
				static bool anti_hook_token()
				{
					HANDLE hToken;
					if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&hToken))
					{
						TOKEN_PRIVILEGES tkp;

						LookupPrivilegeValue( NULL,SE_DEBUG_NAME,&tkp.Privileges[0].Luid );//修改进程权限
						tkp.PrivilegeCount=1;
						tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
						AdjustTokenPrivileges( hToken,FALSE,&tkp,sizeof tkp,NULL,NULL );//通知系统修改进程权限

						if(GetLastError()!=ERROR_SUCCESS)
						{
							return false;
						}
						return true;
					}
					return false;
				}
				static bool inject_process(HANDLE process,const std::wstring& dll)
				{
					int size  = (dll.size() + 1 ) * sizeof(wchar_t);
					LPVOID addr = VirtualAllocEx(process, NULL, size, MEM_COMMIT, PAGE_READWRITE);
					if ( !addr || !WriteProcessMemory(process, addr, dll.c_str(), size, NULL)) 
					{
						return false;
					}

					
					PTHREAD_START_ROUTINE func = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
					if(!func)
					{
						return false;
					}

					DWORD thread_id = 0;
					HANDLE thread = CreateRemoteThread(process, NULL, 0, func, addr, 0, &thread_id);
					if (!thread_id) 
					{
						return false;
					}
					CloseHandle(thread);
					return true;
				}
			};

			class process
			{
			protected:
				HANDLE _process;
			public:
				process(DWORD pid)
				{
					_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
				}
				~process()
				{
					if(_process)
					{
						CloseHandle(_process);
					}
				}
				inline operator bool()const
				{
					return _process != NULL;
				}

				inline bool get_data(DWORD addr,DWORD& val)
				{
					return TRUE == ReadProcessMemory(_process,(LPCVOID)addr,&val,sizeof(val),NULL);
				}

			};
		};
	};
};
#endif