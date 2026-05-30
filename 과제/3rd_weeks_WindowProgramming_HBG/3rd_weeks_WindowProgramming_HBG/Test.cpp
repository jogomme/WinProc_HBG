#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>

HINSTANCE g_hInst;


LPCTSTR IpszClass = L"My Window Class 3";
LPCTSTR IpszWindowName = L"Window Programming 3"; // 타이틀

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int g_yPos{};
int g_xPos{};

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
	WndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH); // 배경 색깔
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = IpszClass; // 윈도우 클래스 이름 정하기
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	

	RegisterClassEx(&WndClass);

	

	// 윈도우 특성 정하기, ex) 스크롤 바
	hWnd = CreateWindow(IpszClass, IpszWindowName, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL | WS_THICKFRAME , 300, 50, 800, 500, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	SetScrollRange(hWnd, SB_VERT, 0, 800-500, TRUE);

	while (GetMessage(&Message, 0, 0, 0)) { // 메시지 반복
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	
	// 마우스 휠을 인식하는 것을 만들기 위해 전역으로 재선언
	// 스크롤 위치를 저장할 변수
	//static int g_yPos{};
	//static int g_xPos{};



	// --메시지 처리하기

	switch (uMsg) {
		case WM_CREATE:
			break;
		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
			
		case WM_VSCROLL: // 세로 스크롤 바 메시지
			switch (LOWORD(wParam)) {
			case SB_LINEUP:    g_yPos -= 10; break; // 위쪽 화살표 클릭
			case SB_LINEDOWN:  g_yPos += 10; break; // 아래쪽 화살표 클릭
			case SB_PAGEUP:    g_yPos -= 1; break; // 위쪽 몸통 클릭
			case SB_PAGEDOWN:  g_yPos += 1; break; // 아래쪽 몸통 클릭
			case SB_THUMBTRACK: g_yPos = HIWORD(wParam); break; // 드래그 중
			}
			// 스크롤 바의 위치를 화면에 업데이트
			SetScrollPos(hWnd, SB_VERT, g_yPos, TRUE);
			InvalidateRect(hWnd, NULL, TRUE); // 화면 다시 그리기 요청
			break;

		case WM_HSCROLL:	// 가로 스크롤바
			switch (LOWORD(wParam)) {
			case SB_LINELEFT: g_xPos -= 10; break;
			case SB_LINERIGHT: g_xPos += 10; break;
			case SB_PAGERIGHT:   g_xPos += 1; break; // 위쪽 몸통 클릭
			case SB_PAGELEFT:  g_xPos -= 1; break; // 아래쪽 몸통 클릭
			case SB_THUMBTRACK: g_xPos = HIWORD(wParam); break;

			}
			SetScrollPos(hWnd, SB_HORZ,g_xPos, TRUE );
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		
		case WM_MOUSEWHEEL: {// 마우스 휠 이래용
			// 마우스 휠을 돌릴 때마다 +- 120이라는 값을 내보내고 이른 WHEEL_DELTA라고 한다.

			//short zDelta = (short)HIWORD(wParam);
			// HIWORD는 32비트중 앞 16비트를 땡겨오는데 이는 unsigned 로 땡겨오기 때문에 int로 하면 굉장히 큰 양수가 됌

			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			// 이것은 매크로 치환으로 int형태로 해도 short로 치환해서 값을 준다.

			if (zDelta > 0) {
				// 휠을 위로 돌림 -> 위로 스크롤 (yPos 감소)
				g_yPos -= 10;
				if (g_yPos < 0) {
					g_yPos = 0;
				}
			}
			else {
				// 휠을 아래로 돌림 -> 아래로 스크롤 (yPos 증가)
				g_yPos += 10;
				if (g_yPos > 300) {
					g_yPos = 300;
				}
			}

			SetScrollPos(hWnd, SB_VERT, g_yPos, TRUE);
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}