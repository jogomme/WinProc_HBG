#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <random>    

using namespace std;
int wide{ 800 };
int height{ 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_int_distribution<int> Numobstacle(20, 40);
uniform_int_distribution<int> arr(0, 39);
uniform_int_distribution<int> obstacle(1, 5);
uniform_int_distribution<int> makeMin(31, 33);
uniform_int_distribution<int> makeMax(41, 43);

uniform_int_distribution<int> colorSET(0, 4);


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

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
int mapTile[40][40]{ -1 };

const TCHAR ALPHABET[] = L"ABCDEFGHI";
uniform_int_distribution<int> alphaDist(0, 8);

COLORREF colorset[5] = {
    {RGB(colorDist(gen), colorDist(gen), colorDist(gen))},
    {RGB(colorDist(gen), colorDist(gen), colorDist(gen))},
    {RGB(colorDist(gen), colorDist(gen), colorDist(gen))},
    {RGB(colorDist(gen), colorDist(gen), colorDist(gen))},
    {RGB(colorDist(gen), colorDist(gen), colorDist(gen))}
};

class PlayerData
{
private:
    int x, y;             // 배열 인덱스 위치
    COLORREF color;       // 색상
    TCHAR currentType;    // 현재 보여줄 글자/모양
    TCHAR originalType;   // 원래 모습 (돌아가기 위해 기억)
    int shapeTimer;       // 모양 유지 남은 횟수

public:
    double size;          // 크기

    bool isGoal = false;
    int GetX() { return x; }
    int GetY() { return y; }
    void SetColor(COLORREF c) { color = c; }
    TCHAR GetType() { return currentType; }
    COLORREF GetColor() { return color; }

    // 초기화 함수
    void Init(int startX, int startY, TCHAR startChar) {
        x = startX;
        y = startY;
        size = 1;
        currentType = startChar;
        originalType = startChar;
        shapeTimer = 0;
        color = colorset[colorSET(gen)];
    }

    // 이동 함수
    void Move(int dx, int dy, int map[40][40]) {
        int nextX = x + dx;
        int nextY = y + dy;

        if (map[nextY][nextX] == 1) return;

        x = nextX;
        y = nextY;

        int tile = map[y][x];

        // 화면 끝으로 가면 반대쪽으로 나오는 로직
        if (x < 0) x = 39;
        if (x >= 40) x = 0;
        if (y < 0) y = 39;
        if (y >= 40) y = 0;

        if (tile == 2) { // 색상 변경
            color = colorset[colorSET(gen)];
        }
        else if (tile >= 30 && tile < 40) { // 축소
            int difsize = tile - 30;
            if (difsize == 1) size *= 0.9;
            else if (difsize == 2) size *= 0.7;
            else if (difsize == 3) size *= 0.5;

            if (size < 0.1) size = 0.1;
        }
        else if (tile >= 40 && tile < 50) { // 확대
            int difsize = tile - 40;
            if (difsize == 1) size *= 1.1;
            else if (difsize == 2) size *= 1.3;
            else if (difsize == 3) size *= 1.5;

            if (size > 2) size = 1.7;
        }
        else if (tile == 5) { // 모양 변경
            currentType = ALPHABET[alphaDist(gen)];
            shapeTimer = 20; // 5턴 뒤에 원래대로 돌아옴
        }

        // 모양이 변해있다면 턴(타이머) 감소
        if (shapeTimer > 0) {
            shapeTimer--;
            if (shapeTimer == 0) {
                currentType = originalType; // 시간이 다 되면 원래 모습으로!
            }
        }
    }

    // 그리기 함수
    void Draw(HDC hDC, int cellWidth, int cellHeight) {
            SetTextColor(hDC, color);

            double fontSize = (cellHeight * size);

            HFONT myFont = CreateFont(
                (int)fontSize, 0, 0, 0, FW_BOLD,
                0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Arial"
            );

            HFONT oldFont = (HFONT)SelectObject(hDC, myFont);

            int textX = (x * cellWidth) + (cellWidth / 2) - ((int)fontSize / 3);
            int textY = (y * cellHeight) + (cellHeight / 2) - ((int)fontSize / 2);

            TextOut(hDC, textX, textY, &currentType, 1);

            SelectObject(hDC, oldFont);
            DeleteObject(myFont);

            SetTextColor(hDC, RGB(0, 0, 0));
        
    }
};

PlayerData Player[2];
PlayerData Goal;

bool freeMove = false;
bool MoveA = true;
bool MoveB = false;

//  승리를 기억하는 스위치 (0: 게임 중, 1: A 승리, 2: B 승리)
int winner = 0;

// --------------------------------------------------------
// 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    static int NumOfOb = Numobstacle(gen);

    switch (uMsg) {
    case WM_CREATE:
        winner = 0; // 시작할 때 승리 상태 초기화

        Player[0].Init(0, 0, L'A');
        Player[1].Init(39, 0, L'B');

        Goal.Init(arr(gen), arr(gen), ALPHABET[alphaDist(gen)]);
        Goal.isGoal = true;
        Goal.SetColor(colorset[colorSET(gen)]);

        for (int i = 0; i < 40; ++i) {
            for (int j = 0; j < 40; ++j) {
                mapTile[j][i] = 0;
            }
        }

        for (int i{}; i < NumOfOb; ++i) {
            POINT p = { arr(gen), arr(gen) };
            if ((p.x == 0 && p.y == 0) || (p.x == 39 && p.y == 0)) {
                p = { arr(gen), arr(gen) };
            }
            mapTile[p.y][p.x] = obstacle(gen);

            if (mapTile[p.y][p.x] == 3) {
                mapTile[p.y][p.x] = makeMin(gen);
            }
            else if (mapTile[p.y][p.x] == 4) {
                mapTile[p.y][p.x] = makeMax(gen);
            }
        }
        break;

    case WM_KEYDOWN:
        //  누군가 승리했다면, 'R'키나 'Q'키 외에는 움직이지 못하게 차단합니다.
        if (winner == 0) {
            if (wParam == 'W') {
                if (MoveA || freeMove) { Player[0].Move(0, -1, mapTile); MoveA = false; MoveB = true; }
            }
            else if (wParam == 'A') {
                if (MoveA || freeMove) { Player[0].Move(-1, 0, mapTile); MoveA = false; MoveB = true; }
            }
            else if (wParam == 'S') {
                if (MoveA || freeMove) { Player[0].Move(0, 1, mapTile); MoveA = false; MoveB = true; }
            }
            else if (wParam == 'D') {
                if (MoveA || freeMove) { Player[0].Move(1, 0, mapTile); MoveA = false; MoveB = true; }
            }
            else if (wParam == 'I') {
                if (MoveB || freeMove) { Player[1].Move(0, -1, mapTile); MoveA = true; MoveB = false; }
            }
            else if (wParam == 'J') {
                if (MoveB || freeMove) { Player[1].Move(-1, 0, mapTile); MoveA = true; MoveB = false; }
            }
            else if (wParam == 'K') {
                if (MoveB || freeMove) { Player[1].Move(0, 1, mapTile); MoveA = true; MoveB = false; }
            }
            else if (wParam == 'L') {
                if (MoveB || freeMove) { Player[1].Move(1, 0, mapTile); MoveA = true; MoveB = false; }
            }
            else if (wParam == 'F') {
                freeMove = !freeMove;
            }
        }

        // 초기화 및 종료 키는 언제든 누를 수 있음
        if (wParam == 'R') {
            SendMessage(hWnd, WM_CREATE, 0, 0);
            MoveA = true; MoveB = false;
        }
        else if (wParam == 'Q') {
            PostQuitMessage(0);
        }

        //  도착 판정 로직 (승리하면 winner 플래그만 변경)
        if (Player[0].GetX() == Goal.GetX() && Player[0].GetY() == Goal.GetY()) {
            if (Player[0].GetType() == Goal.GetType() &&
                Player[0].GetColor() == Goal.GetColor() &&
                Player[0].size == Goal.size
                ) {
                winner = 1; // A 승리
            }
        }
        else if (Player[1].GetX() == Goal.GetX() && Player[1].GetY() == Goal.GetY()) {
            if (Player[1].GetType() == Goal.GetType() &&
                Player[1].GetColor() == Goal.GetColor() &&
                Player[1].size == Goal.size
                ) {
                winner = 2; // B 승리
            }
        }

        // 키보드를 눌렀으니 화면 갱신
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);
        SetBkMode(hDC, TRANSPARENT);

        int cellwide = wide / 40;
        int cellheight = height / 40;

        //  1. 타일 배경 먼저 그리기
        for (int i = 0; i < 40; ++i) {
            for (int j = 0; j < 40; ++j) {
                int tile = mapTile[i][j];
                if (tile > 0) {
                    HBRUSH brush = NULL;
                    if (tile == 1) brush = CreateSolidBrush(RGB(255, 0, 0));       // 장애물 (빨강)
                    else if (tile == 2) brush = CreateSolidBrush(RGB(150, 200, 100)); // 색변경 (연두)
                    else if (tile >= 30 && tile <= 40) brush = CreateSolidBrush(RGB(255, 255, 0));   // 축소 (노랑)
                    else if (tile >= 41) brush = CreateSolidBrush(RGB(135, 206, 235)); // 확대 (하늘)
                    else if (tile == 5) brush = CreateSolidBrush(RGB(255, 165, 0));   // 모양변경 (주황)

                    if (brush != NULL) {
                        HBRUSH oldB = (HBRUSH)SelectObject(hDC, brush);
                        Rectangle(hDC, j * cellwide, i * cellheight, (j + 1) * cellwide, (i + 1) * cellheight);
                        SelectObject(hDC, oldB);
                        DeleteObject(brush);

                        if ((tile >= 31 && tile <= 33) || (tile >= 41 && tile <= 43)) {
                            SetTextColor(hDC, RGB(255, 0, 0));
                            int level = tile % 10;
                            TCHAR c = level + '0';
                            TextOut(hDC, j * cellwide + (cellwide / 3), i * cellheight + (cellheight / 5), &c, 1);
                            SetTextColor(hDC, RGB(0, 0, 0));
                        }
                    }
                }
            }
        }

        //  2. 바둑 판 뼈대 선 그리기
        for (int i = 0; i <= 40; ++i) {
            MoveToEx(hDC, cellwide * i, 0, NULL);
            LineTo(hDC, cellwide * i, height);
            MoveToEx(hDC, 0, cellheight * i, NULL);
            LineTo(hDC, wide, cellheight * i);
        }

        //  3. 플레이어와 Goal은 맨 위에 그려야 타일에 파묻히지 않습니다!
        Goal.Draw(hDC, cellwide, cellheight);
        Player[0].Draw(hDC, cellwide, cellheight);
        Player[1].Draw(hDC, cellwide, cellheight);

        //  4. 누군가 승리했다면, 화면 정중앙에 거대한 승리 문구 출력!
        if (winner > 0) {
            SetTextColor(hDC, RGB(255, 0, 0)); // 강렬한 빨간색

            // 50 픽셀짜리 두꺼운 폰트 생성
            HFONT winFont = CreateFont(50, 0, 0, 0, FW_HEAVY, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"맑은 고딕");
            HFONT oldFont = (HFONT)SelectObject(hDC, winFont);

            LPCTSTR winMessage = (winner == 1) ? L"Player 1 (A) 승리!" : L"Player 2 (B) 승리!";

            // 화면 중앙쯤에 출력 (위치를 대략적으로 맞춤)
            TextOut(hDC, wide / 2 - 180, height / 2 - 25, winMessage, lstrlen(winMessage));

            SelectObject(hDC, oldFont);
            DeleteObject(winFont);
            SetTextColor(hDC, RGB(0, 0, 0));
        }

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