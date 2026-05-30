#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "msimg32.lib")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include <iostream>
#include "resource.h"

using namespace std;
int wide{ 1200 };
int height{ 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> sizeDist(10, 50);
uniform_int_distribution<int> colorDist(0, 2);
uniform_int_distribution<int> crazyColor(0, 255);
uniform_int_distribution<int> pointDist(50, 750);
uniform_int_distribution<int> dirDist(1, 4);

// --------------------------------------------------------
// 윈 메인
// --------------------------------------------------------
HINSTANCE g_hInst;
LPCTSTR IpszClass = L"My Window Class 3";
LPCTSTR IpszWindowName = L"실습 5-6: 폭주하는 팩맨 완성본";
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
// 구조체 선언 구간
// --------------------------------------------------------
struct entity
{
    int Demege;
    int hp;
    int size;
    int speed;
    POINT p;
    COLORREF color;

    int dir;
    int mouthFrame;
    int mouthAnimDir;
};

struct History {
    POINT p;
    int dir;
    int mouthFrame;
};

// --------------------------------------------------------
// 함수 선언 구간
// --------------------------------------------------------
void DrowMap(HDC memDC, HWND hWnd);
void ResetMap();
void DrowPac(HDC memDC, int cx, int cy, int size, int dir, int frame, COLORREF color);
void changePac(int i);

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
HBITMAP g_BackGround;
BITMAP bmpInfo;

entity PacMan;
int MaxBarrior = 20;
entity Barrior[40];

entity prey[40];
int CurPrey = 0;

int const PacAnime = 1;
int const PacMove = 2;
int const insertPrey = 3;
int const PacAttack = 4;

POINT ATTACK[10] = {};
int ATTACKdir[10] = {};
bool ATTACKactive[10] = {};

bool isTogleUp = false;
bool isTogleLeft = false;
bool isTogleDown = false;
bool isTogleRight = false;

int jumpY = 0;
int jumpVelocity = 0;
bool isJumping = false;

bool isCrazy = false;
int twinCount = 0;
History pacHistory[100];

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

        g_BackGround = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_WOODPAINT));
        if (g_BackGround != NULL) GetObject(g_BackGround, sizeof(BITMAP), &bmpInfo);

        ResetMap();
        SetTimer(hWnd, PacAnime, 32, NULL);
        SetTimer(hWnd, insertPrey, 1600, NULL);
        SetTimer(hWnd, PacMove, 16, NULL);
        break;

    case WM_LBUTTONDOWN: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);
        int pSize = PacMan.size / 2;

        if (mx > PacMan.p.x - pSize && mx < PacMan.p.x + pSize &&
            my >(PacMan.p.y - jumpY) - pSize && my < (PacMan.p.y - jumpY) + pSize) {

            isCrazy = true;
            PacMan.dir = dirDist(gen);
            SetTimer(hWnd, 99, 2000, NULL);
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        for (int i = 0; i < MaxBarrior; ++i) {
            int s = Barrior[i].size / 2;
            if (mx > Barrior[i].p.x - s && mx < Barrior[i].p.x + s &&
                my > Barrior[i].p.y - s && my < Barrior[i].p.y + s) {

                Barrior[i].p.x += (sizeDist(gen) - 25);
                Barrior[i].p.y += (sizeDist(gen) - 25);
                InvalidateRect(hWnd, NULL, false);
                break;
            }
        }
        break;
    }

    case WM_KEYDOWN: {
        if (wParam == 'Q') PostQuitMessage(0);
        else if (wParam == VK_UP) { isTogleUp = true; }
        else if (wParam == VK_LEFT) { isTogleLeft = true; }
        else if (wParam == VK_DOWN) { isTogleDown = true; }
        else if (wParam == VK_RIGHT) { isTogleRight = true; }

        else if (wParam == VK_RETURN) {
            SetTimer(hWnd, PacAttack, 16, NULL);
            for (int i = 0; i < 10; ++i) {
                if (!ATTACKactive[i]) {
                    ATTACKactive[i] = true;
                    ATTACK[i].x = PacMan.p.x;
                    ATTACK[i].y = PacMan.p.y - jumpY;
                    ATTACKdir[i] = PacMan.dir;
                    break;
                }
            }
        }
        else if (wParam == 'J') {
            if (!isJumping) {
                isJumping = true;
                jumpVelocity = 15;
            }
        }
        else if (wParam == 'T') {
            if (twinCount < 3) twinCount++;
        }
        else if (wParam == 'A') {
            CurPrey = 0;
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_KEYUP: {
        if (wParam == VK_UP) isTogleUp = false;
        else if (wParam == VK_LEFT) isTogleLeft = false;
        else if (wParam == VK_DOWN) isTogleDown = false;
        else if (wParam == VK_RIGHT) isTogleRight = false;

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_TIMER: {
        if (wParam == 99) {
            isCrazy = false;
            KillTimer(hWnd, 99);
        }
        else if (wParam == PacAnime) {
            PacMan.mouthFrame += PacMan.mouthAnimDir;
            if (PacMan.mouthFrame >= 4) { PacMan.mouthFrame = 4; PacMan.mouthAnimDir = -1; }
            else if (PacMan.mouthFrame <= 0) { PacMan.mouthFrame = 0; PacMan.mouthAnimDir = 1; }
        }
        else if (wParam == PacMove) {
            for (int i = 99; i > 0; i--) pacHistory[i] = pacHistory[i - 1];
            pacHistory[0] = { PacMan.p, PacMan.dir, PacMan.mouthFrame };

            if (isJumping) {
                jumpY += jumpVelocity;
                jumpVelocity -= 1;
                if (jumpY <= 0) {
                    jumpY = 0;
                    isJumping = false;
                }
            }

            int nextX = PacMan.p.x;
            int nextY = PacMan.p.y;

            bool isManual = (isTogleUp || isTogleDown || isTogleLeft || isTogleRight);

            // [수정] 조작 상태에 따른 이동 방향 설정
            if (isManual) {
                if (isTogleUp) { PacMan.dir = 1; nextY -= PacMan.speed; }
                else if (isTogleDown) { PacMan.dir = 2; nextY += PacMan.speed; }
                else if (isTogleLeft) { PacMan.dir = 3; nextX -= PacMan.speed; }
                else if (isTogleRight) { PacMan.dir = 4; nextX += PacMan.speed; }
            }
            else {
                // 자동 순찰 모드: 일단 현재 바라보는 방향으로 계속 전진
                if (PacMan.dir == 1) nextY -= PacMan.speed;
                else if (PacMan.dir == 2) nextY += PacMan.speed;
                else if (PacMan.dir == 3) nextX -= PacMan.speed;
                else if (PacMan.dir == 4) nextX += PacMan.speed;
            }

            bool canMove = true;
            int pacSize = PacMan.size / 2;

            // 장애물 충돌 검사
            for (int i = 0; i < MaxBarrior; ++i) {
                if (Barrior[i].hp <= 0) continue;

                int cx = Barrior[i].p.x;
                int cy = Barrior[i].p.y;
                int s = Barrior[i].size / 2;

                if (nextX + pacSize / 2 > cx - s && nextX - pacSize / 2 < cx + s &&
                    nextY + pacSize / 2 > cy - s && nextY - pacSize / 2 < cy + s) {
                    canMove = false;
                    break;
                }
            }

            // 화면 끝 충돌 방지
            if (nextX - pacSize / 2 < 0 || nextX + pacSize / 2 > wide ||
                nextY - pacSize / 2 < 0 || nextY + pacSize / 2 > height) {
                canMove = false;
            }

            // [핵심 로직] 충돌 여부에 따른 처리
            if (canMove) {
                // 부딪힌 게 없으면 그대로 이동 적용
                PacMan.p.x = nextX;
                PacMan.p.y = nextY;
            }
            else {
                // 장애물이나 벽에 부딪혔고, 자동 순찰 모드일 때만 방향 꺾기
                if (!isManual) {
                    if (PacMan.dir == 1) PacMan.dir = 4;      // 상 -> 우
                    else if (PacMan.dir == 4) PacMan.dir = 2; // 우 -> 하
                    else if (PacMan.dir == 2) PacMan.dir = 3; // 하 -> 좌
                    else if (PacMan.dir == 3) PacMan.dir = 1; // 좌 -> 상
                }
            }

            // 먹이 이동 로직
            for (int i = 0; i < CurPrey; i++) {
                if (prey[i].dir == 1) prey[i].p.y -= prey[i].speed;
                else if (prey[i].dir == 2) prey[i].p.y += prey[i].speed;
                else if (prey[i].dir == 3) prey[i].p.x -= prey[i].speed;
                else if (prey[i].dir == 4) prey[i].p.x += prey[i].speed;

                if (prey[i].p.x < 0) { prey[i].p.x = 0; prey[i].dir = 4; }
                else if (prey[i].p.x > wide) { prey[i].p.x = wide; prey[i].dir = 3; }
                if (prey[i].p.y < 0) { prey[i].p.y = 0; prey[i].dir = 2; }
                else if (prey[i].p.y > height) { prey[i].p.y = height; prey[i].dir = 1; }
            }

            // 팩맨 vs 먹이 먹방 충돌
            for (int i = 0; i < CurPrey; ) {
                int px = prey[i].p.x;
                int py = prey[i].p.y;
                int ps = prey[i].size / 2;

                if (PacMan.p.x + pacSize > px - ps && PacMan.p.x - pacSize < px + ps &&
                    PacMan.p.y + pacSize > py - ps && PacMan.p.y - pacSize < py + ps) {

                    changePac(i);
                    prey[i] = prey[CurPrey - 1];
                    CurPrey--;

                    PacMan.size += 5;
                }
                else {
                    i++;
                }
            }
        }
        else if (wParam == insertPrey) {
            if (CurPrey < 40) {
                prey[CurPrey].color = colorDist(gen);
                prey[CurPrey].p.x = pointDist(gen);
                prey[CurPrey].p.y = pointDist(gen);
                prey[CurPrey].dir = dirDist(gen);
                prey[CurPrey].speed = 2;
                prey[CurPrey++].size = sizeDist(gen) + 20;
            }
        }
        else if (wParam == PacAttack) {
            for (int i = 0; i < 10; ++i) {
                if (!ATTACKactive[i]) continue;

                if (ATTACKdir[i] == 1) ATTACK[i].y -= 25;
                else if (ATTACKdir[i] == 2) ATTACK[i].y += 25;
                else if (ATTACKdir[i] == 3) ATTACK[i].x -= 25;
                else if (ATTACKdir[i] == 4) ATTACK[i].x += 25;

                if (ATTACK[i].x < 0 || ATTACK[i].x > wide || ATTACK[i].y < 0 || ATTACK[i].y > height) {
                    ATTACKactive[i] = false;
                    continue;
                }

                bool hit = false;
                for (int j = 0; j < MaxBarrior; ++j) {
                    if (Barrior[j].hp <= 0) continue;

                    int cx = Barrior[j].p.x;
                    int cy = Barrior[j].p.y;
                    int s = Barrior[j].size / 2;

                    if (ATTACK[i].x > cx - s && ATTACK[i].x < cx + s &&
                        ATTACK[i].y > cy - s && ATTACK[i].y < cy + s) {

                        Barrior[j].hp -= 20;
                        if (Barrior[j].hp <= 0) {
                            Barrior[j].p.x = -1000;
                            PacMan.speed += 2;
                        }
                        ATTACKactive[i] = false;
                        hit = true;
                        break;
                    }
                }
                if (hit) continue;

                for (int f = 0; f < CurPrey; ++f) {
                    int px = prey[f].p.x;
                    int py = prey[f].p.y;
                    int ps = prey[f].size / 2;
                    int hitbox = ps + 15;

                    if (ATTACK[i].x > px - hitbox && ATTACK[i].x < px + hitbox &&
                        ATTACK[i].y > py - hitbox && ATTACK[i].y < py + hitbox) {

                        ATTACKactive[i] = false;
                        if (CurPrey < 39) {
                            prey[CurPrey] = prey[f];
                            prey[CurPrey].p.x += 40;
                            prey[CurPrey].dir = dirDist(gen);
                            CurPrey++;
                        }
                        break;
                    }
                }
            }
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);

        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        if (g_BackGround) {
            HDC imgDC = CreateCompatibleDC(hDC);
            SelectObject(imgDC, g_BackGround);
            StretchBlt(memDC, 0, 0, wide, height, imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
            DeleteDC(imgDC);
        }
        else {
            HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &ps.rcPaint, bgBrush);
            DeleteObject(bgBrush);
        }

        DrowMap(memDC, hWnd);

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
        DeleteObject(g_BackGround);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void DrowMap(HDC memDC, HWND hWnd)
{
    for (int i = 0; i < MaxBarrior; ++i) {
        if (Barrior[i].hp <= 0) continue;
        HBRUSH BarriorBrush = CreateSolidBrush(Barrior[i].color);
        HBRUSH oldBarriorBrush = (HBRUSH)SelectObject(memDC, BarriorBrush);

        int cx = Barrior[i].p.x;
        int cy = Barrior[i].p.y;
        int s = Barrior[i].size / 2;

        Rectangle(memDC, cx - s, cy - s, cx + s, cy + s);

        SelectObject(memDC, oldBarriorBrush);
        DeleteObject(BarriorBrush);
    }

    for (int i = 0; i < CurPrey; ++i) {
        COLORREF c = RGB(255, 255, 255);
        if (prey[i].color == 0) c = RGB(255, 0, 0);
        else if (prey[i].color == 1) c = RGB(0, 255, 0);
        else if (prey[i].color == 2) c = RGB(0, 0, 255);

        HBRUSH PreyBrush = CreateSolidBrush(c);
        HBRUSH oldPreyBrush = (HBRUSH)SelectObject(memDC, PreyBrush);

        int cx = prey[i].p.x;
        int cy = prey[i].p.y;
        int s = prey[i].size / 2;

        Ellipse(memDC, cx - s, cy - s, cx + s, cy + s);

        SelectObject(memDC, oldPreyBrush);
        DeleteObject(PreyBrush);
    }

    HBRUSH bulletBrush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH oldBulletBrush = (HBRUSH)SelectObject(memDC, bulletBrush);
    for (int i = 0; i < 10; ++i) {
        if (ATTACKactive[i]) {
            int cx = ATTACK[i].x;
            int cy = ATTACK[i].y;
            Ellipse(memDC, cx - 8, cy - 8, cx + 8, cy + 8);
        }
    }
    SelectObject(memDC, oldBulletBrush);
    DeleteObject(bulletBrush);

    for (int i = 1; i <= twinCount; i++) {
        int idx = i * 15;
        if (idx < 100) {
            DrowPac(memDC, pacHistory[idx].p.x, pacHistory[idx].p.y, PacMan.size - 10, pacHistory[idx].dir, pacHistory[idx].mouthFrame, RGB(200, 200, 200));
        }
    }

    COLORREF drawColor = isCrazy ? RGB(crazyColor(gen), crazyColor(gen), crazyColor(gen)) : PacMan.color;
    DrowPac(memDC, PacMan.p.x, PacMan.p.y - jumpY, PacMan.size, PacMan.dir, PacMan.mouthFrame, drawColor);

    if (isJumping) {
        HBRUSH shadowBrush = CreateSolidBrush(RGB(150, 150, 150));
        HBRUSH oldShadow = (HBRUSH)SelectObject(memDC, shadowBrush);
        Ellipse(memDC, PacMan.p.x - 15, PacMan.p.y - 5, PacMan.p.x + 15, PacMan.p.y + 5);
        SelectObject(memDC, oldShadow);
        DeleteObject(shadowBrush);
    }
}

void DrowPac(HDC memDC, int cx, int cy, int sizeParam, int dir, int frame, COLORREF color)
{
    HBRUSH PacBrush = CreateSolidBrush(color);
    HBRUSH oldPacBrush = (HBRUSH)SelectObject(memDC, PacBrush);

    int size = sizeParam / 2;
    int startX, startY, endX, endY;
    int MouseSize = 5;
    int m = MouseSize * frame;

    if (dir == 1) {
        startX = cx - m; endX = cx + m; startY = cy - size; endY = cy - size;
    }
    else if (dir == 2) {
        startX = cx + m; endX = cx - m; startY = cy + size; endY = cy + size;
    }
    else if (dir == 3) {
        startX = cx - size; endX = cx - size; startY = cy + m; endY = cy - m;
    }
    else if (dir == 4) {
        startX = cx + size; endX = cx + size; startY = cy - m; endY = cy + m;
    }

    if (frame == 0) {
        Ellipse(memDC, cx - size, cy - size, cx + size, cy + size);
    }
    else {
        Pie(memDC, cx - size, cy - size, cx + size, cy + size, startX, startY, endX, endY);
    }

    SelectObject(memDC, oldPacBrush);
    DeleteObject(PacBrush);
}

void ResetMap()
{
    PacMan.color = RGB(255, 255, 0);
    PacMan.speed = 8;
    PacMan.size = 50;
    PacMan.hp = -1;
    PacMan.p.x = wide / 2;
    PacMan.p.y = height / 2;
    PacMan.dir = 4;
    PacMan.mouthFrame = 0;
    PacMan.mouthAnimDir = 1;

    isJumping = false;
    jumpY = 0;
    twinCount = 0;
    isCrazy = false;

    for (int i = 0; i < 100; i++) {
        pacHistory[i].p = PacMan.p;
        pacHistory[i].dir = PacMan.dir;
    }

    for (int i = 0; i < MaxBarrior; ++i) {
        Barrior[i].color = RGB(139, 69, 19);
        Barrior[i].size = sizeDist(gen) + 40;
        Barrior[i].hp = Barrior[i].size * 2;

        Barrior[i].p.x = pointDist(gen);
        Barrior[i].p.y = pointDist(gen);
    }

    CurPrey = 0;
    for (int i = 0; i < 10; ++i) ATTACKactive[i] = false;
}

void changePac(int i)
{
    if (prey[i].color == 0) PacMan.color = RGB(255, 0, 0);
    else if (prey[i].color == 1) PacMan.color = RGB(0, 255, 0);
    else if (prey[i].color == 2) PacMan.color = RGB(0, 0, 255);
}