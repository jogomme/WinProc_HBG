#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <random>    
#include <iostream>
#include "resource.h"

using namespace std;

int wide = 1000;
int height = 800;
HINSTANCE g_hInst;

HBITMAP g_hImg1, g_hImg2;
HBITMAP currentImg;
BITMAP bmpInfo;

// 돋보기 상태 변수
bool isDrawing = false;
bool isMagActive = false;
bool isN = false;
RECT orignalRect;
RECT magRect;
POINT startPt, endPt;

// 돋보기 마우스 조작 관련 변수 (크기 조절 변수 추가)
bool isMovingMag = false;
POINT prevMousePt;
bool isResizingLeft = false;
bool isResizingRight = false;
bool isResizingTop = false;
bool isResizingBottom = false;
const int EDGE_MARGIN = 10; // 모서리 판별 여유 공간

// 돋보기 내부 조작 변수
int innerScale = 100;
bool isInverted = false;
bool isCopied = false;

// p키 붙여넣기 관련
POINT pastedPts[5];
int pastedCount = 0;
bool isFlippedH = false;

bool isFlippedV = false;

// f키 전체 붙여넣기 관련
bool isFullScreenPaste = false;

// 더블클릭 붙여넣기 관련
POINT doubleClickPts[10];
int dcCount = 0;

// 애니메이션 및 이동 상태 (타이머 용)
bool isBouncing = false;
bool isResizing = false;
int magDirX = 5, magDirY = 5;

int magDiX = 5, magDiY = 5;

random_device rd;
mt19937 gen(rd());

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdParam, int nCmdShow)
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
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = L"MagGameClass";
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(L"MagGameClass", L"실습 5-3 돋보기", WS_OVERLAPPEDWINDOW, 0, 0, wide, height, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    switch (uMsg) {
    case WM_CREATE:
        g_hImg1 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
        g_hImg2 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
        currentImg = g_hImg1;
        GetObject(currentImg, sizeof(BITMAP), &bmpInfo);
        SetTimer(hWnd, 1, 30, NULL);
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case '1': currentImg = g_hImg1; break;
        case '2': currentImg = g_hImg2; break;
        case 'E':
            if (isMagActive) {
                innerScale -= 10;
                if (innerScale < 20) innerScale = 100;
            }
            break;
        case 'S':
            if (isMagActive) {
                innerScale += 10;
                if (innerScale > 200) innerScale = 100;
            }
            break;
        case 'B':
            innerScale = 100;
            break;
        case 'C':
            if (isMagActive) isCopied = true;
            break;
        case 'P':
            if (isCopied && pastedCount < 5) {
                pastedPts[pastedCount].x = gen() % (wide - 100);
                pastedPts[pastedCount].y = gen() % (height - 100);
                pastedCount++;
            }
            break;
        case 'F':
            if (isCopied) isFullScreenPaste = !isFullScreenPaste;
            break;
        case 'H':
            isFlippedH = !isFlippedH;
            break;
        case 'V':
            isFlippedV = !isFlippedV;
            break;
        case 'I':
            isInverted = !isInverted;
            break;
        case 'R':
            isMagActive = false;
            isCopied = false;
            pastedCount = 0;
            dcCount = 0;
            isFullScreenPaste = false;
            innerScale = 100;
            isInverted = false;
            break;
        case 'Q':
            PostQuitMessage(0);
            break;
        case VK_LEFT:  if (isMagActive) { magRect.left -= 10; magRect.right -= 10; } break;
        case VK_RIGHT: if (isMagActive) { magRect.left += 10; magRect.right += 10; } break;
        case VK_UP:    if (isMagActive) { magRect.top -= 10; magRect.bottom -= 10; } break;
        case VK_DOWN:  if (isMagActive) { magRect.top += 10; magRect.bottom += 10; } break;
        case 'M': isBouncing = !isBouncing; break;
        case 'N': {
            if (isN) {
                magRect = orignalRect;
            }
            else {
                orignalRect = magRect;
                magDiX = 5;
                magDiY = 5;
            }
            isN = !isN;
            break;
        }
        }
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        POINT pt = { x, y };

        if (!isMagActive) {
            isDrawing = true;
            startPt = pt;
            endPt = pt;
        }
        else {
            // 마우스가 모서리 근처에 있는지 판별
            bool onLeft = abs(x - magRect.left) <= EDGE_MARGIN;
            bool onRight = abs(x - magRect.right) <= EDGE_MARGIN;
            bool onTop = abs(y - magRect.top) <= EDGE_MARGIN;
            bool onBottom = abs(y - magRect.bottom) <= EDGE_MARGIN;

            bool withinY = (y >= magRect.top - EDGE_MARGIN && y <= magRect.bottom + EDGE_MARGIN);
            bool withinX = (x >= magRect.left - EDGE_MARGIN && x <= magRect.right + EDGE_MARGIN);

            // 크기 조절 모드 판별
            if (withinY && (onLeft || onRight)) {
                if (onLeft) isResizingLeft = true;
                if (onRight) isResizingRight = true;
            }
            if (withinX && (onTop || onBottom)) {
                if (onTop) isResizingTop = true;
                if (onBottom) isResizingBottom = true;
            }

            // 모서리가 아니고 돋보기 내부일 경우 이동 모드
            if (!isResizingLeft && !isResizingRight && !isResizingTop && !isResizingBottom) {
                if (PtInRect(&magRect, pt)) {
                    isMovingMag = true;
                    prevMousePt = pt;
                }
            }

            // 돋보기 외부를 클릭한 경우는 R키를 눌러 리셋하기 전까지 무시하여
            // 더블클릭 이벤트가 정상적으로 발생하도록 보호함.
        }
        break;
    }

    case WM_MOUSEMOVE: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        POINT pt = { x, y };

        if (isDrawing) {
            endPt = pt;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (isResizingLeft || isResizingRight || isResizingTop || isResizingBottom) {
            // 가장자리 드래그: 돋보기 크기 변경 (너무 작게 줄어들지 않도록 제한)
            if (isResizingLeft) {
                magRect.left = min(x, magRect.right - 10);
            }
            if (isResizingRight) {
                magRect.right = max(x, magRect.left + 10);
            }
            if (isResizingTop) {
                magRect.top = min(y, magRect.bottom - 10);
            }
            if (isResizingBottom) {
                magRect.bottom = max(y, magRect.top + 10);
            }
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (isMovingMag) {
            // 돋보기 드래그 이동
            int dx = x - prevMousePt.x;
            int dy = y - prevMousePt.y;

            OffsetRect(&magRect, dx, dy);
            prevMousePt = pt;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_LBUTTONUP: {
        if (isDrawing) {
            isDrawing = false;
            isMagActive = true;
            magRect.left = min(startPt.x, endPt.x);
            magRect.top = min(startPt.y, endPt.y);
            magRect.right = max(startPt.x, endPt.x);
            magRect.bottom = max(startPt.y, endPt.y);

            if (magRect.right - magRect.left < 10) isMagActive = false;
        }

        // 모든 마우스 조작 모드 해제
        isMovingMag = false;
        isResizingLeft = false;
        isResizingRight = false;
        isResizingTop = false;
        isResizingBottom = false;

        InvalidateRect(hWnd, NULL, FALSE);
        break;
    }

    case WM_LBUTTONDBLCLK: {
        // 더블클릭 시 랜덤 위치에 복사 (돋보기가 활성화된 상태라면 어디를 누르든 작동)
        if (isMagActive) {
            if (dcCount >= 10) {
                dcCount = 0;
            }
            else {
                doubleClickPts[dcCount].x = gen() % (wide - 100);
                doubleClickPts[dcCount].y = gen() % (height - 100);
                dcCount++;
            }
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_TIMER: {
        if (isBouncing && isMagActive) {
            magRect.left += magDirX;
            magRect.right += magDirX;
            magRect.top += magDirY;
            magRect.bottom += magDirY;

            if (magRect.left <= 0 || magRect.right >= wide) magDirX = -magDirX;
            if (magRect.top <= 0 || magRect.bottom >= height) magDirY = -magDirY;

            InvalidateRect(hWnd, NULL, FALSE);
        }

        if (isN && isMagActive) {
            magRect.left -= magDiX;
            magRect.right += magDiX;
            magRect.top -= magDiY;
            magRect.bottom += magDiY;

            int currentWidth = magRect.right - magRect.left;
            int originalWidth = orignalRect.right - orignalRect.left;

            if (currentWidth >= originalWidth + 150 || currentWidth <= 50) {
                magDiX = -magDiX;
                magDiY = -magDiY;
            }
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);

        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBit = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

        HBRUSH backBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &ps.rcPaint, backBrush);
        DeleteObject(backBrush);

        HDC imgDC = CreateCompatibleDC(hDC);
        SelectObject(imgDC, currentImg);

        StretchBlt(memDC, 0, 0, wide, height, imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);

        if (isDrawing) {
            HPEN drawPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
            HPEN oldPen = (HPEN)SelectObject(memDC, drawPen);
            HBRUSH clearBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, clearBrush);

            Rectangle(memDC, startPt.x, startPt.y, endPt.x, endPt.y);

            SelectObject(memDC, oldBrush);
            SelectObject(memDC, oldPen);
            DeleteObject(drawPen);
        }

        if (isMagActive) {
            int magW = magRect.right - magRect.left;
            int magH = magRect.bottom - magRect.top;

            int srcW = (magW * innerScale) / 100;
            int srcH = (magH * innerScale) / 100;

            int srcX = (magRect.left * bmpInfo.bmWidth) / wide;
            int srcY = (magRect.top * bmpInfo.bmHeight) / height;

            DWORD rop = SRCCOPY;
            if (isInverted) rop = NOTSRCCOPY;

            if (isFullScreenPaste) {
                StretchBlt(memDC, 0, 0, wide, height, memDC, magRect.left, magRect.top, magW, magH, SRCCOPY);
            }

            StretchBlt(memDC, magRect.left, magRect.top, magW, magH, imgDC, srcX, srcY, srcW, srcH, rop);

            // 돋보기 빨간 테두리 그리기
            HPEN redPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
            HPEN oldPen = (HPEN)SelectObject(memDC, redPen);
            HBRUSH clearBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, clearBrush);

            // 여기서 끊어졌었습니다.
            Rectangle(memDC, magRect.left, magRect.top, magRect.right, magRect.bottom);

            SelectObject(memDC, oldBrush);
            SelectObject(memDC, oldPen);
            DeleteObject(redPen);

            // p키 복사본 그리기 (1/2 크기, 반전 처리 적용) 
            for (int i = 0; i < pastedCount; i++) {
                int pasteW = magW / 2;
                int pasteH = magH / 2;
                int destX = pastedPts[i].x;
                int destY = pastedPts[i].y;

                // 좌우 상하 반전 구현: 복사 대상의 시작 좌표와 폭을 음수로 주어 뒤집음 
                int printW = isFlippedH ? -pasteW : pasteW;
                int printH = isFlippedV ? -pasteH : pasteH;
                int printX = isFlippedH ? destX + pasteW : destX;
                int printY = isFlippedV ? destY + pasteH : destY;

                StretchBlt(memDC, printX, printY, printW, printH, imgDC, magRect.left, magRect.top, magW, magH, SRCCOPY);
            }

            // 더블클릭 복사본 그리기 
            for (int i = 0; i < dcCount; i++) {
                StretchBlt(memDC, doubleClickPts[i].x, doubleClickPts[i].y, magW, magH, imgDC, magRect.left, magRect.top, magW, magH, SRCCOPY);
            }
        }

        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);

        DeleteDC(imgDC);
        SelectObject(memDC, oldBit);
        DeleteObject(hBit);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_SIZE:
        wide = LOWORD(lParam);
        height = HIWORD(lParam);
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_DESTROY:
        DeleteObject(g_hImg1);
        DeleteObject(g_hImg2);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}