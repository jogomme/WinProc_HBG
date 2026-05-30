#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random> 
#include<iostream>
#include "resource.h"

using namespace std;
int wide{ 1200 };
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
    WndClass.style = CS_HREDRAW | CS_VREDRAW;;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 배경 색깔

    WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); // 메뉴 
    
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

int roadWidth = 200;

// 0: 빨강(정지), 1: 파랑(이동), 2: 노랑(대기)
int vLight = 0; // 상하(수직) 신호등 (초기값: 빨강)
int hLight = 1; // 좌우(수평) 신호등 (초기값: 파랑)
int LightCounter = 0;

bool isAuto = true;
bool isLButtonDown = false;
bool isRButtonDown = false;

COLORREF Light[3]
{
    {RGB(255,0,0)},
    {RGB(0,255,0)},
    {RGB(255,255,0)}
};

// 1(좌상)   2(우상)
// 3(좌하)   4(우하)
RECT area[4]{};

struct Car 
{
    double x, y;
    int dir;       // 0: 아래, 1: 위, 2: 왼, 3: 오른
    double orignalSpeed;
    double speed;
    int width, height; // 자동차의 가로, 세로 길이
};
Car cars[8];

struct Pedestrian {
    double x, y;
    double speed;
    bool isWalking;
    int dir;
    int state;
};

Pedestrian ped = { 0, 0, 4.0, false, 0, 0 }; 

// --------------------------------------------------------
//  타이머 전용 함수
// --------------------------------------------------------
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
// --------------------------------------------------------
//  타이머 전용 함수
// --------------------------------------------------------
const int CarMove{ 1 };
const int AutoLight{ 2 };

// --------------------------------------------------------
// 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;


    switch (uMsg) {
    case WM_CREATE: {
        int cx = wide / 2;
        int cy = height / 2;
        int rw = roadWidth / 2;    // 도로 너비의 절반 (100)

        ped.x = cx - rw;
        ped.y = cy - rw;
        ped.state = 0;

        // 사거리를 만들기 위한 4개의 모서리 구역 계산
        area[0] = { 0, 0, cx - rw, cy - rw };               // 좌상
        area[1] = { cx + rw, 0, wide, cy - rw };            // 우상
        area[2] = { 0, cy + rw, cx - rw, height };          // 좌하
        area[3] = { cx + rw, cy + rw, wide, height };       // 우하

        // 자동차 초기화 
        int carW = 60, carH = 30; // 가로로 가는 차 기준 크기

        // 0, 1번차: 아래로 이동 (Down)
        cars[0] = { (double)(cx - rw + 10), -100, 0, 3.0, 3.0, carH, carW };
        cars[1] = { (double)(cx - rw + 10), -250, 0, 4.0, 4.0, carH, carW };
        // 2, 3번차: 위로 이동 (Up)
        cars[2] = { (double)(cx + 10), (double)height + 100, 1, 3.5, 3.5, carH, carW };
        cars[3] = { (double)(cx + 10), (double)height + 250, 1, 2.5, 2.5, carH, carW };
        // 4, 5번차: 왼쪽으로 이동 (Left)
        cars[4] = { (double)wide + 100, (double)(cy - rw + 10), 2, 4.0, 4.0, carW, carH };
        cars[5] = { (double)wide + 250, (double)(cy - rw + 10), 2, 3.0, 3.0,carW, carH };
        // 6, 7번차: 오른쪽으로 이동 (Right)
        cars[6] = { -100, (double)(cy + 10), 3, 3.5, 3.5, carW, carH };
        cars[7] = { -250, (double)(cy + 10), 3, 2.5, 2.5, carW, carH };

        SetTimer(hWnd, AutoLight, 1000, (TIMERPROC)TimerProc);
        SetTimer(hWnd, CarMove, 30, (TIMERPROC)TimerProc);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_MENU_QUIT: {
            PostQuitMessage(0);
            DestroyWindow(hWnd);
            break;
        }
        case ID_LIGHT_H_BLUE: { 
            vLight = 0; 
            hLight = 1; 
            LightCounter = 5; 
            break;
        }
        case ID_LIGHT_H_RED: {
            vLight = 1; 
            hLight = 0;
            LightCounter = 0; 
            break;
        }
        case ID_MENU_AUTO: {
            if (!isAuto) {
                SetTimer(hWnd, AutoLight, 1000, (TIMERPROC)TimerProc);
                isAuto = true;
                cout << "신호등 자동으로 바뀜" << '\n';
            }
            break;
        }
        case ID_MENU_STOP: {
            if (isAuto) {
                KillTimer(hWnd, AutoLight);
                isAuto = false;
                cout << "신호등 색깔 고정 " << '\n';
            }
            break;
        }
        } 
        InvalidateRect(hWnd, NULL, true);
        break;
    }
    case WM_RBUTTONDOWN: {
        cout << "우클릭 감지 " << '\n';
    }
    case WM_LBUTTONDOWN: {
        cout << "좌클릭 감지" << '\n';
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        if ((x >= 35 && x <= 75 && y >= 20 && y <= 60)
            || (x >= wide - 175 + 55 && x <= wide - 175 + 40  + 55&& y >= height - 60 && y <= height - 20)
            ) {
            LightCounter = 5;
            vLight = 0;
            hLight = 1;
        }
        else if (x >= wide - 175 + 55 * 2 && x <= wide - 175 + 40 + 55 * 2&& y >= height - 60 && y <= height - 20) {
            LightCounter = 9;
            vLight = 0;
            hLight = 2;
        }
        else if ((x >= 35 + 55 && x <= 75 + 55 && y >= 20 && y <= 60) ||
            (x >= wide - 175 && x <= wide - 175 + 40 && y >= height - 60 && y <= height - 20)
            ) {
            LightCounter = 0;
            vLight = 1;
            hLight = 0;
        }
        else if (x >= 35 + 55 * 2 && x <= 75 + 55 * 2 && y >= 20 && y <= 60) {
            LightCounter = 4;
            vLight = 2;
            hLight = 0;
        }
        else if (isLButtonDown == false) {
            isLButtonDown = true;
            if (isAuto) {
                KillTimer(hWnd, AutoLight);
                isAuto = false;
                cout << "신호등 색깔 고정 " << '\n';
            }
        }
        else {
            isLButtonDown = false;
            if (!isAuto) {
                SetTimer(hWnd, AutoLight, 1000, (TIMERPROC)TimerProc);
                isAuto = true;
                cout << "신호등 자동으로 바뀜" << '\n';
            }
        }
        break;
    }
    case WM_KEYDOWN:
        if (wParam == 'Q') {
            PostQuitMessage(0);
        }
        else if (wParam == VK_OEM_PLUS) {
            for (int i = 0; i < 8; ++i) {
                cars[i].speed += 1;
                cars[i].orignalSpeed = cars[i].speed;
                cout << "자동차 " << i << " 의 속도 : " << cars[i].speed << '\n';
            }
        }
        else if (wParam == VK_OEM_MINUS) {
            for (int i = 0; i < 8; ++i) {
                cars[i].speed -= 1;
                cars[i].orignalSpeed = cars[i].speed;
                if (cars[i].speed < 0) {
                    cars[i].speed = 0;
                    cars[i].orignalSpeed = 0;
                    cout << "자동차 " << i << " 의 속도 : " << cars[i].speed << '\n';
                }
            }
        }
        else if (wParam == 'A') {
            if (isAuto) {
                KillTimer(hWnd, AutoLight);
                isAuto = false;
                cout << "신호등 색깔 고정 " << '\n';
            }
            else {
                SetTimer(hWnd, AutoLight, 1000, (TIMERPROC)TimerProc);
                isAuto = true;
                cout << "신호등 자동으로 바뀜" << '\n';
            }
        }
        break;

    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);

        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        // 1. 도로 배경
        HBRUSH roadBrush = CreateSolidBrush(RGB(200, 200, 200));
        FillRect(memDC, &ps.rcPaint, roadBrush);
        DeleteObject(roadBrush);

        // 2. 보도 배경 및 신호등 영역
        HBRUSH blockBrush = CreateSolidBrush(RGB(255, 255, 255));
        for (int i = 0; i < 4; ++i) {
            FillRect(memDC, &area[i], blockBrush);
        }

        Rectangle(memDC, 10, 10, 200, 70);
        Rectangle(memDC, wide - 200, height - 70, wide - 10, height - 10);

        DeleteObject(blockBrush);

        // 3. 중앙 차선
        HPEN linePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0)); // 노란색 중앙선
        HPEN oldPen = (HPEN)SelectObject(memDC, linePen);

        // 세로 중앙선
        MoveToEx(memDC, wide / 2, 0, NULL);
        LineTo(memDC, wide / 2, height);
        // 가로 중앙선
        MoveToEx(memDC, 0, height / 2, NULL);
        LineTo(memDC, wide, height / 2);

        SelectObject(memDC, oldPen);
        DeleteObject(linePen);


        // 4. 자동차 그리기
        HBRUSH CarBrush = CreateSolidBrush(RGB(0, 0, 255));
        HBRUSH oldCarBrush = (HBRUSH)SelectObject(memDC, CarBrush);

        for (int i = 0; i < 8; ++i) {
            int cx = (int)cars[i].x;
            int cy = (int)cars[i].y;
            int cw = cars[i].width;
            int ch = cars[i].height;

            Rectangle(memDC, cx, cy, cx + cw, cy + ch);

            if (cx + cw > wide && cx < wide) Rectangle(memDC, cx - wide, cy, cx - wide + cw, cy + ch);
            else if (cx < 0 && cx + cw > 0) Rectangle(memDC, cx + wide, cy, cx + wide + cw, cy + ch);

            if (cy + ch > height && cy < height) Rectangle(memDC, cx, cy - height, cx + cw, cy - height + ch);
            else if (cy < 0 && cy + ch > 0) Rectangle(memDC, cx, cy + height, cx + cw, cy + height + ch);
        }

        SelectObject(memDC, oldCarBrush);
        DeleteObject(CarBrush);


        // 신호등 그리기
        HBRUSH offBrush = CreateSolidBrush(RGB(255, 255, 255)); // 꺼진 상태의 어두운 색
        HBRUSH onBrush;

        for (int i = 0; i < 3; ++i) {
            if (vLight == i) onBrush = CreateSolidBrush(Light[i]);
            else onBrush = offBrush;

            SelectObject(memDC, onBrush);
            Ellipse(memDC, 35 + (i * 55), 20, 35 + (i * 55) + 40, 60);

            if (vLight == i) DeleteObject(onBrush); 
        }

        for (int i = 0; i < 3; ++i) {
            if (hLight == i) onBrush = CreateSolidBrush(Light[i]);
            else onBrush = offBrush;

            SelectObject(memDC, onBrush);
            Ellipse(memDC, (wide - 175) + (i * 55), height - 60, (wide - 175) + (i * 55) + 40, height - 20);

            if (hLight == i) DeleteObject(onBrush);
        }

        DeleteObject(offBrush);

        // 보행자 그리기
        HBRUSH pedBrush = CreateSolidBrush(RGB(0, 255, 0));
        HBRUSH oldPedBrush = (HBRUSH)SelectObject(memDC, pedBrush);

        Ellipse(memDC, (int)ped.x - 10, (int)ped.y - 10, (int)ped.x +10 , (int)ped.y + 10);

        SelectObject(memDC, oldPedBrush);
        DeleteObject(pedBrush);
        

        // 그린것을 hDC로 복사
        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);
        SelectObject(memDC, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(memDC);

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



// --------------------------------------------------------
// 타이머 콜 백 함수 구현
// --------------------------------------------------------
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    if (idEvent == CarMove) {
        bool canMove = true;
        for (int i = 0; i < 8; ++i) {
            int j{};
            // 1. 방향에 따라 좌표 이동
            if (cars[i].dir == 0) {
                canMove = true;

                if (vLight == 0 || vLight == 2 || isLButtonDown) {
                    int stopLine = (height / 2) - (roadWidth / 2); 
                    if (cars[i].y + cars[i].height >= stopLine - 10 && cars[i].y + cars[i].height <= stopLine) {
                        canMove = false;
                    }
                }

                for (j = 0; j < 8;++j) {
                    if (i == j) continue;

                    if (cars[i].dir == cars[j].dir) {
                        if (cars[i].y <= cars[j].y && (cars[i].y + cars[i].height + 20) > cars[j].y) {
                            canMove = false;
                            break;
                        }
                    }
                }

                if (canMove) {
                    cars[i].y += cars[i].speed;      // 아래로
                }
            }
            else if (cars[i].dir == 1) {
                canMove = true;

                if (vLight == 0 || vLight == 2 || isLButtonDown) {
                    int stopLine = (height / 2) + (roadWidth / 2);
                    if (cars[i].y <= stopLine + 40 && cars[i].y >= stopLine) {
                        canMove = false;
                    }
                }

                for (j = 0; j < 8;++j) {
                    if (i == j) continue;

                    if (cars[i].dir == cars[j].dir) {

                        if (cars[i].y >= cars[j].y && (cars[i].y - cars[i].height - 20) <= cars[j].y) {
                            canMove = false;
                            break;
                        }
                    }
                }
                if(canMove) cars[i].y -= cars[i].speed; // 위로
            }
            else if (cars[i].dir == 2) {
                canMove = true;

                if (hLight == 0 || hLight == 2 || isLButtonDown) {
                    int stopLine = (wide / 2) + (roadWidth / 2);
                    if (cars[i].x <= stopLine + 40 && cars[i].x > stopLine) {
                        canMove = false;
                    }
                }

                for (j = 0; j < 8;++j) {
                    if (i == j) continue;

                    if (cars[i].dir == cars[j].dir) {

                        if (cars[i].x >= cars[j].x && (cars[i].x - cars[i].width - 20) <= cars[j].x) {
                            canMove = false;
                            break;
                        }
                    }
                }
                if (canMove) {
                    cars[i].x -= cars[i].speed; // 왼쪽으로
                }
            }
            else if (cars[i].dir == 3) {

                canMove = true;

                if (hLight == 0 || hLight == 2 || isLButtonDown) {
                    int stopLine = (wide / 2) - (roadWidth / 2);
                    if (cars[i].x + cars[i].width >= stopLine - 10 && cars[i].x + cars[i].width <= stopLine) {
                        canMove = false;
                    }
                }

                for (j = 0; j < 8;++j) {

                    if (i == j) continue;

                    if (cars[i].dir == cars[j].dir) {

                        if (cars[i].x <= cars[j].x && (cars[i].x + cars[i].width + 20) >= cars[j].x) {
                            canMove = false;
                            break;
                        }

                    }
                }
                if (canMove) {
                    cars[i].x += cars[i].speed; // 오른쪽으로
                }
            }


            if (cars[i].dir == 0 && cars[i].y > height) cars[i].y -= height;
            if (cars[i].dir == 1 && cars[i].y + cars[i].height < 0) cars[i].y += height;
            if (cars[i].dir == 2 && cars[i].x + cars[i].width < 0) cars[i].x += (wide);
            if (cars[i].dir == 3 && cars[i].x > wide) cars[i].x -= (wide);
        }
        int cx = wide / 2 + 10;
        int cy = height / 2 + 20;
        int rw = roadWidth / 2;
        int offset = 0; // 직각에 맞추기 위해 여백 0

        if (isLButtonDown && ped.dir < 5) {
            if (ped.state == 0) ped.dir = 5;      // 좌상단 -> 우하단
            else if (ped.state == 1) ped.dir = 6; // 우상단 -> 좌하단
            else if (ped.state == 2) ped.dir = 7; // 우하단 -> 좌상단
            else if (ped.state == 3) ped.dir = 8; // 좌하단 -> 우상단

            ped.isWalking = true;
        }
        // [평상시] 보행자가 멈춰 있을 때 신호등 체크 (기존과 동일)
        else if (!ped.isWalking && !isLButtonDown) {
            if (ped.state == 0 && vLight == 0) { ped.dir = 1; ped.isWalking = true; }
            else if (ped.state == 1 && hLight == 0) { ped.dir = 2; ped.isWalking = true; }
            else if (ped.state == 2 && vLight == 0) { ped.dir = 3; ped.isWalking = true; }
            else if (ped.state == 3 && hLight == 0) { ped.dir = 4; ped.isWalking = true; }
        }

        // [실제 걷기 처리 및 도착 판정]
        if (ped.isWalking) {
            if (ped.dir == 1) { // 1. 오른쪽
                ped.x += ped.speed;
                if (ped.x >= cx + rw + offset) {
                    ped.x = cx + rw + offset; ped.isWalking = false; ped.state = 1;
                }
            }
            else if (ped.dir == 2) { // 2. 아래쪽
                ped.y += ped.speed;
                if (ped.y >= cy + rw + offset) {
                    ped.y = cy + rw + offset; ped.isWalking = false; ped.state = 2;
                }
            }
            else if (ped.dir == 3) { // 3. 왼쪽
                ped.x -= ped.speed;
                if (ped.x <= cx - rw - offset) {
                    ped.x = cx - rw - offset; ped.isWalking = false; ped.state = 3;
                }
            }
            else if (ped.dir == 4) { // 4. 위쪽
                ped.y -= ped.speed;
                if (ped.y <= cy - rw - offset) {
                    ped.y = cy - rw - offset; ped.isWalking = false; ped.state = 0;
                }
            }
            //  5~8. 대각선 횡단 (현재 위치에서 목표 지점을 향해 부드럽게 걸어감)
            else if (ped.dir >= 5) {
                double targetX = cx, targetY = cy;
                int nextState = 0;

                // 목표 모서리 좌표 설정
                if (ped.dir == 5) { targetX = cx + rw; targetY = cy + rw; nextState = 2; }
                else if (ped.dir == 6) { targetX = cx - rw; targetY = cy + rw; nextState = 3; }
                else if (ped.dir == 7) { targetX = cx - rw; targetY = cy - rw; nextState = 0; }
                else if (ped.dir == 8) { targetX = cx + rw; targetY = cy - rw; nextState = 1; }

                // 목표를 향해 x, y 각각 이동 (목표에 도달하지 않았을 때만 이동)
                if (ped.dir == 5 || ped.dir == 8) { if (ped.x < targetX) ped.x += ped.speed; }
                else { if (ped.x > targetX) ped.x -= ped.speed; }

                if (ped.dir == 5 || ped.dir == 6) { if (ped.y < targetY) ped.y += ped.speed; }
                else { if (ped.y > targetY) ped.y -= ped.speed; }

                //  목표 직각 모서리에 거의 도착했다면? (오차 보정)
                if (abs(ped.x - targetX) <= ped.speed && abs(ped.y - targetY) <= ped.speed) {
                    ped.x = targetX; // 직각 모서리에 좌표를 완벽하게 딱 맞춤!
                    ped.y = targetY;
                    ped.isWalking = false;
                    ped.state = nextState; // 도착한 곳의 상태로 변경
                    ped.dir = 0;

                    // 대각선 횡단이 끝났으니 우클릭 예약 확인
                    if (isRButtonDown) {
                        isRButtonDown = false;
                        isLButtonDown = false;
                        if (!isAuto) {
                            SetTimer(hWnd, AutoLight, 1000, (TIMERPROC)TimerProc);
                            isAuto = true;
                            cout << "보행자 대각선 횡단 완료! 신호등 복구\n";
                        }
                    }
                }
            }
        }
        InvalidateRect(hWnd, NULL, false);
    }
    else if (idEvent == AutoLight) {
        // 1초마다 카운터를 1씩 증가
        LightCounter++;

        // 전체 사이클을 10초(0~9)로 나눔
        int cycle = LightCounter % 10;

        if (cycle >= 0 && cycle <= 3) {
            // 0, 1, 2, 3초 (4초간): 수직 파랑(1), 수평 빨강(0)
            vLight = 1;
            hLight = 0;
        }
        else if (cycle == 4) {
            // 4초 (1초간 딜레이): 수직 노랑(2), 수평 빨강(0)
            vLight = 2;
            hLight = 0;
        }
        else if (cycle >= 5 && cycle <= 8) {
            // 5, 6, 7, 8초 (4초간): 수직 빨강(0), 수평 파랑(1)
            vLight = 0;
            hLight = 1;
        }
        else if (cycle == 9) {
            // 9초 (1초간 딜레이): 수직 빨강(0), 수평 노랑(2)
            vLight = 0;
            hLight = 2;
        }

        // 신호등 색이 바뀌었으니 다시 그리라고 명령
        InvalidateRect(hWnd, NULL, FALSE);
    }
}