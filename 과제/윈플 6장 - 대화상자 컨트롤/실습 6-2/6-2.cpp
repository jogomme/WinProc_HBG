#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib, "msimg32.lib")

#include <windows.h>
#include <tchar.h>
#include <math.h>
#include "resource.h" 

#define PI 3.14159265

// --------------------------------------------------------
// 전역 변수 선언 구간 (메인 윈도우와 대화상자가 공유)
// --------------------------------------------------------
int wide = 800;
int height = 800;

// 곡선 위치 및 애니메이션용 오프셋 변수
int offsetX = 0;
int offsetY = 0;
int circleT = 0; // 원이 경로를 따라 이동하는 진행도

// 라디오 버튼(도형 종류) 상태
bool isSin = true;
bool isHalf_Way = false;
bool isSpring = false;
bool isStair_Way = false;

// 체크 박스(색상) 상태
bool isCyan = false;
bool isMagenta = false;
bool isYellow = false;
bool isReverse = false;

// 애니메이션 상태
bool isMove_Circle = false;

// --------------------------------------------------------
// 윈도우 메인 및 프로시저 선언
// --------------------------------------------------------
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class 3";
LPCTSTR lpszWindowName = L"Practice 6-2"; // 💡 한글 깨짐 에러를 막기 위해 영어로 변경

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Dlalog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
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
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, wide, height, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return Message.wParam;
}

// --------------------------------------------------------
// 메인 윈도우 메시지 처리 함수 (그리기 담당)
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    switch (uMsg) {
    case WM_LBUTTONDOWN:
        DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)Dlalog_Proc);
        break;

    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);

        // 더블 버퍼링 설정
        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        // 1. 배경 흰색 채우기
        HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);

        // 2. 십자선 (X, Y축) 그리기 (정중앙 기준)
        int originX = wide / 2;
        int originY = height / 2;

        HPEN axisPen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
        HPEN oldPen1 = (HPEN)SelectObject(memDC, axisPen);
        MoveToEx(memDC, originX, 0, NULL); LineTo(memDC, originX, height);
        MoveToEx(memDC, 0, originY, NULL); LineTo(memDC, wide, originY);
        SelectObject(memDC, oldPen1);
        DeleteObject(axisPen);

        // 3. 색상 조합 로직
        int r = 0, g = 0, b = 0;
        if (isCyan) { g = 255; b = 255; }
        if (isMagenta) { r = 255; b = 255; }
        if (isYellow) { r = 255; g = 255; }
        if (!isCyan && !isMagenta && !isYellow) { r = 0; g = 0; b = 0; }

        if (isReverse) {
            r = 255 - r; g = 255 - g; b = 255 - b;
        }

        HPEN curvePen = CreatePen(PS_SOLID, 2, RGB(r, g, b));
        HPEN oldPen2 = (HPEN)SelectObject(memDC, curvePen);

        // 4. 곡선 그리기 (무한 스크롤 적용)
        int circleX = 0, circleY = 0;

        // 💡 핵심: 현재 이동한 거리(offsetX)를 역산하여, 화면 왼쪽 바깥부터 오른쪽 바깥까지만 그립니다.
        int startI = -offsetX - 400;
        int endI = -offsetX + wide + 400;
        bool firstPoint = true;

        for (int i = startI; i <= endI; i++) {
            int drawX = 0;
            int drawY = 0;

            if (isSin) {
                drawX = i;
                drawY = (int)(sin(i * PI / 50.0) * 100.0);
            }
            else if (isHalf_Way) {
                drawX = i;
                // 음수 좌표 보정
                int pos_i = i % 100;
                if (pos_i < 0) pos_i += 100;
                int cycle = pos_i;
                int insideSqrt = 2500 - (cycle - 50) * (cycle - 50);
                if (insideSqrt < 0) insideSqrt = 0;

                int y = (int)sqrt((double)insideSqrt);
                int segment = (i >= 0) ? (i / 100) : ((i - 99) / 100);
                drawY = (segment % 2 == 0) ? y : -y;
            }
            else if (isSpring) {
                drawX = (int)(i - 30 * sin(i * PI / 20.0));
                drawY = (int)(40 * cos(i * PI / 20.0));
            }
            else if (isStair_Way) {
                drawX = i;
                // 음수 좌표 보정
                int segment = (i >= 0) ? (i / 50) : ((i - 49) / 50);
                drawY = segment * 50;
            }

            int screenX = drawX + offsetX;
            int screenY = originY - drawY + offsetY;

            // 선이 끊기지 않게 연결
            if (firstPoint) {
                MoveToEx(memDC, screenX, screenY, NULL);
                firstPoint = false;
            }
            else {
                LineTo(memDC, screenX, screenY);
            }

            // 원이 현재 그려지는 곡선을 따라 순환하도록 보정
            int range = endI - startI + 1;
            int currentCircle_i = startI + (circleT % range);
            if (i == currentCircle_i) {
                circleX = screenX;
                circleY = screenY;
            }
        }

        // 5. It's moving 원과 텍스트 그리기
        if (isMove_Circle) {
            HPEN redPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
            HPEN oldP = (HPEN)SelectObject(memDC, redPen);
            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH oldB = (HBRUSH)SelectObject(memDC, nullBrush);

            Ellipse(memDC, circleX - 25, circleY - 25, circleX + 25, circleY + 25);

            SetBkMode(memDC, TRANSPARENT);
            TextOut(memDC, circleX - 35, circleY - 45, L"It's moving", 11);

            SelectObject(memDC, oldB);
            SelectObject(memDC, oldP);
            DeleteObject(redPen);
        }

        SelectObject(memDC, oldPen2);
        DeleteObject(curvePen);

        // 더블 버퍼링 복사
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
// 대화상자(리모컨) 프로시저
// --------------------------------------------------------
BOOL CALLBACK Dlalog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg) {
    case WM_INITDIALOG:
        isSin = true;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;

        case IDOK:
            offsetX = 0; offsetY = 0; circleT = 0;
            isSin = true; isHalf_Way = false; isSpring = false; isStair_Way = false;
            isCyan = false; isMagenta = false; isYellow = false; isReverse = false;
            isMove_Circle = false;
            KillTimer(hDlg, 1); KillTimer(hDlg, 2); KillTimer(hDlg, 3);
            break;

            // 라디오 버튼
        case IDC_SIN:
            isSin = true; isHalf_Way = false; isSpring = false; isStair_Way = false; break;
        case IDC_HALF_CIRCLE:
            isSin = false; isHalf_Way = true; isSpring = false; isStair_Way = false; break;
        case IDC_SPRING:
            isSin = false; isHalf_Way = false; isSpring = true; isStair_Way = false; break;
        case IDC_STAIRWAY:
            isSin = false; isHalf_Way = false; isSpring = false; isStair_Way = true; break;

            // 체크 박스
        case IDC_CYAN: isCyan = !isCyan; break;
        case IDC_MEGENTA: isMagenta = !isMagenta; break;
        case IDC_YELLO: isYellow = !isYellow; break;
        case IDC_REVERS: isReverse = !isReverse; break;

            // 조작 버튼
        case IDC_MOVEX:
            SetTimer(hDlg, 1, 30, NULL); KillTimer(hDlg, 2); break;
        case IDC_MOVEY:
            SetTimer(hDlg, 2, 30, NULL); KillTimer(hDlg, 1); break;
        case IDC_STOP:
            isMove_Circle = false;
            KillTimer(hDlg, 1); KillTimer(hDlg, 2); KillTimer(hDlg, 3); break;
        case IDC_RESET:
            offsetX = 0; offsetY = 0; circleT = 0; break;
        case IDC_CIRCLE:
            isMove_Circle = true; SetTimer(hDlg, 3, 30, NULL); break;
        }

        // 부모 창 갱신
        InvalidateRect(GetParent(hDlg), NULL, false);
        break;

    case WM_TIMER:
        if (wParam == 1) offsetX += 5;
        if (wParam == 2) offsetY -= 5;
        if (wParam == 3) circleT += 10; // 무한 이동을 위해 초기화 코드 제거

        InvalidateRect(GetParent(hDlg), NULL, false);
        break;

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    }
    return 0;
}