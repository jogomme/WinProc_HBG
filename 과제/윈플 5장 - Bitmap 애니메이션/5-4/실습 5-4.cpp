#define _CRT_SECURE_NO_WARNINGS

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include<iostream>
#include  "resource.h"
#include<math.h>

using namespace std;
int wide{ 800 };
int height{ 800 };



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
    WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU2);   // 메뉴 이름 !!
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
HBITMAP g_hImg;    // 불러온 비트맵 이미지를 저장할 변수
BITMAP bmpInfo;    // 비트맵의 가로, 세로 크기 등 정보를 저장할 구조체

#define Col 6       // 열
#define Row 6       // 행


int NumOfobstacle = 2;
int NumOfChar = 2;

int direction; // 1 상 2 하 3 좌 4 우

int startX;
int startY;

int map[Col][Row] = { 0 };
int renderMap[Col][Row] =   { 0 };    // 화면에 그릴 때 참조하는 위치(애니메이션용)
double offsetX[Col][Row] =     { 0 };    // 각 블록의 현재 픽셀 오프셋 X
double offsetY[Col][Row] =     { 0 };    // 각 블록의 현재 픽셀 오프셋 Y

bool isAnimating = false;

int GoalPoint = 32;

bool isStart = false;
bool isFail = false;
bool isGoal = false;
bool isDrag = false;

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> YDist(0, Col - 1);
uniform_int_distribution<int> XDist(0, Row - 1);

// --------------------------------------------------------
// 함수 선언 구간
// --------------------------------------------------------
void Move(int direction, HWND hWnd);

void DrawMap(HDC memDC, HDC imgDC, HWND hWnd);

void ResetMap();

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
        g_hImg = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));

        ResetMap();

        // 2. 불러온 그림의 정보를 bmpInfo 구조체에 쏙 빼옵니다. (가로, 세로 크기 등)
        GetObject(g_hImg, sizeof(BITMAP), &bmpInfo);
        break;
    case WM_LBUTTONDOWN: {

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        startX = x;
        startY = y;

        if (!isFail && !isGoal && isStart) {
            isDrag = true;
        }
        else {
            cout << "끝" << '\n';
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_COMMAND: {

        switch (LOWORD(wParam))
        {
        case ID_GAME_START: {
            isStart = true;
            ResetMap();
            break;
        }
        case ID_GAME_END: {
            isStart = false;
            break;
        }
        case ID_SCORE_32: {
            GoalPoint = 32;
            isStart = false;
            break;
        }
        case ID_SCORE_64: {
            GoalPoint = 64;
            isStart = false;
            break;
        }
        case ID_BARRIOR_2: {
            NumOfobstacle = 2;
            isStart = false;
            break;
        }
        case ID_BARRIOR_3: {
            NumOfobstacle = 3;
            isStart = false;
            break;
        }
        case ID_BARRIOR_4: {
            NumOfobstacle = 4;
            isStart = false;
            break;
        }
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_MOUSEMOVE: {

        if (isDrag) {

            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            int dx = x - startX;
            int dy = y - startY;

            if (abs(dx) > abs(dy)) {
                if (dx > 0) {
                    direction = 4;
                }
                else {
                    direction = 3;
                }
            }
            else {
                if (dy > 0) {
                    direction = 2;
                }
                else {
                    direction = 1;
                }
            }

            InvalidateRect(hWnd, NULL, false);
        }
        break;
    }
    case WM_LBUTTONUP: {
        if (isDrag && (!isFail && !isGoal) && isStart) {
            Move(direction, hWnd);
        }
        isDrag = false;

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_RBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        int cell = wide / Row;

        int xPos = x / cell;
        int yPos = y / cell;

        map[yPos][xPos] = 2;

        InvalidateRect(hWnd, NULL, false);
        break;

    }
    case WM_KEYDOWN:
        if (wParam == 'Q') {
            PostQuitMessage(0);
        }
        else if (wParam == 'R') {
            ResetMap();
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    case WM_TIMER: {
        float step = 0.25f;   // 수렴 속도 (클수록 빠름)
        bool allDone = true;

        int cellX = wide / Row;
        int cellY = height / Col;


        for (int c = 0; c < Col; ++c) {
            for (int r = 0; r < Row; ++r) {
                if (offsetX[c][r] != 0 || offsetY[c][r]) {
                    offsetX[c][r] *= 0.6;
                    offsetY[c][r] *= 0.6;

                    if (abs(offsetX[c][r]) < 1.0) offsetX[c][r] = 0;
                    if (abs(offsetY[c][r]) < 1.0) offsetY[c][r] = 0;

                    if (offsetX[c][r] != 0 || offsetY[c][r] != 0) {
                        allDone = false;
                    }
                }
            }
        }

        InvalidateRect(hWnd, NULL, false);

        if (allDone) {
            KillTimer(hWnd, 1);
            isAnimating = false;
        }
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

        // 이미지를 저장할 DC
        HDC imgDC = CreateCompatibleDC(hDC);
        SelectObject(imgDC, g_hImg);

      

        DrawMap(memDC, imgDC, hWnd);

        // 그린것을 hDC로 복사
        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);
        // 다 쓴 브러쉬들 해제하기
        DeleteDC(imgDC);
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

void txt() {
    for (int i = 0; i < Col; ++i) {
        for (int j = 0; j < Row; ++j) {
            cout << map[i][j] << " ";
        }
        cout << '\n';
    }
}

void MakeTwo(int direction)
{
    int r{};
    int c{};
    int Rempty[Row] = {0};
    int Cempty[Col] = {0};
    int cnt{};
    if (direction == 1) {
        for (int i = 0; i < max(Row, Col); ++i) {
            if (map[Col - 1][i] == 0) {
                Rempty[i] = 1;
                ++cnt;
            }
        }
        if (cnt < NumOfChar) {
            isFail = true;
        }
        else {
            cnt = 0;
            while (true) {
                int x = XDist(gen);

                if (cnt >= 2) {
                    break;
                }

                if (Rempty[x] == 1) {
                    ++cnt;
                    Rempty[x] = 0;
                    map[Col - 1][x] = 2;
                }

            }
        }   
    }
    else if (direction == 2) {
        for (int i = 0; i < max(Row, Col); ++i) {
            if (map[0][i] == 0) {
                Rempty[i] = 1;
                ++cnt;
            }
        }
        if (cnt < NumOfChar) {
            isFail = true;
        }
        else {
            cnt = 0;
            while (true) {
                int x = XDist(gen);

                if (cnt >= 2) {
                    break;
                }

                if (Rempty[x] == 1) {
                    ++cnt;
                    Rempty[x] = 0;
                    map[0][x] = 2;
                }

            }
        }
    }
    else if (direction == 3) {
        for (int i = 0; i < max(Row, Col); ++i) {
            if (map[i][Row-1] == 0) {
                Rempty[i] = 1;
                ++cnt;
            }
        }
        if (cnt < NumOfChar) {
            isFail = true;
        }
        else {
            cnt = 0;
            while (true) {
                int x = XDist(gen);

                if (cnt >= 2) {
                    break;
                }

                if (Rempty[x] == 1) {
                    ++cnt;
                    Rempty[x] = 0;
                    map[x][Row-1] = 2;
                }

            }
        }
    }
    else if (direction == 4) {
        for (int i = 0; i < max(Row, Col); ++i) {
            if (map[i][0] == 0) {
                Rempty[i] = 1;
                
                ++cnt;
            }
        }
        if (cnt < NumOfChar) {
            isFail = true;
        }
        else {
            cnt = 0;
            while (true) {
                int x = XDist(gen);

                if (cnt >= 2) {
                    break;
                }

                if (Rempty[x] == 1) {
                    ++cnt;
                    Rempty[x] = 0;
                    map[x][0] = 2;
                }

            }
        }

    }
}

void Move(int direction, HWND hWnd)
{
    int cellX = wide / Row;
    int cellY = height / Col;

    // 1. 이동 시작 전, 모든 타일의 오프셋(잔상)을 0으로 초기화합니다.
    for (int i = 0; i < Col; ++i) {
        for (int j = 0; j < Row; ++j) {
            offsetX[i][j] = 0;
            offsetY[i][j] = 0;
        }
    }

  
    
    if (direction == 1) { // 상 (위로 밀기)
        cout << "상" << '\n';
        for (int i = 0; i < Col; ++i) {
            // 위로 밀 때는 위에서부터(0) 아래로(Col-1) 훑어야 합니다.
            for (int c = 0; c < Col - 1; ++c) {
                for (int r = 0; r < Row; ++r) {
                    // 현재 칸(c+1)에 타일이 있다면
                    if (map[c + 1][r] >= 2) {
                        // 윗 칸(c)이 같은 숫자거나 빈칸(0)이라면 이동 가능!
                        if (map[c][r] == map[c + 1][r] || map[c][r] == 0) {

                            // 빈칸이면 그대로 이동, 같은 숫자면 덧셈(합치기)
                            if (map[c][r] == 0) map[c][r] = map[c + 1][r];
                            else map[c][r] += map[c + 1][r];

                            map[c + 1][r] = 0; // 원래 있던 자리는 비웁니다.

                            // [애니메이션 보정]
                            // 타일이 아래(c+1)에서 위(c)로 올라왔으므로, 
                            // 시각적으로는 원래 있던 아래쪽(+cellY)에 그려지도록 오프셋을 넘겨줍니다.
                            offsetY[c][r] = offsetY[c + 1][r] + cellY;
                            offsetY[c + 1][r] = 0;
                        }
                    }
                }
            }
        }
    }
    else if (direction == 2) { // 하 (아래로 밀기)
        cout << "하" << '\n';
        for (int i = 0; i < Col; ++i) {
            // 아래로 밀 때는 아래쪽(Col-2)부터 위로(0) 훑어야 벽부터 쌓입니다.
            for (int c = Col - 2; c >= 0; --c) {
                for (int r = 0; r < Row; ++r) {
                    if (map[c][r] >= 2) {
                        // 아랫 칸(c+1)이 같거나 빈칸일 때
                        if (map[c + 1][r] == map[c][r] || map[c + 1][r] == 0) {

                            if (map[c + 1][r] == 0) map[c + 1][r] = map[c][r];
                            else map[c + 1][r] += map[c][r];

                            map[c][r] = 0;

                            // 위에서 아래로 내려왔으므로, 원래 있던 위쪽(-cellY)에 그려지도록 보정
                            offsetY[c + 1][r] = offsetY[c][r] - cellY;
                            offsetY[c][r] = 0;
                        }
                    }
                }
            }
        }
    }
    else if (direction == 3) { // 좌 (왼쪽으로 밀기)
        cout << "좌" << '\n';
        for (int i = 0; i < Row; ++i) {
            for (int c = 0; c < Col; ++c) {
                // 왼쪽으로 밀 때는 왼쪽(0)부터 오른쪽으로 훑습니다.
                for (int r = 0; r < Row - 1; ++r) {
                    if (map[c][r + 1] >= 2) {
                        // 왼쪽 칸(r)이 같거나 빈칸일 때
                        if (map[c][r] == map[c][r + 1] || map[c][r] == 0) {

                            if (map[c][r] == 0) map[c][r] = map[c][r + 1];
                            else map[c][r] += map[c][r + 1];

                            map[c][r + 1] = 0;

                            // 우측에서 좌측으로 왔으므로, 원래 있던 오른쪽(+cellX)으로 보정
                            offsetX[c][r] = offsetX[c][r + 1] + cellX;
                            offsetX[c][r + 1] = 0;
                        }
                    }
                }
            }
        }
    }
    else if (direction == 4) { // 우 (오른쪽으로 밀기)
        cout << "우" << '\n';
        for (int i = 0; i < Row; ++i) {
            for (int c = 0; c < Col; ++c) {
                // 오른쪽으로 밀 때는 오른쪽(Row-2)부터 왼쪽으로 훑습니다.
                for (int r = Row - 2; r >= 0; --r) {
                    if (map[c][r] >= 2) {
                        // 오른쪽 칸(r+1)이 같거나 빈칸일 때
                        if (map[c][r + 1] == map[c][r] || map[c][r + 1] == 0) {

                            if (map[c][r + 1] == 0) map[c][r + 1] = map[c][r];
                            else map[c][r + 1] += map[c][r];

                            map[c][r] = 0;

                            // 좌측에서 우측으로 왔으므로, 원래 있던 왼쪽(-cellX)으로 보정
                            offsetX[c][r + 1] = offsetX[c][r] - cellX;
                            offsetX[c][r] = 0;
                        }
                    }
                }
            }
        }
    }

    for (int c = 0; c < Col; ++c) {
        for (int r = 0; r < Row; ++r) {
            if (map[c][r] >= GoalPoint) {
                isGoal = true;
            }
        }
    }

    if (isGoal || isFail) {
        if (isGoal) {
            MessageBox(hWnd, L"퍼즐을 완성했습니다!", L"게임 종료", MB_OK);
        }
        if (isFail) {
            MessageBox(hWnd, L"퍼즐을 완성 못 했습니다!", L"게임 종료", MB_OK);
        }
    }

    // 이동이 모두 끝난 후 새로운 숫자 블록을 랜덤한 빈칸에 생성합니다.
    MakeTwo(direction);

    // 계산된 오프셋을 바탕으로 타이머를 작동시켜 애니메이션을 시작합니다.
    isAnimating = true;
    SetTimer(hWnd, 1, 16, NULL);  // 16ms 마다 갱신 (약 60프레임)
}

void DrawMap(HDC memDC, HDC imgDC, HWND hWnd)
{
    int cellX = wide / Row;
    int cellY = height / Col;

    int imgW = bmpInfo.bmWidth / Row;
    int imgH = bmpInfo.bmHeight / Col;

  

    for (int c = 0; c < Col; ++c) {
        for (int r = 0; r < Row; ++r) {
            // 맵 격자무늬 만들기
            Rectangle(memDC, r * cellX, c * cellY, (r + 1) * cellX, (c + 1) * cellY);

            if (map[c][r] == 1) {
                // 장애물 만들기
                StretchBlt(memDC, r * cellX, c * cellY, cellX, cellY,
                    imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, NOTSRCCOPY);
            }
            else if (map[c][r] >= 2 ) {

                int drawX = r * cellX + (int)offsetX[c][r];
                int drawY = c * cellY + (int)offsetY[c][r];

                // 타일 그리기
                StretchBlt(memDC, drawX, drawY, cellX, cellY,
                    imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
                SetBkMode(memDC, TRANSPARENT);
                TCHAR k[10];
                wsprintf(k, L"%d", map[c][r]);
                TextOut(memDC, drawX + cellX / 5 * 2, drawY + cellY / 5 * 2, k, lstrlen(k));
            }
        }
    }
}

void ResetMap()
{
    for (int i = 0; i < Col; ++i) {
        for (int j = 0; j < Row; ++j) {
            map[i][j] = 0;
        }
    }

    int Cnt{};

    while (true) {
        if (Cnt == NumOfobstacle) break;

        int x = XDist(gen);
        int y = YDist(gen);

        if (map[y][x] == 1) {
            continue;
        }

        map[y][x] = 1;
        Cnt++;
    }

    Cnt = 0;

    while (true) {
        if (Cnt == NumOfChar) break;

        int x = XDist(gen);
        int y = YDist(gen);

        if (map[y][x] == 1 || map[y][x] == 2) {
            continue;
        }

        map[y][x] = 2;
        Cnt++;
    }


}