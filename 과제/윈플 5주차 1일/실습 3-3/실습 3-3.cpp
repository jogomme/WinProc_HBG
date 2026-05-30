#define _CRT_SECURE_NO_WARNINGS

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include <iostream>
#include<math.h>

using namespace std;
int wide{ 800 };
int height{ 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_int_distribution<int> uidPoint(0, 39);

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
int map[40][40];
int objCount = 0;
// --------------------------------------------------------
// 클래스, 구조체 선언 구간
// --------------------------------------------------------
class GameObject
{
protected:
// protected는 private와 같지만 자식들에게는 멤버 변수 사용가능하게 한다.
    double m_size;
    int m_x, m_y;
    // 0 = 원, 1 = 사각형, 2 = 삼각형
    int m_shape{-1};
    COLORREF m_color;

public:

    bool isJumping = false;
    int jumpOffset = 0;

    GameObject()
    {
        m_size = 0;
        m_x = m_y = 0;
        m_color = RGB(0, 0, 0);
    }
    // virtual(가상) 키워드를 붙이면 자식 클래스가 함수를 개조할 수 있다.
    virtual void Move() 
    {

    }

    virtual void Draw(HDC memDC)
    {
        if (m_shape == -1) return;

        HBRUSH myBrush = CreateSolidBrush(m_color);
        HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, myBrush);

        // 1. 현재 서 있는 바둑판 1칸의 가로/세로 길이를 구합니다.
        double cellW = wide / 40.0;
        double cellH = height / 40.0;

        // 2. 현재 칸의 '정중앙(Center) 좌표'를 구합니다. (절대 변하지 않는 닻 역할)
        double centerX = (m_x * cellW) + (cellW / 2.0);
        double centerY = (m_y * cellH) + (cellH / 2.0);

        // 3. m_size를 곱해서 이번에 그릴 도형의 '절반 크기'를 계산합니다.
        double halfW = (cellW * m_size) / 2.0;
        double halfH = (cellH * m_size) / 2.0;

        // 4. 정중앙에서 절반 크기만큼만 상하좌우로 벌려서 사각형(틀)을 잡습니다.
        int left = (int)(centerX - halfW);
        int top = (int)(centerY - halfH) - jumpOffset;      // 점프 오프셋 적용
        int right = (int)(centerX + halfW);
        int bottom = (int)(centerY + halfH) - jumpOffset;   // 점프 오프셋 적용

        if (m_shape == 0) {
            Ellipse(memDC, left, top, right, bottom);
        }
        else if (m_shape == 1) {
            Rectangle(memDC, left, top, right, bottom);
        }
        else if (m_shape == 2) {
            POINT pts[3]{
                {(left + right) / 2, top},
                {right, bottom},
                {left, bottom}
            };
            Polygon(memDC, pts, 3);
        }
        else {
            cout << m_shape << "가 왜나오지?" << '\n';
        }

        SelectObject(memDC, oldBrush);
        DeleteObject(myBrush);
    }

    

    // 위치나 색상을 외부에서 읽고 쓸 수 있는 공통 함수들 (GetX, SetColor 등)
    int GetX() { return m_x; }
    int GetY() { return m_y; }
    // 0 = 원 , 1 = 사각형, 2 = 삼각형
    int GetShape() { return m_shape; }
    COLORREF GetColor() { return m_color; }
    void SetColor(COLORREF c) { m_color = c; }
    // 0 = 원 , 1 = 사각형, 2 = 삼각형
    void SetShape(int s) { m_shape = s; }
    void SetSize(double s) { m_size = s; }
};
// GameObject 끝


class Player : public GameObject
{
private :
    // 0 = 상, 1 = 하, 2 = 좌, 3 = 우
    int direction;
    bool yFlag;
    bool xFlag;
public:
    bool isFever = false;   
    int old_x = 0, old_y = 0; // 내 옛날 위치 백업
    int tails[20] = { 0 };    // 내 뒤에 붙은 꼬리들 번호 적어두는 수첩
    int tailCount = 0;        // 현재 꼬리가 몇 개인지


    Player()
    {
        yFlag = false;
        xFlag = false;
        direction = 3;
    }

    // 0 = 상, 1 = 하, 2 = 좌, 3 = 우
    void SetDirection(int direc)
    {
        direction = direc;
    }

    void Init(int startX, int startY) {
        m_x = startX; // 부모의 변수를 내 것처럼 쓸 수 있다
        m_y = startY;
        m_size = 1;
        m_shape = 0;
    }

    void Move() override { // override를 붙혀서 재설정 가능하다
        int nextX = m_x;
        int nextY = m_y;
        bool hitObstacle = (isFever == false && map[nextY][nextX] == 1);
        if (direction == 0) nextY -= 1;      // 상
        else if (direction == 1) nextY += 1; // 하
        else if (direction == 2) nextX -= 1; // 좌
        else if (direction == 3) nextX += 1; // 우

        if (nextX < 0 || nextX > 39 || nextY < 0 || nextY > 39 || map[nextY][nextX] == 1 || hitObstacle) {

            int sideX = m_x;
            int sideY = m_y;

            if (direction == 0) { direction = 1; sideX += 1; } 
            else if (direction == 1) { direction = 0; sideX += 1; }
            else if (direction == 2) { direction = 3; sideY += 1; }
            else if (direction == 3) { direction = 2; sideY += 1; }
            bool sideHitObstacle = (isFever == false && map[sideY][sideX] == 1);
            if (sideX >= 0 && sideX <= 39 && sideY >= 0 && sideY <= 39 && map[sideY][sideX] == 0) {
                m_x = sideX;
                m_y = sideY;
            }
        }
        else {
            m_x = nextX;
            m_y = nextY;
        }
    }

    void jumping()
    {
        if (direction == 0) {
            m_x += 1;;
        }
        else if (direction == 1) {
            m_x -= 1;
        }
        else if (direction == 2) {
            m_y += 1;
        }
        else if (direction == 3) {
            m_y -= 1;
        }
    }

    void jumped()
    {
        if (direction == 0) {
            m_x -= 1;
        }
        else if (direction == 1) {
            m_x += 1;
        }
        else if (direction == 2) {
            m_y -= 1;
        }
        else if (direction == 3) {
            m_y += 1;
        }
    }
    // 0 = 상, 1 = 하, 2 = 좌, 3 = 우
    int GetDirection() { return direction; }

    void SetX(int x) { m_x = x;  }
    void SetY(int y) { m_y = y; }
};

class entity : public GameObject
{
private:
    int state;   // 0: 먹이, 1: 야생 꼬리, 2: Player 뒤에 붙은 꼬리
    int pattern; // 이동 패턴 (1~5)

public:
    // 패턴 구현을 위해 추가된 변수들
    int dir;       // 현재 이동 방향 (0:상, 1:하, 2:좌, 3:우)
    int step;      // 사각형 이동 시 걸음 수 체크
    bool sizeUp;   // 크기가 커지는 중인지 여부 (패턴 5번용)

    // 나중에 기차놀이(꼬리 따라가기)를 할 때 꼭 필요한 이전 위치 백업용 변수
    int old_x, old_y;

    entity()
    {
        state = 0;
        pattern = 0;
        dir = 0;
        step = 0;
        sizeUp = true;
    }

    void Init(int startX, int startY, int stat, int shape) {
        m_x = startX;
        m_y = startY;
        m_shape = shape;
        state = stat;
        m_size = 1.0;

        // 야생 꼬리가 생성될 때 변수 초기화
        dir = rand() % 4;
        step = 0;
        sizeUp = true;
    }

    void setPos(int x, int y)
    {
        m_x = x;
        m_y = y;
    }

    // 꼬리만의 이동 방식 (5가지 패턴 완벽 적용)
    void Move() override {
        // 이동하기 직전의 내 위치를 무조건 백업해둠 (나중에 꼬리가 내 자리를 따라와야 하니까!)
        old_x = m_x;
        old_y = m_y;

        if (state == 0 || state == 2) return;

        // state == 1 (야생 꼬리) 일 때만 5가지 패턴으로 움직임
        int nx = m_x;
        int ny = m_y;

        if (pattern == 1) {
            // 이동 방법 1: 좌우로 이동. 가장자리면 방향 전환 
            if (dir != 2 && dir != 3) dir = 3; // 좌우가 아니면 우측으로 초기화

            if (dir == 2) nx -= 1;
            else nx += 1;

            // 맵 밖이거나 장애물(1)이면 반대 방향으로 턴
            if (nx < 0 || nx > 39 || map[ny][nx] == 1) {
                dir = (dir == 2) ? 3 : 2;
            }
            else {
                m_x = nx;
            }
        }
        else if (pattern == 2) {
            // 이동 방법 2: 상하로 이동. 가장자리면 방향 전환 
            if (dir != 0 && dir != 1) dir = 1; // 상하가 아니면 아래로 초기화

            if (dir == 0) ny -= 1;
            else ny += 1;

            // 맵 밖이거나 장애물(1)이면 반대 방향으로 턴
            if (ny < 0 || ny > 39 || map[ny][nx] == 1) {
                dir = (dir == 0) ? 1 : 0;
            }
            else {
                m_y = ny;
            }
        }
        else if (pattern == 3) {
            // 이동 방법 3: 사각형 경로(위->우->아래->좌)로 이동 
            if (dir == 0) ny -= 1;
            else if (dir == 3) nx += 1;
            else if (dir == 1) ny += 1;
            else if (dir == 2) nx -= 1;

            // 막히면 바로 다음 90도 방향으로 꺾음 
            if (nx < 0 || nx > 39 || ny < 0 || ny > 39 || map[ny][nx] == 1) {
                dir = (dir == 0) ? 3 : (dir == 3) ? 1 : (dir == 1) ? 2 : 0;
                step = 0;
            }
            else {
                m_x = nx;
                m_y = ny;
                step++;
                // 2칸 전진했으면 90도 꺾기 (사각형 모양 만들기)
                if (step >= 2) {
                    dir = (dir == 0) ? 3 : (dir == 3) ? 1 : (dir == 1) ? 2 : 0;
                    step = 0;
                }
            }
        }
        else if (pattern == 4) {
            // 이동 방법 4: 제자리에 가만히 있는다 
            // 아무 좌표도 바꾸지 않음
        }
        else if (pattern == 5) {
            // 이동 방법 5: 제자리에서 크기만 커졌다 작아졌다 한다 
            if (sizeUp) {
                m_size += 0.2;
                if (m_size >= 1.5) sizeUp = false; // 너무 커지면 줄어들기 시작
            }
            else {
                m_size -= 0.2;
                if (m_size <= 0.5) sizeUp = true;  // 너무 작아지면 커지기 시작
            }
        }
    }

    // 0: 먹이, 1: 야생 꼬리, 2: Player 뒤에 붙은 꼬리
    int GetState() { return state; }
    // 0: 먹이, 1: 야생 꼬리, 2: Player 뒤에 붙은 꼬리
    void SetState(int s) { state = s; }
    void SetPattern(int p) { pattern = p; }
};

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
entity g_entity[20];
Player g_Player;

// --------------------------------------------------------
// 타이머 관련 전역 변수
// --------------------------------------------------------

const int TimerMove = 0;
const int TimerJump = 1;
int TickRate = 100;

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
    static RECT rectView;


    switch (uMsg) {
    case WM_CREATE:
        GetClientRect(hWnd, &rectView);
        break;
    case WM_LBUTTONDOWN: {
        int Mx = LOWORD(lParam);
        int My = HIWORD(lParam);

        int x = (Mx * 40) / wide;
        int y = (My * 40) / height;

        if (x == g_Player.GetX() && y == g_Player.GetY()) {
            for (int i = 0; i < 20; ++i) {
                if (g_entity[i].GetState() == 2) {
                    g_entity[i].SetState(0);       

                }
            }

            if (g_Player.GetShape() == 0) {
                g_Player.SetShape(2); 
            }
            else {
                g_Player.SetShape(0); 
            }
        }
        else {
            bool clickedTail = false;

            // 1. 내가 클릭한 곳이 내 '꼬리'들 중 하나인지 검사
            for (int i = 0; i < g_Player.tailCount; ++i) {
                int tIdx = g_Player.tails[i];
                if (g_entity[tIdx].GetX() == x && g_entity[tIdx].GetY() == y) {

                    // 클릭한 꼬리부터 맨 끝 꼬리까지 전부 야생(state 1)으로 강등!
                    for (int j = i; j < g_Player.tailCount; ++j) {
                        int detachIdx = g_Player.tails[j];
                        g_entity[detachIdx].SetState(1);
                        g_entity[detachIdx].SetPattern((uidPoint(gen) % 5) + 1); // 랜덤 패턴 부여
                    }
                    // 내 꼬리 개수를 잘라낸 위치 앞까지만 남김
                    g_Player.tailCount = i;
                    clickedTail = true;
                    break;
                }
            }

            //  2. 꼬리를 누른 게 아니라 진짜 '빈 땅'을 눌렀다면 기존처럼 방향 전환
            if (clickedTail == false) {
                int difX = g_Player.GetX() - x;
                int difY = g_Player.GetY() - y;

                if (abs(difX) >= abs(difY)) {
                    if (difX >= 0) g_Player.SetDirection(2);
                    else g_Player.SetDirection(3);
                }
                else {
                    if (difY >= 0) g_Player.SetDirection(0);
                    else g_Player.SetDirection(1);
                }
            }
        }
        break;
    }
    case WM_RBUTTONDOWN: {

        if (objCount < 20) {
            int Mx = LOWORD(lParam);
            int My = HIWORD(lParam);

            int x = (Mx * 40) / wide;
            int y = (My * 40) / height;
            if(map[y][x] != 1) ++objCount;
            map[y][x] = 1;
        }
        break;
    }
    case WM_KEYDOWN:
        if (wParam == 'Q') {
            PostQuitMessage(2025180028);
        }
        else if (wParam == 'S') {
            g_Player.Init(uidPoint(gen), uidPoint(gen));
            g_Player.SetColor(RGB(0, 0, 255));

            for (int i = 0; i < 20; ++i) {
                g_entity[i].Init(uidPoint(gen), uidPoint(gen), 0, 1);
                g_entity[i].SetColor(RGB(colorDist(gen), colorDist(gen), colorDist(gen)));
            }
            cout << "S키 인식" << '\n';
            SetTimer(hWnd, TimerMove, TickRate, (TIMERPROC)TimerProc);

            for (int i = 0; i < 40; ++i) {
                for (int j = 0; j < 40; ++j) {
                    map[i][j] = 0;
                }
            }
            objCount = 0;

        }
        else if (wParam == 'J') {
            cout << "J키 인식" << '\n';
            SetTimer(hWnd, TimerJump, TickRate, (TIMERPROC)TimerProc);
        }
        else if (wParam == 'T') {
            cout << "T키 인식" << '\n';
            if (g_Player.tailCount > 0) {
                int firstIdx = g_Player.tails[0];

                // 1. 주인공 데이터 백업
                int px = g_Player.GetX(), py = g_Player.GetY();
                int pox = g_Player.old_x, poy = g_Player.old_y;
                int pShape = g_Player.GetShape();
                COLORREF pColor = g_Player.GetColor();

                // 2. 주인공 자리에 1번 꼬리 데이터 덮어쓰기 (새로운 주인공 등극!)
                g_Player.SetX(g_entity[firstIdx].GetX());
                g_Player.SetY(g_entity[firstIdx].GetY());
                g_Player.old_x = g_entity[firstIdx].old_x;
                g_Player.old_y = g_entity[firstIdx].old_y;
                g_Player.SetShape(g_entity[firstIdx].GetShape());
                g_Player.SetColor(g_entity[firstIdx].GetColor());

                // 3. 수첩(배열)을 앞으로 한 칸씩 당기기
                for (int i = 0; i < g_Player.tailCount - 1; i++) {
                    g_Player.tails[i] = g_Player.tails[i + 1];
                }

                // 4. 맨 뒤에 원래 주인공 쑤셔넣기 (꼬리로 좌천)
                g_entity[firstIdx].setPos(px, py);
                g_entity[firstIdx].old_x = pox;
                g_entity[firstIdx].old_y = poy;
                g_entity[firstIdx].SetShape(pShape);
                g_entity[firstIdx].SetColor(pColor);
                g_Player.tails[g_Player.tailCount - 1] = firstIdx;

                // 5. 바뀐 순서에 맞춰서 크기 예쁘게 다시 줄여주기
                for (int i = 0; i < g_Player.tailCount; i++) {
                    double newSize = 1.0 - ((i + 1) * 0.1);
                    if (newSize < 0.2) newSize = 0.2;
                    g_entity[g_Player.tails[i]].SetSize(newSize);
                }
            }
        }
        else if (wParam == 'A') {
            cout << "A키 인식" << '\n';
            g_Player.isFever = !g_Player.isFever; // A를 누를 때마다 껐다 켰다(토글)

            if (g_Player.isFever) {
                // 피버 ON: 초고속 이동 모드
                KillTimer(hWnd, TimerMove);
                SetTimer(hWnd, TimerMove, 30, (TIMERPROC)TimerProc);

                // 야생 꼬리(state 1) 강제 블랙홀 흡수
                for (int i = 0; i < 20; i++) {
                    if (g_entity[i].GetState() == 1) {
                        g_entity[i].SetState(2);
                        g_entity[i].SetColor(g_Player.GetColor());
                        double newSize = 1.0 - ((g_Player.tailCount + 1) * 0.1);
                        if (newSize < 0.2) newSize = 0.2;
                        g_entity[i].SetSize(newSize);

                        g_Player.tails[g_Player.tailCount] = i;
                        g_Player.tailCount++;
                    }
                }
            }
            else {
                // 피버 OFF: 원래 속도로 복구
                KillTimer(hWnd, TimerMove);
                SetTimer(hWnd, TimerMove, TickRate, (TIMERPROC)TimerProc);
            }
        }
        else if (wParam == 'C') {
            cout << "TickRate : ";
            cin >> TickRate;
            KillTimer(hWnd, TimerMove);
            SetTimer(hWnd, TimerMove, TickRate, (TIMERPROC)TimerProc);
        }
        //좌우상하 키보드를 입력하면 주인공 원의 방향이 좌우상하로 바뀌고 이동한다
        else if (wParam == VK_UP) {
            cout << "^ 키 인식" << '\n';
            g_Player.SetDirection(0);
        }
        else if (wParam == VK_DOWN) {
            cout << "> 키 인식" << '\n';
            g_Player.SetDirection(1);
        }
        else if (wParam == VK_LEFT) {
            cout << "< 키 인식" << '\n';
            g_Player.SetDirection(2);
        }
        else if (wParam == VK_RIGHT) {
            cout << "v 키 인식" << '\n';
            g_Player.SetDirection(3);
        }
        else if (wParam == VK_OEM_PLUS) {
         
            if (TickRate >= 100) {
                SetTimer(hWnd, TimerMove, TickRate, (TIMERPROC)TimerProc);
            }

            TickRate -= 10;
            
            if (TickRate <= 0) {
                TickRate = 1;
            }

            KillTimer(hWnd, TimerMove);
            SetTimer(hWnd, TimerMove, TickRate, (TIMERPROC)TimerProc);
            cout << "+ 키 인식 : " << TickRate<< '\n';
        }
        else if (wParam == VK_OEM_MINUS) {
            cout << "- 키 인식 : " << TickRate << '\n';
            TickRate += 10;

            KillTimer(hWnd, TimerMove);
            SetTimer(hWnd, TimerMove, TickRate, (TIMERPROC)TimerProc);

            if (TickRate >= 100) {
                KillTimer(hWnd, TimerMove);
            }
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

        for (int i = 0; i <= 40; ++i) {
            MoveToEx(memDC, 0, (height * i) / 40, NULL);
            LineTo(memDC, wide, (height * i)/40);

            MoveToEx(memDC,wide * i /40, 0, NULL);
            LineTo(memDC, wide * i / 40, height);

            HBRUSH myBrush = CreateSolidBrush(RGB(255, 0, 0));
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, myBrush);

            for (int j = 0; j < 40; ++j) {
                if (map[i][j] == 1) {
                    Rectangle(memDC, (wide* j) / 40, (height* i) / 40, wide* (j + 1) / 40, height* (i + 1) / 40);
                }
            }

            SelectObject(memDC, oldBrush); // 제자리 돌아가기
            DeleteObject(myBrush);

        }

        for (int i = 0; i < 20; ++i) {
            g_entity[i].Draw(memDC);
        }

        g_Player.Draw(memDC);


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
        GetClientRect(hWnd, &rectView);
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
        // 1. 이동하기 전에 모두의 현재 위치를 '옛날 위치'로 백업!
        g_Player.old_x = g_Player.GetX();
        g_Player.old_y = g_Player.GetY();
        for (int i = 0; i < 20; ++i) {
            g_entity[i].old_x = g_entity[i].GetX();
            g_entity[i].old_y = g_entity[i].GetY();
        }

        // 2. 주인공 한 칸 이동
        g_Player.Move();

        // 3. 꼬리 기차놀이 당기기! (주인공이 실제로 움직였을 때만)
        if (g_Player.GetX() != g_Player.old_x || g_Player.GetY() != g_Player.old_y) {
            // 맨 뒤쪽 꼬리부터 차례대로 자기 앞사람의 발자국으로 이동함
            for (int i = g_Player.tailCount - 1; i >= 0; --i) {
                int tailIdx = g_Player.tails[i];

                if (i == 0) {
                    // 1번 꼬리(맨 앞)는 주인공의 옛날 발자국으로 이동
                    g_entity[tailIdx].setPos(g_Player.old_x, g_Player.old_y);
                }
                else {
                    // 2번 꼬리부터는 내 앞 꼬리의 옛날 발자국으로 이동
                    int frontIdx = g_Player.tails[i - 1];
                    g_entity[tailIdx].setPos(g_entity[frontIdx].old_x, g_entity[frontIdx].old_y);
                }
            }
        }

        // 4. 충돌 판정 및 야생 꼬리 이동
        for (int i = 0; i < 20; ++i) {
            if (g_Player.GetX() == g_entity[i].GetX() && g_Player.GetY() == g_entity[i].GetY()) {
                cout << g_Player.GetX() << ", " << g_Player.GetY() << "에서 충돌!!" << endl;

                if (g_entity[i].GetState() == 0) {
                    // [1] 먹이를 먹었을 때
                    g_Player.SetColor(g_entity[i].GetColor());

                    //  핵심 수정: 주인공 색이 바뀌면 기존에 달고 있던 꼬리들도 싹 다 같은 색으로 변경!
                    for (int j = 0; j < g_Player.tailCount; ++j) {
                        g_entity[g_Player.tails[j]].SetColor(g_Player.GetColor());
                    }

                    // 먹이는 야생 꼬리로 변신해서 랜덤 스폰
                    g_entity[i].Init(uidPoint(gen), uidPoint(gen), 1, 0);
                    g_entity[i].SetPattern((uidPoint(gen) % 5) + 1);
                }
                else if (g_entity[i].GetState() == 1) {
                    // [2] 야생 꼬리와 부딪혔을 때 (내 꼬리로 흡수)
                    g_entity[i].SetState(2);

                    //  핵심 수정: 새로 들어온 꼬리도 현재 주인공 색깔에 맞춰줌!
                    g_entity[i].SetColor(g_Player.GetColor());

                    double newSize = 1.0 - ((g_Player.tailCount + 1) * 0.1);
                    if (newSize < 0.2) newSize = 0.2; // 너무 안 보일까 봐 최소 크기 제한
                    g_entity[i].SetSize(newSize);

                    // 수첩에 꼬리 등록
                    g_Player.tails[g_Player.tailCount] = i;
                    g_Player.tailCount++;
                }
            }

            // 야생 상태(1)인 꼬리들만 스스로 패턴에 맞춰 이동
            if (g_entity[i].GetState() == 1) {
                g_entity[i].Move();
            }
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case TimerJump: {
        if( g_Player.isJumping == true) {
            g_Player.jumpOffset -= 1;
            g_Player.jumped();
            if (g_Player.jumpOffset == 0) {
                g_Player.isJumping = false;
                KillTimer(hWnd, TimerJump);
            }
        }
        else if (g_Player.jumpOffset < 2) {
            g_Player.jumpOffset += 1;
            g_Player.jumping();
            if (g_Player.jumpOffset == 2) {
                g_Player.isJumping = true;
            }
        }
        

        InvalidateRect(hWnd, NULL, false);

        // 모두가 바닥에 무사히 착지했다면 점프 타이머 종료 (메모리 절약)
        break;
    }


    }
}
