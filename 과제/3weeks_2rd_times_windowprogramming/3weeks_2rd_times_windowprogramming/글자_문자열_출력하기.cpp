#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <random>
#include <string>

using namespace std;

default_random_engine dre{100};

// 1. 시드 생성을 위한 장치 (실행 시마다 다른 값을 가져옴)
random_device rd;

// 2. 난수 생성기 엔진 초기화 (시드 전달)
mt19937 gen(rd());

// 3. 균등 분포 정의 (예: 1부터 100 사이의 정수)
uniform_int_distribution<int> dis(1, 100);

uniform_int_distribution Ruid{ 0,255 };
uniform_int_distribution Guid{ 0,255 };
uniform_int_distribution Buid{ 0,255 };

uniform_int_distribution<int> CharUid{ 'a','z'};


// 들어갈 칸 수 정하기



int GetRect(int num)
{
	uniform_int_distribution<int> rand(0, num / 2);
	return rand(gen);
}

int GetPoint(int num) {
	uniform_int_distribution<int> point(0, num / 3);
	return point(gen);
}


int wide{ 800 };
int height{ 600 };

int x[15]{};
int y[15]{};

HINSTANCE g_hInst;

int g_yPos{};
int g_xPos{};

LPCTSTR IpszClass = L"My Window Class 3";
LPCTSTR IpszWindowName = L"Window Programming 3"; // 타이틀

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR IpzeCmdParam, int nCmdShow)
{
	

	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 배경 색깔
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = IpszClass; // 윈도우 클래스 이름 정하기
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);



	RegisterClassEx(&WndClass);



	// 윈도우 특성 정하기, ex) 스크롤 바
	hWnd = CreateWindow(IpszClass, IpszWindowName, WS_OVERLAPPEDWINDOW , 0, 0, wide, height, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	

	while (GetMessage(&Message, 0, 0, 0)) { // 메시지 반복
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;

}






LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	WCHAR lpOut1[10000];
	WCHAR lpOut2[10000];
	WCHAR lpOut3[10000];
	WCHAR lpOut4[10000];


	HDC hDC;
	int cnt{};

	int RectLeft = GetRect(wide)/8;
	int RectTop = GetRect(height)/15;
	int RectRight = (GetRect(wide)+100)/8;
	int RectBottom = (GetRect(height)+100)/15;

	int PointLeft = GetPoint(wide);
	int PointTop = GetPoint(height);



	if (RectLeft > RectRight) {
		int c{RectLeft};
		RectLeft = RectRight;
		RectRight = c;
	}

	if (RectTop > RectBottom) {
		int c{ RectTop };
		RectTop = RectBottom;
		RectBottom = c;
	}

	int RectWidth = RectRight - RectLeft; // 가로길이
	int RectHeight = RectBottom - RectTop; // 세로길이



	// 마우스 휠을 인식하는 것을 만들기 위해 전역으로 재선언
	// 스크롤 위치를 저장할 변수
	//static int g_yPos{};
	//static int g_xPos{};




	// --메시지 처리하기

	switch (uMsg) {
	case WM_CREATE:
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		SetTextColor(hDC, RGB(Ruid(dre), Guid(dre), Buid(dre)));


		
		for (int i = 0; i < RectWidth; ++i) {
			lpOut1[i] = static_cast<wchar_t>(CharUid(gen));
			lpOut2[i] = static_cast<wchar_t>(CharUid(gen));
		}
		
		for (int i = 0; i < RectHeight ; ++i) {
			lpOut3[i] = static_cast<wchar_t>(CharUid(gen));
			lpOut4[i] = static_cast<wchar_t>(CharUid(gen));
		}
		
		

		for (int i = 0; i < (RectHeight); ++i) {
			TextOut(hDC, PointLeft, PointTop + i * 15, &lpOut3[i], 1);
			SetTextColor(hDC, RGB(Ruid(dre), Guid(dre), Buid(dre)));

			TextOut(hDC, PointLeft + RectWidth*9, PointTop + i * 15, &lpOut4[i], 1);
		}
		for (int i = 0; i < (RectWidth); ++i) {
			TextOut(hDC, PointLeft + i*9, PointTop, &lpOut1[i], 1);
			SetTextColor(hDC, RGB(Ruid(dre), Guid(dre), Buid(dre)));

			TextOut(hDC, PointLeft + i*9, PointTop + RectHeight*15, &lpOut2[i], 1);
		}

		

		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE :
		height = HIWORD(lParam);
		wide = LOWORD(lParam);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}