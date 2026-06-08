#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>

// ──────────────────────────────────────────────
// 컨트롤 ID 및 상수 정의
// ──────────────────────────────────────────────
#define IDC_MAP_VIEW      200
#define IDC_BTN_LEFT      201
#define IDC_BTN_RIGHT     202
#define IDC_BTN_TILE1     211
#define IDC_BTN_TILE2     212
#define IDC_BTN_TILE3     213
#define IDC_BTN_TILE4     214
#define IDC_BTN_TILE5     215
#define IDC_BTN_TILE6     216
#define IDC_BTN_ERASER    217
#define IDC_LIST_BG       218
#define IDC_BTN_TEST      219

#define CELL_SIZE   40   // 격자 1칸의 픽셀 크기
#define GRID_COLS   15   // 가로 칸 수
#define GRID_ROWS   15   // 세로 칸 수
#define MAX_MAPS    3    // 최대 맵 개수

// ──────────────────────────────────────────────
// 전역 변수
// ──────────────────────────────────────────────
HINSTANCE g_hInst;
HWND g_hMapView;

// 맵 데이터: 3개의 맵, 15x15 격자. (0: 빈칸, 1~6: 타일)
int g_mapData[MAX_MAPS][GRID_ROWS][GRID_COLS] = { 0 };
int g_currentMap = 0;      // 현재 보고 있는 맵 인덱스 (0, 1, 2)
int g_selectedTile = 1;    // 현재 마우스로 찍을 타일 번호 (1~6, 0은 지우개)

// 배경 및 모드 상태
int g_bgIndex[MAX_MAPS] = { 0 }; // 각 맵의 배경 인덱스
BOOL g_isTestMode = FALSE;       // 테스트 모드 여부

// 플레이어(테스트 모드용) 위치
int g_playerX = 300, g_playerY = 300;

// 함수 선언
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MapViewProc(HWND, UINT, WPARAM, LPARAM);
void DrawMap(HWND hWnd);

// ──────────────────────────────────────────────
// WinMain
// ──────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInst = hInstance;

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0,
                       hInstance, LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
                       (HBRUSH)(COLOR_BTNFACE + 1), NULL, L"MainClass", LoadIcon(NULL, IDI_APPLICATION) };
    RegisterClassExW(&wc);

    wc.lpfnWndProc = MapViewProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = L"MapViewClass";
    RegisterClassExW(&wc);

    HWND hWnd = CreateWindowW(L"MainClass", L"맵 툴 만들기 (15x15)", WS_OVERLAPPEDWINDOW,
        100, 100, 1000, 750, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

// ──────────────────────────────────────────────
// 맵 그리기 (더블 버퍼링 적용)
// ──────────────────────────────────────────────
void DrawMap(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    int viewW = GRID_COLS * CELL_SIZE; // 15 * 40 = 600
    int viewH = GRID_ROWS * CELL_SIZE; // 15 * 40 = 600

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBit = CreateCompatibleBitmap(hdc, viewW, viewH);
    HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

    // 1. 배경 그리기 (하늘색 등)
    HBRUSH bgBrush = CreateSolidBrush(RGB(200, 240, 255));
    RECT bgRect = { 0, 0, viewW, viewH };
    FillRect(memDC, &bgRect, bgBrush);
    DeleteObject(bgBrush);

    // 2. 격자 그리기 (테스트 모드가 아닐 때만)
    if (!g_isTestMode) {
        HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
        HPEN oldPen = (HPEN)SelectObject(memDC, gridPen);
        for (int i = 0; i <= GRID_COLS; i++) {
            MoveToEx(memDC, i * CELL_SIZE, 0, NULL); LineTo(memDC, i * CELL_SIZE, viewH);
            MoveToEx(memDC, 0, i * CELL_SIZE, NULL); LineTo(memDC, viewW, i * CELL_SIZE);
        }
        SelectObject(memDC, oldPen);
        DeleteObject(gridPen);
    }

    // 3. 맵 배열에 배치된 타일(점프 발판) 그리기
    for (int y = 0; y < GRID_ROWS; y++) {
        for (int x = 0; x < GRID_COLS; x++) {
            int tile = g_mapData[g_currentMap][y][x];
            if (tile > 0) {
                // 타일 종류별로 다른 색상 사각형 그리기 (여기에 비트맵 BitBlt을 적용하면 됩니다)
                HBRUSH tileBrush = CreateSolidBrush(RGB(50 * tile, 200 - (tile * 20), 50));
                RECT tRect = { x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE };
                FillRect(memDC, &tRect, tileBrush);
                DeleteObject(tileBrush);

                // 타일 테두리
                FrameRect(memDC, &tRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
            }
        }
    }

    // 4. 테스트 모드: 캐릭터 그리기
    if (g_isTestMode) {
        HBRUSH playerBrush = CreateSolidBrush(RGB(255, 0, 0)); // 빨간색 캐릭터
        RECT pRect = { g_playerX, g_playerY, g_playerX + 20, g_playerY + 30 };
        FillRect(memDC, &pRect, playerBrush);
        DeleteObject(playerBrush);
    }

    BitBlt(hdc, 0, 0, viewW, viewH, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBit);
    DeleteObject(hBit);
    DeleteDC(memDC);
    EndPaint(hWnd, &ps);
}

// ──────────────────────────────────────────────
// 차일드 뷰 (맵 캔버스) 프로시저
// ──────────────────────────────────────────────
LRESULT CALLBACK MapViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL bLButtonDown = FALSE;

    switch (uMsg) {
    case WM_LBUTTONDOWN:
        if (!g_isTestMode) {
            bLButtonDown = TRUE;
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = HIWORD(lParam) / CELL_SIZE;
            if (x >= 0 && x < GRID_COLS && y >= 0 && y < GRID_ROWS) {
                g_mapData[g_currentMap][y][x] = g_selectedTile;
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;
    case WM_MOUSEMOVE:
        if (bLButtonDown && !g_isTestMode) {
            int x = LOWORD(lParam) / CELL_SIZE;
            int y = HIWORD(lParam) / CELL_SIZE;
            if (x >= 0 && x < GRID_COLS && y >= 0 && y < GRID_ROWS) {
                g_mapData[g_currentMap][y][x] = g_selectedTile;
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;
    case WM_LBUTTONUP:
        bLButtonDown = FALSE;
        break;
    case WM_PAINT:
        DrawMap(hWnd);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// ──────────────────────────────────────────────
// 메인 윈도우 프로시저 (UI 컨트롤 패널)
// ──────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
    {
        // 1. 맵 뷰 (600x600) 생성
        g_hMapView = CreateWindowW(L"MapViewClass", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
            80, 40, GRID_COLS * CELL_SIZE, GRID_ROWS * CELL_SIZE,
            hWnd, (HMENU)IDC_MAP_VIEW, g_hInst, NULL);

        // 2. 맵 전환 화살표 버튼
        CreateWindowW(L"button", L"◀", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 300, 50, 50, hWnd, (HMENU)IDC_BTN_LEFT, g_hInst, NULL);
        CreateWindowW(L"button", L"▶", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 690, 300, 50, 50, hWnd, (HMENU)IDC_BTN_RIGHT, g_hInst, NULL);

        // 3. 우측 타일 선택 버튼들 (1~6번 타일)
        int btnX = 760, btnY = 40;
        CreateWindowW(L"button", L"타일 1", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX, btnY, 60, 60, hWnd, (HMENU)IDC_BTN_TILE1, g_hInst, NULL);
        CreateWindowW(L"button", L"타일 2", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX + 70, btnY, 60, 60, hWnd, (HMENU)IDC_BTN_TILE2, g_hInst, NULL);
        CreateWindowW(L"button", L"타일 3", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX + 140, btnY, 60, 60, hWnd, (HMENU)IDC_BTN_TILE3, g_hInst, NULL);

        btnY += 70;
        CreateWindowW(L"button", L"타일 4", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX, btnY, 60, 60, hWnd, (HMENU)IDC_BTN_TILE4, g_hInst, NULL);
        CreateWindowW(L"button", L"타일 5", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX + 70, btnY, 60, 60, hWnd, (HMENU)IDC_BTN_TILE5, g_hInst, NULL);
        CreateWindowW(L"button", L"타일 6", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX + 140, btnY, 60, 60, hWnd, (HMENU)IDC_BTN_TILE6, g_hInst, NULL);

        // 지우개(취소) 버튼
        CreateWindowW(L"button", L"지우개", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX, btnY + 70, 200, 40, hWnd, (HMENU)IDC_BTN_ERASER, g_hInst, NULL);

        // 배경 리스트 박스
        HWND hList = CreateWindowW(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD,
            btnX, btnY + 130, 200, 150, hWnd, (HMENU)IDC_LIST_BG, g_hInst, NULL);
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)L"배경 1 (숲)");
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)L"배경 2 (동굴)");
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)L"배경 3 (성)");

        // 테스트 버튼
        CreateWindowW(L"button", L"▶ 테스트 시작", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            btnX, btnY + 300, 200, 60, hWnd, (HMENU)IDC_BTN_TEST, g_hInst, NULL);
        break;
    }
    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        // 타일 및 지우개 선택
        if (id >= IDC_BTN_TILE1 && id <= IDC_BTN_TILE6) {
            g_selectedTile = id - IDC_BTN_TILE1 + 1;
        }
        else if (id == IDC_BTN_ERASER) {
            g_selectedTile = 0; // 지우개는 배열 값을 0으로 만듦
        }
        // 맵 전환 (좌우 버튼)
        else if (id == IDC_BTN_LEFT) {
            if (g_currentMap > 0) g_currentMap--;
            InvalidateRect(g_hMapView, NULL, FALSE);
        }
        else if (id == IDC_BTN_RIGHT) {
            if (g_currentMap < MAX_MAPS - 1) g_currentMap++;
            InvalidateRect(g_hMapView, NULL, FALSE);
        }
        // 테스트 모드 토글
        else if (id == IDC_BTN_TEST) {
            g_isTestMode = !g_isTestMode;
            if (g_isTestMode) {
                SetWindowTextW((HWND)lParam, L"■ 테스트 종료");
                g_playerX = 300; g_playerY = 300; // 캐릭터 위치 초기화
                SetFocus(hWnd); // 방향키 입력을 위해 메인 창으로 포커스
            }
            else {
                SetWindowTextW((HWND)lParam, L"▶ 테스트 시작");
            }
            InvalidateRect(g_hMapView, NULL, FALSE);
        }
        break;
    }
    // 테스트 모드용 캐릭터 이동 (A, D 키 또는 화살표)
    case WM_KEYDOWN:
        if (g_isTestMode) {
            if (wParam == VK_LEFT)  g_playerX -= 10;
            if (wParam == VK_RIGHT) g_playerX += 10;
            if (wParam == VK_UP)    g_playerY -= 10;
            if (wParam == VK_DOWN)  g_playerY += 10;
            InvalidateRect(g_hMapView, NULL, FALSE);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}