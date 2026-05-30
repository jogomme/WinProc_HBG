#define _CRT_SECURE_NO_WARNINGS

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include <iostream>
#include "resource.h"

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
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = IpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(IpszClass, IpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, wide, height, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return Message.wParam;
}

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
HBITMAP g_hImg;    // 불러온 비트맵 이미지를 저장할 변수
BITMAP bmpInfo;    // 비트맵의 가로, 세로 크기 등 정보를 저장할 구조체

int drawWidth = 0;  // 그릴 가로 크기
int drawHeight = 0; // 그릴 세로 크기
bool isFill = false; // 화면 꽉 채우기 모드인지 여부

int Row = 1; // 화면 분할 (줄)
int Cal = 1; // 화면 분할 (칸)

int selRow = -1; // 마우스로 선택된 줄 (-1은 선택 안됨)
int selCal = -1; // 마우스로 선택된 칸 (-1은 선택 안됨)

bool isR_Grid[10][10] = { false, }; // 각 칸마다 색상이 반전되었는지 기억하는 2차원 배열

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
        g_hImg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));

        // 2. 불러온 그림의 정보를 bmpInfo 구조체에 빼옵니다.
        GetObject(g_hImg, sizeof(BITMAP), &bmpInfo);

        // 3. 그림이 처음 그려질 때 원본 크기로 세팅
        drawWidth = bmpInfo.bmWidth;
        drawHeight = bmpInfo.bmHeight;
        break;

    case WM_KEYDOWN: {
        // A 키: 화면 꽉 차게 <-> 원본 크기 토글
        if (wParam == 'A') {
            isFill = !isFill;

            if (isFill == true) {
                drawWidth = wide;
                drawHeight = height;
            }
            else {
                drawWidth = bmpInfo.bmWidth;
                drawHeight = bmpInfo.bmHeight;
            }
        }
        // 1, 2, 4, 6 분할 키 설정
        else if (wParam == '1') {
            Row = 1; Cal = 1; selRow = -1; selCal = -1;
        }
        else if (wParam == '2') {
            Row = 1; Cal = 2; selRow = -1; selCal = -1;
        }
        else if (wParam == '4') {
            Row = 2; Cal = 2; selRow = -1; selCal = -1;
        }
        else if (wParam == '6') {
            Row = 2; Cal = 3; selRow = -1; selCal = -1;
        }
        // R 키: 선택된 칸이 있을 때만 그 칸의 색상 스위치 켜기/끄기
        else if (wParam == 'R') {
            if (selRow != -1 && selCal != -1) {
                isR_Grid[selRow][selCal] = !isR_Grid[selRow][selCal];
            }
        }
        // Q 키: 종료
        else if (wParam == 'Q') {
            PostQuitMessage(0);
        }
        // + 키: 조금씩 크게 
        else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
            isFill = false;
            drawWidth += 20;
            drawHeight += 20;
        }
        // - 키: 조금씩 작게
        else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
            isFill = false;
            // 너무 작아져서 뒤집히지 않게 방어막 설정
            if (drawWidth > 20 && drawHeight > 20) {
                drawWidth -= 20;
                drawHeight -= 20;
            }
        }

        InvalidateRect(hWnd, NULL, true);
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        int cellWidth = wide / Cal;
        int cellHeight = height / Row;

        // 클릭한 곳이 몇 번째 행과 열인지 계산하여 저장
        selCal = x / cellWidth;
        selRow = y / cellHeight;

        InvalidateRect(hWnd, NULL, true);
        break;
    }

    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);

        // 더블 버퍼링: 전체 화면 크기의 메모리 DC 생성
        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        // 배경을 하얀색으로 깔끔하게 지우기
        HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &rectView, bgBrush);
        DeleteObject(bgBrush);

        // 비트맵 전용 임시 스케치북(imgDC) 만들기
        HDC imgDC = CreateCompatibleDC(hDC);
        HBITMAP oldImg = (HBITMAP)SelectObject(imgDC, g_hImg);

        int cellWidth = wide / Cal;
        int cellHeight = height / Row;

        // 이중 for문으로 화면 쪼개서 그림 그리기
        for (int r = 0; r < Row; ++r) {
            for (int c = 0; c < Cal; ++c) {
                int startX = c * cellWidth;
                int startY = r * cellHeight;

                // 개별 색상 반전 처리 (isR_Grid 확인)
                DWORD ropOption = SRCCOPY;
                if (isR_Grid[r][c] == true) {
                    ropOption = NOTSRCCOPY;
                }

                // 그려질 실제 크기 설정
                int currentDrawWidth = cellWidth;
                int currentDrawHeight = cellHeight;

                // 1등분일 때는 +,-,a 키로 조절한 크기를 적용
                if (Row == 1 && Cal == 1) {
                    currentDrawWidth = drawWidth;
                    currentDrawHeight = drawHeight;
                }

                // 그림 그리기
                StretchBlt(memDC, startX, startY, currentDrawWidth, currentDrawHeight, imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, ropOption);

                // 마우스로 선택된 칸이라면 두꺼운 빨간색 테두리 그리기
                if (r == selRow && c == selCal) {
                    HPEN redPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
                    HPEN oldPen = (HPEN)SelectObject(memDC, redPen);

                    // 안쪽이 투명한 브러시 선택
                    HBRUSH clearBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, clearBrush);

                    Rectangle(memDC, startX, startY, startX + currentDrawWidth, startY + currentDrawHeight);

                    SelectObject(memDC, oldBrush);
                    SelectObject(memDC, oldPen);
                    DeleteObject(redPen);
                }
            }
        }

        // 비트맵 전용 스케치북 해제
        SelectObject(imgDC, oldImg);
        DeleteDC(imgDC);

        // 완성된 더블 버퍼링 메모리 DC를 실제 화면(hDC)에 복사
        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);

        // 메모리 DC 해제
        SelectObject(memDC, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_SIZE:
        height = HIWORD(lParam);
        wide = LOWORD(lParam);
        GetClientRect(hWnd, &rectView); // 화면 크기가 변하면 rectView도 갱신
        InvalidateRect(hWnd, NULL, true);
        break;

    case WM_DESTROY:
        DeleteObject(g_hImg); // 비트맵 메모리 누수 방지
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}