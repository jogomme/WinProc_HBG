#define _CRT_SECURE_NO_WARNINGS

// 라이브러리 링크
#pragma comment(lib, "msimg32.lib") // TransparentBlt 사용을 위함
#pragma comment(lib, "ole32.lib")   // PNG 리소스 스트림 변환을 위함

#include <windows.h>
#include <tchar.h>
#include <random>
#include <iostream>
#include <math.h>

#include <atlimage.h> 
#include "resource.h" // 등록하신 리소스 ID를 사용하기 위해 포함합니다.

using namespace std;

int wide{ 800 };
int height{ 800 };

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_real_distribution<double> angleDist(0.0, 2.0 * 3.14159265);

HINSTANCE g_hInst;
LPCTSTR IpszClass = L"My Window Class 3";
LPCTSTR IpszWindowName = L"5-5 고양이가 쥐 잡기";
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

    hWnd = CreateWindow(IpszClass, IpszWindowName, WS_OVERLAPPEDWINDOW,
        0, 0, wide, height,
        NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return Message.wParam;
}

// --------------------------------------------------------
// 고양이 이미지를 저장할 CImage 배열
// --------------------------------------------------------
CImage g_CatImg[4];

// 고양이 상태
double catX = 400.0;
double catY = 400.0;
double catAngle = 0.0;
double catSpeed = 3.0;
int    catSize = 30;
int    catFrame = 0;

// 고양이 클릭 상태
bool catClicked = false;
int  catClickTimer = 0;

// 쥐 상태 (기본 도형 유지)
bool   mouseVisible = false;
double mouseX = 0.0;
double mouseY = 0.0;
int    mouseFrame = 0;
bool   isDragging = false;

// 먹이 상태 (기본 도형 유지)
bool   foodVisible = false;
double foodX = 0.0;
double foodY = 0.0;

void DrawScene(HDC memDC);
void DrawCat(HDC memDC);
void DrawMouse(HDC memDC);
void DrawFood(HDC memDC);
void ResetGame(HWND hWnd);

double Distance(double x1, double y1, double x2, double y2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

bool IsOnCat(int mx, int my)
{
    return Distance((double)mx, (double)my, catX, catY) < (double)catSize;
}

// --------------------------------------------------------
// PNG 리소스 로드 전용 헬퍼 함수
// 리소스에 등록된 PNG 파일의 바이너리 데이터를 메모리로 읽어와서
// CImage 객체에 스트림 형태로 집어넣는 역할을 합니다.
// --------------------------------------------------------
void LoadPngResource(CImage& img, int resourceID)
{
    // 리소스 뷰에 PNG 파일이 추가되면 보통 L"PNG" 라는 사용자 지정 형식으로 분류됩니다.
    HRSRC hResource = FindResource(g_hInst, MAKEINTRESOURCE(resourceID), L"PNG");
    if (!hResource) return;

    DWORD imageSize = SizeofResource(g_hInst, hResource);
    HGLOBAL hGlobal = LoadResource(g_hInst, hResource);
    const void* pResourceData = LockResource(hGlobal);

    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (hBuffer)
    {
        void* pBuffer = GlobalLock(hBuffer);
        memcpy(pBuffer, pResourceData, imageSize);
        GlobalUnlock(hBuffer);

        IStream* pStream = NULL;
        if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) == S_OK)
        {
            img.Load(pStream);
            pStream->Release(); // 해제 시 hBuffer 메모리도 같이 자동 해제됨
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    static RECT rectView;

    switch (uMsg) {
    case WM_CREATE:
        GetClientRect(hWnd, &rectView);

        // --------------------------------------------------------
        // 알려주신 매크로 ID를 바탕으로 리소스에서 이미지를 불러옵니다.
        // --------------------------------------------------------

        // 1번 프레임 (PNG 형식)
        LoadPngResource(g_CatImg[0], IDB_PNG1);

        // 2번 프레임 (BMP 형식) - CImage의 기본 기능으로 즉시 로드 가능
        g_CatImg[1].LoadFromResource(g_hInst, IDB_BITMAP1);

        // 3번 프레임 (PNG 형식)
        LoadPngResource(g_CatImg[2], IDB_PNG2);

        // 4번 프레임 (PNG 형식)
        LoadPngResource(g_CatImg[3], IDB_PNG3);

        ResetGame(hWnd);

        SetTimer(hWnd, 1, 16, NULL);
        SetTimer(hWnd, 2, 100, NULL); // 애니메이션 프레임 전환 속도
        break;

    case WM_TIMER: {
        if (wParam == 1) {
            if (mouseVisible) {
                catAngle = atan2(mouseY - catY, mouseX - catX);
            }
            else if (foodVisible) {
                catAngle = atan2(foodY - catY, foodX - catX);
            }

            catX += cos(catAngle) * catSpeed;
            catY += sin(catAngle) * catSpeed;

            if (catX < catSize) {
                catX = catSize;
                catAngle = 3.14159265 - catAngle;
            }
            if (catX > wide - catSize) {
                catX = wide - catSize;
                catAngle = 3.14159265 - catAngle;
            }
            if (catY < catSize) {
                catY = catSize;
                catAngle = -catAngle;
            }
            if (catY > height - catSize) {
                catY = height - catSize;
                catAngle = -catAngle;
            }

            if (foodVisible && Distance(catX, catY, foodX, foodY) < catSize + 15) {
                foodVisible = false;
                catSpeed += 0.5;
            }

            if (catClicked) {
                catClickTimer--;
                if (catClickTimer <= 0) {
                    catClicked = false;
                }
            }
        }
        else if (wParam == 2) {
            catFrame = (catFrame + 1) % 4;
            mouseFrame = (mouseFrame + 1) % 2;
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    }

    case WM_LBUTTONDOWN: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        if (IsOnCat(mx, my)) {
            catClicked = true;
            catClickTimer = 30;
            catAngle = angleDist(gen);
        }
        else {
            mouseVisible = true;
            mouseX = (double)mx;
            mouseY = (double)my;
            isDragging = true;
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    }

    case WM_MOUSEMOVE: {
        if (isDragging) {
            mouseX = (double)LOWORD(lParam);
            mouseY = (double)HIWORD(lParam);
            InvalidateRect(hWnd, NULL, false);
        }
        break;
    }

    case WM_LBUTTONUP: {
        if (isDragging) {
            mouseVisible = false;
            isDragging = false;
            catAngle = angleDist(gen);
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    }

    case WM_RBUTTONDOWN: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        foodVisible = true;
        foodX = (double)mx;
        foodY = (double)my;

        InvalidateRect(hWnd, NULL, false);
        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
            catSpeed += 1.0;
        }
        else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
            catSpeed -= 1.0;
            if (catSpeed < 1.0) {
                catSpeed = 1.0;
            }
        }
        else if (wParam == 'R') {
            ResetGame(hWnd);
        }
        else if (wParam == 'Q') {
            PostQuitMessage(0);
        }
        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);

        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBmp = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBmp);

        // 배경색
        HBRUSH bgBrush = CreateSolidBrush(RGB(220, 240, 220));
        FillRect(memDC, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);

        DrawScene(memDC);

        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBmp);
        DeleteObject(hBmp);
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
        KillTimer(hWnd, 1);
        KillTimer(hWnd, 2);

        // 이미지 객체 메모리 해제
        for (int i = 0; i < 4; i++) {
            if (!g_CatImg[i].IsNull()) {
                g_CatImg[i].Destroy();
            }
        }

        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// --------------------------------------------------------
// 고양이 애니메이션 출력
// --------------------------------------------------------
void DrawCat(HDC memDC)
{
    if (!g_CatImg[catFrame].IsNull()) {
        int imgWidth = g_CatImg[catFrame].GetWidth();
        int imgHeight = g_CatImg[catFrame].GetHeight();

        int drawX = (int)catX - (imgWidth / 2);
        int drawY = (int)catY - (imgHeight / 2);

        // 배경의 하얀색을 투명하게 지우고 출력합니다.
        g_CatImg[catFrame].TransparentBlt(memDC, drawX, drawY, imgWidth, imgHeight, RGB(164, 117, 160));
    }
    else {
        // 이미지 로드 실패 시 노란 삼각형 출력
        HBRUSH catBrush = CreateSolidBrush(RGB(255, 220, 0));
        HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, catBrush);
        POINT pts[3];
        pts[0].x = (LONG)(catX + cos(catAngle) * catSize);
        pts[0].y = (LONG)(catY + sin(catAngle) * catSize);
        pts[1].x = (LONG)(catX + cos(catAngle + 2.4) * catSize);
        pts[1].y = (LONG)(catY + sin(catAngle + 2.4) * catSize);
        pts[2].x = (LONG)(catX + cos(catAngle - 2.4) * catSize);
        pts[2].y = (LONG)(catY + sin(catAngle - 2.4) * catSize);
        Polygon(memDC, pts, 3);
        SelectObject(memDC, oldBrush);
        DeleteObject(catBrush);
    }

    SetBkMode(memDC, TRANSPARENT);
    TCHAR info[64];
    wsprintf(info, L"speed: %d", (int)catSpeed);
    TextOut(memDC, 10, 10, info, lstrlen(info));
}

void DrawMouse(HDC memDC)
{
    if (!mouseVisible) return;

    int r = 18 + mouseFrame * 4;

    HBRUSH br = CreateSolidBrush(RGB(220, 30, 30));
    HBRUSH old = (HBRUSH)SelectObject(memDC, br);
    HPEN   pn = CreatePen(PS_SOLID, 2, RGB(150, 0, 0));
    HPEN   op = (HPEN)SelectObject(memDC, pn);

    Ellipse(memDC, (int)mouseX - r, (int)mouseY - r, (int)mouseX + r, (int)mouseY + r);

    SelectObject(memDC, old);
    SelectObject(memDC, op);
    DeleteObject(br);
    DeleteObject(pn);
}

void DrawFood(HDC memDC)
{
    if (!foodVisible) return;

    int r = 12;

    HBRUSH br = CreateSolidBrush(RGB(50, 200, 50));
    HBRUSH old = (HBRUSH)SelectObject(memDC, br);

    Ellipse(memDC, (int)foodX - r, (int)foodY - r, (int)foodX + r, (int)foodY + r);

    SelectObject(memDC, old);
    DeleteObject(br);
}

void DrawScene(HDC memDC)
{
    DrawFood(memDC);
    DrawMouse(memDC);
    DrawCat(memDC);
}

void ResetGame(HWND hWnd)
{
    catX = wide / 2.0;
    catY = height / 2.0;
    catSpeed = 3.0;
    catAngle = angleDist(gen);
    catFrame = 0;

    catClicked = false;
    catClickTimer = 0;

    mouseVisible = false;
    isDragging = false;

    foodVisible = false;

    InvalidateRect(hWnd, NULL, false);
}