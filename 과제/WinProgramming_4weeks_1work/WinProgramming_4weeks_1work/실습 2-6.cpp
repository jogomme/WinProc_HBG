#define _CRT_SECURE_NO_WARNINGS

#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <random>
#include <string>

using namespace std;

// --- 1. 데이터 저장을 위한 구조체 ---
struct RectData {
	int x, y, n, count;
	COLORREF textColor;
	COLORREF bkColor;
	bool active = false;
};

static RectData history[10]; 
static int historyCount = 0;
static bool showAll = false;  

// 난수 생성기
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);

int wide{ 800 };
int height{ 600 };



HINSTANCE g_hInst;

LPCTSTR IpszClass = L"My Window Class 3";
LPCTSTR IpszWindowName = L"Window Programming 3"; // 타이틀

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






LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static TCHAR Nums[100] = { 0 };
    static int Count = 0;
    static bool isInputError = false;
    static SIZE size;
    PAINTSTRUCT ps;
    HDC hDC;

    switch (uMsg) {
    case WM_CREATE:
        CreateCaret(hWnd, NULL, 5, 15);
        ShowCaret(hWnd);
        break;

    case WM_CHAR:
        if (wParam == 'q' || wParam == 'Q') PostQuitMessage(0);

        else if (wParam == 'r' || wParam == 'R') {
            for (int i = 0; i < 10; i++) history[i].active = false;
            historyCount = 0;
        }

        else if (wParam == 'a' || wParam == 'A') {
            showAll = !showAll;
        }

        else if (wParam == VK_BACK) {
            if (Count > 0) { Nums[--Count] = '\0'; }
        }

        else if (wParam == VK_RETURN) {
            int tx, ty, tn, tc;
            int result = swscanf(Nums, L"%d %d %d %d", &tx, &ty, &tn, &tc);

            if (result == 4 && tx <= 600 && ty <= 400 && tc >= 5 && tc <= 20) {
                int idx = historyCount % 10;
                history[idx].x = tx;
                history[idx].y = ty;
                history[idx].n = tn;
                history[idx].count = tc;
                history[idx].textColor = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
                history[idx].bkColor = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
                history[idx].active = true;

                historyCount++;
                Count = 0;
                memset(Nums, 0, sizeof(Nums));
                isInputError = false;
            }
            else {
                isInputError = true;
                Count = 0;
                memset(Nums, 0, sizeof(Nums));
            }
        }
        else {
            if (Count < 99) {
                Nums[Count++] = wParam;
                Nums[Count] = '\0';
            }
        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);

        for (int h = 0; h < 10; h++) {

            TextOut(hDC, 0, 0, L"x y n count를 입력하세요:", lstrlen(L"x y n count를 입력하세요:"));
            TextOut(hDC, 210, 0, Nums, lstrlen(Nums));

            if (isInputError) {
                SetTextColor(hDC, RGB(255, 0, 0));
                TextOut(hDC, 0, 20, L"입력 오류! 다시 입력하세요.", lstrlen(L"입력 오류! 다시 입력하세요."));
            }

            // 1. 데이터가 비어있으면 패스
            if (!history[h].active) continue;

            // 2. 'a'를 안 눌렀을 때(showAll == false)의 필터링
            if (showAll == false) {
                // 가장 최근에 입력된 인덱스 계산 (10개 단위 순환)
                int lastIdx = (historyCount - 1) % 10;

                // 만약 지금 그리는 h가 마지막 인덱스가 아니면 그냥 다음으로 넘어감(출력 안 함)
                if (h != lastIdx) continue;
            }

            // --- 여기서부터 실제 그리기 (조건을 통과한 것만 그려짐) ---
            SetTextColor(hDC, history[h].textColor);
            SetBkColor(hDC, history[h].bkColor);
            SetBkMode(hDC, OPAQUE);

            TCHAR outStr[10];
            wsprintf(outStr, L"%d", history[h].n);

            for (int row = 0; row < history[h].count; row++) {
                for (int col = 0; col < history[h].count; col++) {
                    TextOut(hDC, history[h].x + (col * 20), history[h].y + (row * 20), outStr, lstrlen(outStr));
                }
            }
        }

        GetTextExtentPoint32(hDC, Nums, lstrlen(Nums), &size);
        SetCaretPos(210 + size.cx, 0);
        
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}