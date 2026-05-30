#define _CRT_SECURE_NO_WARNINGS

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib, "Msimg32.lib") // TransparentBlt 사용 시 필요

#include <math.h>
#include <windows.h>
#include <tchar.h>
#include <random>    
#include <iostream>
#include <algorithm>
#include  "resource.h"

using namespace std;
int wide{ 1200 };
int height{ 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_int_distribution<int> speedDist(1, 10);
uniform_int_distribution<int> PointDist(30, wide - 30);


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

void SetMap();

void DrawTransparentBitmap(HDC destDC, HBITMAP hBmp, int destX, int destY, int destW, int destH);

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
HBITMAP g_hImg;    // 불러온 비트맵 이미지를 저장할 변수
HBITMAP g_BackGround; // 배경

struct ch
{
    int x;
    int y;
    int speed;

    int frame;
};

ch Ani1[20];
ch Ani2[20];

HBITMAP g_Anime1[7]; // 도는 애니메이션
HBITMAP g_Anime2[7]; // 좌우 애니메이션

BITMAP bmpInfo;    // 비트맵의 가로, 세로 크기 등 정보를 저장할 구조체

RECT magRect;
POINT startPt, endPt;
bool rectActive = false;
bool isDrag = false;
bool isDraw = false;
bool isin = false;
bool RDown = false;

// 스프라이트 크기 상수
const int SPRITE_W = 40;
const int SPRITE_H = 40;

// 상태 관리: 0 = Ani2 (기본), 1 = Ani1 (대체, Ani2가 사각형 밑바닥에 닿을 때 전환)
int stateArr[20] = { 0 };
int life1[20] = { 0 }; // Ani1 유지 프레임 카운터

// 루버밴드 관련
RECT focusPrev = { 0,0,0,0 };
bool hasFocusRect = false;

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

        g_BackGround = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
        g_Anime1[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));
        g_Anime1[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));
        g_Anime1[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));
        g_Anime1[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP11));
        g_Anime1[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP12));
        g_Anime1[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP13));
        g_Anime1[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP14));

        g_Anime2[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
        g_Anime2[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
        g_Anime2[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP4));
        g_Anime2[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP5));
        g_Anime2[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
        g_Anime2[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));

        SetMap();
        // 타이머 시작 (최초 실행)
        SetTimer(hWnd, 1, 16, NULL);
        // 2. 불러온 그림의 정보를 bmpInfo 구조체에 쏙 빼옵니다. (가로, 세로 크기 등)
        GetObject(g_BackGround, sizeof(BITMAP), &bmpInfo);
        break;
    case WM_KEYDOWN:
        if (wParam == 'Q') {
            PostQuitMessage(0);
        }
        else if (wParam == 'D') {
            isDraw = false;
            rectActive = false;
            magRect = {};
            // 루버밴드 있으면 지우기
            if (hasFocusRect) {
                HDC hdc = GetDC(hWnd);
                DrawFocusRect(hdc, &focusPrev);
                ReleaseDC(hWnd, hdc);
                hasFocusRect = false;
            }
        }
        else if (wParam == 'R') {
            SetMap();
        }
        else if (wParam == 'P') {
            SetTimer(hWnd, 1, 16, NULL);
        }
        break;
    case WM_TIMER: {
        if (wParam == 1) {

            // 정규화된 사각형 계산
            int rleft = min(magRect.left, magRect.right);
            int rright = max(magRect.left, magRect.right);
            int rtop = min(magRect.top, magRect.bottom);
            int rbottom = max(magRect.top, magRect.bottom);
            bool rectExists = rectActive || isDrag;

            for (int i = 0; i < 20; ++i) {
                // 프레임 갱신
                Ani1[i].frame = (Ani1[i].frame + 1) % 7;
                Ani2[i].frame = (Ani2[i].frame + 1) % 6;

                // Ani2 (기본) 동작: 낙하 처리 및 사각형 밑바닥 충돌 시 Ani1으로 전환
                if (stateArr[i] == 0) {
                    int nextY2 = Ani2[i].y + Ani2[i].speed;

                    if (rectExists && (Ani2[i].x + SPRITE_W > rleft && Ani2[i].x < rright)) {
                        int targetY = rbottom - SPRITE_H;
                        if (Ani2[i].y < targetY) {
                            if (nextY2 >= targetY) {
                                // Ani2가 사각형 밑바닥에 닿음 -> Ani1으로 전환
                                Ani2[i].y = targetY;
                                stateArr[i] = 1;
                                life1[i] = true; // Ani1 유지 시간(프레임)
                                Ani1[i].x = Ani2[i].x;
                                Ani1[i].y = Ani2[i].y;
                                Ani1[i].frame = 0;
                                // Ani2는 다시 위에서 대기
                                Ani2[i].y = -SPRITE_H;
                                Ani2[i].x = PointDist(gen);
                                Ani2[i].speed = max(1, speedDist(gen));
                            }
                            else {
                                Ani2[i].y = nextY2;
                            }
                        }
                        else {
                            // 이미 target에 있다면 즉시 전환
                            stateArr[i] = 1;
                            life1[i] = true;
                            Ani1[i].x = Ani2[i].x;
                            Ani1[i].y = Ani2[i].y;
                            Ani1[i].frame = 0;
                            Ani2[i].y = -SPRITE_H;
                            Ani2[i].x = PointDist(gen);
                            Ani2[i].speed = max(1, speedDist(gen));
                        }
                    }
                    else {
                        // 사각형 영향 없으면 평상시 낙하
                        Ani2[i].y = nextY2;
                    }
                }
                else {
                    // state == 1 : Ani1 표시 상태 (고정, life1 카운트 감소)
                    if (life1[i] > 0) {
                        life1[i] = true;
                    }
                    if (!(rectExists && (Ani1[i].x + SPRITE_W > rleft && Ani1[i].x < rright))) {
                        life1[i] = false;
                        stateArr[i] = 0;
                        Ani2[i].y = Ani1[i].y;
                        Ani2[i].x = Ani1[i].x;
                        Ani2[i].speed = max(1, speedDist(gen));
                    }
                }

                // 화면 아래로 나가면 Ani2 리셋 (Ani1 상태일 때는 영향 없음)
                if (Ani2[i].y >= height) {
                    Ani2[i].y = -SPRITE_H;
                    Ani2[i].x = PointDist(gen);
                    Ani2[i].speed = max(1, speedDist(gen));
                }
                // Ani1는 표시 상태에서 특별히 아래로 내려가지 않음 (원하면 여기서 움직임 추가)
            }
        }

        InvalidateRect(hWnd, NULL, FALSE);
        break;
    }
    case WM_LBUTTONDOWN: {
        // 루버밴드 시작. 기존 루버밴드 있으면 지운다.
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        // 기존 루버밴드 지우기
        if (hasFocusRect) {
            HDC hdc = GetDC(hWnd);
            DrawFocusRect(hdc, &focusPrev);
            ReleaseDC(hWnd, hdc);
            hasFocusRect = false;
        }

        int rleft = min(magRect.left, magRect.right);
        int rright = max(magRect.left, magRect.right);
        int rtop = min(magRect.top, magRect.bottom);
        int rbottom = max(magRect.top, magRect.bottom);

        if (rleft <= x && x <= rright && rtop <= y && y <= rbottom) {
            // 사각형 내부 클릭: 드래그 시작하지 않음 (사각형 유지)
            isDrag = false;
            isDraw = true;
            isin = true;
        }
        else {
            // 새 사각형 그리기 시작
            isDrag = true;
            isDraw = true;
            rectActive = false;
            startPt = { x, y };
            endPt = startPt;
            magRect = { startPt.x, startPt.y, endPt.x, endPt.y };
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_MOUSEMOVE: {

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        POINT pt = { x, y };

        if (isDrag) {
            // 루버밴드 (고무줄) 효과: 이전 루버밴드 지우고 새로 그림
            RECT newRect;
            newRect.left = min(startPt.x, x);
            newRect.top = min(startPt.y, y);
            newRect.right = max(startPt.x, x);
            newRect.bottom = max(startPt.y, y);

            HDC hdc = GetDC(hWnd);
            if (hasFocusRect) {
                DrawFocusRect(hdc, &focusPrev); // 이전 루버밴드 지우기
            }
            DrawFocusRect(hdc, &newRect); // 새 루버밴드 그리기
            ReleaseDC(hWnd, hdc);

            focusPrev = newRect;
            hasFocusRect = true;

            endPt = pt;
            magRect = newRect; // 정규화된 값 저장
            InvalidateRect(hWnd, NULL, FALSE);
        }
        // 드래그 아닐 경우 루버밴드는 유지(화면에 직접 그려진 루버밴드는 GetDC 으로 처리되어 있음)
        break;
    }
    case WM_LBUTTONUP: {
        if (isDrag) {
            // 루버밴드 지우기
            if (hasFocusRect) {
                HDC hdc = GetDC(hWnd);
                DrawFocusRect(hdc, &focusPrev); // 지우기
                ReleaseDC(hWnd, hdc);
                hasFocusRect = false;
            }

            isDrag = false;
            // magRect는 WM_MOUSEMOVE에서 정규화되어 저장됨
            rectActive = true;
        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }
    case WM_RBUTTONDOWN: {
        // 우클릭으로 사각형 내부에서 토글 동작 구현(예: RDown 토글)
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        int rleft = min(magRect.left, magRect.right);
        int rright = max(magRect.left, magRect.right);
        int rtop = min(magRect.top, magRect.bottom);
        int rbottom = max(magRect.top, magRect.bottom);

        if (rleft <= x && x <= rright && rtop <= y && y <= rbottom) {
            RDown = !RDown;
        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }
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

        HDC imgDC = CreateCompatibleDC(hDC);

        // 배경 그리기
        
        if (g_BackGround) {
            SelectObject(imgDC, g_BackGround);
            GetObject(g_BackGround, sizeof(BITMAP), &bmpInfo);
            StretchBlt(memDC, 0, 0, wide, height, imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
        }
       

        // Ani 그리기: stateArr에 따라 Ani1 또는 Ani2 표시
        for (int i = 0; i < 20; ++i) {
            if (stateArr[i] == 1) {
                int frameIdx = Ani1[i].frame % 7;
                DrawTransparentBitmap(memDC, g_Anime1[frameIdx], Ani1[i].x, Ani1[i].y, SPRITE_W, SPRITE_H);
            }
            else {
                int frameIdx = Ani2[i].frame % 6;
                DrawTransparentBitmap(memDC, g_Anime2[frameIdx], Ani2[i].x, Ani2[i].y, SPRITE_W, SPRITE_H);
            }
        }

        if (RDown) {

            InvertRect(memDC, &magRect);
        }
        HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, hBrush);

        if (isDraw || rectActive) {
            // Rectangle은 left, top, right, bottom가 정규화되지 않았을 수 있으므로 정규화하여 그림
            int left = min(startPt.x, endPt.x);
            int right = max(startPt.x, endPt.x);
            int top = min(startPt.y, endPt.y);
            int bottom = max(startPt.y, endPt.y);
            Rectangle(memDC, left, top, right, bottom);
        }
        SelectObject(memDC, hOldBrush);

        DeleteDC(imgDC);

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

void SetMap()
{
    for (int i = 0; i < 20; ++i) {

        Ani1[i].frame = 0;
        Ani2[i].frame = 0;

        Ani1[i].x = -50;
        Ani2[i].x = PointDist(gen);

        Ani1[i].y = 0;
        Ani2[i].y = 0;

        Ani1[i].speed = speedDist(gen);
        Ani2[i].speed = speedDist(gen);

        isDrag = false;
        isDraw = false;

        stateArr[i] = 0;
        life1[i] = 0;
    }
}

void DrawTransparentBitmap(HDC destDC, HBITMAP hBmp, int destX, int destY, int destW, int destH)
{
    if (!hBmp) return;

    HDC srcDC = CreateCompatibleDC(destDC);
    HBITMAP oldBmp = (HBITMAP)SelectObject(srcDC, hBmp);

    BITMAP bm = {};
    GetObject(hBmp, sizeof(BITMAP), &bm);

    // 투명색: 소스 비트맵의 왼쪽 상단 픽셀 색을 투명색으로 사용
    COLORREF transColor = RGB(255, 0, 255); // 기본값(마젠타)
    // GetPixel은 선택된 비트맵에서 색을 읽는다.
    transColor = GetPixel(srcDC, 0, 0);

    // TransparentBlt의 width/height는 픽셀 크기 (출력될 너비와 높이)
    TransparentBlt(destDC, destX, destY, destW, destH,
        srcDC, 0, 0, bm.bmWidth, bm.bmHeight,
        transColor);

    SelectObject(srcDC, oldBmp);
    DeleteDC(srcDC);
}