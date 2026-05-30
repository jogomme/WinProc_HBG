#define _CRT_SECURE_NO_WARNINGS

// --------------------------------------------------------
// 라이브러리 추가 구간
// TransparentBlt 함수를 사용하기 위해 msimg32.lib 라이브러리를 링크합니다.
// 이 함수는 비트맵 이미지의 특정 배경색(예: 하얀색)을 투명하게 만들어
// 캐릭터 모양만 화면에 깔끔하게 출력할 수 있게 해줍니다.
// --------------------------------------------------------
#pragma comment(lib, "msimg32.lib")

#include <windows.h>
#include <tchar.h>
#include <random>
#include <iostream>
#include <math.h>
#include "reason.h"

using namespace std;

// --------------------------------------------------------
// 임시 리소스 ID 매크로 정의
// 아직 리소스 뷰에 이미지를 등록하지 않았을 때 발생하는 컴파일 에러를 막기 위함입니다.
// 나중에 프로젝트에 실제 이미지를 넣고 resource.h가 생성되면 이 부분을 지워주세요.
// --------------------------------------------------------
#define IDB_BG 100
#define IDB_CAT_SHOCK1 105
#define IDB_CAT_SHOCK2 106
#define IDB_CAT_SHOCK3 107
#define IDB_CAT_SHOCK4 108
#define IDB_MOUSE1 109
#define IDB_MOUSE2 110
#define IDB_FOOD 111

int wide{ 800 };
int height{ 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_real_distribution<double> angleDist(0.0, 2.0 * 3.14159265);

// --------------------------------------------------------
// WinMain 및 윈도우 초기화
// --------------------------------------------------------
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
// 전역 변수 선언 구간
// --------------------------------------------------------

// 비트맵 애니메이션 프레임을 저장할 배열
// 매번 파일에서 그림을 불러오면 프로그램이 멈추거나 느려지므로,
// 시작할 때 메모리에 한 번만 올려두고 배열 인덱스로 접근하여 사용합니다.
HBITMAP g_hBgImg;             // 배경 이미지
HBITMAP g_hCatImg[4];         // 평상시 걷는 고양이 프레임 4장
HBITMAP g_hCatClickImg[4];    // 클릭 시 놀란 고양이 프레임 4장
HBITMAP g_hMouseImg[2];       // 쥐 애니메이션 프레임 2장
HBITMAP g_hFoodImg;           // 먹이 이미지 (애니메이션 없음)

// 고양이 상태
double catX = 400.0;
double catY = 400.0;
double catAngle = 0.0;
double catSpeed = 3.0;
int    catSize = 30;
int    catFrame = 0;          // 현재 화면에 그릴 고양이 프레임 번호 (0~3)

// 고양이 클릭 상태
bool catClicked = false;
int  catClickTimer = 0;

// 쥐 상태
bool   mouseVisible = false;
double mouseX = 0.0;
double mouseY = 0.0;
int    mouseFrame = 0;        // 현재 화면에 그릴 쥐 프레임 번호 (0~1)
bool   isDragging = false;

// 먹이 상태
bool   foodVisible = false;
double foodX = 0.0;
double foodY = 0.0;

// --------------------------------------------------------
// 함수 선언 구간
// --------------------------------------------------------
void DrawScene(HDC memDC);
void DrawCat(HDC memDC);
void DrawMouse(HDC memDC);
void DrawFood(HDC memDC);
void ResetGame(HWND hWnd);

// --------------------------------------------------------
// 거리 계산 함수
// 두 좌표 사이의 직선 거리를 피타고라스의 정리로 구합니다.
// --------------------------------------------------------
double Distance(double x1, double y1, double x2, double y2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

// --------------------------------------------------------
// 고양이 클릭 판별 함수
// 마우스 좌표가 고양이의 중심으로부터 catSize 이내에 있으면 클릭으로 간주합니다.
// --------------------------------------------------------
bool IsOnCat(int mx, int my)
{
    return Distance((double)mx, (double)my, catX, catY) < (double)catSize;
}

// --------------------------------------------------------
// 윈도우 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    static RECT rectView;

    switch (uMsg) {
    case WM_CREATE:
        GetClientRect(hWnd, &rectView);

        // 1. 프로그램이 시작될 때 필요한 모든 프레임 이미지를 메모리에 로드합니다.
        // 리소스 ID를 사용하여 배열의 각 칸에 비트맵 이미지를 순서대로 넣습니다.
        g_hBgImg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BG));

        g_hCatImg[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CAT_WALK1));
        g_hCatImg[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CAT_WALK2));
        g_hCatImg[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CAT_WALK3));
        g_hCatImg[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CAT_WALK4));

        g_hCatClickImg[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CAT_SHOCK1));
        g_hCatClickImg[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CAT_SHOCK2));
        g_hCatClickImg[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CAT_SHOCK3));
        g_hCatClickImg[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CAT_SHOCK4));

        g_hMouseImg[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_MOUSE1));
        g_hMouseImg[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_MOUSE2));

        g_hFoodImg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_FOOD));

        ResetGame(hWnd);

        // 2. 타이머 2개 설정
        // 1번 타이머 (16ms): 위치 계산용 (약 60프레임 속도로 움직임)
        // 2번 타이머 (100ms): 애니메이션 프레임 변경용 (다리가 빠르게 교차하도록)
        SetTimer(hWnd, 1, 16, NULL);
        SetTimer(hWnd, 2, 100, NULL);
        break;

    case WM_TIMER: {
        // 타이머 1번: 오브젝트들의 좌표를 갱신합니다.
        if (wParam == 1) {
            // 쥐나 먹이가 화면에 있으면 그쪽으로 각도를 틉니다.
            if (mouseVisible) {
                catAngle = atan2(mouseY - catY, mouseX - catX);
            }
            else if (foodVisible) {
                catAngle = atan2(foodY - catY, foodX - catX);
            }

            // 구한 각도를 바탕으로 x와 y 이동량을 계산하여 위치에 더합니다.
            catX += cos(catAngle) * catSpeed;
            catY += sin(catAngle) * catSpeed;

            // 화면 벽에 부딪히면 튕겨 나오는 로직
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

            // 고양이가 먹이 근처에 도달하면 속도가 0.5 증가합니다.
            if (foodVisible && Distance(catX, catY, foodX, foodY) < catSize + 15) {
                foodVisible = false;
                catSpeed += 0.5;
            }

            // 고양이 클릭 애니메이션 지속 시간을 줄여서 0이 되면 해제합니다.
            if (catClicked) {
                catClickTimer--;
                if (catClickTimer <= 0) {
                    catClicked = false;
                }
            }
        }
        // 타이머 2번: 애니메이션 프레임 인덱스를 증가시킵니다.
        // 예를 들어 catFrame은 0, 1, 2, 3이 반복되며 다음 그림을 지시하게 됩니다.
        else if (wParam == 2) {
            catFrame = (catFrame + 1) % 4;
            mouseFrame = (mouseFrame + 1) % 2;
        }

        // 화면 갱신 요청
        InvalidateRect(hWnd, NULL, false);
        break;
    }

    case WM_LBUTTONDOWN: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        // 고양이를 클릭한 경우 타이머와 플래그를 설정하고, 새로운 방향으로 도망가게 합니다.
        if (IsOnCat(mx, my)) {
            catClicked = true;
            catClickTimer = 30; // 약 0.5초간 유지
            catAngle = angleDist(gen);
        }
        // 배경을 클릭한 경우 쥐를 생성하고 드래그 모드로 들어갑니다.
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
        // 드래그 중일 때 쥐의 좌표를 마우스를 따라오게 만듭니다.
        if (isDragging) {
            mouseX = (double)LOWORD(lParam);
            mouseY = (double)HIWORD(lParam);
            InvalidateRect(hWnd, NULL, false);
        }
        break;
    }

    case WM_LBUTTONUP: {
        // 마우스를 떼면 쥐를 없애고 고양이의 방향을 다시 랜덤하게 바꿉니다.
        if (isDragging) {
            mouseVisible = false;
            isDragging = false;
            catAngle = angleDist(gen);
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    }

    case WM_RBUTTONDOWN: {
        // 배경 우클릭 시 마우스 좌표에 먹이를 생성합니다.
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

        // 더블 버퍼링을 위한 메모리 DC 생성
        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBmp = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBmp);

        // 1. 배경 출력
        // 배경 이미지가 정상적으로 로드되었다면 화면 크기에 맞춰 꽉 차게 그립니다.
        // 리소스가 없어서 로드 실패했을 경우를 대비해 기본 연두색 배경도 준비해 둡니다.
        if (g_hBgImg != NULL) {
            HDC bgDC = CreateCompatibleDC(memDC);
            SelectObject(bgDC, g_hBgImg);

            BITMAP bmp;
            GetObject(g_hBgImg, sizeof(BITMAP), &bmp);
            StretchBlt(memDC, 0, 0, wide, height, bgDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
            DeleteDC(bgDC);
        }
        else {
            HBRUSH bgBrush = CreateSolidBrush(RGB(220, 240, 220));
            FillRect(memDC, &ps.rcPaint, bgBrush);
            DeleteObject(bgBrush);
        }

        // 2. 씬 구성 요소 그리기 (먹이 -> 쥐 -> 고양이 순)
        DrawScene(memDC);

        // 3. 완성된 메모리 DC를 화면 hDC로 단번에 복사
        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);

        // 뒷정리
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
        // 타이머 해제
        KillTimer(hWnd, 1);
        KillTimer(hWnd, 2);

        // 게임 종료 시 메모리에 로드해둔 모든 비트맵 리소스를 해제합니다. (메모리 누수 방지)
        DeleteObject(g_hBgImg);
        DeleteObject(g_hFoodImg);
        for (int i = 0; i < 4; i++) {
            DeleteObject(g_hCatImg[i]);
            DeleteObject(g_hCatClickImg[i]);
        }
        for (int i = 0; i < 2; i++) {
            DeleteObject(g_hMouseImg[i]);
        }

        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// --------------------------------------------------------
// 고양이 애니메이션 출력 함수
// --------------------------------------------------------
void DrawCat(HDC memDC)
{
    HBITMAP currentFrameBitmap;

    // 클릭 여부에 따라 재생할 애니메이션 프레임(이미지) 배열을 선택합니다.
    if (catClicked) {
        currentFrameBitmap = g_hCatClickImg[catFrame];
    }
    else {
        currentFrameBitmap = g_hCatImg[catFrame];
    }

    // 이미지가 정상적으로 배열에 들어있다면 투명 처리를 거쳐 화면에 출력합니다.
    if (currentFrameBitmap != NULL) {
        HDC imgDC = CreateCompatibleDC(memDC);
        HBITMAP oldBmp = (HBITMAP)SelectObject(imgDC, currentFrameBitmap);

        BITMAP bmp;
        GetObject(currentFrameBitmap, sizeof(BITMAP), &bmp);

        // 고양이의 좌표(catX, catY)를 기준으로 정중앙에 이미지가 오도록 좌상단 좌표를 계산합니다.
        int drawX = (int)catX - (bmp.bmWidth / 2);
        int drawY = (int)catY - (bmp.bmHeight / 2);

        // TransparentBlt를 사용하여 이미지의 배경을 빼고 출력합니다.
        // 여기서 마지막 인자 RGB(255, 255, 255)는 '하얀색'을 투명하게 없애라는 뜻입니다.
        // 준비하신 고양이 이미지의 배경이 다른 색(예: 마젠타)이라면 그 색에 맞춰 RGB 값을 수정해야 합니다.
        TransparentBlt(memDC, drawX, drawY, bmp.bmWidth, bmp.bmHeight,
            imgDC, 0, 0, bmp.bmWidth, bmp.bmHeight, RGB(255, 255, 255));

        SelectObject(imgDC, oldBmp);
        DeleteDC(imgDC);
    }

    // 현재 고양이의 속도를 좌측 상단에 텍스트로 출력합니다.
    SetBkMode(memDC, TRANSPARENT);
    TCHAR info[64];
    wsprintf(info, L"speed: %d", (int)catSpeed);
    TextOut(memDC, 10, 10, info, lstrlen(info));
}

// --------------------------------------------------------
// 쥐 애니메이션 출력 함수
// --------------------------------------------------------
void DrawMouse(HDC memDC)
{
    if (!mouseVisible) return;

    // 마우스 프레임 변수(mouseFrame)를 인덱스로 사용하여 이미지를 번갈아 그립니다.
    if (g_hMouseImg[mouseFrame] != NULL) {
        HDC imgDC = CreateCompatibleDC(memDC);
        HBITMAP oldBmp = (HBITMAP)SelectObject(imgDC, g_hMouseImg[mouseFrame]);

        BITMAP bmp;
        GetObject(g_hMouseImg[mouseFrame], sizeof(BITMAP), &bmp);

        int drawX = (int)mouseX - (bmp.bmWidth / 2);
        int drawY = (int)mouseY - (bmp.bmHeight / 2);

        // 쥐 이미지 배경 역시 하얀색을 투명으로 처리하여 출력합니다.
        TransparentBlt(memDC, drawX, drawY, bmp.bmWidth, bmp.bmHeight,
            imgDC, 0, 0, bmp.bmWidth, bmp.bmHeight, RGB(255, 255, 255));

        SelectObject(imgDC, oldBmp);
        DeleteDC(imgDC);
    }
}

// --------------------------------------------------------
// 먹이 비트맵 출력 함수
// 먹이는 애니메이션이 없으므로 단일 이미지를 사용합니다.
// --------------------------------------------------------
void DrawFood(HDC memDC)
{
    if (!foodVisible) return;

    if (g_hFoodImg != NULL) {
        HDC imgDC = CreateCompatibleDC(memDC);
        HBITMAP oldBmp = (HBITMAP)SelectObject(imgDC, g_hFoodImg);

        BITMAP bmp;
        GetObject(g_hFoodImg, sizeof(BITMAP), &bmp);

        int drawX = (int)foodX - (bmp.bmWidth / 2);
        int drawY = (int)foodY - (bmp.bmHeight / 2);

        // 먹이 이미지 역시 배경을 하얀색으로 투명 처리합니다.
        TransparentBlt(memDC, drawX, drawY, bmp.bmWidth, bmp.bmHeight,
            imgDC, 0, 0, bmp.bmWidth, bmp.bmHeight, RGB(255, 255, 255));

        SelectObject(imgDC, oldBmp);
        DeleteDC(imgDC);
    }
}

// --------------------------------------------------------
// 전체 씬 구성 함수
// 화면에 그려질 객체들의 z-order(앞뒤 순서)를 결정합니다.
// 나중에 호출되는 함수가 화면 맨 앞으로 옵니다. (고양이가 가장 위)
// --------------------------------------------------------
void DrawScene(HDC memDC)
{
    DrawFood(memDC);
    DrawMouse(memDC);
    DrawCat(memDC);
}

// --------------------------------------------------------
// 게임 초기화 함수
// r 키를 눌렀거나, 프로그램 시작 시 초기 상태로 돌려놓습니다.
// --------------------------------------------------------
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