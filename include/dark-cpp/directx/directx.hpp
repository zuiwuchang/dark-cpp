#ifndef _DARK_CPP__DIRECTX_DIRECTX__HPP_
#define _DARK_CPP__DIRECTX_DIRECTX__HPP_

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>

namespace dark
{
	namespace directx
	{
//窗口定義
#define DARK_DIRECTX_WINDOW_TITLE	L"dark directx window"
#define DARK_DIRECTX_WINDOW_CLASS	L"dark directx window class"

		//d3dx 設備 封裝
		class device
		{
		protected:
			//d3dx 設備
			IDirect3DDevice9*		_device;

			//窗口 寬/高 
			int _width;
			int _height;
			int _fps;
		public:
			inline IDirect3DDevice9* device9()
			{
				return _device;
			}
			inline int width()
			{
				return _width;
			}
			inline int height()
			{
				return _height;
			}
			inline int fps()
			{
				return _fps;
			}

			bool init_device(HINSTANCE hInstance		//應用程序實例
				,HICON hIcon										//圖標
				,HICON hIconSm										//圖標
				,WNDPROC window_fun									//窗口函數
				,const wchar_t* title = DARK_DIRECTX_WINDOW_TITLE	//窗口標題
				,int fps = 60
				,int width = 640									//窗口寬
				,int height = 480									//窗口高
				,bool window = true									//窗口模式 而非 全屏
				,D3DDEVTYPE device_type= D3DDEVTYPE_HAL				//使用 硬件/軟件 運算
			)
			{
				_fps = fps;
				_height = height;
				_width = width;

				//註冊窗口類
				WNDCLASSEX wcex;

				wcex.cbSize = sizeof(WNDCLASSEX);

				wcex.style			= CS_HREDRAW | CS_VREDRAW;
				wcex.lpfnWndProc	= window_fun;
				wcex.cbClsExtra		= 0;
				wcex.cbWndExtra		= 0;
				wcex.hInstance		= hInstance;
				wcex.hIcon			= hIcon;
				wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
				wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
				wcex.lpszMenuName	= 0;//MAKEINTRESOURCE(IDC_DIRECT3DTEST);;
				wcex.lpszClassName	= DARK_DIRECTX_WINDOW_CLASS;
				wcex.hIconSm		= hIconSm;

				RegisterClassEx(&wcex);


				//創建窗口 並顯示
				DWORD style	=	WS_EX_TOPMOST;				//頂層窗口
				style	|=	WS_SYSMENU | WS_MINIMIZEBOX;	//最小化 關閉 按鈕
				HWND hwnd	=	CreateWindow(DARK_DIRECTX_WINDOW_CLASS, title
					,style
					,CW_USEDEFAULT, 0, width, height
					,0 /*parent hwnd*/, 0 /* menu */, hInstance, 0 /*extra*/); 
				if(!hwnd)
				{
					return false;
				}
				ShowWindow(hwnd, SW_SHOW);
				UpdateWindow(hwnd);

				//開始初始化 Direct3D
				//1	獲取IDirect3D9接口 指針
				IDirect3D9* _direct	=	Direct3DCreate9(D3D_SDK_VERSION);
	
				if(!_direct)
				{
					return false;
				}

				//2 獲取硬件特性
				D3DCAPS9 caps;
				_direct->GetDeviceCaps(D3DADAPTER_DEFAULT, device_type, &caps);

				int vp = 0;
				if(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
				{
					vp	=	D3DCREATE_HARDWARE_VERTEXPROCESSING;
				}
				else
				{
					vp	=	D3DCREATE_SOFTWARE_VERTEXPROCESSING;
				}

				//3	填充 D3DPRESENT_PARAMETERS
				D3DPRESENT_PARAMETERS d3dpp;
				d3dpp.BackBufferWidth            = width;
				d3dpp.BackBufferHeight           = height;
				d3dpp.BackBufferFormat           = D3DFMT_A8R8G8B8;
				d3dpp.BackBufferCount            = 1;
				d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
				d3dpp.MultiSampleQuality         = 0;
				d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD; 
				d3dpp.hDeviceWindow              = hwnd;
				d3dpp.Windowed                   = window;
				d3dpp.EnableAutoDepthStencil     = true; 
				d3dpp.AutoDepthStencilFormat     = D3DFMT_D24S8;
				d3dpp.Flags                      = 0;
				d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
				d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;


				//4 創建 設備
				HRESULT hr	=	_direct->CreateDevice(D3DADAPTER_DEFAULT
					,device_type
					,hwnd
					,vp
					,&d3dpp
					,&_device
					);
				if(FAILED(hr))
				{
					//使用16位深 重試
					d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
					hr	=	_direct->CreateDevice(D3DADAPTER_DEFAULT
						,device_type
						,hwnd
						,vp
						,&d3dpp
						,&_device
						);
					if(FAILED(hr))
					{
						_direct->Release();
						return false;
					}
				}

				if(!init())
				{
					_direct->Release();
					return false;
				}
				//釋放IDirect3D9接口 指針
				_direct->Release();
				return true;
			}
			device()
			{
				_fps = 60;
				_device = NULL;
			}
			virtual ~device()
			{
				release();
			}

			//進入消息循環
			void msg_loop()
			{
				MSG msg;
				memset(&msg,0,sizeof(MSG));
				DWORD time_begin = GetTickCount(); 
				DWORD time_now;
				DWORD time_interval;
				DWORD fps_n = _fps / 10;
				DWORD fps_sum = 0;
				DWORD fps_now = time_begin;
				while(msg.message != WM_QUIT)
				{
					if(::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
					
					//計算 場景繪製 時間間隔
					time_now		=	GetTickCount(); 
					time_interval	=	time_now - time_begin;
					run(time_interval);
					_draw(time_interval);
					time_begin = time_now;	

					//同步 fps
					++fps_sum;
					if(fps_sum == fps_n)
					{
						DWORD wait = GetTickCount() - fps_now;
						if(wait < 100)
						{
							Sleep(100  - wait);
						}
						fps_now = GetTickCount();
						fps_sum = 0;
					}
				}
			}

		protected:
			void _draw(const DWORD time_interval)
			{
				//清除背景
				_device->Clear(0,0,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER ,0xffffffff,1.0f,0);
				//繪製場景
				_device->BeginScene();
				{
					draw(time_interval);
				}
				_device->EndScene();

				//交換緩衝區到 屏幕
				_device->Present(0,0,0,0);
			}

			//應該重載此函數 在裡面增加 資源釋放操作 釋放資源
			virtual void release()
			{
				if(_device)
				{
					_device->Release();
					_device	=	NULL;
				}
			}
			//應該重載此函數 在裡面增加 初始化 設備
			//返回 false 導致 init_device 失敗
			virtual bool init()
			{
				return true;
			}
			//應該重載此函數 以改變場景繪製 time_interval 是距離上次繪製的 時間間隔
			virtual void draw(const DWORD time_interval) = 0;
			//應該重載此函數 在此執行 遊戲 沒幀邏輯
			virtual void run(const DWORD time_interval) = 0;
		};
	};
};

#endif	//_DARK_CPP__DIRECTX_DIRECTX__HPP_