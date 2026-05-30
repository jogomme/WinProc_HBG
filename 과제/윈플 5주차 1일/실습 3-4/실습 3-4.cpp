#define _CRT_SECURE_NO_WARNINGS

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include <iostream>
#include<math.h>

using namespace std;
int wide{ 600 };
int height{ 600 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_int_distribution<int> uidPoint(0, 9);
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
    WndClass.style = CS_HREDRAW | CS_VREDRAW;;
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
int map[31][31]{};
int objCount = 0;
// --------------------------------------------------------
// 클래스, 구조체 선언 구간
// --------------------------------------------------------

struct Player
{
    POINT point;
    int shape = 0;
    int color;
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
Entity entity[100];
int cnt{};

int RowNum = 10;
int CalNum = 10;

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

void insertEntity(HWND hWnd,int x, int y, int color = -1);

void deleteEntity(HWND hWnd, int x, int y);

void Draw(HDC memDC, int shape, int color, POINT p);

void Move(HWND hWnd,int direction);

void InitMap();

void clearLine(HWND hWnd);

void setLine(HWND hWnd,int x, int y, int color);

void spin(HWND hWnd);

// --------------------------------------------------------
// 타이머 관련 전역 변수
// --------------------------------------------------------
const int TimerMove = 0;
int TickRate = 300;

// --------------------------------------------------------
// 타이머 콜 백 함수 선언
// --------------------------------------------------------
void CALLBACK TimerProc(HWND hWnd, UINT iMsg, UINT idEvent, DWORD dwTime);

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
        cout << x << ", " << y <<" / " << map[y][x] << endl;
        if (map[y][x] == 0) {
            map[y][x] = 1;
            insertEntity(hWnd,x,y);
        }

        break;
    }
    case WM_RBUTTONDOWN: {


        int Mx = LOWORD(lParam);
        int My = HIWORD(lParam);

        int x = (Mx * CalNum) / wide;
        int y = (My * RowNum) / height;

        if (map[y][x] == 1) {
            deleteEntity(hWnd,x, y);
            map[y][x] = 0;
        }

        cout << x << ", " << y << " / " << map[y][x] << endl;
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
            if (TickRate >= 500) {
                KillTimer(hWnd, TimerMove);
            }
        }
        else if (wParam == 'R') {
            InitMap();
        }
        else if (wParam == 'K') {
            int x{}; int y{};

            cout << "변환 할 칸 입력 : ";
            cin >> x >> y;
            cout << endl;

            int color{};

            cout << "변환 할 색깔 입력 : ";
            cin >> color;

            setLine(hWnd, x, y, color);

        }
        else if (wParam == VK_UP) {
            cout << "^ 키 인식" << '\n';
            Move(hWnd,0);
        }
        else if (wParam == VK_DOWN) {
            cout << "v 키 인식" << '\n';
            Move(hWnd, 1);
        }
        else if (wParam == VK_LEFT) {
            cout << "< 키 인식" << '\n';
            Move(hWnd, 2);
        }
        else if (wParam == VK_RIGHT) {
            cout << "> 키 인식" << '\n';
            Move(hWnd, 3);
        }
        else if (wParam == VK_RETURN) {
            cout << "ENTER 키 입력" << '\n';
            spin(hWnd);
        }

        InvalidateRect(hWnd, NULL, false);
        break;

    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);


        // 계속해서 화면을 다시 그릴 때 활용 <- 더블 버퍼링
        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        // 배경을 하얗게 초기화 시키기
        HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);
        // memDC를 사용해서 그리기

        for (int i = 0; i <= CalNum; ++i) {
            MoveToEx(memDC, 0, (height * i) / CalNum, NULL);
            LineTo(memDC, wide, (height * i) / CalNum);
        }
        for (int i = 0; i <= RowNum; ++i) {
            MoveToEx(memDC, wide * i / RowNum, 0, NULL);
            LineTo(memDC, wide * i / RowNum, height);
        }

        for (int i = 0; i < objCount; ++i) {
            Draw(memDC, entity[i].shape, entity[i].color, entity[i].point);
        }

        Draw(memDC, player.shape, player.color, player.point);

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
// 타이머 콜 백 함수 선언
// --------------------------------------------------------
void CALLBACK TimerProc(HWND hWnd, UINT iMsg, UINT idEvent, DWORD dwTime)
{
    switch (idEvent)
    {
    case TimerMove: {
        Move(hWnd, 1);
    }


    }
}

// --------------------------------------------------------
// 함수 구현
// --------------------------------------------------------
void spin(HWND hWnd)
{

}

void setLine(HWND hWnd, int x, int y, int color)
{
    if (map[y][x] == 1) {
        deleteEntity(hWnd, x, y);
    }

    insertEntity(hWnd, x, y, color);
}

void clearLine(HWND hwnd)
{
    for (int y = RowNum - 1; y >= 0; y--) {
        bool isFull = true;

        // 1. 해당 줄이 빈틈없이 꽉 찼는지 검사
        for (int x = 0; x < CalNum; x++) {
            if (map[y][x] == 0) {
                isFull = false;
                break;
            }
        }

        // 2. 꽉 찼다면, 과제 조건인 '모두 같은 색상'인지 추가 검사!
        bool isSameColor = true;
        if (isFull) {
            int firstColor = -1;
            for (int x = 0; x < CalNum; x++) {
                int currentColor = -1;
                // 해당 칸(x, y)에 있는 그림(entity)의 색상을 찾아냄
                for (int e = 0; e < objCount; e++) {
                    if (entity[e].point.x == x && entity[e].point.y == y) {
                        currentColor = entity[e].color;
                        break;
                    }
                }

                if (x == 0) firstColor = currentColor; // 첫 칸 색상 기억
                else if (currentColor != firstColor) {
                    isSameColor = false; // 색이 하나라도 다르면 파괴 취소!
                    break;
                }
            }
        }

        // 3. 꽉 찼고 색깔도 모두 같다면? -> 펑 터트리고 끌어내리기!
        if (isFull && isSameColor) {

            // (1) 투명 벽(map) 윗줄들을 한 칸씩 아래로 복사해서 떨어뜨리기
            for (int pullY = y; pullY > 0; pullY--) {
                for (int x = 0; x < CalNum; x++) {
                    map[pullY][x] = map[pullY - 1][x];
                }
            }
            for (int x = 0; x < CalNum; x++) map[0][x] = 0; // 맨 윗줄 비우기

            // (2) 화면의 그림(entity) 지우고 떨어뜨리기
            for (int i = 0; i < objCount; ) {
                if (entity[i].point.y == y) {
                    // 터진 줄에 있던 그림은 삭제! (뒤에서 당겨옴)
                    for (int j = i; j < objCount - 1; j++) {
                        entity[j] = entity[j + 1];
                    }
                    objCount--;
                    // 삭제하고 당겨왔으니 i를 증가시키지 않고 제자리에서 다시 검사
                }
                else {
                    // 터진 줄보다 '위에' 있던 그림들은 아래로 한 칸씩 뚝 떨어짐
                    if (entity[i].point.y < y) {
                        entity[i].point.y += 1;
                    }
                    i++; // 처리 완료, 다음 엔티티로 넘어감
                }
            }

            // 매우 중요: 한 줄이 지워지고 윗줄들이 내려왔으니, 
            // 방금 검사한 그 자리(y)를 한 번 더 검사해야 연속 콤보가 터집니다!
            y++;
        }
    }
}

void Move(HWND hWnd , int direction)
{
    int nextX = player.point.x;
    int nextY = player.point.y;

    int dx = 0, dy = 0;

    if (direction == 0) dy = -1; // UP (상)
    else if (direction == 1) dy = 1;  // DOWN (하)
    else if (direction == 2) dx = -1; // LEFT (좌)
    else if (direction == 3) dx = 1;  // RIGHT (우)

    nextX += dx;
    nextY += dy;

    if (nextX > CalNum-1 || nextX < 0 || nextY < 0) return;

    if (nextY > 9 ) {
        nextY = 0;
        nextX = uidPoint(gen);
    }

    if (map[nextY][nextX] == 0) {
        player.point.x = nextX;
        player.point.y = nextY;
    }
    else if (map[nextY][nextX] == 1) {
        int endX = nextX;
        int endY = nextY;

        while (endX >= 0 && endX < CalNum && endY >= 0 && endY < RowNum && map[endY][endX] == 1) {
            endX += dx;
            endY += dy;
        }

        if (endX >= 0 && endX < CalNum && endY >= 0 && endY < RowNum && map[endY][endX] == 0) {
            int currX = endX;
            int currY = endY;

            while (currX != nextX || currY != nextY) {
                int prevX = currX - dx;
                int prevY = currY - dy;

                // 내 바로 앞의 블록을 내 자리로 끌어옴
                map[currY][currX] = map[prevY][prevX];

                for (int k = 0; k < objCount; ++k) {
                    // 예전 위치(prevX, prevY)에 있던 그림을 찾아서
                    if (entity[k].point.x == prevX && entity[k].point.y == prevY) {
                        // 새 위치(currX, currY)로 업데이트
                        entity[k].point.x = currX;
                        entity[k].point.y = currY;
                        break;
                    }
                }

                currX = prevX;
                currY = prevY;
            }

            // (4) 다 밀었으니 첫 아이템이 있던 자리는 비워주고, 플레이어를 거기로 쏙 이동시킴
            map[nextY][nextX] = 0;
            player.point.x = nextX;
            player.point.y = nextY;
            clearLine(hWnd);
        }
        else {
            player.point.y = 0;
            player.point.x = uidPoint(gen);
        }
    }

    InvalidateRect(hWnd, NULL, false);
}

void deleteEntity(HWND hWnd,int x, int y)
{
    POINT p{ y,x };
    for (int i = 0; i < objCount; ++i) {
        if (entity[i].point.x == x && entity[i].point.y == y) {
            for (int j = i; j < objCount - 1; ++j) {
                entity[j] = entity[j + 1];
            }
            objCount--;
            break;
        }
    }
    InvalidateRect(hWnd, NULL, false);
}

void Draw(HDC memDC, int shape, int color, POINT p)
{
    HBRUSH myBrush = CreateSolidBrush(colorSet[color]);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, myBrush);
    // 1. 현재 서 있는 바둑판 1칸의 가로/세로 길이를 구합니다.

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
    if (color == -1) {
        entity[objCount].color = 1;
    }
    else {
        entity[objCount].color = color;
    }
    entity[objCount].shape = 1;
    entity[objCount].point.x = x;
    entity[objCount].point.y = y;    
    objCount++;
    
    InvalidateRect(hWnd, NULL, false);
}

void InitMap()
{
    objCount = 0;
    player.point = { uidPoint(gen), 0 };
    for (int i = 0; i < RowNum; ++i) {
        for (int j = 0; j < CalNum; ++j) {
            map[i][j] = 0;
        }
    }

    for (int i = 0; i < 10; ++i) {
        objCount++;
        int x = 0; int y = 0;

        x = uidPoint(gen);
        y = uidPoint(gen);

        while (map[y][x] == 1) {
            x = uidPoint(gen);
            y = uidPoint(gen);
        }
        entity[i].point.x = x;
        entity[i].point.y = y;
        entity[i].color = uidColorSet(gen);
        entity[i].shape = 1;
        map[y][x] = 1;
    }
}