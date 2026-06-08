// 실습 7-1: 차일드 윈도우와 컨트롤을 이용하여 연결된 그림 만들기
// 2026년도 1학기 윈도우 프로그래밍

#include <windows.h>
#include "resource.h"

// ──────────────────────────────────────────────
// 컨트롤 ID 정의
// ──────────────────────────────────────────────
#define IDC_CHILD_VIEW    200   // 이미지 표시용 차일드 윈도우
#define IDC_BTN_LEFT      201   // ← 이동 버튼
#define IDC_BTN_RIGHT     202   // → 이동 버튼
#define IDC_LISTBOX       203   // 이미지 리스트 박스
#define IDC_BTN_SELECT    204   // 선택 버튼
#define IDC_BTN_DONE      205   // 완성 버튼
#define IDC_BTN_MOVE      206   // 이동(자동 스크롤) 버튼
#define IDC_BTN_STOP      207   // 멈춤 버튼
#define IDC_EDIT_INFO     208   // 현재 선택 이미지 순서 표시 에디트 박스
#define IDC_TIMER         1001  // 타이머 ID

// ──────────────────────────────────────────────
// 상수 정의
// ──────────────────────────────────────────────
#define MAX_IMAGES        10    // 최대 연결 이미지 수
#define IMG_W             320   // 각 이미지 폭
#define IMG_H             240   // 각 이미지 높이
#define SCROLL_SPEED      4     // 자동 스크롤 픽셀/틱

// ──────────────────────────────────────────────
// 전역 변수
// ──────────────────────────────────────────────
HINSTANCE g_hInst;

// 윈도우 핸들
HWND g_hChildView = NULL;
HWND g_hBtnLeft = NULL;
HWND g_hBtnRight = NULL;
HWND g_hListBox = NULL;
HWND g_hBtnSelect = NULL;
HWND g_hBtnDone = NULL;
HWND g_hBtnMove = NULL;
HWND g_hBtnStop = NULL;
HWND g_hEditInfo = NULL;

// 이미지 관련
HBITMAP g_hBitmaps[10] = { NULL };            // 리소스에서 로드한 비트맵 (IDB_BITMAP1~10)
int     g_imageList[MAX_IMAGES] = { -1 };     // 연결된 이미지 인덱스 배열 (-1 = 비어있음)
int     g_imageCount = 0;                     // 현재 연결된 이미지 수
int     g_scrollOffset = 0;                   // 현재 스크롤 위치 (픽셀)
int     g_scrollDir = 1;                      // 스크롤 방향 (1: 오른쪽, -1: 왼쪽)
BOOL    g_bAutoScroll = FALSE;                // 자동 스크롤 활성 여부
BOOL    g_bDone = FALSE;                      // 완성 여부

// 함수 선언
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildViewProc(HWND, UINT, WPARAM, LPARAM);
void LoadAllBitmaps();
void FreeAllBitmaps();
void UpdateEditInfo();
void DrawImages(HWND hWnd);

// ──────────────────────────────────────────────
// WinMain
// ──────────────────────────────────────────────
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    g_hInst = hInstance;

    // ── 메인 윈도우 클래스 등록
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"ParentClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&wc);

    // ── 차일드 뷰 윈도우 클래스 등록
    wc.lpfnWndProc = ChildViewProc;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"ChildViewClass";
    RegisterClassEx(&wc);

    // ── 메인 윈도우 생성 (1300 x 700)
    HWND hWnd = CreateWindow(L"ParentClass", L"실습 7-1: 연결된 그림 만들기s",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1300, 700,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

// ──────────────────────────────────────────────
// 비트맵 로드 / 해제
// ──────────────────────────────────────────────
void LoadAllBitmaps()
{
    int resIds[10] = {
        IDB_BITMAP1, IDB_BITMAP2, IDB_BITMAP3, IDB_BITMAP4, IDB_BITMAP5,
        IDB_BITMAP6, IDB_BITMAP7, IDB_BITMAP8, IDB_BITMAP9, IDB_BITMAP10
    };
    for (int i = 0; i < 10; i++)
        g_hBitmaps[i] = LoadBitmap(g_hInst, MAKEINTRESOURCE(resIds[i]));
}

void FreeAllBitmaps()
{
    for (int i = 0; i < 10; i++)
        if (g_hBitmaps[i]) { DeleteObject(g_hBitmaps[i]); g_hBitmaps[i] = NULL; }
}

// ──────────────────────────────────────────────
// 에디트 박스 업데이트
// ──────────────────────────────────────────────
void UpdateEditInfo()
{
    if (!g_hEditInfo) return;

    if (g_imageCount == 0)
    {
        SetWindowText(g_hEditInfo, L"(없음)");
        return;
    }

    // 💡 버그 수정: 정확히 화면 중앙에 위치한 이미지를 인식하도록 보정
    int cur = (g_scrollOffset + (IMG_W / 2)) / IMG_W;
    if (cur >= g_imageCount) cur = g_imageCount - 1;
    if (cur < 0) cur = 0;

    WCHAR buf[64];
    wsprintf(buf, L"Image%02d  ( %d / %d )", g_imageList[cur] + 1, cur + 1, g_imageCount);
    SetWindowText(g_hEditInfo, buf);
}

// ──────────────────────────────────────────────
// 차일드 뷰 그리기 (더블 버퍼링)
// ──────────────────────────────────────────────
void DrawImages(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    RECT rc;
    GetClientRect(hWnd, &rc);
    int viewW = rc.right;
    int viewH = rc.bottom;

    HDC     memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, viewW, viewH);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    HBRUSH bgBrush = CreateSolidBrush(RGB(40, 40, 40));
    FillRect(memDC, &rc, bgBrush);
    DeleteObject(bgBrush);

    for (int i = 0; i < g_imageCount; i++)
    {
        int imgIdx = g_imageList[i];
        if (imgIdx < 0 || imgIdx > 9) continue;

        // 💡 여백(10px)을 주어 차일드 윈도우 정중앙에 예쁘게 배치
        int drawX = i * IMG_W - g_scrollOffset + 10;
        int drawY = (viewH - IMG_H) / 2;

        HBITMAP hBit = g_hBitmaps[imgIdx];

        if (hBit) {
            // 실제 비트맵 이미지가 로드된 경우
            HDC imgDC = CreateCompatibleDC(hdc);
            HBITMAP oldImg = (HBITMAP)SelectObject(imgDC, hBit);
            BITMAP bm;
            GetObject(hBit, sizeof(BITMAP), &bm);
            StretchBlt(memDC, drawX, drawY, IMG_W, IMG_H, imgDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            SelectObject(imgDC, oldImg);
            DeleteDC(imgDC);
        }
        else {
            // 💡 리소스(.bmp) 등록을 잊었을 경우를 대비한 대체 렌더링 (안전 장치)
            HBRUSH fbBrush = CreateSolidBrush(RGB(100 + imgIdx * 15, 150, 250 - imgIdx * 15));
            RECT fbRect = { drawX, drawY, drawX + IMG_W, drawY + IMG_H };
            FillRect(memDC, &fbRect, fbBrush);
            DeleteObject(fbBrush);

            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            WCHAR buf[32];
            wsprintf(buf, L"Image %02d (No Bitmap)", imgIdx + 1);
            DrawText(memDC, buf, -1, &fbRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    // 선택 강조 테두리
    if (g_imageCount > 0)
    {
        int cur = (g_scrollOffset + (IMG_W / 2)) / IMG_W;
        if (cur >= g_imageCount) cur = g_imageCount - 1;

        int drawX = cur * IMG_W - g_scrollOffset + 10;
        int drawY = (viewH - IMG_H) / 2;

        HPEN pen = CreatePen(PS_SOLID, 4, RGB(255, 220, 0));
        HPEN old = (HPEN)SelectObject(memDC, pen);
        HBRUSH nb = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH ob = (HBRUSH)SelectObject(memDC, nb);
        Rectangle(memDC, drawX, drawY, drawX + IMG_W, drawY + IMG_H);
        SelectObject(memDC, old);
        SelectObject(memDC, ob);
        DeleteObject(pen);
    }

    // 완성 텍스트
    if (g_bDone)
    {
        SetBkMode(memDC, TRANSPARENT);
        SetTextColor(memDC, RGB(255, 220, 0));
        HFONT hFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Arial");
        HFONT oldFont = (HFONT)SelectObject(memDC, hFont);
        RECT textRc = { 0, 10, viewW, 50 };
        DrawText(memDC, L"★ 이미지 연결 완성! ★", -1, &textRc, DT_CENTER);
        SelectObject(memDC, oldFont);
        DeleteObject(hFont);
    }

    BitBlt(hdc, 0, 0, viewW, viewH, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(hWnd, &ps);
}

// ──────────────────────────────────────────────
// 차일드 뷰 윈도우 프로시저
// ──────────────────────────────────────────────
LRESULT CALLBACK ChildViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_PAINT:
        DrawImages(hWnd);
        return 0;
    case WM_ERASEBKGND:
        return 1;   // 깜빡임 방지
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// ──────────────────────────────────────────────
// 메인 윈도우 프로시저
// ──────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static WCHAR listLabels[10][16] = {
        L"Image01", L"Image02", L"Image03", L"Image04", L"Image05",
        L"Image06", L"Image07", L"Image08", L"Image09", L"Image10"
    };

    switch (uMsg)
    {
    case WM_CREATE:
    {
        LoadAllBitmaps();

        int viewX = 70, viewY = 50;
        int viewW_ctrl = IMG_W + 20;
        int viewH_ctrl = IMG_H + 20;

        g_hChildView = CreateWindow(L"ChildViewClass", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
            viewX, viewY, viewW_ctrl, viewH_ctrl, hWnd, (HMENU)IDC_CHILD_VIEW, g_hInst, NULL);

        g_hBtnLeft = CreateWindow(L"button", L"←", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            viewX - 60, viewY + viewH_ctrl / 2 - 25, 50, 50, hWnd, (HMENU)IDC_BTN_LEFT, g_hInst, NULL);

        g_hBtnRight = CreateWindow(L"button", L"→", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            viewX + viewW_ctrl + 10, viewY + viewH_ctrl / 2 - 25, 50, 50, hWnd, (HMENU)IDC_BTN_RIGHT, g_hInst, NULL);

        g_hEditInfo = CreateWindow(L"edit", L"(없음)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
            viewX, viewY - 35, viewW_ctrl, 28, hWnd, (HMENU)IDC_EDIT_INFO, g_hInst, NULL);

        int panelX = viewX + viewW_ctrl + 80;
        int panelY = viewY;

        g_hListBox = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD | WS_VSCROLL,
            panelX, panelY, 160, 200, hWnd, (HMENU)IDC_LISTBOX, g_hInst, NULL);

        for (int i = 0; i < 10; i++)
            SendMessage(g_hListBox, LB_ADDSTRING, 0, (LPARAM)listLabels[i]);

        int btnX = panelX;
        int btnY = panelY + 210;
        int btnW = 160, btnH = 40, btnGap = 10;

        g_hBtnSelect = CreateWindow(L"button", L"선택", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            btnX, btnY, btnW, btnH, hWnd, (HMENU)IDC_BTN_SELECT, g_hInst, NULL);

        g_hBtnDone = CreateWindow(L"button", L"완성", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            btnX, btnY + (btnH + btnGap), btnW, btnH, hWnd, (HMENU)IDC_BTN_DONE, g_hInst, NULL);

        g_hBtnMove = CreateWindow(L"button", L"이동", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            btnX, btnY + 2 * (btnH + btnGap), btnW, btnH, hWnd, (HMENU)IDC_BTN_MOVE, g_hInst, NULL);

        g_hBtnStop = CreateWindow(L"button", L"멈춤", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            btnX, btnY + 3 * (btnH + btnGap), btnW, btnH, hWnd, (HMENU)IDC_BTN_STOP, g_hInst, NULL);

        break;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);

        // 💡 버그 수정: 수동 이동(←) 시 이미지가 딱 맞게 멈추도록 Snap 보정
        if (id == IDC_BTN_LEFT)
        {
            if (g_imageCount > 0 && g_scrollOffset > 0)
            {
                g_scrollOffset -= IMG_W;
                if (g_scrollOffset < 0) g_scrollOffset = 0;

                // 중간에 멈췄더라도 정확한 이미지 컷으로 맞춤
                g_scrollOffset = (g_scrollOffset / IMG_W) * IMG_W;
                g_scrollDir = -1; // 방향 기억

                InvalidateRect(g_hChildView, NULL, FALSE);
                UpdateEditInfo();
            }
        }
        // 💡 버그 수정: 수동 이동(→) 시 이미지가 딱 맞게 멈추도록 Snap 보정
        else if (id == IDC_BTN_RIGHT)
        {
            int maxOffset = (g_imageCount - 1) * IMG_W;
            if (g_imageCount > 0 && g_scrollOffset < maxOffset)
            {
                g_scrollOffset += IMG_W;

                g_scrollOffset = ((g_scrollOffset + IMG_W - 1) / IMG_W) * IMG_W;
                if (g_scrollOffset > maxOffset) g_scrollOffset = maxOffset;
                g_scrollDir = 1; // 방향 기억

                InvalidateRect(g_hChildView, NULL, FALSE);
                UpdateEditInfo();
            }
        }
        else if (id == IDC_BTN_SELECT)
        {
            if (g_bDone)
            {
                MessageBox(hWnd, L"이미지 연결이 이미 완성되었습니다.", L"알림", MB_OK);
                break;
            }
            int sel = (int)SendMessage(g_hListBox, LB_GETCURSEL, 0, 0);
            if (sel == LB_ERR)
            {
                MessageBox(hWnd, L"리스트 박스에서 이미지를 선택하세요.", L"알림", MB_OK);
                break;
            }
            if (g_imageCount >= MAX_IMAGES)
            {
                MessageBox(hWnd, L"최대 10개까지 연결할 수 있습니다.", L"알림", MB_OK);
                break;
            }
            g_imageList[g_imageCount] = sel;
            g_imageCount++;

            g_scrollOffset = (g_imageCount - 1) * IMG_W;
            InvalidateRect(g_hChildView, NULL, FALSE);
            UpdateEditInfo();
        }
        else if (id == IDC_BTN_DONE)
        {
            if (g_imageCount == 0)
            {
                MessageBox(hWnd, L"연결된 이미지가 없습니다.", L"알림", MB_OK);
                break;
            }
            g_bDone = TRUE;
            g_bAutoScroll = FALSE;
            KillTimer(hWnd, IDC_TIMER);
            InvalidateRect(g_hChildView, NULL, FALSE);
            MessageBox(hWnd, L"이미지 연결을 완성했습니다!", L"완성", MB_OK | MB_ICONINFORMATION);
        }
        else if (id == IDC_BTN_MOVE)
        {
            if (g_imageCount < 2)
            {
                MessageBox(hWnd, L"2개 이상의 이미지가 연결되어야 이동할 수 있습니다.", L"알림", MB_OK);
                break;
            }
            g_bAutoScroll = TRUE;
            SetTimer(hWnd, IDC_TIMER, 30, NULL);
        }
        else if (id == IDC_BTN_STOP)
        {
            g_bAutoScroll = FALSE;
            KillTimer(hWnd, IDC_TIMER);
        }

        break;
    }

    case WM_TIMER:
    {
        if (wParam == IDC_TIMER && g_bAutoScroll && g_imageCount > 1)
        {
            int maxOffset = (g_imageCount - 1) * IMG_W;

            // 💡 버그 수정: 자연스러운 Ping-Pong 자동 스크롤
            g_scrollOffset += (SCROLL_SPEED * g_scrollDir);

            if (g_scrollOffset >= maxOffset) {
                g_scrollOffset = maxOffset;
                g_scrollDir = -1; // 끝에 도달하면 방향 전환
            }
            else if (g_scrollOffset <= 0) {
                g_scrollOffset = 0;
                g_scrollDir = 1;  // 처음으로 돌아오면 다시 오른쪽으로
            }

            InvalidateRect(g_hChildView, NULL, FALSE);
            UpdateWindow(g_hChildView);
            UpdateEditInfo();
        }
        break;
    }

    case WM_DESTROY:
        KillTimer(hWnd, IDC_TIMER);
        FreeAllBitmaps();
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}