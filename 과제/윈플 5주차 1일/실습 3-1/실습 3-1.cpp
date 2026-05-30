#define _CRT_SECURE_NO_WARNINGS

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    

using namespace std;
int wide{ 800 };
int height{ 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);

// --------------------------------------------------------
// 윈 메인
// --------------------------------------------------------
HINSTANCE g_hInst;
LPCTSTR IpszClass = L"My Window Class 3";
LPCTSTR IpszWindowName = L"도형 그리기 과제";
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR IpzeCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;
    g_hInst = hInstance;

    WndClass.cbSize = sizeof(WndClass);                     
    WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;//CS_DBLCLKS <- 더블클릭 사용
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

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------

int cell = 30;
int MoveSpeedX = 5;
int MoveSpeedY = 5;

RECT rectView;

bool RFlag = true;
bool EFlag = false;
bool TFlag = false;

bool HTimer = false;
bool VTimer = false;
bool STimer = false;

bool reversX = false;
bool reversY = false;

// --------------------------------------------------------
// 함수 선언 구간
// --------------------------------------------------------


// --------------------------------------------------------
// 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    static POINT point{cell,cell};

    switch (uMsg) {
    case WM_CREATE:
        GetClientRect(hWnd, &rectView);
        break;
    case WM_KEYDOWN:
        if (wParam == 'Q') {
            PostQuitMessage(0);
        }
        else if (wParam == 'R') {
            RFlag = true;
            EFlag = false;
            TFlag = false;
        }
        else if (wParam == 'E') {
            RFlag = false;
            EFlag = true;
            TFlag = false;
        }
        else if (wParam == 'T') {
            RFlag = false;
            EFlag = false;
            TFlag = true;
        }
        else if (wParam == 'H') {
            SetTimer(hWnd, 1, 10, NULL);
            HTimer = true;

            if (VTimer) {
                KillTimer(hWnd, 2);
                VTimer = false;
            }
            if (STimer) {
                KillTimer(hWnd, 3);
                STimer = false;
            }
        }
        else if (wParam == 'V') {
            SetTimer(hWnd, 2, 10, NULL);
            VTimer = true;
            if (HTimer) {
                KillTimer(hWnd, 1);
                HTimer = false;
            }
            if (STimer) {
                KillTimer(hWnd, 3);
                STimer = false;
            }
        }
        else if (wParam == 'S') {
            SetTimer(hWnd, 3, 10, NULL);
            STimer = true;
            if (HTimer) {
                KillTimer(hWnd, 1);
                HTimer = false;
            }
            if (VTimer) {
                KillTimer(hWnd, 2);
                VTimer = false;
            }
        }
        else if (wParam == 'P') {
            if (HTimer) { 
                KillTimer(hWnd, 1); 
                HTimer = false;
            }
            if (VTimer) { 
                KillTimer(hWnd, 2);
                VTimer = false;
            }
            if (STimer) { 
                KillTimer(hWnd, 3);
                STimer = false;
            }
        }
        else if (wParam == VK_OEM_PLUS) {
            MoveSpeedX += 5;
            MoveSpeedY += 5;
        }
        else if (wParam == VK_OEM_MINUS) {
            MoveSpeedX -= 5;
            MoveSpeedY -= 5;

            if (MoveSpeedX < 0 && MoveSpeedY < 0) {
                MoveSpeedX = MoveSpeedY = 0;
            }
        }

        InvalidateRect(hWnd, NULL, true);

        break;
    case WM_LBUTTONDOWN :

        point.x = LOWORD(lParam);
        point.y = HIWORD(lParam);

        InvalidateRect(hWnd, NULL, true);

        break;
    case WM_TIMER :
        switch (wParam) {
        case 1 :
            if (reversX == false) {
                point.x += MoveSpeedX;
            }
            else {
                point.x -= MoveSpeedX;
            }
            if (point.x > rectView.right - cell) {
                point.x = rectView.right - cell;
                reversX = true;
                point.y += cell * 2;
            }
            if (point.x < rectView.left + cell) {
                point.x = rectView.left + cell;
                reversX = false;

                point.y += cell * 2;
            }
            break;
        case 2:
            if (reversY == false) {
                point.y += MoveSpeedX;
            }
            else {
                point.y -= MoveSpeedY;
            }
            if (point.y > rectView.bottom - cell) {
                point.y = rectView.bottom - cell;
                reversY = true;
                point.x += cell * 2;
            }
            if (point.y < rectView.top + cell) {
                point.y = rectView.top + cell;
                reversY = false;
                point.x += cell * 2;
            }
            if (point.x > rectView.right - cell) {
                point.x = 0;
                point.x += cell * 2;
            }
            break;
        case 3:
            if (reversY == false) {
                point.y += MoveSpeedX;
            }
            else {
                point.y -= MoveSpeedY;
            }
            if (reversX == false) {
                point.x += MoveSpeedX;
            }
            else {
                point.x -= MoveSpeedX;
            }
            
            if (point.y > rectView.bottom - cell) {
                point.y = rectView.bottom - cell;
                reversY = true;
            }
            if (point.y < rectView.top + cell) {
                point.y = rectView.top + cell;
                reversY = false;
            }
            if (point.x > rectView.right - cell) {
                point.x = rectView.right - cell;
                reversX = true;
            }
            if (point.x < rectView.left + cell) {
                point.x = rectView.left + cell;
                reversX = false;
            }
            break;
        }


        InvalidateRect(hWnd, NULL, true);
        break;
    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);

        if (RFlag) {
            Rectangle(hDC, point.x - cell, point.y - cell, point.x + cell, point.y + cell);
        }
        else if (EFlag) {
            Ellipse(hDC, point.x - cell, point.y - cell, point.x + cell, point.y + cell);
        }
        else if (TFlag) {
            POINT pts[3] = {
                {point.x, point.y - cell},
                {point.x + cell , point.y + cell},
                {point.x - cell, point.y + cell}
            };
            Polygon(hDC, pts, 3);
        }

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_SIZE:
        height = HIWORD(lParam);
        wide = LOWORD(lParam);
        InvalidateRect(hWnd, NULL, true);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}