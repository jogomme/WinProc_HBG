#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <random>
#include <string>

using namespace std;

// 1. 시드 생성을 위한 장치 (실행 시마다 다른 값을 가져옴)
random_device rd;

// 2. 난수 생성기 엔진 초기화 (시드 전달)
mt19937 gen(rd());

// 3. 균등 분포 정의 (예: 1부터 100 사이의 정수)
uniform_int_distribution<int> dis(1, 100);

uniform_int_distribution<int> Ruid{ 0,255 };
uniform_int_distribution<int> Guid{ 0,255 };
uniform_int_distribution<int> Buid{ 0,255 };

uniform_int_distribution<int> Xuid{ 0,600 };
uniform_int_distribution<int> Yuid{ 0,500 };



int wide{ 800 };
int height{ 600 };



HINSTANCE g_hInst;

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
	hWnd = CreateWindow(IpszClass, IpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, wide, height, NULL, (HMENU)NULL, hInstance, NULL);
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

	static SIZE size;

	static TCHAR str[5][30];
	static int Count{};
	static int r{};

	static int rCnt{};

	static TCHAR Point[100];

	static int xPos = Xuid(gen);
	static int yPos = Yuid(gen);
	static int zPos = yPos;


	HDC hDC;
	int cnt{};

	static int R = Ruid(gen);
	static int G = Guid(gen);
	static int B = Buid(gen);
	int currentY{};


	// --메시지 처리하기

	switch (uMsg) {
	case WM_CREATE:
		CreateCaret(hWnd, NULL, 5, 15);
		ShowCaret(hWnd);

		break;

	case WM_CHAR :
		hDC = GetDC(hWnd);
		if (wParam == VK_BACK) {
			if (Count == 0 && r>0) {
				--r;
				Count = 20;
			}
			else if (Count == 0) {
				yPos -= 20;
			}
			else {
				Count--;
			}
		}
		else if(wParam == VK_RETURN){
			
			yPos += 20;
		}
		else {
			str[r][Count++] = wParam;
			if (Count == 20) {
				++r;
				Count = 0;
				if (r >= 5) {
					break;
				}
			}
		}
		str[r][Count] = '\0';

		InvalidateRect(hWnd, NULL, true);

		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		wsprintf(Point, L"문자열의 위치 ( %d, %d )", xPos,yPos);
		TextOut(hDC, 0, 0, Point, lstrlen(Point));

		SetTextColor(hDC, RGB(R,G,B));
		
		GetTextExtentPoint32(hDC, str[r], lstrlen(str[r]), &size);
		for (int i = 0; i <= r; ++i) {
			currentY = yPos + (i * 20);
			TextOut(hDC, xPos, currentY, str[i], lstrlen(str[i]));
		}


		SetCaretPos(xPos + size.cx, currentY);
		
		
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
		height = HIWORD(lParam);
		wide = LOWORD(lParam);
		break;

	case WM_DESTROY:
		HideCaret(hWnd);
		DestroyCaret();
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}