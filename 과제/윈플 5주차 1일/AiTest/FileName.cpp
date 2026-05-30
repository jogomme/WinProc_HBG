#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include <iostream>
#include <math.h>

using namespace std;

int wide{ 600 };
int height{ 600 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_int_distribution<int> uidPoint(0, 29);
uniform_int_distribution<int> uidColorSet(0, 3);

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
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 배경 색깔
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = IpszClass; // 윈도우 클래스 이름 정하기
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(IpszClass, IpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, wide, height, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) { // 메시지 반복
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return Message.wParam;
}

int map[31][31]{};
int objCount = 0;
int RowNum = 30;
int CalNum = 30;

// --------------------------------------------------------
// 클래스, 구조체 선언 구간
// --------------------------------------------------------
struct Player
{
    POINT firstPoint;
    POINT point;
    int shape = 0;
    int color;

    // ?? 꼬리 시스템 관련 변수 추가!
    int tailCount = 0;
    int tailColors[100] = { 0 };
    int rot = 0; // 꼬리가 뻗는 방향 (0:위, 1:우, 2:아래, 3:좌)
};

struct Entity
{
    POINT point;
    int shape = 1;
    int color;
};

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
Player player;

// ?? 메모리 터짐 방지를 위해 배열 크기를 1000으로 넉넉하게!
Entity entity[1000];
int cnt{};

COLORREF colorSet[4] =
{
    {RGB(colorDist(gen),colorDist(gen),colorDist(gen))},
    {RGB(colorDist(gen),colorDist(gen),colorDist(gen))},
    {RGB(colorDist(gen),colorDist(gen),colorDist(gen))},
    {RGB(colorDist(gen),colorDist(gen),colorDist(gen))}
};

// --------------------------------------------------------
// 함수 선언
// --------------------------------------------------------
void insertEntity(HWND hWnd, int x, int y, int color = -1);
void deleteEntity(HWND hWnd, int x, int y);
void Draw(HDC memDC, int shape, int color, POINT p);
void Move(HWND hWnd, int direction);
void InitMap();
void clearLine(HWND hWnd);
void setLine(HWND hWnd, int x, int y, int color);
void spin(HWND hWnd);

// --------------------------------------------------------
// 타이머 관련
// --------------------------------------------------------
const int TimerMove = 0;
int TickRate = 300;

void CALLBACK TimerProc(HWND hWnd, UINT iMsg, UINT idEvent, DWORD dwTime)
{
    switch (idEvent) {
    case TimerMove:
        Move(hWnd, 1); // 기본적으로 아래로 떨어짐
        break;
    }
}

// --------------------------------------------------------
// 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    switch (uMsg) {
    case WM_CREATE:
        SetTimer(hWnd, TimerMove, TickRate, TIMERPROC(TimerProc));
        InitMap();
        break;

    case WM_LBUTTONDOWN: {
        int Mx = LOWORD(lParam);
        int My = HIWORD(lParam);

        int x = (Mx * CalNum) / wide;
        int y = (My * RowNum) / height;

        if (map[y][x] == 0) {
            map[y][x] = 1;
            insertEntity(hWnd, x, y);
            InvalidateRect(hWnd, NULL, false); // ?? 즉시 새로고침
        }
        break;
    }
    case WM_RBUTTONDOWN: {
        int Mx = LOWORD(lParam);
        int My = HIWORD(lParam);

        int x = (Mx * CalNum) / wide;
        int y = (My * RowNum) / height;

        if (map[y][x] == 1) {
            deleteEntity(hWnd, x, y);
            map[y][x] = 0;
            InvalidateRect(hWnd, NULL, false); // ?? 즉시 새로고침
        }
        break;
    }
    case WM_KEYDOWN:
        if (wParam == 'Q') {
            PostQuitMessage(2025180028);
        }
        else if (wParam == 'C') {
            cout << "TickRate : ";
            cin >> TickRate;
            KillTimer(hWnd, TimerMove);
            SetTimer(hWnd, TimerMove, TickRate, (TIMERPROC)TimerProc);
        }
        else if (wParam == 'R') {
            InitMap();
            InvalidateRect(hWnd, NULL, false);
        }
        else if (wParam == 'K') {
            int x{}, y{}, color{};
            cout << "변환 할 칸 입력 (x y) : "; cin >> x >> y;
            cout << "변환 할 색깔 입력 (0~3) : "; cin >> color;
            setLine(hWnd, x, y, color);
        }
        else if (wParam == VK_UP) {
            Move(hWnd, 0);
        }
        else if (wParam == VK_DOWN) {
            Move(hWnd, 1);
        }
        else if (wParam == VK_LEFT) {
            Move(hWnd, 2);
        }
        else if (wParam == VK_RIGHT) {
            Move(hWnd, 3);
        }
        else if (wParam == VK_RETURN) {
            spin(hWnd); // ?? 엔터키 회전
        }
        break;

    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);

        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);

        for (int i = 0; i <= CalNum; ++i) {
            MoveToEx(memDC, 0, (height * i) / CalNum, NULL);
            LineTo(memDC, wide, (height * i) / CalNum);
        }
        for (int i = 0; i <= RowNum; ++i) {
            MoveToEx(memDC, wide * i / RowNum, 0, NULL);
            LineTo(memDC, wide * i / RowNum, height);
        }

        // 1. 맵에 깔린 아이템 그리기
        for (int i = 0; i < objCount; ++i) {
            Draw(memDC, entity[i].shape, entity[i].color, entity[i].point);
        }

        // 2. 주인공 원 그리기
        Draw(memDC, player.shape, player.color, player.point);

        // ?? 3. 주인공 꼬리들 줄줄이 달아서 그리기
        for (int i = 0; i < player.tailCount; ++i) {
            POINT tailPt = player.point;

            if (player.rot == 0) tailPt.y -= (i + 1);
            else if (player.rot == 1) tailPt.x += (i + 1);
            else if (player.rot == 2) tailPt.y += (i + 1);
            else if (player.rot == 3) tailPt.x -= (i + 1);

            Draw(memDC, 1, player.tailColors[i], tailPt); // 꼬리는 사각형
        }

        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);
        SelectObject(memDC, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_SIZE:
        wide = LOWORD(lParam);
        height = HIWORD(lParam);
        InvalidateRect(hWnd, NULL, true);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// --------------------------------------------------------
// 함수 구현
// --------------------------------------------------------

void spin(HWND hWnd)
{
    int px = player.point.x;
    int py = player.point.y;

    // ?? 1단계: Move 함수처럼 주인공과 연결된 '한 덩어리'를 싹 다 찾습니다.
    bool visited[31][31] = { false };
    POINT cluster[1000];
    int clusterSize = 0;

    POINT q[1000];
    int front = 0, rear = 0;

    q[rear++] = { px, py };
    visited[py][px] = true;

    while (front < rear) {
        POINT curr = q[front++];
        cluster[clusterSize++] = curr;

        int dirX[] = { 0, 0, -1, 1 };
        int dirY[] = { -1, 1, 0, 0 };

        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dirX[i];
            int ny = curr.y + dirY[i];

            if (nx >= 0 && nx < CalNum && ny >= 0 && ny < RowNum) {
                if (!visited[ny][nx] && map[ny][nx] == 1) {
                    visited[ny][nx] = true;
                    q[rear++] = { nx, ny };
                }
            }
        }
    }

    // 주인공 혼자면 돌릴 게 없으니 종료
    if (clusterSize <= 1) return;

    // ?? 2단계: 돌렸을 때 벽에 박거나 남의 블록에 겹치는지 시뮬레이션 해봅니다.
    bool canSpin = true;
    POINT newCluster[1000]; // 회전 후 새 좌표를 저장할 곳

    for (int i = 0; i < clusterSize; i++) {
        if (cluster[i].x == px && cluster[i].y == py) {
            newCluster[i] = { px, py }; // 주인공은 중심축이므로 제자리
            continue;
        }

        // 시계방향 90도 회전 공식 적용! (주인공을 0,0 축으로 계산)
        int rx = cluster[i].x - px;
        int ry = cluster[i].y - py;

        int nx = px - ry;
        int ny = py + rx;

        newCluster[i] = { nx, ny };

        // 돌렸는데 화면 밖으로 나가면 실패
        if (nx < 0 || nx >= CalNum || ny < 0 || ny >= RowNum) {
            canSpin = false;
            break;
        }
        // 돌렸는데 우리 덩어리가 아닌 엉뚱한 벽에 부딪히면 실패
        if (map[ny][nx] == 1 && !visited[ny][nx]) {
            canSpin = false;
            break;
        }
    }

    // ?? 3단계: 안전하게 돌아간다면 진짜로 돌려줍니다!
    if (canSpin) {
        // (1) 맵에서 기존 덩어리들을 투명하게 지웁니다.
        for (int i = 0; i < clusterSize; i++) {
            if (cluster[i].x != px || cluster[i].y != py) {
                map[cluster[i].y][cluster[i].x] = 0;
            }
        }

        // (2) 그림(entity)들을 안전하게 옮기기 위해 임시 우주(-1000)로 보냅니다.
        // (바로 좌표를 바꾸면 블록끼리 겹쳐서 증발하는 버그가 생길 수 있어서 대피시키는 것!)
        for (int i = 0; i < clusterSize; i++) {
            if (cluster[i].x == px && cluster[i].y == py) continue;
            for (int k = 0; k < objCount; ++k) {
                if (entity[k].point.x == cluster[i].x && entity[k].point.y == cluster[i].y) {
                    entity[k].point.x = -1000 - i;
                    entity[k].point.y = -1000 - i;
                    break;
                }
            }
        }

        // (3) 대피시켰던 그림들을 새로운 좌표로 불러오고 맵에 블록(1)을 칠해줍니다.
        for (int i = 0; i < clusterSize; i++) {
            if (cluster[i].x == px && cluster[i].y == py) continue;

            map[newCluster[i].y][newCluster[i].x] = 1;

            for (int k = 0; k < objCount; ++k) {
                if (entity[k].point.x == -1000 - i && entity[k].point.y == -1000 - i) {
                    entity[k].point.x = newCluster[i].x;
                    entity[k].point.y = newCluster[i].y;
                    break;
                }
            }
        }
    }

    InvalidateRect(hWnd, NULL, false);
}

void setLine(HWND hWnd, int x, int y, int color)
{
    if (map[y][x] == 1) {
        deleteEntity(hWnd, x, y);
    }
    insertEntity(hWnd, x, y, color);
}

void clearLine(HWND hWnd)
{
    // 한 줄 꽉 차면 지우는 함수 (필요 시 추후 연동)
    // 현재는 주인공이 바닥에 굳는 로직이 흡수 시스템으로 대체되어 대기 상태
}

void Move(HWND hWnd, int direction)
{
    int px = player.point.x;
    int py = player.point.y;
    int dx = 0, dy = 0;

    if (direction == 0) dy = -1; // UP
    else if (direction == 1) dy = 1;  // DOWN
    else if (direction == 2) dx = -1; // LEFT
    else if (direction == 3) dx = 1;  // RIGHT

    // ?? 1단계: BFS 알고리즘을 이용해 주인공과 연결된 모든 '한 덩어리(블록들)' 찾기
    bool visited[31][31] = { false };
    POINT cluster[1000]; // 덩어리에 속한 칸들을 모아둘 배열
    int clusterSize = 0;

    POINT q[1000]; // 탐색을 위한 대기열(큐)
    int front = 0, rear = 0;

    // 일단 주인공부터 큐에 넣고 시작
    q[rear++] = { px, py };
    visited[py][px] = true;

    // 연결된 모든 블록을 싹 다 찾아냅니다!
    while (front < rear) {
        POINT curr = q[front++];
        cluster[clusterSize++] = curr; // 찾은 놈은 덩어리 배열에 등록

        // 상하좌우 검사
        int dirX[] = { 0, 0, -1, 1 };
        int dirY[] = { -1, 1, 0, 0 };

        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dirX[i];
            int ny = curr.y + dirY[i];

            if (nx >= 0 && nx < CalNum && ny >= 0 && ny < RowNum) {
                // 검사한 곳에 블록(1)이 있고 아직 검사 안한 곳이면 큐에 추가 (연결됨!)
                if (!visited[ny][nx] && map[ny][nx] == 1) {
                    visited[ny][nx] = true;
                    q[rear++] = { nx, ny };
                }
            }
        }
    }

    // ?? 2단계: 찾아낸 이 거대한 덩어리가 통째로 이동할 수 있는지 벽 검사하기
    bool canMove = true;
    for (int i = 0; i < clusterSize; i++) {
        int nx = cluster[i].x + dx;
        int ny = cluster[i].y + dy;

        // 벽이나 바닥에 닿으면 이동 불가
        if (nx < 0 || nx >= CalNum || ny >= RowNum || ny < 0) {
            canMove = false;
            break;
        }
        // 이동할 자리에 블록(1)이 있는데, 그게 우리 덩어리에 속한 놈이 아니면 막힌 것!
        if (map[ny][nx] == 1 && !visited[ny][nx]) {
            canMove = false;
            break;
        }
    }

    // ?? 3단계: 이동이 가능하다면 덩어리를 통째로 옮겨버리기!
    if (canMove) {
        // (1) 맵에서 현재 덩어리들을 투명하게 지워줍니다.
        for (int i = 0; i < clusterSize; i++) {
            if (cluster[i].x != px || cluster[i].y != py) { // 주인공은 map에 없으므로 예외처리
                map[cluster[i].y][cluster[i].x] = 0;
            }
        }

        // (2) 화면에 그려진 그림(entity)들의 좌표도 한꺼번에 이동!
        for (int k = 0; k < objCount; ++k) {
            // 이 그림이 우리 덩어리(visited)에 속해있는 애라면 같이 이동
            if (visited[entity[k].point.y][entity[k].point.x]) {
                entity[k].point.x += dx;
                entity[k].point.y += dy;
            }
        }

        // (3) 이동한 새 위치에 맵(1)을 다시 채워줍니다.
        for (int i = 0; i < clusterSize; i++) {
            if (cluster[i].x != px || cluster[i].y != py) {
                map[cluster[i].y + dy][cluster[i].x + dx] = 1;
            }
        }

        // (4) 주인공 위치 최종 이동
        player.point.x += dx;
        player.point.y += dy;
    }
    // 아래로 내려가려다 바닥/장애물에 막혔을 때 (스폰 리셋)
    else if (dy == 1) {
        player.point.y = 0;
        player.point.x = uidPoint(gen);
        clearLine(hWnd);
    }

    InvalidateRect(hWnd, NULL, false);
}
void deleteEntity(HWND hWnd, int x, int y)
{
    for (int i = 0; i < objCount; ++i) {
        if (entity[i].point.x == x && entity[i].point.y == y) {
            for (int j = i; j < objCount - 1; ++j) {
                entity[j] = entity[j + 1];
            }
            objCount--;
            break; // ?? 찾았으면 즉시 탈출!
        }
    }
}

void Draw(HDC memDC, int shape, int color, POINT p)
{
    HBRUSH myBrush = CreateSolidBrush(colorSet[color]);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, myBrush);

    int left = (wide * p.x) / CalNum;
    int top = (height * p.y) / RowNum;
    int right = (wide * (p.x + 1)) / CalNum;
    int bottom = (height * (p.y + 1)) / RowNum;

    if (shape == 0) {
        Ellipse(memDC, left, top, right, bottom);
    }
    else if (shape == 1) {
        Rectangle(memDC, left, top, right, bottom);
    }

    SelectObject(memDC, oldBrush);
    DeleteObject(myBrush);
}

void insertEntity(HWND hWnd, int x, int y, int color)
{
    if (objCount >= 1000) return; // ?? 배열 꽉 참 방지

    if (color == -1) {
        entity[objCount].color = uidColorSet(gen);
    }
    else {
        entity[objCount].color = color;
    }
    entity[objCount].shape = 1;
    entity[objCount].point.x = x;
    entity[objCount].point.y = y;
    objCount++;
}

void InitMap()
{
    objCount = 0; // ?? R키 누를 때마다 완벽히 초기화
    player.point = { uidPoint(gen), 0 };
    player.tailCount = 0; // 꼬리도 초기화
    player.rot = 0;

    for (int i = 0; i < RowNum; ++i) {
        for (int j = 0; j < CalNum; ++j) {
            map[i][j] = 0;
        }
    }

    for (int i = 0; i < 60; ++i) {
        int x = uidPoint(gen);
        int y = uidPoint(gen);

        while (map[y][x] == 1) {
            x = uidPoint(gen);
            y = uidPoint(gen);
        }

        entity[objCount].point.x = x;
        entity[objCount].point.y = y;
        entity[objCount].color = uidColorSet(gen);
        entity[objCount].shape = 1;
        map[y][x] = 1;

        objCount++;
    }
}