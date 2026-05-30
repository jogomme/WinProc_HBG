#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <random>    

using namespace std;
int wide{ 800 };
int height{ 600 };

// --------------------------------------------------------
// 고품질 난수 생성기 세팅 (유지)
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_int_distribution<int> numDist(1, 99);
uniform_int_distribution<int> xDist(50, wide - 50);
uniform_int_distribution<int> yDist(50, height - 50);



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
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = IpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(IpszClass, IpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, wide, height, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return Message.wParam;
}

// --------------------------------------------------------
// 도형 구조체 및 고정 배열 (vector 제거)
// --------------------------------------------------------
struct Shape {
    int region;
    int type;
    int x, y;
    COLORREF color;
    int number;
};

// 최대 30개를 담는 일반 C배열입니다.
static Shape shapes[30];
static const int MAX_SHAPES = 30;

RECT rectOuter = { wide - 600 , 100, wide - 200, 500 };
RECT rectInner = { wide - 350 , 200, wide-450, 400 };

// --------------------------------------------------------
// 구역별 색상 및 숫자 업데이트 함수
// --------------------------------------------------------
void UpdateShapes(int targetRegion) {
    for (int i = 0; i < MAX_SHAPES; ++i) {
        if (targetRegion == 0 || shapes[i].region == targetRegion) {
            // C++ random 분배기를 사용하여 다시 난수를 뽑습니다.
            shapes[i].color = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
            shapes[i].number = numDist(gen);
            bool check = true;
            while (check) {
                shapes[i].x = xDist(gen);
                shapes[i].y = yDist(gen);

                int r = 20; // 도형의 반지름(여백) 크기 설정

                // 1. 안쪽 사각형 안에 "완전히" 들어가는가? (반지름 r만큼 선에서 안쪽으로 들어왔는지 확인)
                bool inInner = (shapes[i].x - r > rectInner.left && shapes[i].x + r < rectInner.right &&
                    shapes[i].y - r > rectInner.top && shapes[i].y + r < rectInner.bottom);

                // 2. 바깥쪽 사각형 안에 "완전히" 들어가는가?
                bool inOuter = (shapes[i].x - r > rectOuter.left && shapes[i].x + r < rectOuter.right &&
                    shapes[i].y - r > rectOuter.top && shapes[i].y + r < rectOuter.bottom);

                // 3. 안쪽 사각형 밖으로 "완전히" 벗어났는가? (선에 몸통이 걸치지 않았는지 확인)
                bool outOfInner = (shapes[i].x + r <= rectInner.left || shapes[i].x - r >= rectInner.right ||
                    shapes[i].y + r <= rectInner.top || shapes[i].y - r >= rectInner.bottom);

                // 4. 바깥쪽 사각형 밖으로 "완전히" 벗어났는가?
                bool outOfOuter = (shapes[i].x + r <= rectOuter.left || shapes[i].x - r >= rectOuter.right ||
                    shapes[i].y + r <= rectOuter.top || shapes[i].y - r >= rectOuter.bottom);

                if (inInner && shapes[i].region == 3) {
                    check = false;
                }
                else if (inOuter && outOfInner && shapes[i].region == 2) {
                    check = false;
                }
                else if (outOfOuter && shapes[i].region == 1) {
                    check = false;
                }

            }
        }
    }
}

void ReCreatPol() {
    for (int i = 0; i < MAX_SHAPES; ++i) {
            // C++ random 분배기를 사용하여 다시 난수를 뽑습니다.
            bool check = true;

                int r = 20; // 도형의 반지름(여백) 크기 설정

                // 1. 안쪽 사각형 안에 "완전히" 들어가는가? (반지름 r만큼 선에서 안쪽으로 들어왔는지 확인)
                bool inInner = (shapes[i].x - r > rectInner.left && shapes[i].x + r < rectInner.right &&
                    shapes[i].y - r > rectInner.top && shapes[i].y + r < rectInner.bottom);

                // 2. 바깥쪽 사각형 안에 "완전히" 들어가는가?
                bool inOuter = (shapes[i].x - r > rectOuter.left && shapes[i].x + r < rectOuter.right &&
                    shapes[i].y - r > rectOuter.top && shapes[i].y + r < rectOuter.bottom);

                // 3. 안쪽 사각형 밖으로 "완전히" 벗어났는가? (선에 몸통이 걸치지 않았는지 확인)
                bool outOfInner = (shapes[i].x + r <= rectInner.left || shapes[i].x - r >= rectInner.right ||
                    shapes[i].y + r <= rectInner.top || shapes[i].y - r >= rectInner.bottom);

                // 4. 바깥쪽 사각형 밖으로 "완전히" 벗어났는가?
                bool outOfOuter = (shapes[i].x + r <= rectOuter.left || shapes[i].x - r >= rectOuter.right ||
                    shapes[i].y + r <= rectOuter.top || shapes[i].y - r >= rectOuter.bottom);

                if (inInner && shapes[i].region == 3) {
                    check = false;
                }
                else if (inOuter && outOfInner && shapes[i].region == 2) {
                    check = false;
                }
                else if (outOfOuter && shapes[i].region == 1) {
                    check = false;
                }
                else {
                    shapes[i].x = 0;
                    shapes[i].y = 0;
                }

        
    }
}

// --------------------------------------------------------
// 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    rectOuter = { wide / 5, height / 5, wide / 5 * 4, height / 5 * 4 };
    rectInner = { wide / 5 * 2, height / 5 * 2, wide / 5 * 3, height / 5 * 3 };

    PAINTSTRUCT ps;
    HDC hDC;

    switch (uMsg) {

    case WM_CREATE:
    {
        for (int i = 0; i < MAX_SHAPES; ++i) {
            shapes[i].color = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
            shapes[i].number = numDist(gen);
            bool bValid = false;
            while (!bValid) {
                shapes[i].x = xDist(gen);
                shapes[i].y = yDist(gen);

                int r = 20; // 도형의 반지름(여백) 크기 설정

                // 1. 안쪽 사각형 안에 "완전히" 들어가는가? (반지름 r만큼 선에서 안쪽으로 들어왔는지 확인)
                bool inInner = (shapes[i].x - r > rectInner.left && shapes[i].x + r < rectInner.right &&
                    shapes[i].y - r > rectInner.top && shapes[i].y + r < rectInner.bottom);

                // 2. 바깥쪽 사각형 안에 "완전히" 들어가는가?
                bool inOuter = (shapes[i].x - r > rectOuter.left && shapes[i].x + r < rectOuter.right &&
                    shapes[i].y - r > rectOuter.top && shapes[i].y + r < rectOuter.bottom);

                // 3. 안쪽 사각형 밖으로 "완전히" 벗어났는가? (선에 몸통이 걸치지 않았는지 확인)
                bool outOfInner = (shapes[i].x + r <= rectInner.left || shapes[i].x - r >= rectInner.right ||
                    shapes[i].y + r <= rectInner.top || shapes[i].y - r >= rectInner.bottom);

                // 4. 바깥쪽 사각형 밖으로 "완전히" 벗어났는가?
                bool outOfOuter = (shapes[i].x + r <= rectOuter.left || shapes[i].x - r >= rectOuter.right ||
                    shapes[i].y + r <= rectOuter.top || shapes[i].y - r >= rectOuter.bottom);


                if (inInner) {
                    // [3번 구역] 완전히 안쪽 사각형 내부
                    shapes[i].region = 3;
                    shapes[i].type = 3;
                    bValid = true;
                }
                else if (inOuter && outOfInner) {
                    // [2번 구역] 바깥 사각형 안이면서, 안쪽 사각형엔 몸통도 닿지 않음
                    shapes[i].region = 2;
                    shapes[i].type = 2;
                    bValid = true;
                }
                else if (outOfOuter) {
                    // [1번 구역] 바깥 사각형 밖으로 완전히 벗어남
                    shapes[i].region = 1;
                    shapes[i].type = 1;
                    bValid = true;
                }
                // 만약 선에 조금이라도 걸쳤다면? 조건에 아무것도 맞지 않으므로 다시 while문을 돌아 새로운 좌표를 뽑습니다!
            }
        }
    }
    break;

    case WM_KEYDOWN:
        if (wParam == VK_RETURN) UpdateShapes(0);
        else if (wParam == '1') UpdateShapes(1);
        else if (wParam == '2') UpdateShapes(2);
        else if (wParam == '3') UpdateShapes(3);

        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);

        // 1. 기준선 그리기
        HBRUSH hNullBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hNullBrush);

        Rectangle(hDC, rectOuter.left, rectOuter.top, rectOuter.right, rectOuter.bottom);
        Rectangle(hDC, rectInner.left, rectInner.top, rectInner.right, rectInner.bottom);

        SelectObject(hDC, oldBrush);
        SetBkMode(hDC, TRANSPARENT);

        // 2. 일반 배열(shapes)에서 도형 꺼내서 그리기
        for (int i = 0; i < MAX_SHAPES; ++i) {
            HBRUSH hBrush = CreateSolidBrush(shapes[i].color);
            HBRUSH hOldB = (HBRUSH)SelectObject(hDC, hBrush);

            if (shapes[i].type == 1) {
                POINT pts[3];
                pts[0] = { shapes[i].x, shapes[i].y - 20 };
                pts[1] = { shapes[i].x - 20, shapes[i].y + 20 };
                pts[2] = { shapes[i].x + 20, shapes[i].y + 20 };
                Polygon(hDC, pts, 3);
            }
            else if (shapes[i].type == 2) {
                Rectangle(hDC, shapes[i].x - 20, shapes[i].y - 20, shapes[i].x + 20, shapes[i].y + 20);
            }
            else if (shapes[i].type == 3) {
                Ellipse(hDC, shapes[i].x - 20, shapes[i].y - 20, shapes[i].x + 20, shapes[i].y + 20);
            }

            SelectObject(hDC, hOldB);
            DeleteObject(hBrush);

            // 3. 도형 내부에 숫자 출력하기 (std::string 제외)
            TCHAR numStr[10];
            wsprintf(numStr, L"%d", shapes[i].number); // 문자로 변환
            TextOut(hDC, shapes[i].x - 10, shapes[i].y - 8, numStr, lstrlen(numStr));
        }

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_SIZE:
        height = HIWORD(lParam);
        wide = LOWORD(lParam);
        UpdateShapes(0);
        InvalidateRect(hWnd, NULL, true);

        break;


    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}