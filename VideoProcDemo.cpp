// VideoProcDemo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "VideoProcDemo.h"
#include <stdio.h>
#include <Commdlg.h>

#define MAX_LOADSTRING 100

HHOOK mouse_Hook;
LRESULT CALLBACK LowLevelMouseProc(INT nCode,
	WPARAM wParam,
	LPARAM lParam
);
BOOL UninstallHook();  //卸载  
BOOL InstallHook();     //安装

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
bool stop = false;
bool Ldown = false;

long width;
long height;
long nowPinP;
long xmove = 0;
long ymove = 0;
bool bianyuan = false;

float gaosi[3][3] = {
	{ 0.11, 0.11, 0.11},
	{ 0.11, 0.11, 0.11 },
	{ 0.11, 0.11, 0.11 }
};

#define IMAGE_WIDTH    352
#define IMAGE_HIGHT    288

typedef struct COLOR              //rgb缓冲区
{
    BYTE b;
    BYTE g;
    BYTE r;
}RGBCOLOR;
//用于加载视频文件
static FILE* ifp;  //file pointer
char filename[MAX_PATH] = "mother.yuv";
static BYTE      mybuf[45621248];      //存放整个yuv文件内容
static BYTE* pBity, * pBitu, * pBitv;       //指向一张画面的YUV分区首地址
static int       y[288][352], u[144][176], v[144][176];   //存放一张画面的yuv分量

static FILE* ifp1;  //file pointer
static const char* filename1 = "foreman.yuv";
static BYTE      mybuf1[45621248];      //存放整个yuv文件内容
static BYTE* pBity1, *pBitu1, *pBitv1;       //指向一张画面的YUV分区首地址
static int       y1[288][352], u1[144][176], v1[144][176];   //存放一张画面的yuv分量

//背景图像
static FILE* ifpback;
static const char* filenameback = "background.bmp";
static unsigned char mybufback[IMAGE_WIDTH * IMAGE_HIGHT * 3 + 100];
static BITMAPFILEHEADER* pbmfh;
static BITMAPINFO* pbmi;
static BYTE* pbits;
static int              cxDib, cyDib;

//融合图像
static COLOR det_image[IMAGE_HIGHT][IMAGE_WIDTH];//要显示的目标图像结构体
static COLOR det_image1[IMAGE_HIGHT][IMAGE_WIDTH];//要显示的目标图像结构体
static int n = 0;
static int n1 = 0;


// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//qita
OPENFILENAME ofn;
TCHAR szFile[MAX_PATH];//存放文件名  
char File[MAX_PATH];

long sumr;
long sumg;
long sumb;
int iLength;

bool f1 = false;

//末尾
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    //打开视频文件
	if (filename != "")
	{
		fopen_s(&ifp, filename, "r");
		fread(mybuf, 45621248, 1, ifp);
		pBity = mybuf;
		pBitu = mybuf + 352 * 288;
		pBitv = mybuf + 352 * 288 + 176 * 144;
	}
	if (filename1 != "")
	{
		fopen_s(&ifp1, filename1, "r");
		fread(mybuf1, 45621248, 1, ifp1);
		pBity1 = mybuf1;
		pBitu1 = mybuf1 + 352 * 288;
		pBitv1 = mybuf1 + 352 * 288 + 176 * 144;
	}

    //打开图像文件
    fopen_s(&ifpback, filenameback, "r");
    fread(mybufback, 307200, 1, ifpback);
    pbmfh = (BITMAPFILEHEADER*)mybufback;
    pbmi = (BITMAPINFO*)(pbmfh + 1);
    pbits = (BYTE*)pbmfh + pbmfh->bfOffBits;
    cxDib = pbmi->bmiHeader.biWidth;
    cyDib = pbmi->bmiHeader.biHeight;

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VIDEOPROCDEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VIDEOPROCDEMO));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIDEOPROCDEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VIDEOPROCDEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case ID_PLAY1:
				SetTimer(hWnd, 1, 40, NULL);
				stop = false;
				break;
			case ID_PLAY2:
				SetTimer(hWnd, 2, 40, NULL);
				break;
			case ID_STOP1:
				KillTimer(hWnd, 1);
				stop = true;
				break;
			case ID_STOP2:
				KillTimer(hWnd, 2);
				break;
			case ID_RESET1:
				KillTimer(hWnd, 1);
				n = 0;
				stop = true;
				break;
			case ID_RESET2:
				KillTimer(hWnd, 2);
				n1 = 0;
				break;
			case ID_V1:
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = _T("Text files (*.yuv)\0*.yuv\0\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				ofn.lpstrTitle = _T("打开");
				if (GetOpenFileName(&ofn))
				{
					n = 0;
					iLength = WideCharToMultiByte(CP_ACP, 0, szFile, -1, NULL, 0, NULL, NULL);
					WideCharToMultiByte(CP_ACP, 0, szFile, -1, File, iLength, NULL, NULL);
					fopen_s(&ifp, File, "r");
					fread(mybuf, 45621248, 1, ifp);
					pBity = mybuf;
					pBitu = mybuf + 352 * 288;
					pBitv = mybuf + 352 * 288 + 176 * 144;
				}
				SetTimer(hWnd, 1, 40, NULL);   //设定定时器
				
				break;
			case ID_V2:
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = '\0';
				// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
				// use the contents of szFile to initialize itself.
				//
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = _T("Text files (*.yuv)\0*.yuv\0\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				ofn.lpstrTitle = _T("打开");
				if (GetOpenFileName(&ofn))
				{
					iLength = WideCharToMultiByte(CP_ACP, 0, szFile, -1, NULL, 0, NULL, NULL);
					WideCharToMultiByte(CP_ACP, 0, szFile, -1, File, iLength, NULL, NULL);
					fopen_s(&ifp1, File, "r");
					fread(mybuf1, 45621248, 1, ifp1);
					pBity1 = mybuf1;
					pBitu1 = mybuf1 + 352 * 288;
					pBitv1 = mybuf1 + 352 * 288 + 176 * 144;
				}
				f1 = true;
				SetTimer(hWnd, 2, 40, NULL);
				break;
			case ID_BIAN:
				bianyuan = !bianyuan;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_TIMER:
		if(wParam == 1)
		{
			hdc = GetDC(hWnd);
        n = n + 1;
        if (n > 299) n = 0;
        pBity = pBity + (352 * 288 + 2 * (176 * 144)) * n;
        pBitu = pBity + 352 * 288;
        pBitv = pBitu + 176 * 144;

        //read a new frame of the yuv file
        for (int i = 0; i < 144; i++)
            for (int j = 0; j < 176; j++)
            {
                u[i][j] = *(pBitu + j + 176 * (i));
                v[i][j] = *(pBitv + j + 176 * (i));
            }


        //read y，and translate yuv int rgb and display the pixel
        for (int i = 0; i < 288; i++)
            for (int j = 0; j < 352; j++)
            {
                //read y
                y[i][j] = *(pBity + j + (i) * 352);
                //translate
                int r = (298 * (y[i][j] - 16) + 409 * (v[i / 2][j / 2] - 128) + 128) / 256;// >> 8;
                if (r < 0) r = 0;
                if (r > 255) r = 255;
                int g = (298 * (y[i][j] - 16) - 100 * (u[i / 2][j / 2] - 128) - 208 * (v[i / 2][j / 2] - 128) + 128) / 255;// >> 8;
                if (g < 0) g = 0;
                if (g > 255) g = 255;
                int b = (298 * (y[i][j] - 16) + 516 * (u[i / 2][j / 2] - 128) + 128) / 256;// >> 8;
                if (b < 0) b = 0;
                if (b > 255) b = 255;

                det_image[288 - i - 1][j].r = r;
                det_image[288 - i - 1][j].g = g;
                det_image[288 - i - 1][j].b = b;

            }
		/*
        SetDIBitsToDevice(hdc,
            0,
            0,
            352,
            288,
            0,
            0,
            0,
            288,
            det_image,
            pbmi,
            DIB_RGB_COLORS);
			*/
		RECT rc;
		GetWindowRect(hWnd, &rc);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
		nowPinP =  - xmove + 600;
		StretchDIBits(
			hdc,
			0,
			0,
			width,
			height,
			0,
			0,
			352,
			288,
			det_image,
			pbmi,
			DIB_RGB_COLORS,
			SRCCOPY
		);
		if (f1)
		{
			StretchDIBits(
				hdc,
				nowPinP,
				-ymove,
				width / 3,
				height / 3,
				0,
				0,
				352,
				288,
				det_image1,
				pbmi,
				DIB_RGB_COLORS,
				SRCCOPY
			);
			pBity1 = mybuf1;
		}


        pBity = mybuf; // let pBity to point at the first place of the file
		
        ReleaseDC(hWnd, hdc);
		}
		else
		{
			hdc = GetDC(hWnd);

			n1 = n1 + 1;
			if (n1 > 299) n1 = 0;
			pBity1 = pBity1 + (352 * 288 + 2 * (176 * 144)) * n1;
			pBitu1 = pBity1 + 352 * 288;
			pBitv1 = pBitu1 + 176 * 144;


			for (int i = 0; i < 144; i++)
				for (int j = 0; j < 176; j++)
				{
					u1[i][j] = *(pBitu1 + j + 176 * (i));
					v1[i][j] = *(pBitv1 + j + 176 * (i));
				}


			for (int i = 0; i < 288; i++)
				for (int j = 0; j < 352; j++)
				{
					//read y
					y1[i][j] = *(pBity1 + j + (i) * 352);
					//translate
					int r = (298 * (y1[i][j] - 16) + 409 * (v1[i / 2][j / 2] - 128) + 128) / 256;// >> 8;
					if (r < 0) r = 0;
					if (r > 255) r = 255;
					int g = (298 * (y1[i][j] - 16) - 100 * (u1[i / 2][j / 2] - 128) - 208 * (v1[i / 2][j / 2] - 128) + 128) / 255;// >> 8;
					if (g < 0) g = 0;
					if (g > 255) g = 255;
					int b = (298 * (y1[i][j] - 16) + 516 * (u1[i / 2][j / 2] - 128) + 128) / 256;// >> 8;
					if (b < 0) b = 0;
					if (b > 255) b = 255;

					det_image1[288 - i - 1][j].r = r;
					det_image1[288 - i - 1][j].g = g;
					det_image1[288 - i - 1][j].b = b;

				}

			if(bianyuan)
			for (int i = 2; i < 288-2; i++)
				for (int j = 2; j < 352 - 2; j++)
				{
					for (int y1 = -2; y1 < 3; y1++)
					{
						for (int x1 = -2; x1 < 3; x1++)
						{
							sumr += det_image1[i + y1][j + x1].r * gaosi[y1][x1];
							sumg += det_image1[i + y1][j + x1].g * gaosi[y1][x1];
							sumb += det_image1[i + y1][j + x1].b * gaosi[y1][x1];
						}
					}
					det_image1[i][j].r = det_image1[i][j].r - sumr / 352;
					det_image1[i][j].g = det_image1[i][j].g - sumg / 352;
					det_image1[i][j].b = det_image1[i][j].b - sumb / 352;
					//SetPixel(hdc, j + 50, i + 50, RGB(sumr, sumg, sumb));
				}
				
			/*
			SetDIBitsToDevice(hdc,
			0,
			0,
			352,
			288,
			0,
			0,
			0,
			288,
			det_image,
			pbmi,
			DIB_RGB_COLORS);
			*/
			RECT rc;
			GetWindowRect(hWnd, &rc);
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			nowPinP =  - xmove + 600;

			/*
			StretchDIBits(
				hdc,
				0,
				0,
				width,
				height,
				0,
				0,
				352,
				288,
				det_image,
				pbmi,
				DIB_RGB_COLORS,
				SRCCOPY
			);
			*/
			StretchDIBits(
				hdc,
				nowPinP,
				-ymove,
				width / 3,
				height / 3,
				0,
				0,
				352,
				288,
				det_image1,
				pbmi,
				DIB_RGB_COLORS,
				SRCCOPY
			);

 // let pBity to point at the first place of the file
			pBity1 = mybuf1;
			ReleaseDC(hWnd, hdc);
		}
        
        break;
	case WM_KEYDOWN:
		if (wParam == VK_SPACE)
		{
			if (!stop)
			{
				KillTimer(hWnd, 1);
				stop = true;
			}
			else
			{
				SetTimer(hWnd, 1, 40, NULL);
				stop = false;
			}
		}
		break;
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON)
		{
			xmove = -LOWORD(lParam) + width / 10 + 600;
			ymove = -HIWORD(lParam) + height / 10;
		}
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
