#define _CRT_SECURE_NO_WARNINGS

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include<iostream>
#include "resource.h"

using namespace std;
int wide    { 800 };
int height  { 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_int_distribution<int> uidBoard(0, 9);

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
    WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);;
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

void InitBoard();
void DrawItem(HDC memDC, HWND hWnd, int x, int y);
void openPie(HDC memDC, int x, int y);

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------

int pieType[5]{ 3,4,5,6,7 };

COLORREF colorSet[5]{
    {RGB(colorDist(gen),colorDist(gen),colorDist(gen))},
    {RGB(colorDist(gen),colorDist(gen),colorDist(gen))},
    {RGB(colorDist(gen),colorDist(gen),colorDist(gen))},
    {RGB(colorDist(gen),colorDist(gen),colorDist(gen))},
    {RGB(colorDist(gen),colorDist(gen),colorDist(gen))}
};


int Row = 10;
int Col = 10;

// pie는 11~14, 21~24, 31~34, 41~44 순으로 되어있음
// 1 지뢰, 2 아이템
int Board[10][10]{};

int RCell = wide / Row;
int CCell = height / Col;

bool isOpen[10][10]{ false };
bool isHint = false;
bool isOver = false;
bool isScore = true;

int NumOfcolor  = 5;
int NumOfmine   = 20;
int NumOfpieSet = 5;
int NumOfitem   = 10;

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
        InitBoard();
        break;
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_GAME_START :
            InitBoard();
            break;
        case ID_GAME_END :
            isOver = true;
            break;
        case ID_GAME_HINT :
            isHint = true;
            SetTimer(hWnd, 1, 1000, NULL);
            break;
        case ID_GAME_SCORE :
            isScore = !isScore;
            break;
        }
        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_TIMER: {

        switch (wParam)
        {
        case 1: 
            isHint = false;      
            KillTimer(hWnd, 1);  
            break;
        }

        InvalidateRect(hWnd, NULL, false); 
        break;
    }
    case WM_LBUTTONDOWN: {
        if (isOver == true) break;

        int xPos = LOWORD(lParam)/CCell;
        int yPos = HIWORD(lParam)/RCell;

        if (!isOpen[yPos][xPos]) {
            isOpen[yPos][xPos] = true; 

            if (Board[yPos][xPos] == 1) {
                isOver = true;
            }

            if (Board[yPos][xPos] == 2) {

                // 1단계: 지금 보드에 어떤 색깔의 파이가 열려있는지 조사 (1번~5번 색상)
                bool openedColors[6] = { false };

                for (int r = 0; r < Row; ++r) {
                    for (int c = 0; c < Col; ++c) {
                        // 열려있는 타일 중 파이(11 이상)를 발견하면
                        if (isOpen[r][c] == true && Board[r][c] >= 11) {
                            int color = Board[r][c] / 10; // 파이 색상 알아내기
                            openedColors[color] = true;   // 그 색상은 '열린 적 있음'으로 체크
                        }
                    }
                }

                // 2단계: 체크된 색상과 같은 파이는 무조건 싹 다 열어버리기!
                for (int r = 0; r < Row; ++r) {
                    for (int c = 0; c < Col; ++c) {
                        if (Board[r][c] >= 11) {
                            int color = Board[r][c] / 10;
                            // 1단계에서 체크된 색상이라면? 나머지 조각 오픈!
                            if (openedColors[color] == true) {
                                isOpen[r][c] = true;
                            }
                        }
                    }
                }
            }
        }


        cout << "( " << xPos << ", " << yPos << " )" << '\n';
        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_KEYDOWN:
       if (wParam == 'H') {
            if (isHint == true) {
                isHint = false;
            }
            else {
                isHint = true;
            }
       }
       else if (wParam == 'Q') {
           PostQuitMessage(2025180028);
       }
       InvalidateRect(hWnd, NULL, false);
       break;

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

        HPEN bluePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 120));
        HPEN oldPen = (HPEN)SelectObject(memDC, bluePen);

        HBRUSH RectBrush = CreateSolidBrush(RGB(80,80, 80));
        HBRUSH RectoldBrush = (HBRUSH)SelectObject(memDC, RectBrush);

        for (int r = 0; r < Row; ++r) {
            for (int c = 0; c < Col; ++c) {
                int x = c * CCell;
                int y = r * RCell;

                Rectangle(memDC, x, y, x + CCell, y + RCell);

            }
        }

        SelectObject(memDC, RectoldBrush);
        DeleteObject(RectBrush);

        SelectObject(memDC, oldPen);
        DeleteObject(bluePen);

        for (int r = 0; r < Col; ++r) {
            for (int c = 0; c < Row; ++c) {
                if (isOpen[r][c] == true || isHint == true) {
                    int x = c * CCell;
                    int y = r * RCell;

                    Rectangle(memDC, x, y, x + CCell, y + RCell);

                    if (Board[r][c] == 1) { 
                        SetTextColor(memDC, RGB(255, 0, 0));
                        TextOut(memDC, x + (CCell / 3), y + (RCell / 3), L"B", 1);
                    }
                    else if (Board[r][c] == 2) { 
                        DrawItem(memDC, hWnd, x, y);
                    }
                    else if (Board[r][c] >= 11) { 
                        int color = Board[r][c] / 10; 
                        int piece = Board[r][c] % 10; 

                        HBRUSH PieBrush = CreateSolidBrush(colorSet[color]);
                        HBRUSH PieoldBrush = (HBRUSH)SelectObject(memDC, PieBrush);

                        switch (piece)
                        {
                        case 1: {
                            Pie(memDC, x, y, x + CCell, y + RCell, x + CCell, y + (RCell / 2), x + (CCell / 2), y);
                            break;
                        }
                        case 2: {
                            Pie(memDC, x, y, x + CCell, y + RCell, x + (CCell / 2), y, x, y + (RCell / 2));
                            break;
                        }
                        case 3: {
                            Pie(memDC, x, y, x + CCell, y + RCell, x, y + (RCell / 2), x + (CCell / 2), y + RCell);
                            break;
                        }
                        case 4: {
                            Pie(memDC, x, y, x + CCell, y + RCell, x + (CCell / 2), y + RCell, x + CCell, y + (RCell / 2));
                            break;
                        }
                        }

                        SelectObject(memDC, PieoldBrush);
                        DeleteObject(PieBrush);
                    }
                }
            }
        }

        if (isScore == true) {
            int completedPies = 0; // 완성된 파이 개수

            // 1번 색상부터 5번 색상까지 각각 4조각이 다 모였는지 검사
            for (int color = 1; color <= 5; ++color) {
                int pieceCount = 0;

                // 보드 전체를 뒤져서 현재 색상(color)과 일치하면서 열려있는 조각 개수 세기
                for (int r = 0; r < Row; ++r) {
                    for (int c = 0; c < Col; ++c) {
                        if (isOpen[r][c] == true && Board[r][c] >= 11) {
                            if (Board[r][c] / 10 == color) {
                                pieceCount++;
                            }
                        }
                    }
                }

                // 해당 색상의 조각 4개를 모두 찾았다면 완성된 파이 1개 추가!
                if (pieceCount == 4) {
                    completedPies++;
                }
            }

            // 계산된 점수를 화면 특정 위치(예: 좌측 상단)에 출력
            TCHAR scoreStr[50];
            wsprintf(scoreStr, L"Score : %d", completedPies);

            SetBkMode(memDC, OPAQUE); // 글자 배경을 불투명하게 해서 눈에 띄게 만듦
            SetBkColor(memDC, RGB(0, 0, 0)); // 글자 배경은 검은색
            SetTextColor(memDC, RGB(255, 255, 0)); // 글자는 노란색

            // 화면의 왼쪽 위 (10, 10) 위치에 출력
            TextOut(memDC, 10, 10, scoreStr, lstrlen(scoreStr));
        }

        if (isOver == true) {
            SetBkMode(memDC, TRANSPARENT); // 배경 투명하게
            SetTextColor(memDC, RGB(255, 0, 0)); // 새빨간 글씨

            // 글씨 크기를 100 픽셀로 무지하게 크게 키우기 위한 폰트 설정!
            HFONT hFont = CreateFont(100, 0, 0, 0, FW_HEAVY, 0, 0, 0,
                HANGEUL_CHARSET, 0, 0, 0, 0, TEXT("맑은 고딕"));
            HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

            RECT rt = { 0, 0, wide, height };
            DrawText(memDC, L"GAME OVER", -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

            // 폰트 정리
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
        }

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

        CCell = wide / Col;   // 가로 한 칸 크기
        RCell = height / Row;

        InvalidateRect(hWnd, NULL, true);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void openPie(HDC memDC, int x, int y)
{

}

void InitBoard()
{
    isOver = false;
    isScore = true;

    for (int r = 0; r < 10; ++r) {
        for (int c = 0; c < 10; ++c) {
            Board[r][c] = 0;
            isOpen[r][c] = false;
        }
    }

    int mineCount = 0;
    while (mineCount < NumOfmine) {
        int x = uidBoard(gen);
        int y = uidBoard(gen);
        if (Board[y][x] == 0) {
            Board[y][x] = 1;
            mineCount++;
        }
    }

    int itemCnt = 0;
    while (itemCnt < NumOfitem) {
        int x = uidBoard(gen);
        int y = uidBoard(gen);
        if (Board[y][x] == 0) {
            Board[y][x] = 2;
            itemCnt++;
        }
    }

    for (int pieColor = 1; pieColor <= NumOfcolor; ++pieColor) {
        for (int piece = 1; piece <= 4; ++piece) { 
            while (true) {
                int x = uidBoard(gen);
                int y = uidBoard(gen);
                if (Board[y][x] == 0) {
                    Board[y][x] = (pieColor * 10) + piece;
                    break; 
                }
            }
        }
    }
}

void DrawItem(HDC memDC, HWND hWnd, int x, int y)
{
    int margin = 5;

    HBRUSH itemBrush = CreateSolidBrush(RGB(255, 212, 0));
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, itemBrush);

    POINT p[4] = {
        { x + (CCell / 2), y + margin },               
        { x + CCell - margin, y + (RCell / 2) },       
        { x + (CCell / 2), y + RCell - margin },       
        { x + margin, y + (RCell / 2) }                
    };

    Polygon(memDC, p, 4);

    SelectObject(memDC, oldBrush);
    DeleteObject(itemBrush);

    for (int r = 0; r < Row; ++r) {
        for (int c = 0; c < Col; ++c) {

        }
    }
}