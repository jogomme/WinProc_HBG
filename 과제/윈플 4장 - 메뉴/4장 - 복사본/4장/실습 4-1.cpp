#define _CRT_SECURE_NO_WARNINGS

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include <math.h>
#include<iostream>
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
    WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 배경 색깔
    WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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
int cell = 150;
int moveSpeed[4]{ 3,3,3,3 };
POINT point[4]{};
double angle[4]{ 270,270,270,270 };

bool isRunning = false; // 과제 조건: "처음 시작하면 공전을 하고 있지 않는다"
COLORREF revolveColor[4] = { RGB(80,80,80), RGB(80,80,80), RGB(80,80,80), RGB(80,80,80) }; // 회전하는 도형의 각각의 색상

// 0 = 원, 1 = 사각형, 2 = 삼각형 
int shape[4]{ 0,0,0,0 };

// 0 = 원, 1 = 사각형, 2 = 삼각형 
int revolveShape[4]{ 0,0,0,0 };

int R = cell * 2 / 3;

bool revers[4]{ false, false, false, false };

bool Drag[4]{ false, false, false, false };

bool over[4]{ false, false, false, false };

bool ColorRevers[4]{ false,false, false, false };

POINT Points{};

// 0 -> 배경, 1 -> 중앙 원, 2-> 회전 도형
COLORREF orignalColor[3] =
{
    {RGB(255,255,255)},
    {RGB(255,0,0)},
    {RGB(80,80,80)}
};

// 0 -> 배경, 1 -> 중앙 원, 2-> 회전 도형
COLORREF reversColor[3] =
{
    {RGB(0,0,0)},
    {RGB(0,255,255)},
    {RGB(255 - 80,255 - 80,255 - 80)}
};

// --------------------------------------------------------
// 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    static RECT rectView;
    static POINT center{};
    static int SelectArea{ -1 };

    static POINT newPoint;

    // 1   2
    // 4   3
    static POINT area[4] = {};

    //cout << angle[0] << endl;

    double rad{};
    switch (uMsg) {
    case WM_CREATE:
        GetClientRect(hWnd, &rectView);
        center = { rectView.right / 2, rectView.bottom / 2 };
        area[0] = { center.x / 2, center.y / 2 };
        area[1] = { center.x * 3 / 2, center.y / 2 };
        area[2] = { center.x * 3 / 2, center.y * 3 / 2 };
        area[3] = { center.x / 2, center.y * 3 / 2 };

        for (int i = 0; i < 4; ++i) {
            point[i] = { area[i].x, area[i].y - cell * 2 / 3 };
        }

        SetTimer(hWnd, 1, 10, NULL);
        SetTimer(hWnd, 2, 10, NULL);
        SetTimer(hWnd, 3, 10, NULL);
        SetTimer(hWnd, 4, 10, NULL);

        break;
    case WM_COMMAND: {

        int wmId = LOWORD(wParam);
        switch (wmId)
        {
            //---------------------------------- GAME ----------------------------------//
        case ID_GAME_START: {
            isRunning = true;
            break;
        }
        case ID_GAME_STOP: {
            isRunning = false;
            break;
        }
        case ID_GAME_QUITE: {
            PostQuitMessage(0);
            DestroyWindow(hWnd);
        }
        //---------------------------------- Selection ----------------------------------//
        case ID_SELECTION_1: SelectArea = 1; break; 
        case ID_SELECTION_2: SelectArea = 2; break; 
        case ID_SELECTION_3: SelectArea = 3; break; 
        case ID_SELECTION_4: SelectArea = 4; break; 
        //---------------------------------- Option ----------------------------------//
        case ID_SPEED_FAST:
            if (SelectArea > 0) moveSpeed[SelectArea - 1] = 10;
            break;
        case ID_SPEED_MEDIUM:
            if (SelectArea > 0) moveSpeed[SelectArea - 1] = 5;
            break;
        case ID_SPEED_SLOW:
            if (SelectArea > 0) moveSpeed[SelectArea - 1] = 1;
            break;
            // (2) Color
        case ID_COLOR_CYAN:
            if (SelectArea > 0) revolveColor[SelectArea - 1] = RGB(0, 255, 255);
            break;
        case ID_COLOR_MAGENTA:
            if (SelectArea > 0) revolveColor[SelectArea - 1] = RGB(255, 0, 0);
            break;
        case ID_COLOR_YELLOW:
            if (SelectArea > 0) revolveColor[SelectArea - 1] = RGB(255, 255, 0);
            break;

            // (3) Shape
        case ID_SHAPE_CIRCLE:
            if (SelectArea > 0) revolveShape[SelectArea - 1] = 0;
            break;
        case ID_SHAPE_RECTANGLE:
            if (SelectArea > 0) revolveShape[SelectArea - 1] = 1;
            break;
        case ID_SHAPE_TRIANGLE:
            if (SelectArea > 0) revolveShape[SelectArea - 1] = 2;
            break;
        }
        break;
    }
    case WM_KEYDOWN:
        if (wParam == 'Q') {
            PostQuitMessage(0);
        }
        else if (wParam >= '1' && wParam <= '4') {
            if (SelectArea == wParam - '0') {
                SelectArea = -1;
                for (int i = 0; i < 4; ++i) {
                    Drag[i] = false;
                }
            }
            else {
                SelectArea = wParam - '0';
                for (int i = 0; i < 4; ++i) {
                    Drag[i] = false;
                }
            }
        }
        else if (wParam == 'C') {
            if (SelectArea - 1 < 0) {
                break;
            }
            if (revers[SelectArea - 1] == true) {
                revers[SelectArea - 1] = false;
            }
            else {
                revers[SelectArea - 1] = true;
            }

        }
        else if (wParam == 'M') {
            if (SelectArea - 1 < 0) {
                break;
            }

            revolveShape[SelectArea - 1] = (revolveShape[SelectArea - 1] + 1) % 3;

        }
        else if (wParam == 'R') {
            if (SelectArea - 1 < 0) {
                break;
            }
            else if (shape[SelectArea - 1] == 1) {
                shape[SelectArea - 1] = 0;
            }
            else {
                shape[SelectArea - 1] = 1;
            }
        }
        else if (wParam == 'T') {
            if (SelectArea - 1 < 0) {
                break;
            }
            else if (shape[SelectArea - 1] == 2) {
                shape[SelectArea - 1] = 0;
            }
            else {
                shape[SelectArea - 1] = 2;
            }
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    case WM_RBUTTONDBLCLK: {
        newPoint.x = LOWORD(lParam);
        newPoint.y = HIWORD(lParam);
        cout << newPoint.x << " " << newPoint.y << " " << Drag[SelectArea - 1] << endl;

        for (int i = 0; i < 4; ++i) {
            if (area[i].x - cell * 2 / 3 < newPoint.x &&
                area[i].x + cell * 2 / 3 > newPoint.x &&
                area[i].y - cell * 2 / 3 < newPoint.y &&
                area[i].y + cell * 2 / 3 > newPoint.y
                ) {
                ColorRevers[i] = !ColorRevers[i];
                cout << i + 1 << " 영역이 색상 반전 되었습니다. : " << ColorRevers[i] << endl;
            }
        }

        break;
    }
    case WM_LBUTTONDOWN: {
        cout << moveSpeed[SelectArea - 1] << " " << over[SelectArea - 1] << endl;

        if (SelectArea - 1 < 0) {
            break;
        }

        if (moveSpeed[SelectArea - 1] > 30) {
            over[SelectArea - 1] = true;
        }
        else if (moveSpeed[SelectArea - 1] < 0) {
            moveSpeed[SelectArea - 1] = 0;
            over[SelectArea - 1] = false;
        }

        if (over[SelectArea - 1] == true) {
            moveSpeed[SelectArea - 1] -= 5;
            if (moveSpeed[SelectArea - 1] < 0) {
                moveSpeed[SelectArea - 1] = 0;
                over[SelectArea - 1] = false;
            }
        }
        else {
            moveSpeed[SelectArea - 1] += 5;
        }

        break;
    }
    case WM_RBUTTONDOWN: {
        newPoint.x = LOWORD(lParam);
        newPoint.y = HIWORD(lParam);
        cout << newPoint.x << " " << newPoint.y << " " << Drag[SelectArea - 1] << endl;
        if (SelectArea - 1 < 0) {
            break;
        }

        if (Drag[SelectArea - 1]) {
            Drag[SelectArea - 1] = false;
        }
        else {
            for (int i = 0; i < 4; ++i) {
                Drag[i] = false;
            }
            Drag[SelectArea - 1] = true;
        }
        break;
    }
    case WM_TIMER: {
        int i{};
        i = wParam - 1;
        if (isRunning) {
            if (i >= 0 && i < 4) {
                R = cell * 2 / 3;

                if (revers[i] == true) {
                    angle[i] -= moveSpeed[i];
                }
                else {
                    angle[i] += moveSpeed[i];
                }

                if (angle[i] >= 360.0) {
                    angle[i] -= 360.0;
                }
                else if (angle[i] < 0.0) {
                    angle[i] += 360.0;
                }

                POINT currentCenter;
                if (Drag[i] == true) {
                    currentCenter = newPoint; // 드래그 중이면 마우스 위치가 중심
                }
                else {
                    currentCenter = area[i];  // 아니면 원래 자기 자리가 중심
                }

                if (shape[i] == 0) {
                    rad = angle[i] * 3.141592 / 180.0;
                    point[i].x = currentCenter.x + R * cos(rad);
                    point[i].y = currentCenter.y + R * sin(rad);
                }
                else if (shape[i] == 1) {
                    if (angle[i] >= 0 && angle[i] < 90) {
                        double t = angle[i] / 90.0;
                        if (t < 0.5) {
                            point[i].x = currentCenter.x + R;
                            point[i].y = currentCenter.y + (R * 2 * t);
                        }
                        else {
                            point[i].y = currentCenter.y + R;
                            point[i].x = currentCenter.x + R - (R * 2 * (t - 0.5));
                        }
                    }
                    else if (angle[i] >= 90 && angle[i] < 180) {
                        double t = (angle[i] - 90) / 90.0;
                        if (t < 0.5) {
                            point[i].y = currentCenter.y + R;
                            point[i].x = currentCenter.x - (R * 2 * (t));
                        }
                        else {
                            point[i].y = currentCenter.y + R - (R * 2 * (t - 0.5));
                            point[i].x = currentCenter.x - R;
                        }
                    }
                    else if (angle[i] >= 180 && angle[i] < 270) {
                        double t = (angle[i] - 180) / 90.0;
                        if (t < 0.5) {
                            point[i].y = currentCenter.y - (R * 2 * (t));
                            point[i].x = currentCenter.x - R;
                        }
                        else {
                            point[i].y = currentCenter.y - R;
                            point[i].x = currentCenter.x - R + (R * 2 * (t - 0.5));
                        }
                    }
                    else if (angle[i] >= 270 && angle[i] < 360) {
                        double t = (angle[i] - 270) / 90.0;
                        if (t < 0.5) {
                            point[i].y = currentCenter.y - R;
                            point[i].x = currentCenter.x + (2 * R * t);
                        }
                        else {
                            point[i].y = currentCenter.y - R + (R * 2 * (t - 0.5));
                            point[i].x = currentCenter.x + R;
                        }
                    }
                }
                else if (shape[i] == 2) {
                    if (angle[i] >= 0 && angle[i] < 120) {
                        double t = (angle[i]) / 120;
                        if (t < 0.5) {
                            point[i].x = currentCenter.x + (R / 2) + (R / 2 * 2 * (t));
                            point[i].y = currentCenter.y + (R * 2 * t);
                        }
                        else {
                            point[i].x = currentCenter.x + R - (R * 2 * (t - 0.5));
                            point[i].y = currentCenter.y + R;
                        }
                    }
                    else if (angle[i] >= 120 && angle[i] < 240) {
                        double t = (angle[i] - 120) / 120;
                        if (t < 0.5) {
                            point[i].x = currentCenter.x - (R * 2 * t);
                        }
                        else {
                            point[i].x = currentCenter.x - R + (R / 2 * 2 * (t - 0.5));
                            point[i].y = currentCenter.y + R - (R * 2 * (t - 0.5));
                        }
                    }
                    else if (angle[i] >= 240 && angle[i] < 360) {
                        double t = (angle[i] - 240) / 120;
                        if (t < 0.5) {
                            point[i].x = currentCenter.x - (R / 2) + (R / 2 * 2 * (t));
                            point[i].y = currentCenter.y - (R * 2 * t);
                        }
                        else {
                            point[i].x = currentCenter.x + (R / 2 * 2 * (t - 0.5));
                            point[i].y = currentCenter.y - R + (R * 2 * (t - 0.5));
                        }
                    }
                }
            }
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);

        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);

        if (SelectArea > 0) {
            if (Drag[SelectArea - 1] == false)
                Rectangle(memDC, area[SelectArea - 1].x - cell, area[SelectArea - 1].y - cell, area[SelectArea - 1].x + cell, area[SelectArea - 1].y + cell);
        }
        HBRUSH myBrush = CreateSolidBrush(RGB(255, 255, 255));
        HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
        // 배경 도형
        SelectObject(memDC, oldBrush); // 제자리 돌아가기
        DeleteObject(myBrush);

        for (int i = 0; i < 4; ++i) {
            myBrush = CreateSolidBrush(RGB(255, 255, 255));
            oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
            if (ColorRevers[i] == true) {
                SelectObject(memDC, oldBrush); // 제자리 돌아가기
                DeleteObject(myBrush);
                myBrush = CreateSolidBrush(reversColor[0]);
                oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
            }
            if (Drag[i] == false) {
                if (shape[i] == 0) {
                    Ellipse(memDC, area[i].x - cell * 2 / 3, area[i].y - cell * 2 / 3, area[i].x + cell * 2 / 3, area[i].y + cell * 2 / 3);
                }
                else if (shape[i] == 1) {
                    Rectangle(memDC, area[i].x - R, area[i].y - R, area[i].x + R, area[i].y + R);
                }
                else if (shape[i] == 2) {

                    POINT pts[3] = {
                           {area[i].x, area[i].y - R},
                           {area[i].x + R , area[i].y + R},
                           {area[i].x - R, area[i].y + R}
                    };

                    Polygon(memDC, pts, 3);
                }
            }
            else {

            }
            SelectObject(memDC, oldBrush); // 제자리 돌아가기
            DeleteObject(myBrush);
        }


        // 회전 도형
        for (int i = 0; i < 4; ++i) {

            myBrush = CreateSolidBrush(revolveColor[i]);
            oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
            if (ColorRevers[i] == true) {
                SelectObject(memDC, oldBrush); // 제자리 돌아가기
                DeleteObject(myBrush);
                myBrush = CreateSolidBrush(reversColor[2]);
                oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
            }

            if (Drag[i] == false) {
                if (revolveShape[i] == 0) {
                    Ellipse(memDC, point[i].x - 7, point[i].y - 7, point[i].x + 7, point[i].y + 7);
                }
                else if (revolveShape[i] == 1) {
                    Rectangle(memDC, point[i].x - 7, point[i].y - 7, point[i].x + 7, point[i].y + 7);
                }
                else if (revolveShape[i] == 2) {
                    POINT pts[3] = {
                        {point[i].x, point[i].y - 7},
                        {point[i].x + 7 , point[i].y + 7},
                        {point[i].x - 7, point[i].y + 7}
                    };

                    Polygon(memDC, pts, 3);
                }
            }
            SelectObject(memDC, oldBrush); // 제자리 돌아가기
            DeleteObject(myBrush);
        }

        // 원 그리기
        for (int i = 0; i < 4; ++i) {
            myBrush = CreateSolidBrush(RGB(255, 0, 0));
            oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
            if (ColorRevers[i] == true) {
                SelectObject(memDC, oldBrush); // 제자리 돌아가기
                DeleteObject(myBrush);
                myBrush = CreateSolidBrush(reversColor[1]);
                oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
            }
            if (Drag[i] == false)
                Ellipse(memDC, area[i].x - 5, area[i].y - 5, area[i].x + 5, area[i].y + 5);
            SelectObject(memDC, oldBrush); // 제자리 돌아가기
            DeleteObject(myBrush);
        }



        // 선택된 도형 그리기
        for (int i = 0; i < 4; ++i) {
            SelectObject(memDC, oldBrush); // 제자리 돌아가기
            DeleteObject(myBrush);
            if (Drag[i]) {
                // 선택된거 틀
                Rectangle(memDC, newPoint.x - cell, newPoint.y - cell, newPoint.x + cell, newPoint.y + cell);

                myBrush = CreateSolidBrush(RGB(255, 255, 255));
                oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
                if (ColorRevers[i] == true) {
                    SelectObject(memDC, oldBrush); // 제자리 돌아가기
                    DeleteObject(myBrush);
                    myBrush = CreateSolidBrush(reversColor[0]);
                    oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
                }
                // 바깥 도형
                if (shape[i] == 0) {
                    Ellipse(memDC, newPoint.x - cell * 2 / 3, newPoint.y - cell * 2 / 3, newPoint.x + cell * 2 / 3, newPoint.y + cell * 2 / 3);
                }
                else if (shape[i] == 1) {
                    Rectangle(memDC, newPoint.x - R, newPoint.y - R, newPoint.x + R, newPoint.y + R);
                }
                else if (shape[i] == 2) {

                    POINT pts[3] = {
                           {newPoint.x, newPoint.y - R},
                           {newPoint.x + R , newPoint.y + R},
                           {newPoint.x - R, newPoint.y + R}
                    };
                    Polygon(memDC, pts, 3);
                }




                // 중앙 원
                myBrush = CreateSolidBrush(RGB(255, 0, 0));
                oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
                if (ColorRevers[i] == true) {
                    SelectObject(memDC, oldBrush); // 제자리 돌아가기
                    DeleteObject(myBrush);
                    myBrush = CreateSolidBrush(reversColor[1]);
                    oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
                }

                Ellipse(memDC, newPoint.x - 5, newPoint.y - 5, newPoint.x + 5, newPoint.y + 5);

                SelectObject(memDC, oldBrush); // 제자리 돌아가기
                DeleteObject(myBrush);

                myBrush = CreateSolidBrush(revolveColor[i]);
                oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
                if (ColorRevers[i] == true) {
                    SelectObject(memDC, oldBrush); // 제자리 돌아가기
                    DeleteObject(myBrush);
                    myBrush = CreateSolidBrush(reversColor[2]);
                    oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
                }
                // 회전 도형
                if (revolveShape[i] == 0) {

                    Ellipse(memDC, point[i].x - 7, point[i].y - 7, point[i].x + 7, point[i].y + 7);
                }
                else if (revolveShape[i] == 1) {
                    Rectangle(memDC, point[i].x - 7, point[i].y - 7, point[i].x + 7, point[i].y + 7);
                }
                else if (revolveShape[i] == 2) {
                    POINT pts[3] = {
                        {point[i].x, point[i].y - 7},
                        {point[i].x + 7 , point[i].y + 7},
                        {point[i].x - 7, point[i].y + 7}
                    };
                    Polygon(memDC, pts, 3);
                }
            }
            SelectObject(memDC, oldBrush); // 제자리 돌아가기
            DeleteObject(myBrush);
        }


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