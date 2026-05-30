#define _CRT_SECURE_NO_WARNINGS

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include<iostream>
#include <ctime>
#include "resource.h"

using namespace std;
int wide{ 800 };
int height{ 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);

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
    WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);  // 메뉴 이름 !!
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
// 함수 선언 구간
// --------------------------------------------------------

void InitGame();
void ShowGameTime(HWND hWnd);

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
// 1. 상태 변수
bool isPlaying = false; // 공이 튀기고 있는지 여부
bool isPaused = false;  // p키를 눌러 멈췄는지 여부
bool isDrag = false;
bool isOver = false;

time_t startTime;

int PreMouseXPos = 0;

// 2. 바(Bar) 정보
int barX = 350;         // 바의 좌측 상단 X 좌표
int barY = 700;         // 바의 고정된 Y 좌표
int barWidth = 100;     // 바의 가로 길이
int barHeight = 20;     // 바의 세로 두께

// 3. 공(Ball) 정보
int ballX = 400;        // 공의 중심 X
int ballY = 680;        // 공의 중심 Y (처음엔 바 바로 위에 위치)
int ballRadius = 10;    // 공의 반지름
int ballDX =    3;         // 공의 X축 이동 속도 (대각선 방향)
int ballDY =   -3;        // 공의 Y축 이동 속도 (위로 쏘아올림)

int ballDropCnt = 0;
int ballOddColorCnt = 0;


// 4. 벽돌(Brick) 정보
#define brickRows  3      // 벽돌 줄 숫자 (변경 가능)
#define brickCols  10     // 벽돌 칸 숫자 (변경 가능)
int brickWidth = ( wide - 100 )/ brickCols;
int brickHeight = 30;
int brickOffsetX = 50; 
int brickDropSpeed = 10;

int brickLine[brickRows]{brickCols, brickCols ,brickCols };

struct Brick {
    bool isAlive;       // 벽돌이 위에 살아있는가?
    bool isDropping;    // 맞아서 아래로 떨어지는 중인가?
    int dropX, dropY;   // 떨어질 때의 현재 좌표
    COLORREF color;     // 벽돌의 색상
};

Brick bricks[brickRows][brickCols];  

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
        InitGame();
        void ShowGameTime(HWND hWnd);
        SetTimer(hWnd, 1, 3, NULL);
        GetClientRect(hWnd, &rectView);
        break;
    case WM_KEYDOWN:
        if (wParam == 'Q') {
            PostQuitMessage(2025180028);
        }
        else if (wParam == 'S') {
            isPlaying = true;
        }
        else if (wParam == 'P') {
            if (isPaused == true) {
                isPaused = false;
            }
            else {
                isPaused = true;
            }
        }
        else if (wParam == 'R') {
            InitGame();
        }
        else if (wParam == 'T') {
            ShowGameTime(hWnd);
        }
        else if (wParam == VK_OEM_PLUS) {
            if (ballDX >= 0) {
                ballDX += 1;
            }
            else {
                ballDX -= 1;
            }

            if (ballDY > 0) {
                ballDY += 1;
            }
            else {
                ballDY -= 1;
            }
        }
        else if (wParam == VK_OEM_MINUS) {
            if (ballDX == 0) {
                
            }
            else if (ballDX > 0) {
                ballDX -= 1;
            }
            else {
                ballDX += 1;
            }

            if (ballDY == 0) {

            }
            else if (ballDY < 0) {
                ballDY += 1;
            }
            else {
                ballDY -= 1;
            }
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    case WM_LBUTTONDOWN: {

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        if (barX <= x && barX + barWidth >= x && barY <= y && barY + barHeight >= y) {
            cout << "바 인식" << '\n';
            isDrag = true;
            PreMouseXPos = x;
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_COMMAND: {
        int wmid = LOWORD(wParam);
        switch (wmid)
        {
        case ID_MENU_START :
            isPlaying = true;
            break;
        case ID_MENU_PAUSE :
            isPaused = true;
            break;
        case ID_MENU_RESTART :
            isPaused = false;
            break;
        case ID_MENU_QUIT :
            isPlaying = false;
            break;
        case ID_MENU_RESET :
            InitGame();
            break;
        case ID_MENU_TIME:
            ShowGameTime(hWnd);
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_MOUSEMOVE: {

        if (isDrag == true) {
            int x = LOWORD(lParam);

            int dx = x - PreMouseXPos;

            barX += dx;

            PreMouseXPos = x;

            if (barX + barWidth > wide) {
                barX = wide - barWidth;
            }

            if (barX < 0) {
                barX = 0;
            }
            InvalidateRect(hWnd, NULL, false);
        }
        break;
    }
    case WM_LBUTTONUP : 

        isDrag = false;

        InvalidateRect(hWnd, NULL, false);
        break;
    case WM_TIMER: {

        if (isPlaying && !isPaused) {

            if (isPlaying && !isPaused) {
                ballX += ballDX;
                ballY += ballDY;

                if (ballX - ballRadius <= 0) { // 왼쪽 벽
                    ballX = ballRadius;
                    ballDX = -ballDX;
                }
                else if (ballX + ballRadius >= wide) { // 오른쪽 벽
                    ballX = wide - ballRadius;
                    ballDX = -ballDX;
                }

                if (ballY - ballRadius <= 0) { // 천장
                    ballY = ballRadius;
                    ballDY = -ballDY;
                }

                if (ballY > height) {
                    ballY = barY - barHeight;
                    ballX = barX + (barWidth / 2);
                    ballDY = -ballDY;
                }

                // 3. 벽돌 충돌

                for (int r = 0; r < brickRows; ++r) {
                    for (int c = 0; c < brickCols; ++c) {
                        if (bricks[r][c].isAlive) {
                            int bx = brickOffsetX + (c * brickWidth);
                            int by = 50 + (r * brickHeight);

                            if (ballY - ballRadius <= by + brickHeight && ballX >= bx && bx + brickWidth > ballX) {
                                ballDY = -ballDY;
                                bricks[r][c].isAlive = false;
                               
                                
                                ballDropCnt++;
                                
                                --brickLine[r];

                              
                                
                                bricks[r][c].dropX = bx;
                                bricks[r][c].dropY = by;
                            }

                            if (ballY - ballRadius <= by + brickHeight) {
                                bricks[r][c].color = RGB(120, 120, 250);
                                
                            }
                        }
                    }
                }

                // 4. 떨어지는 벽돌 처리 
                for (int r = 0; r < brickRows; ++r) {
                    for (int c = 0; c < brickCols; ++c) {
                        if (bricks[r][c].isAlive == false && bricks[r][c].dropY < height ) {
                            bricks[r][c].dropY += brickDropSpeed;
                        }
                    }
                }

                if (barX <= ballX && barX + barWidth >= ballX && barY <= ballY && barY + barHeight >= ballY) {
                    ballDY = -ballDY;
                }
            }
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);


        // 계속해서 화면을 다시 그릴 때 활용 <- 더블 버퍼링
        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        HBRUSH bgBrush = CreateSolidBrush(RGB(120, 120, 200));
        FillRect(memDC, &ps.rcPaint, bgBrush);
        DeleteObject(bgBrush);
        // memDC를 사용해서 그리기

        for (int r = 0; r < brickRows; ++r) {
            for (int c = 0; c < brickCols; ++c) {
                HBRUSH brBrush = CreateSolidBrush(bricks[r][c].color);
                HBRUSH oldBr = (HBRUSH)SelectObject(memDC, brBrush);
                if (bricks[r][c].isAlive) {
                    int bx = brickOffsetX + (c * brickWidth);
                    int by = 50 + (r * brickHeight);


                    Rectangle(memDC, bx, by, bx + brickWidth, by + brickHeight);

                }
                else {
                    HBRUSH DropBrush = CreateSolidBrush(RGB(colorDist(gen), colorDist(gen), colorDist(gen)));
                    HBRUSH DropoldBr = (HBRUSH)SelectObject(memDC, DropBrush);
                    
                    Rectangle(memDC, bricks[r][c].dropX, bricks[r][c].dropY, bricks[r][c].dropX + brickWidth, bricks[r][c].dropY + brickHeight);
                    
                    SelectObject(memDC, DropoldBr);
                    DeleteObject(DropBrush);
                }
                SelectObject(memDC, oldBr);
                DeleteObject(brBrush);
            }
        }
        if (isPaused) {
            SetBkMode(memDC, TRANSPARENT); // 배경 투명하게
            SetTextColor(memDC, RGB(0, 0, 0)); // 새빨간 글씨

            // 글씨 크기를 100 픽셀로 무지하게 크게 키우기 위한 폰트 설정!
            HFONT hFont = CreateFont(30, 0, 0, 0, FW_HEAVY, 0, 0, 0,
                HANGEUL_CHARSET, 0, 0, 0, 0, TEXT("맑은 고딕"));
            HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

            RECT rt = { 0, 0, wide, height };

            ballOddColorCnt = 0;

            cout << "떨어진 돌 계수 : " << ballDropCnt << '\n';

            for (int i = 0; i < brickRows; ++i) {
                if (brickLine[i] != brickCols) {
                    ballOddColorCnt += brickLine[i];
                }
            }
            cout << "변한 색깔 계수 : " << ballOddColorCnt << '\n';

            TCHAR scoreStr[50];
            wsprintf(scoreStr, L"떨어진 블럭 계수 : %d, 색바뀐 블럭 계수 : %d ", ballDropCnt, ballOddColorCnt);

            TextOut(memDC, 10, 10, scoreStr, lstrlen(scoreStr));

            // 폰트 정리
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
        }

        HBRUSH BarBrush = CreateSolidBrush(RGB(255, 212, 0));
        HBRUSH BarOldBrush = (HBRUSH)SelectObject(memDC, BarBrush);

        Rectangle(memDC, barX, barY, barX + barWidth, barY + barHeight);

        SelectObject(memDC, BarOldBrush);
        DeleteObject(BarBrush);


        HBRUSH BallBrush = CreateSolidBrush(RGB(0, 0, 255));
        HBRUSH BallOldBrush = (HBRUSH)SelectObject(memDC, BallBrush);

        Ellipse(memDC, ballX - ballRadius, ballY - ballRadius, ballX + ballRadius, ballY + ballRadius);

        SelectObject(memDC, BallOldBrush);
        DeleteObject(BallBrush);

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

void InitGame() {
    isPlaying = false;
    isPaused = false;

    startTime = time(NULL);

    // 바를 가운데로
    barX = (wide - barWidth) / 2;

    if (ballDY > 0) {
        ballDY = -ballDY;
    }

    // 공을 바 위 한가운데로
    ballX = barX + (barWidth / 2);
    ballY = barY - ballRadius - 1;

    // 3x10 벽돌 초기화
    for (int r = 0; r < brickRows; ++r) {
        // 줄마다 임의의 색상 지정
        COLORREF rowColor = RGB(255, 212, 0);
        for (int c = 0; c < brickCols; ++c) {
            bricks[r][c].isAlive = true;
            bricks[r][c].isDropping = false;
            bricks[r][c].color = rowColor;
        }
    }
}

void ShowGameTime(HWND hWnd) {
    // 1. 현재 시간 구하기 및 플레이 시간(초) 계산
    time_t currentTime = time(NULL);
    int playTime = (int)difftime(currentTime, startTime);

    struct tm* tmInfo;

    // 2. 시작 시간 분해하기 (시, 분, 초)
    tmInfo = localtime(&startTime);
    int sHour = tmInfo->tm_hour;
    int sMin = tmInfo->tm_min;
    int sSec = tmInfo->tm_sec;

    // 3. 현재 시간 분해하기 (시, 분, 초)
    tmInfo = localtime(&currentTime);
    int cHour = tmInfo->tm_hour;
    int cMin = tmInfo->tm_min;
    int cSec = tmInfo->tm_sec;

    // 4. 예쁜 문자열로 조립하기
    TCHAR msg[256];
    wsprintf(msg, L"시작 시간\t: %02d시 %02d분 %02d초\n현재 시간\t: %02d시 %02d분 %02d초\n\n플레이 시간\t: %d초",
        sHour, sMin, sSec, cHour, cMin, cSec, playTime);

    // 5. 팝업창을 띄우는 동안 잠시 게임 멈춰주기 (센스!)
    bool wasPlaying = isPlaying;
    isPlaying = false;

    // 6. 알림창 띄우기
    MessageBox(hWnd, msg, L"Time Info", MB_OK | MB_ICONINFORMATION);

    // 7. 창을 닫으면 원래 상태로 복구
    isPlaying = wasPlaying;
}