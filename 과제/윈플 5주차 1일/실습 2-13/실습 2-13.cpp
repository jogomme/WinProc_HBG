#define _CRT_SECURE_NO_WARNINGS

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <tchar.h>
#include <random>    

using namespace std;
int wide{ 800 };
int height{ 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_int_distribution<int> UidPoint(0, 39);
uniform_int_distribution<int> uidType(0, 2);

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

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
class Shape
{
private:
    int Type;
    COLORREF color;
    double size;
    POINT point;

    int OrignalType;
    COLORREF OrignalColor;
    bool isChanged;


public:
    bool isit;

    int GetType() { return Type; }

    COLORREF GetColor() { return color; }

    void Init(int shapeType) {
        Type = shapeType;
        OrignalType = shapeType;
        color = RGB(colorDist(gen), colorDist(gen), colorDist(gen));
        OrignalColor = color;
        point.x = UidPoint(gen);
        point.y = UidPoint(gen);
        size = 1.0;
        isChanged = false;
        isit = false;
    }

    void Draw(HDC hDC, int cellwide, int cellheight)
    {

        POINT center = {
            (point.x * cellwide) + (cellwide / 2),
            (point.y * cellheight) + (cellheight / 2)
        };

        POINT SizeRate = {
            (cellwide / 2) * size,
            (cellheight / 2) * size
        };

        int left = center.x - SizeRate.x;
        int top = center.y - SizeRate.y;
        int right = center.x + SizeRate.x;
        int bottom = center.y + SizeRate.y;

        HBRUSH myBrush = CreateSolidBrush(color);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, myBrush);

        if (isit) {
            // CreatePen(선의 종류, 굵기, 색상)
            // PS_SOLID는 실선(끊어지지 않은 띠)을 의미합니다. 굵기는 5, 색상은 빨간색!
            HPEN myPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));

            // 화면(hDC)에 내가 만든 펜을 장착! (원래 들고 있던 펜은 백업해둡니다)
            HPEN oldPen = (HPEN)SelectObject(hDC, myPen);

            if (Type == 0) {
                Ellipse(hDC, left, top, right, bottom);
            }
            else if (Type == 1) {
                POINT pts[3] = {
                    {center.x, top},
                    {right, bottom},
                    {left, bottom}
                };
                Polygon(hDC, pts, 3);
            }
            else if (Type == 2) {
                Rectangle(hDC, left, top, right, bottom);
            }

            SelectObject(hDC, oldPen);
            DeleteObject(myPen);

        }
        else {

            if (Type == 0) {
                Ellipse(hDC, left, top, right, bottom);
            }
            else if (Type == 1) {
                POINT pts[3] = {
                    {center.x, top},
                    {right, bottom},
                    {left, bottom}
                };
                Polygon(hDC, pts, 3);
            }
            else if (Type == 2) {
                Rectangle(hDC, left, top, right, bottom);
            }
        }
        SelectObject(hDC, oldBrush); // 제자리 돌아가기
        DeleteObject(myBrush);
    }

    void move(int dx, int dy)
    {
        point.x += dx;
        point.y += dy;

        if (point.x < 0)   point.x = 39;
        if (point.x >= 40) point.x = 0;
        if (point.y < 0)   point.y = 39;
        if (point.y >= 40) point.y = 0;
    }

    void ChangeSize(int num)
    {
        if (num == 1) {
            size *= 1.1;
            if (size > 3) {
                size = 3;
            }
        }
        else if (num == 0) {
            size *= 0.9;
            if (size < 0.5) {
                size = 0.5;
            }
        }
    }

    void ChangeType(COLORREF SelectedColor, int num)
    {
        if (isChanged == true) {
            Type = OrignalType;
            color = OrignalColor;
            isChanged = false;
        }
        else if (isChanged == false && num == 1) {
            int newType = uidType(gen);
            while (Type == newType) {
                newType = uidType(gen);
            }
            Type = newType;
            color = SelectedColor;
            isChanged = true;
        }
    }
};

Shape shape[11];

int shapeCount = 0;  // 현재 화면에 그려진 도형의 개수
int selectedIndex = -1; // 선택된 도형의 인덱스 (-1: 아무것도 선택안됨, 0번부터 시작)

// --------------------------------------------------------
// 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    static int count{};
    static int NUM{};

    switch (uMsg) {
    case WM_CREATE:

    case WM_KEYDOWN:
        if (wParam == 'Q') {
            PostQuitMessage(2025180028);
        }
        // 원 그리기
        else if (wParam == 'E') {
            if (count >= 10) {
                for (int i = 0; i < 9; ++i) {
                    shape[i] = shape[i + 1];
                }
                count = 9;
            }
            shape[count++].Init(0);
        }
        // 삼각형 그리기
        else if (wParam == 'T') {
            if (count >= 10) {
                for (int i = 0; i < 9; ++i) {
                    shape[i] = shape[i + 1];
                }
                count = 9;
            }
            shape[count++].Init(1);
        }
        // 사각형 그리기
        else if (wParam == 'R') {
            if (count >= 10) {
                for (int i = 0; i < 9; ++i) {
                    shape[i] = shape[i + 1];
                }
                count = 9;
            }
            shape[count++].Init(2);
        }
        // 숫자 선택
        else if (wParam >= '0' && wParam <= '9') {
            int isit = wParam - '0';
            if (isit == 0) isit = 10;

            if (shape[isit - 1].isit != true) {

                for (int i = 0; i < count; ++i) {
                    if (shape[i].isit == true) {
                        shape[i].isit = false;
                    }
                }
                shape[isit - 1].isit = true;
            }
            else {
                shape[isit - 1].isit = false;
            }
        }
        // 화살표 선택
        // 상
        else if (wParam == VK_UP) {
            for (int i = 0; i < count; ++i) {
                if (shape[i].isit == true) {
                    shape[i].move(0, -1);
                }
            }
        }
        // 하
        else if (wParam == VK_DOWN) {
            for (int i = 0; i < count; ++i) {
                if (shape[i].isit == true) {
                    shape[i].move(0, +1);
                }
            }
        }
        // 좌
        else if (wParam == VK_LEFT) {
            for (int i = 0; i < count; ++i) {
                if (shape[i].isit == true) {
                    shape[i].move(-1, 0);
                }
            }
        }
        // 우
        else if (wParam == VK_RIGHT) {
            for (int i = 0; i < count; ++i) {
                if (shape[i].isit == true) {
                    shape[i].move(1, 0);
                }
            }
        }
        // +
        else if (wParam == VK_OEM_PLUS) {
            for (int i = 0; i < count; ++i) {
                if (shape[i].isit == true) {
                    shape[i].ChangeSize(1);
                }
            }
        }
        // -
        else if (wParam == VK_OEM_MINUS) {
            for (int i = 0; i < count; ++i) {
                if (shape[i].isit == true) {
                    shape[i].ChangeSize(0);
                }
            }
        }
        //같은 모양의 도형을 찾아 다른 모양으로 바꾸고 랜덤한 색상을 해당 도형의 색상으로 바꾼다.
        else if (wParam == 'C') {
            int TypeNum{};
            COLORREF color{};
            for (int i = 0; i < count; ++i) {
                if (shape[i].isit == true) {
                    TypeNum = shape[i].GetType();
                    color = shape[i].GetColor();
                }
            }
            for (int i = 0; i < count; ++i) {
                if (shape[i].GetType() == TypeNum && shape[i].isit == false) {
                    shape[i].ChangeType(color, 1);
                }
                else {
                    shape[i].ChangeType(color, 0);
                }
            }
        }
        //선택된 도형이 삭제된다
        else if (wParam == 'D') {
            for (int i = 0; i < count; ++i) {
                if (shape[i].isit == true) {
                    for (int j = i; j < count; ++j) {
                        shape[j] = shape[j + 1];
                    }
                    count--;
                    break;
                }
            }
        }
        //모든 도형을 삭제하고 초기화
        else if (wParam == 'P') {
            count = 0;
        }
        
        InvalidateRect(hWnd, NULL, true);

        break;

    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);

        int cellwide = wide / 40;
        int cellheight = height / 40;
        
        for (int i = 0; i <= 40; ++i) {
            MoveToEx(hDC, 0, cellheight * i, NULL);
            LineTo(hDC, wide, cellheight * i);

            MoveToEx(hDC, cellwide * i, 0, NULL);
            LineTo(hDC, cellwide * i, height);
        }

        for (int i = 0; i < count; ++i) {
            shape[i].Draw(hDC,cellwide,cellheight);
        }
        for (int i = 0; i < count; ++i) {
            if(shape[i].isit) shape[i].Draw(hDC, cellwide, cellheight);
        }

        EndPaint(hWnd, &ps);
        return 0;
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