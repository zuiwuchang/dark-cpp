#ifndef _DARK_CPP__WINDOWS_UTIL__HPP_
#define _DARK_CPP__WINDOWS_UTIL__HPP_

namespace dark
{
	namespace windows
	{
		class util
		{
		private:
			util();
			~util();
			util(const util& copy);
			util& operator=(const util& copy);
		public:
			//„“½¨ ¿ØÖÆÌ¨
			static bool alloc_console()
			{
				if(!AllocConsole())
				{
					return false;
				}

				//static FILE *f_in = NULL; 
				static FILE *f_out = NULL; 
				static FILE *f_err = NULL; 
				/*if(f_in)
				{
					fclose(f_in);
					f_in = NULL;
				}*/
				if(f_out)
				{
					fclose(f_out);
					f_out = NULL;
				}
				if(f_err)
				{
					fclose(f_err);
					f_err = NULL;
				}

				errno_t err;
			
				/*err = freopen_s( &f_in, "conin$", "r", stdin );
				if(err)
				{
					return false;
				}*/

				err = freopen_s( &f_out, "conout$", "w", stdout );
				if(err)
				{
					return false;
				}

				err = freopen_s( &f_err, "conout$", "w", stderr );
				if(err)
				{
					return false;
				}
				
				return true;
			}
			static bool free_console()
			{
				return FreeConsole() == TRUE;
			}
		};
		
	};
};
#endif	//_DARK_CPP__WINDOWS_UTIL__HPP_