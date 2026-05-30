#define _CRT_SECURE_NO_WARNINGS

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <random>
#include <string>
#include<iostream>

using namespace std;

// 난수 생성기
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);

int wide{ 800 };
int height{ 600 };



HINSTANCE g_hInst;

LPCTSTR IpszClass = L"My Window Class 3";
LPCTSTR IpszWindowName = L"메모장"; // 타이틀

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
// -----------------------------------------------------------------------------------------------
//  전역 변수 선언 구간
// -----------------------------------------------------------------------------------------------
RECT area[5];
POINT point[5];
COLORREF shapeColors[5];
COLORREF tempColor;

int shapeType[5] = { 0, 1, 2, 3, 4 };

bool SFlag = false;
bool CFlag = false;
bool PFlag = false;
bool EFlag = false;


// -----------------------------------------------------------------------------------------------
// Draw함수 선언 구간
// -----------------------------------------------------------------------------------------------
void DrawPentagon(HDC hDC, POINT center, RECT box)
{
    POINT pts[5];

    // 1. 구역(box)의 높이를 절반으로 나눠서, 이 오각형이 가질 수 있는 최대 반지름(r)을 구합니다.
    // (선 밖으로 튀어나가지 않게 여백을 5픽셀 정도 빼줍니다.)
    int r = (box.bottom - box.top) / 2 - 5;

    // 2. 중심점과 반지름(r)을 곱해서 5개 꼭짓점을 계산합니다. 
    // (0.951, 0.588 등의 숫자는 정오각형을 그리기 위한 불변의 수학적 황금 비율입니다!)
    pts[0] = { center.x,                       center.y - r };                       // 1. 위쪽 (12시 방향)
    pts[1] = { center.x + (int)(r * 0.951),    center.y - (int)(r * 0.309) };        // 2. 오른쪽 위
    pts[2] = { center.x + (int)(r * 0.588),    center.y + (int)(r * 0.809) };        // 3. 오른쪽 아래
    pts[3] = { center.x - (int)(r * 0.588),    center.y + (int)(r * 0.809) };        // 4. 왼쪽 아래
    pts[4] = { center.x - (int)(r * 0.951),    center.y - (int)(r * 0.309) };        // 5. 왼쪽 위

    Polygon(hDC, pts, 5); // 계산된 5개의 점을 그려서 색칠합니다.

}

void DrawReversPentagon(HDC hDC, POINT center, RECT box)
{
    POINT pts[5];

    // 1. 구역(box)의 높이를 바탕으로 최대 반지름(r)을 구합니다.
    int r = (box.bottom - box.top) / 2 - 5;

    // 2. 중심점과 반지름(r)을 곱해서 5개 꼭짓점을 계산합니다.
    // 정오각형 코드와 비교해 보세요! center.y 뒤의 +와 -가 반대로 바뀌었습니다.
    pts[0] = { center.x,                       center.y + r };                       // 1. 아래쪽 끝 (6시 방향)
    pts[1] = { center.x + (int)(r * 0.951),    center.y + (int)(r * 0.309) };        // 2. 오른쪽 아래
    pts[2] = { center.x + (int)(r * 0.588),    center.y - (int)(r * 0.809) };        // 3. 오른쪽 위
    pts[3] = { center.x - (int)(r * 0.588),    center.y - (int)(r * 0.809) };        // 4. 왼쪽 위
    pts[4] = { center.x - (int)(r * 0.951),    center.y + (int)(r * 0.309) };        // 5. 왼쪽 아래

    // 3. 계산된 5개의 점을 잇고 현재 장착된 붓으로 색칠합니다.
    Polygon(hDC, pts, 5);

}

void DrawTimer(HDC hDC, RECT box) 
{
    POINT ptr[4];

    ptr[0] = { box.left + 10, box.top + 10 };
    ptr[1] = { box.right - 10, box.bottom - 10 };
    ptr[2] = { box.left + 10, box.bottom - 10 };
    ptr[3] = { box.right - 10, box.top + 10 };

    Polygon(hDC, ptr, 4);

}

void DrawRightTimer(HDC hDC, RECT box)
{
    POINT ptr[4];

    ptr[0] = { box.left + 10, box.top + 10};
    ptr[1] = { box.right - 10, box.bottom - 10 };
    ptr[2] = { box.right - 10, box.top + 10 };
    ptr[3] = { box.left + 10, box.bottom - 10 };

    Polygon(hDC, ptr, 4);

}

// -----------------------------------------------------------------------------------------------
// 콜 백 함수 선언 구간
// -----------------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    int size{};

    static POINT left{ wide / 3, height / 2 };
    static POINT top{ wide / 2, height / 3 };
    static POINT right{ wide / 3 * 2, height / 2 };
    static POINT bottom{ wide / 2, height / 3 * 2 };

    HBRUSH hBrush, oldBrush;

    switch (uMsg) {
    case WM_CREATE :
        
        for (int i = 0; i < 5; ++i) {
            shapeColors[i] = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
        }

        break;
    case WM_KEYDOWN :
        if (wParam == 'Q') {
            PostQuitMessage(0);
        }
        else if (wParam == 'S') {
            SFlag = true; 
            tempColor = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
            InvalidateRect(hWnd, NULL, TRUE); 
        }
        else if(wParam == 'C') {
            CFlag = true; 
            tempColor = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
            InvalidateRect(hWnd, NULL, TRUE); 
        }
        else if (wParam == 'P') {
            PFlag = true; 
            tempColor = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
            InvalidateRect(hWnd, NULL, TRUE); 
        }
        else if (wParam == 'E') {
            EFlag = true; 
            tempColor = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
            InvalidateRect(hWnd, NULL, TRUE); 
        }
        else if (wParam == VK_RIGHT) {
            int temp = shapeType[4]; // 4번 명찰 빼두기
            for (int i = 3; i >= 1; --i) { // 3->4, 2->3, 1->2 로 한 칸씩 당기기
                shapeType[i + 1] = shapeType[i];
            }
            shapeType[1] = temp; // 1번 자리에 아까 빼둔 4번 명찰 넣기
            shapeType[0] = shapeType[2];
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == VK_LEFT) {
            int temp = shapeType[1]; // 1번 명찰 빼두기
            for (int i = 1; i < 4; ++i) { // 2->1, 3->2, 4->3 으로 한 칸씩 당기기
                shapeType[i] = shapeType[i + 1];
            }
            shapeType[4] = temp; // 4번 자리에 아까 빼둔 1번 명찰 넣기
            shapeType[0] = shapeType[2];
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == VK_UP) {
            // 2번(위) 자리와 4번(아래) 자리의 명찰을 스왑!
            swap(shapeType[2], shapeType[4]);
            shapeType[0] = shapeType[2];
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == VK_DOWN) {
            // 1번(왼쪽) 자리와 3번(오른쪽) 자리의 명찰을 스왑!
            swap(shapeType[1], shapeType[3]);
            shapeType[0] = shapeType[2];
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    case WM_KEYUP :
        if (wParam == 'S') {
            SFlag = false;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == 'C') {
            CFlag = false;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == 'P') {
            PFlag = false;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == 'E') {
            EFlag = false;
            InvalidateRect(hWnd, NULL, TRUE);
        }

        break;
    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps); // DC 얻어오기
       // 윈도우가 제공하는 객체 가져오기
       // hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH); 
       // oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

        Rectangle(hDC, area[0].left, area[0].top, area[0].right, area[0].bottom);


        for (int i = 0; i < 5; ++i) {

            int type = shapeType[i];

            COLORREF currentColor = shapeColors[type];

            if (type == 1 && CFlag) currentColor = tempColor;
            if (type == 2 && SFlag) currentColor = tempColor;
            if (type == 3 && PFlag) currentColor = tempColor;
            if (type == 4 && EFlag) currentColor = tempColor;

            HBRUSH myBrush = CreateSolidBrush(currentColor);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, myBrush);

            if (type == 1) {
                if (CFlag) {
                    Ellipse(hDC, area[0].left, area[0].top + 10, area[0].right, area[0].bottom - 10);
                }
                Ellipse(hDC, area[i].left + 10, area[i].top + 10, area[i].right - 10, area[i].bottom - 10);
            }
            else if (type == 2) {
                if (SFlag) {
                    DrawRightTimer(hDC, area[0]);
                }
                DrawTimer(hDC, area[i]);
            }
            else if (type == 3) {
                if (PFlag) {
                    DrawReversPentagon(hDC, point[0], area[0]);
                }
                DrawPentagon(hDC, point[i], area[i]);
            }
            else if ( type == 4) {
                if (EFlag) {
                    Pie(hDC, area[0].left + 10, area[0].top + 10, area[0].right - 10, area[0].bottom - 10, area[0].right - 10, point[0].y, point[0].x, area[0].top+10);
                }
                Pie(hDC, area[i].left + 10, area[i].top + 10, area[i].right - 10, area[i].bottom - 10, point[i].x, area[i].top + 10, area[i].right - 10, point[i].y);
            }
            SelectObject(hDC, oldBrush); // 제자리 돌아가기
            DeleteObject(myBrush);
        }

        // DC 해제하기
        EndPaint(hWnd, &ps); return 0;

    case WM_SIZE :
        height = HIWORD(lParam);
        wide = LOWORD(lParam);

        size = min(wide, height) / 10;
        
        point[0] = { wide / 2, height / 2 };
        point[1] = { point[0].x - size * 2, point[0].y };
        point[2] = { point[0].x, point[0].y - size * 2 };
        point[3] = { point[0].x + size * 2, point[0].y };
        point[4] = { point[0].x, point[0].y + size * 2 };

        for (int i{}; i < 5; ++i) {
            area[i] = {
                point[i].x - size,
                point[i].y - size,
                point[i].x + size,
                point[i].y + size
            };
        }

        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}