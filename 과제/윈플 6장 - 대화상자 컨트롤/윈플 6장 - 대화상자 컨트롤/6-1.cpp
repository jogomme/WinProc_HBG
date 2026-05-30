#define _CRT_SECURE_NO_WARNINGS

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment (lib, "msimg32.lib")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include<iostream>
#include  "resource.h"

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
BOOL CALLBACK Dlalog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

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
    WndClass.lpszMenuName = NULL;   // 메뉴 이름 !!
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

void Move(int x, int y);

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
HBITMAP g_hImg;    // 불러온 비트맵 이미지를 저장할 변수
BITMAP bmpInfo;    // 비트맵의 가로, 세로 크기 등 정보를 저장할 구조체

// 움지이는 도형의 시작점과 끝점 좌표
int StartX = 0;
int StartY = 0;
int EndX = 0;
int EndY = 0;

// 마우스의 현재 위치 좌표
int xPos = 0;
int yPos = 0;

// 콘솔 창이 열려있는지 여부를 추적하는 변수
bool isConsoleOpen = false;


// 도형 그리기, 움직이기, 크기 조절, 색상 변경, 도형 종류 선택, 격자 유무 등
bool isDraw = false;
bool isDown = false;

bool isMove = false;

bool isSmall = false;
bool isMedium = false;
bool isLarge = false;

bool isRed = false;
bool isGreen = false;
bool isBlue = false;

bool isRect = false;

bool isGrid = false;

int Size[3] = { 10, 50, 100 };

// 0: 빨강, 1: 초록, 2: 파랑
COLORREF colorSet[3] = { RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255) };
int colorIndex = -1;

int sizeIndex = 0;

int speed{10};


// --------------------------------------------------------
// 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    static RECT rectView;


    switch (uMsg) {
    case WM_CREATE:
        GetClientRect(hWnd, &rectView);
        // 1. 리소스에서 그림을 불러와 g_hImg에 저장합니다.
       // g_hImg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));

        // 2. 불러온 그림의 정보를 bmpInfo 구조체에 쏙 빼옵니다. (가로, 세로 크기 등)
        GetObject(g_hImg, sizeof(BITMAP), &bmpInfo);
        break;
    case WM_LBUTTONDOWN: {
        DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)Dlalog_Proc);
        break;
    }
    case WM_KEYDOWN:
        if (wParam == 'O' || wParam == 'o') { // 'O' 키를 누르면 콘솔 오픈!
            if (!isConsoleOpen) {
                // 1. 운영체제에 콘솔 창 생성을 요청합니다.
                AllocConsole();

                // 2. 표준 출력(stdout)과 표준 입력(stdin)의 방향을 콘솔 창으로 재지정합니다.
                // C++의 cout과 C의 printf가 모두 이 콘솔을 바라보게 만듭니다.
                freopen("CONOUT$", "w", stdout);
                freopen("CONIN$", "r", stdin);

                // iostream과 콘솔 동기화
                ios_base::sync_with_stdio(false);

                isConsoleOpen = true;
                cout << "[서버 디버그] 콘솔 창이 활성화되었습니다.\n";
            }
        }
        else if (wParam == 'P' || wParam == 'p') { // 'P' 키를 누르면 콘솔 클로즈!
            if (isConsoleOpen) {
                cout << "[서버 디버그] 콘솔 창을 닫습니다.\n";

                // 3. 열려있던 표준 스트림을 안전하게 닫아줍니다.
                fclose(stdout);
                fclose(stdin);

                // 4. 운영체제에 콘솔 자원을 반납하여 화면에서 지웁니다.
                FreeConsole();

                isConsoleOpen = false;
            }
        }
        break;

    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);


        // 계속해서 화면을 다시 그릴 때 활용 <- 더블 버퍼링
        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);
        // memDC를 사용해서 그리기



        // 그린것을 hDC로 복사
        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);
        // 다 쓴 브러쉬들 해제하기
        SelectObject(memDC, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
        return 0;

        break;
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

BOOL CALLBACK Dlalog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    PAINTSTRUCT ps;

    static RECT DrawArea;
	static RECT rectView;

    switch (iMsg) {
    case WM_INITDIALOG:
		SetRect(&DrawArea, StartX, StartY, EndX, EndY);
        GetClientRect(hDlg, &rectView);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: //--- 버튼
            MessageBox(hDlg, L"test", L"test, ", MB_OK);
            break;
        case IDCANCEL: //--- 버튼
            EndDialog(hDlg, 0);
            break;
        case IDC_BUTTON1:
            cout << "Draw" << endl;
            isDraw = true;
            isMove = false;
			KillTimer(hDlg, 1);
            break;
        case IDC_BUTTON2:
            cout << "Move" << endl;
            isDraw = false;
			isMove = true;

            xPos = StartX;
			yPos = StartY;

			SetTimer(hDlg, 1, 16, NULL);

            break;
        case IDC_BUTTON3:
            cout << "Speed Up : " << speed << '\n';
            isDraw = false;
            speed += 10;
            break; 
        case IDC_BUTTON4:
            cout << "Speed Down : " << speed << '\n';
            isDraw = false;
            if ((speed - 10) < 0) {
                speed = 0;
            }
            else {
                speed -= 10;
            }
            break;
        case IDC_BUTTON5:
            isDraw = false;
            cout << "Quit" << endl;
            EndDialog(hDlg, 0);
            break;
        case IDC_RADIO1: {
			cout << "Rect" << '\n';
			isRect = true;
            break;
        }
        case IDC_RADIO2: {
			cout << "Circle" << '\n';
            isRect = false;
            break;
        }
        case IDC_RADIO3: {
            cout << "small" << '\n';
			sizeIndex = 0;
            break;
        }
        case IDC_RADIO4: {
			cout << "Medium" << '\n';
            sizeIndex = 1;
            break;
        }
        case IDC_RADIO5: {
			cout << "Large" << '\n';
            sizeIndex = 2;
            break;
        }
        case IDC_RADIO6: {
			cout << "Grid on" << '\n';
			isGrid = true;
			break;
        }
        case IDC_RADIO7: {
			cout << "Grid off" << '\n';
			isGrid = false;
            break;
        }
        case IDC_RADIO8: {
			cout << "Red" << '\n';
			colorIndex = 0;
            break;
        }
        case IDC_RADIO9: {
			cout << "Green" << '\n';
			colorIndex = 1;
            break;
        }
        case IDC_RADIO10: {
			cout << "Blue" << '\n';
			colorIndex = 2;
            break;
        }
        }
        InvalidateRect(hDlg, NULL, false);
        break;
    case WM_LBUTTONDOWN: {
		isDown = true;
        if (isDraw) {
            StartX = LOWORD(lParam);
            StartY = HIWORD(lParam);
            EndX = StartX;
            EndY = StartY;
        }
		InvalidateRect(hDlg,NULL, false);
		break;
    }
    case WM_MOUSEMOVE: {
        if (isDown && isDraw) {
            EndX = LOWORD(lParam);
            EndY = HIWORD(lParam);
            InvalidateRect(hDlg, NULL, false);
        }
        break;
    }
    case WM_LBUTTONUP: {
        isDown = false;
		int size = Size[sizeIndex] / 2;
        SetRect(&DrawArea, StartX - size, StartY- size, EndX + size, EndY + size);
		InvalidateRect(hDlg, NULL, false);
        break;
    }
    case WM_TIMER: {
        switch (wParam)
        {
        case 1: {
            Move(EndX, EndY);
			if (StartX == EndX && StartY == EndY) {
				KillTimer(hDlg, 1);
			}
            break;
        }
        }
		InvalidateRect(hDlg,NULL, false);
        break;
    }
    case WM_PAINT: {
        hDC = BeginPaint(hDlg, &ps);

        // 계속해서 화면을 다시 그릴 때 활용 <- 더블 버퍼링
        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC,rectView.right, rectView.right);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);
        // memDC를 사용해서 그리기

		

        if (isDraw || isMove) {

            if (isGrid) {
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
                HPEN oldPen = (HPEN)SelectObject(memDC, hPen);

                MoveToEx(memDC, StartX, StartY, NULL);
                LineTo(memDC, EndX, EndY);

                DeleteObject(hPen);
            }

			int size = Size[sizeIndex] / 2;

			HBRUSH hBrush = CreateSolidBrush(colorSet[colorIndex]);
			HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

            if (isRect) {
                Rectangle(memDC, StartX - size, StartY - size, StartX + size, StartY + size);
            }
            else if (!isRect) {
                Ellipse(memDC, StartX - size, StartY - size, StartX + size, StartY + size);
            }

			DeleteObject(hBrush);
        }

        // 그린것을 hDC로 복사
        BitBlt(hDC, 0, 0, rectView.right, rectView.bottom, memDC, 0, 0, SRCCOPY);
        // 다 쓴 브러쉬들 해제하기
        SelectObject(memDC, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(memDC);

        EndPaint(hDlg, &ps);
        break;
    }
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    }
    return 0;
}

void Move(int x, int y) {
	double dx = x - StartX;
	double dy = y - StartY;

	double distance = sqrt(dx * dx + dy * dy);

    if (distance <= speed)
    {
        StartX = x;
        StartY = y;
        return;
    }

    double dirX = dx / distance;
    double dirY = dy / distance;

	StartX += dirX * speed;
	StartY += dirY * speed;
}