#define _CRT_SECURE_NO_WARNINGS

#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <random>
#include <string>

using namespace std;

// 난수 생성기
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);

int wide{ 800 };
int height{ 600 };



HINSTANCE g_hInst;

LPCTSTR IpszClass = L"My Window Class 3";
LPCTSTR IpszWindowName = L"메모장"; // 타이틀

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



struct StrData
{
    TCHAR str[40];
    int len;
};

static StrData MemoData[10];
static int CurrentLine{};


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    PAINTSTRUCT ps;
    HDC hDC;

    HBRUSH hBrush, oldBrush;
    switch (uMsg) {
    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps); // DC 얻어오기
        // 윈도우가 제공하는 객체 가져오기
        hBrush = (HBRUSH)GetStockObject(GRAY_BRUSH); oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
        Rectangle(hDC, 50, 50, 300, 200);
        SelectObject(hDC, oldBrush); // 제자리 돌아가기
        // DC 해제하기
        EndPaint(hWnd, &ps); return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}