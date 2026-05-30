#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <random>    
#include <iostream>
#include <ctime>
#include "resource.h"
#include <stack>

using namespace std;

// 전역 변수
int wide{ 800 };
int height{ 800 };
HINSTANCE g_hInst;

int hiddenRow = -1;          
int hiddenCol = -1;

// 비트맵 관련
HBITMAP g_hImg1, g_hImg2;
HBITMAP currentImg;
BITMAP bmpInfo;

// 게임 모드: 0=격자(기본), 1=세로분할(V), 2=가로분할(H)
int gameMode = 0;

// 게임 로직 관련
int puzzleSize = 3;
int puzzleRows = 3;
int puzzleCols = 3;
int board[5][5];
bool isVisible[5][5];
int hiddenPiece = -1;
int emptyRow, emptyCol;

bool isPlaying = false;
bool isShowOriginal = false;

// 드래그 애니메이션 관련
bool isDragging = false;
int startMouseX = 0, startMouseY = 0;
int movingRow = -1, movingCol = -1;
int slideOffsetX = 0, slideOffsetY = 0;

random_device rd;
mt19937 gen(rd());

// 함수 선언
void InitPuzzle();
void ShufflePuzzle();
bool CheckClear();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;
    g_hInst = hInstance;

    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    WndClass.lpszClassName = L"PuzzleGameClass";
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(L"PuzzleGameClass", L"조각 퍼즐 맞추기", WS_OVERLAPPEDWINDOW, 0, 0, wide, height, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return Message.wParam;
}

// 퍼즐판 초기화
void InitPuzzle() {
    // 모드에 따라 행렬 크기 설정
    if (gameMode == 0) { puzzleRows = puzzleSize; puzzleCols = puzzleSize; }
    else if (gameMode == 1) { puzzleRows = 1; puzzleCols = puzzleSize; }
    else if (gameMode == 2) { puzzleRows = puzzleSize; puzzleCols = 1; }

    int pieceNum = 0;
    for (int r = 0; r < puzzleRows; r++) {
        for (int c = 0; c < puzzleCols; c++) {
            board[r][c] = pieceNum++;
            isVisible[r][c] = true;
        }
    }

    if (gameMode == 0) {
        emptyRow = puzzleRows - 1;
        emptyCol = puzzleCols - 1;
    }
    else {
        emptyRow = -1;
        emptyCol = -1;
    }

    hiddenPiece = -1;
    isPlaying = false;
    isShowOriginal = false;
}

// 퍼즐 섞기
void ShufflePuzzle() {
    for (int i = 0; i < 200; i++) {
        int r1 = gen() % puzzleRows;
        int c1 = gen() % puzzleCols;
        int r2 = gen() % puzzleRows;
        int c2 = gen() % puzzleCols;

        int temp = board[r1][c1];
        board[r1][c1] = board[r2][c2];
        board[r2][c2] = temp;
    }

    if (gameMode == 0) {
        int lastPiece = puzzleRows * puzzleCols - 1;
        for (int r = 0; r < puzzleRows; r++) {
            for (int c = 0; c < puzzleCols; c++) {
                if (board[r][c] == lastPiece) {
                    emptyRow = r;
                    emptyCol = c;
                }
            }
        }
    }
    isPlaying = true;
}

// 퍼즐 완성 여부 체크
bool CheckClear() {
    if (!isPlaying || isShowOriginal) return false;

    int num = 0;
    for (int r = 0; r < puzzleRows; r++) {
        for (int c = 0; c < puzzleCols; c++) {
            if (board[r][c] != num++) return false;
        }
    }
    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    switch (uMsg) {
    case WM_CREATE:
        // 이미지 두 개 로드 (각각 다른 비트맵 리소스 ID 사용)
        g_hImg1 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
        g_hImg2 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
        // 기본값 세팅
        currentImg = g_hImg1;
        GetObject(currentImg, sizeof(BITMAP), &bmpInfo);
        InitPuzzle();
        break;

    case WM_COMMAND:
        // 메뉴 명령어 처리
        switch (LOWORD(wParam)) {
        case ID_MENU_IMG1: currentImg = g_hImg1; GetObject(currentImg, sizeof(BITMAP), &bmpInfo); InitPuzzle(); InvalidateRect(hWnd, NULL, TRUE); break;
        case ID_MENU_IMG2: currentImg = g_hImg2; GetObject(currentImg, sizeof(BITMAP), &bmpInfo); InitPuzzle(); InvalidateRect(hWnd, NULL, TRUE); break;
        case ID_MENU_SIZE3: puzzleSize = 3; gameMode = 0; InitPuzzle(); InvalidateRect(hWnd, NULL, TRUE); break;
        case ID_MENU_SIZE4: puzzleSize = 4; gameMode = 0; InitPuzzle(); InvalidateRect(hWnd, NULL, TRUE); break;
        case ID_MENU_SIZE5: puzzleSize = 5; gameMode = 0; InitPuzzle(); InvalidateRect(hWnd, NULL, TRUE); break;
        case ID_MENU_START: ShufflePuzzle(); InvalidateRect(hWnd, NULL, TRUE); break;
        case ID_MENU_ORIGNAL : isShowOriginal = !isShowOriginal; InvalidateRect(hWnd, NULL, TRUE); break;
        case ID_MENU_QUIT: PostQuitMessage(0); break;
        }
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case 'S': ShufflePuzzle(); break;
        case 'F': isShowOriginal = !isShowOriginal; break;
        case 'Q': PostQuitMessage(0); break;
        case '3': puzzleSize = 3; gameMode = 0; InitPuzzle(); break;
        case '4': puzzleSize = 4; gameMode = 0; InitPuzzle(); break;
        case '5': puzzleSize = 5; gameMode = 0; InitPuzzle(); break;
        case 'V': gameMode = 1; InitPuzzle(); ShufflePuzzle(); break; // V모드: 세로분할 및 섞기
        case 'H': gameMode = 2; InitPuzzle(); ShufflePuzzle(); break; // H모드: 가로분할 및 섞기
        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_LBUTTONDOWN: {
        if (!isPlaying || isShowOriginal) break;

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        int cWidth = wide / puzzleCols;
        int cHeight = height / puzzleRows;
        int row = y / cHeight;
        int col = x / cWidth;

        if (gameMode == 0) { // 격자 모드 드래그 시작
            if ((abs(row - emptyRow) == 1 && col == emptyCol) || (abs(col - emptyCol) == 1 && row == emptyRow)) {
                isDragging = true;
                startMouseX = x; startMouseY = y;
                movingRow = row; movingCol = col;
                slideOffsetX = 0; slideOffsetY = 0;
            }
        }
        else { // V, H 모드 드래그 시작 (모든 조각 드래그 가능)
            isDragging = true;
            startMouseX = x; startMouseY = y;
            movingRow = row; movingCol = col;
            slideOffsetX = 0; slideOffsetY = 0;
        }
        break;
    }

    case WM_MOUSEMOVE: {
        if (isDragging) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int dx = x - startMouseX;
            int dy = y - startMouseY;

            int cWidth = wide / puzzleCols;
            int cHeight = height / puzzleRows;

            if (gameMode == 0) { // 빈칸으로만 이동 제한
                if (movingRow == emptyRow) {
                    dy = 0;
                    if (movingCol < emptyCol) dx = max(0, min(dx, cWidth));
                    else                      dx = max(-cWidth, min(dx, 0));
                }
                else {
                    dx = 0;
                    if (movingRow < emptyRow) dy = max(0, min(dy, cHeight));
                    else                      dy = max(-cHeight, min(dy, 0));
                }
            }
            else if (gameMode == 1) { // V모드: 가로로만 이동 제한
                dy = 0;
            }
            else if (gameMode == 2) { // H모드: 세로로만 이동 제한
                dx = 0;
            }

            slideOffsetX = dx;
            slideOffsetY = dy;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_LBUTTONUP: {
        if (isDragging) {
            isDragging = false;
            int cWidth = wide / puzzleCols;
            int cHeight = height / puzzleRows;


            if (gameMode == 0) { // 격자 모드 스왑
                if (abs(slideOffsetX) > cWidth / 2 || abs(slideOffsetY) > cHeight / 2) {
                    int temp = board[movingRow][movingCol];
                    board[movingRow][movingCol] = board[emptyRow][emptyCol];
                    board[emptyRow][emptyCol] = temp;
                    emptyRow = movingRow;
                    emptyCol = movingCol;
                }
            }
            else { // V, H 모드 스왑 (놓은 위치의 타일과 교환)
                int dropX = startMouseX + slideOffsetX;
                int dropY = startMouseY + slideOffsetY;

                // 화면 밖을 벗어나는 오류 방지
                dropX = max(0, min(dropX, wide - 1));
                dropY = max(0, min(dropY, height - 1));

                int targetCol = dropX / cWidth;
                int targetRow = dropY / cHeight;

                if (targetRow != movingRow || targetCol != movingCol) {
                    int temp = board[movingRow][movingCol];
                    board[movingRow][movingCol] = board[targetRow][targetCol];
                    board[targetRow][targetCol] = temp;
                }
            }

            slideOffsetX = 0; slideOffsetY = 0;
            movingRow = -1; movingCol = -1;
            InvalidateRect(hWnd, NULL, FALSE);

            // 이동 후 클리어 체크
            if (CheckClear()) {
                MessageBox(hWnd, L"퍼즐을 완성했습니다!", L"게임 종료", MB_OK);
                isPlaying = false;
            }
        }
        break;
    }

    case WM_LBUTTONDBLCLK: { // 더블클릭 로직 (격자 모드에서만 사용)
        if (!isPlaying || isShowOriginal || gameMode != 0) break;

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        int cWidth = wide / puzzleCols;
        int cHeight = height / puzzleRows;
        int row = y / cHeight;
        int col = x / cWidth;

        if (row != emptyRow || col != emptyCol) {
            int temp = board[row][col];
            board[row][col] = board[emptyRow][emptyCol];
            board[emptyRow][emptyCol] = temp;

            emptyRow = row;
            emptyCol = col;
            InvalidateRect(hWnd, NULL, FALSE);

            if (CheckClear()) {
                MessageBox(hWnd, L"퍼즐을 완성했습니다!", L"게임 종료", MB_OK);
                isPlaying = false;
            }
        }
        break;
    }

    case WM_RBUTTONDOWN: { // 우클릭: 조각 숨기기 및 추가
        if (!isPlaying || isShowOriginal || gameMode != 0) break;

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        int cWidth = wide / puzzleCols;
        int cHeight = height / puzzleRows;
        int row = y / cHeight;
        int col = x / cWidth;

        stack<int> stack;

        // 1. 그림이 없는 칸을 클릭한 경우 (진짜 빈칸 이거나 이미 숨겨진 칸)
        if ((row == emptyRow && col == emptyCol) || !isVisible[row][col]) {
            if (hiddenPiece == -1) {
                //  추가할 그림이 없을 때 에러 메시지
                MessageBox(hWnd, L"추가할 그림이 없습니다", L"Error", MB_OK);
            }
            else {
                // A. 진짜 빈칸을 우클릭해서 숨겨진 조각을 여기로 부르는 경우 (스왑)
                if (row == emptyRow && col == emptyCol) {
                    board[hiddenRow][hiddenCol] = board[emptyRow][emptyCol]; // 기존 숨겨진 자리에 빈칸 값을 넣음
                    board[emptyRow][emptyCol] = hiddenPiece;                 // 진짜 빈칸 자리에 숨겨진 조각을 넣음

                    isVisible[hiddenRow][hiddenCol] = true; // 숨겼던 자리는 이제 진짜 빈칸이 되었으니 표시(true)
                    isVisible[row][col] = true;             // 클릭한 자리도 조각이 채워졌으니 표시(true)

                    // 빈칸 위치 업데이트
                    emptyRow = hiddenRow;
                    emptyCol = hiddenCol;
                }
                // B. 숨겨진 칸을 다시 우클릭해서 제자리에 복구하는 경우
                else if (!isVisible[row][col]) {
                    isVisible[row][col] = true;
                }

                // 조각을 화면에 추가했으니, 숨겨진 정보는 초기화
                hiddenPiece = -1;
                hiddenRow = -1;
                hiddenCol = -1;

                // 조각이 이동했으니 퍼즐이 완성되었는지 클리어 체크
                if (CheckClear()) {
                    InvalidateRect(hWnd, NULL, FALSE);
                    MessageBox(hWnd, L"퍼즐을 완성했습니다!", L"게임 종료", MB_OK);
                    isPlaying = false;
                }
            }
        }
        // 2. 그림이 있는 칸을 클릭한 경우 (숨기기)
        else {
            // 만약 이미 숨겨놓은 다른 조각이 있다면, 그 조각을 원래 자리에 다시 토해냅니다. (1개만 숨기기 위해)
            if (hiddenPiece != -1) {
                isVisible[hiddenRow][hiddenCol] = true;
            }

            // 클릭한 그림 숨기기 및 정보 저장
            hiddenPiece = board[row][col];
            hiddenRow = row;
            hiddenCol = col;
            isVisible[row][col] = false;
        }

        InvalidateRect(hWnd, NULL, FALSE);
        break;
    }
    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);

        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBit = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

        HBRUSH backBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &ps.rcPaint, backBrush);
        DeleteObject(backBrush);

        HDC imgDC = CreateCompatibleDC(hDC);
        SelectObject(imgDC, currentImg);

        int cWidth = wide / puzzleCols;
        int cHeight = height / puzzleRows;
        int imgW = bmpInfo.bmWidth / puzzleCols;
        int imgH = bmpInfo.bmHeight / puzzleRows;

        if (isShowOriginal || !isPlaying) {
            StretchBlt(memDC, 0, 0, wide, height, imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
        }
        else {
            // 퍼즐 타일 그리기
            for (int r = 0; r < puzzleRows; r++) {
                for (int c = 0; c < puzzleCols; c++) {
                    if (gameMode == 0 && (!isVisible[r][c] || (r == emptyRow && c == emptyCol))) continue;

                    // 현재 드래그 중인 타일은 나중에 맨 위에 그리기 위해 스킵
                    if (isDragging && r == movingRow && c == movingCol) continue;

                    int pieceIdx = board[r][c];
                    int srcRow = pieceIdx / puzzleCols;
                    int srcCol = pieceIdx % puzzleCols;

                    StretchBlt(memDC, c * cWidth, r * cHeight, cWidth, cHeight,
                        imgDC, srcCol * imgW, srcRow * imgH, imgW, imgH, SRCCOPY);
                }
            }

            // 드래그 중인 타일은 오프셋을 적용하여 가장 마지막에(맨 위에) 그립니다.
            if (isDragging) {
                int pieceIdx = board[movingRow][movingCol];
                int srcRow = pieceIdx / puzzleCols;
                int srcCol = pieceIdx % puzzleCols;

                int destX = (movingCol * cWidth) + slideOffsetX;
                int destY = (movingRow * cHeight) + slideOffsetY;

                StretchBlt(memDC, destX, destY, cWidth, cHeight,
                    imgDC, srcCol * imgW, srcRow * imgH, imgW, imgH, SRCCOPY);
            }
        }

        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);

        DeleteDC(imgDC);
        SelectObject(memDC, oldBit);
        DeleteObject(hBit);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_SIZE:
        wide = LOWORD(lParam);
        height = HIWORD(lParam);
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_DESTROY:
        DeleteObject(g_hImg1);
        DeleteObject(g_hImg2);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}