#define _CRT_SECURE_NO_WARNINGS

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <random>
#include <iostream>
#include <string>

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

struct StrData
{
    // F2, F3 등으로 문자열이 길어질 수 있으므로 넉넉하게 잡습니다.
    TCHAR str[200];
    int len;
};

static StrData MemoData[10];
static int CurrentLine{};

// 상태 플래그 모음
static bool InsertFlag = true;

// F1은 실제 입력 조작, F6~F8은 일회성 조작이므로 필터 플래그에서 제외
static bool F1Flag = false;
static bool F2Flag = false;
static bool F3Flag = false;
static bool F4Flag = false;
static bool F5Flag = false;

// -----------------------------------------------------
// [렌더링 필터 전용 함수들 (원본 보존)]
// -----------------------------------------------------

void F2Function(const TCHAR* original, TCHAR* result) {
    int d = 0;
    bool inNum = false;
    for (int i = 0; original[i] != L'\0'; ++i) {
        TCHAR c = original[i];
        if (c >= L'0' && c <= L'9') {
            if (!inNum) {
                result[d++] = L'*';
                result[d++] = L'*';
                result[d++] = L'*';
                result[d++] = L'*';
                inNum = true;
            }
            result[d++] = c;
        }
        else {
            inNum = false;
            result[d++] = c;
        }
    }
    result[d] = L'\0';
}

void F3Function(const TCHAR* original, TCHAR* result) {
    int d = 0;
    bool inWord = false;
    for (int i = 0; original[i] != L'\0'; ++i) {
        TCHAR c = original[i];
        bool isLetter = ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9'));
        if (isLetter) {
            if (!inWord) { result[d++] = L'('; inWord = true; }
            if (c >= L'a' && c <= L'z') result[d++] = c - 32;
            else result[d++] = c;
        }
        else {
            if (inWord) { result[d++] = L')'; inWord = false; }
            if (c != L' ') result[d++] = c;
        }
    }
    if (inWord) result[d++] = L')';
    result[d] = L'\0';
}

void F4Function(const TCHAR* original, TCHAR* result) {
    int d = 0;
    for (int i = 0; original[i] != L'\0'; ++i) {
        TCHAR c = original[i];
        if (c == L' ') continue;
        if (c >= L'A' && c <= L'Z') result[d++] = c + 32;
        else result[d++] = c;
    }
    result[d] = L'\0';
}

void F5Function(const TCHAR* original, TCHAR* result) {
    int counts[256] = { 0 };
    int maxCount = 0;
    TCHAR maxChar = L'\0';

    for (int i = 0; original[i] != L'\0'; ++i) {
        TCHAR c = original[i];
        if (c != L' ') {
            counts[c]++;
            if (counts[c] > maxCount) {
                maxCount = counts[c];
                maxChar = c;
            }
        }
    }
    int d = 0;
    for (int i = 0; original[i] != L'\0'; ++i) {
        if (original[i] == maxChar && maxCount > 0) result[d++] = L'@';
        else result[d++] = original[i];
    }
    result[d] = L'\0';
}

// -----------------------------------------------------
// [메인 파이프라인]
// 화면 출력용으로만 사용 (F2, F3, F4, F5 전용)
// -----------------------------------------------------
void Changer(const TCHAR* original, TCHAR* result)
{
    // 필터 중첩 시 글자가 길어질 수 있으므로 충분히 큰 공간 할당
    TCHAR TempOriginal[800] = { 0 };
    TCHAR TempResult[800] = { 0 };

    lstrcpy(TempOriginal, original);

    if (F2Flag == true) {
        F2Function(TempOriginal, TempResult);
        lstrcpy(TempOriginal, TempResult);
    }
    if (F3Flag == true) {
        F3Function(TempOriginal, TempResult);
        lstrcpy(TempOriginal, TempResult);
    }
    if (F4Flag == true) {
        F4Function(TempOriginal, TempResult);
        lstrcpy(TempOriginal, TempResult);
    }
    if (F5Flag == true) {
        F5Function(TempOriginal, TempResult);
        lstrcpy(TempOriginal, TempResult);
    }

    lstrcpy(result, TempOriginal);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static int Count = 0;
    static SIZE size;
    PAINTSTRUCT ps;
    HDC hDC;

    static int CurRow{};
    static int r{}; // 줄

    switch (uMsg) {
    case WM_CREATE:
        CreateCaret(hWnd, NULL, 5, 15);
        ShowCaret(hWnd);
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_ESCAPE:
            for (int i = 0; i < 10; ++i) {
                r = 0;
                Count = 0;
                SetCaretPos(0, 0);
            }
            break;
        case VK_LEFT:
            Count--;
            if (Count < 0) { Count = 0; }
            break;
        case VK_RIGHT:
            if (MemoData[CurRow].str[Count] != '\0') { Count++; }
            break;
        case VK_UP:
            if (CurRow > 0) {
                if (MemoData[CurRow - 1].str[Count] == '\0') {
                    CurRow--;
                    Count = lstrlen(MemoData[CurRow].str);
                }
                else { CurRow--; }
            }
            break;
        case VK_DOWN:
            if (CurRow < 9) {
                if (MemoData[CurRow + 1].str[Count] == '\0') {
                    CurRow++;
                    Count = 0;
                }
                else { CurRow++; }
            }
            break;
        case VK_HOME:
            Count = 0;
            break;
        case VK_END:
            Count = lstrlen(MemoData[CurRow].str);
            if (Count > 29) {
                Count = 0;
                if (CurRow < 9) { CurRow++; }
                else { CurRow = 0; }
            }
            break;

        case VK_DELETE:
        {
            int len = lstrlen(MemoData[CurRow].str);
            if (Count < len) {
                int deleteLen = 0;
                if (MemoData[CurRow].str[Count] == L' ') {
                    deleteLen = 1;
                }
                else {
                    for (int i = Count; i < len; ++i) {
                        if (MemoData[CurRow].str[i] == L' ') { break; }
                        deleteLen++;
                    }
                }
                for (int i = Count; i <= len - deleteLen; ++i) {
                    MemoData[CurRow].str[i] = MemoData[CurRow].str[i + deleteLen];
                }
            }
            for (int i = CurRow; i < 9; ++i) {
                int curL = lstrlen(MemoData[i].str);
                int nxtL = lstrlen(MemoData[i + 1].str);
                while (curL < 30 && nxtL > 0) {
                    MemoData[i].str[curL] = MemoData[i + 1].str[0];
                    MemoData[i].str[curL + 1] = L'\0';
                    for (int j = 0; j < nxtL; ++j) {
                        MemoData[i + 1].str[j] = MemoData[i + 1].str[j + 1];
                    }
                    curL++;
                    nxtL--;
                }
            }
        }
        break;

        case VK_INSERT:
            InsertFlag = !InsertFlag;
            break;

        case VK_PRIOR:
            for (int i = 0; i < 3; ++i) {
                if (CurRow > 0) {
                    if (MemoData[CurRow - 1].str[Count] == '\0') {
                        CurRow--;
                        Count = lstrlen(MemoData[CurRow].str);
                    }
                    else { CurRow--; }
                }
                else { CurRow = 9; }
            }
            break;

        case VK_NEXT:
            for (int i = 0; i < 3; ++i) {
                if (CurRow < 9) {
                    if (MemoData[CurRow + 1].str[Count] == '\0') {
                        CurRow++;
                        Count = 0;
                    }
                    else { CurRow++; }
                }
                else { CurRow = 0; }
            }
            break;

            // F1: Caps Lock 기능 켜기/끄기
        case VK_F1:
            F1Flag = !F1Flag;
            break;
            // F2~F5: 화면 출력용 렌더링 필터 켜기/끄기
        case VK_F2:
            F2Flag = !F2Flag;
            InvalidateRect(hWnd, NULL, true);
            break;
        case VK_F3:
            if (F4Flag == false) {
                F3Flag = !F3Flag;
                InvalidateRect(hWnd, NULL, true);
            }
            break;
        case VK_F4:
            if (F3Flag == false) {
                F4Flag = !F4Flag;
                InvalidateRect(hWnd, NULL, true);
            }
            break;
        case VK_F5:
            F5Flag = !F5Flag;
            InvalidateRect(hWnd, NULL, true);
            break;

            // F6: 실제 줄 순서 밀어내기 (1->3, 2->1, 3->2) 액션
        case VK_F6:
        {
            int maxRow = 0;
            // 가장 아래에 글씨가 있는 줄(maxRow)을 찾습니다.
            for (int i = 0; i < 10; ++i) {
                if (MemoData[i].str[0] != L'\0') {
                    maxRow = i;
                }
            }
            // 커서가 더 아래에 있다면 커서 기준
            if (CurRow > maxRow) maxRow = CurRow;

            if (maxRow > 0) {
                // 1번째 줄(0번)을 빼두고, 나머지 줄들을 한 칸씩 위로 당겨온 뒤, 맨 밑에 1번째 줄을 넣습니다.
                StrData temp = MemoData[0];
                for (int i = 0; i < maxRow; ++i) {
                    MemoData[i] = MemoData[i + 1];
                }
                MemoData[maxRow] = temp;
                InvalidateRect(hWnd, NULL, true);
            }
        }
        break;

        // F7: 모든 숫자 1 더하기 액션
        case VK_F7:
            for (int i = 0; i < 10; ++i) {
                TCHAR temp[400]{};

                int d{};
                int j{};

                while (MemoData[i].str[j] != '\0') {
                    TCHAR temp2[40]{};

                    int index{};
                    bool isDigit = (MemoData[i].str[j] >= L'0' && MemoData[i].str[j] <= L'9');
                    bool isNegative = (MemoData[i].str[j] == L'-' && MemoData[i].str[j + 1] >= L'0' && MemoData[i].str[j + 1] <= L'9');

                    if (isDigit || isNegative) {

                        if (isNegative) {
                            temp2[index++] = MemoData[i].str[j++];
                        }

                        while (MemoData[i].str[j] >= L'0' && MemoData[i].str[j] <= L'9') {
                            temp2[index++] = MemoData[i].str[j++];
                        }
                        temp2[index] = '\0';

                        int num = _ttoi(temp2) + 1;

                        TCHAR newTemp[40]{};

                        wsprintf(newTemp, L"%d", num);

                        for (int k = 0; newTemp[k] != L'\0'; ++k) {
                            temp[d++] = newTemp[k];
                        }

                    }
                    else {
                        temp[d++] = MemoData[i].str[j++];
                    }
                }

                temp[d] = L'\0';
                lstrcpy(MemoData[i].str, temp);
                CurrentLine = lstrlen(MemoData[CurRow].str);
                if (Count > CurrentLine) {
                    Count = CurrentLine;
                }


            }
            InvalidateRect(hWnd, NULL, true);
            break;

            // F8: 모든 숫자 1 빼기 액션
        case VK_F8:
            for (int i = 0; i < 10; ++i) {
                TCHAR temp[400]{};

                int d{};
                int j{};
                while (MemoData[i].str[j] != '\0') {

                    TCHAR temp2[40]{};

                    int index{};

                    bool isDigit = (MemoData[i].str[j] >= L'0' && MemoData[i].str[j] <= L'9');
                    bool isNegative = (MemoData[i].str[j] == L'-' && MemoData[i].str[j + 1] >= L'0' && MemoData[i].str[j + 1] <= L'9');

                    if (isDigit || isNegative) {
                        if (isNegative) {
                            temp2[index++] = MemoData[i].str[j++];
                        }
                        while (MemoData[i].str[j] >= L'0' && MemoData[i].str[j] <= L'9') {
                            temp2[index++] = MemoData[i].str[j++];
                        }
                        temp2[index] = '\0';

                        int num = _ttoi(temp2) - 1;

                        TCHAR newTemp[40]{};

                        wsprintf(newTemp, L"%d", num);

                        for (int k = 0; newTemp[k] != L'\0'; ++k) {
                            temp[d++] = newTemp[k];
                        }
                    }
                    else {
                        temp[d++] = MemoData[i].str[j++];
                    }
                }
                
                temp[d] = L'\0';
                lstrcpy(MemoData[i].str, temp);
                CurrentLine = lstrlen(MemoData[CurRow].str);
                if (Count > CurrentLine) {
                    Count = CurrentLine;
                }

            // for문 종료
            }


            InvalidateRect(hWnd, NULL, true);
            break;
        }

        break;
    case WM_CHAR:
        hDC = GetDC(hWnd);
        if (wParam == VK_ESCAPE) {
            for (int i = 0; i <= CurRow; ++i) {
                for (int j = 0; j < lstrlen(MemoData[i].str); ++j) {
                    MemoData[i].str[j] = L'\0';
                }
            }
            Count = 0;
            CurRow = 0;
            InvalidateRect(hWnd, NULL, true);
            break;
        }
        else if (wParam == 'Q') {
            PostQuitMessage(2025180028);
        }
        else if (wParam == VK_BACK) {
            int len = lstrlen(MemoData[CurRow].str);
            if (Count > 0) {
                for (int i = Count - 1; i < len; ++i) {
                    MemoData[CurRow].str[i] = MemoData[CurRow].str[i + 1];
                }
                Count--;
            }
            else if (CurRow > 0) {
                CurRow--;
                Count = lstrlen(MemoData[CurRow].str);
            }

            for (int i = CurRow; i < 9; ++i) {
                int curL = lstrlen(MemoData[i].str);
                int nxtL = lstrlen(MemoData[i + 1].str);

                while (curL < 30 && nxtL > 0) {
                    MemoData[i].str[curL] = MemoData[i + 1].str[0];
                    MemoData[i].str[curL + 1] = L'\0';
                    for (int j = 0; j < nxtL; ++j) {
                        MemoData[i + 1].str[j] = MemoData[i + 1].str[j + 1];
                    }
                    curL++;
                    nxtL--;
                }
            }
        }
        else if (wParam == VK_RETURN) {
            CurRow++;
            Count = 0;
            if (CurRow > 9) {
                CurRow = 0;
                Count = 0;
            }
        }
        else if (wParam == VK_TAB) {
            for (int i = 0; i < 4; ++i) {
                int len = lstrlen(MemoData[CurRow].str);
                if (InsertFlag == true) {
                    for (int j = len; j >= Count; --j) {
                        MemoData[CurRow].str[j + 1] = MemoData[CurRow].str[j];
                    }
                    if (MemoData[CurRow].str[30] != L'\0') {
                        MemoData[CurRow].str[30] = L'\0';
                    }
                }
                MemoData[CurRow].str[Count] = L' ';
                Count++;

                if (Count >= 30) {
                    Count = 0;
                    CurRow++;
                }
            }
        }
        else {
            if (Count >= 30) {
                Count = 0;
                CurRow++;
                if (CurRow >= 10) { CurRow = 0; }
            }

            if (InsertFlag == true) {
                int len = lstrlen(MemoData[CurRow].str);
                for (int j = len; j >= Count; --j) {
                    MemoData[CurRow].str[j + 1] = MemoData[CurRow].str[j];
                }

                if (MemoData[CurRow].str[30] != L'\0') {
                    for (int i = CurRow; i < 9; ++i) {
                        int CurL = lstrlen(MemoData[i].str);
                        int NexL = lstrlen(MemoData[i + 1].str);

                        for (int j = NexL; j >= 0; --j) {
                            if (j + 1 <= 30) {
                                MemoData[i + 1].str[j + 1] = MemoData[i + 1].str[j];
                            }
                        }
                        MemoData[i + 1].str[0] = MemoData[i].str[30];
                        MemoData[i].str[30] = L'\0';

                        if (MemoData[9].str[30] != L'\0') {
                            MemoData[9].str[30] = L'\0';
                        }
                    }
                }
            }

            int len = lstrlen(MemoData[CurRow].str);

            // 입력된 키 받아오기
            TCHAR inputChar = (TCHAR)wParam;

            // ?? F1: Caps Lock 기능! 스위치가 켜져있으면 실제 배열에 넣을 때 대문자로 변환해서 넣습니다.
            if (F1Flag == true) {
                if (inputChar >= L'a' && inputChar <= L'z') {
                    inputChar -= 32;
                }
            }

            // 진짜 데이터 쓰기
            MemoData[CurRow].str[Count] = inputChar;

            if (Count >= len) {
                MemoData[CurRow].str[Count + 1] = L'\0';
            }

            ++Count;
        }
        InvalidateRect(hWnd, NULL, true);
        ReleaseDC(hWnd, hDC);
        break;

    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);

        // 1. 화면에 출력 (F2~F5 필터만 거침, F6/F7/F8은 이미 실제 데이터를 변경했으므로 거치지 않음!)
        for (int i = 0; i < 10; ++i) {
            TCHAR filteredStr[800] = { 0 };
            Changer(MemoData[i].str, filteredStr);
            TextOut(hDC, 0, i * 20, filteredStr, lstrlen(filteredStr));
        }

        // 2. 캐럿 위치 잡기
        if (Count == 0) {
            SetCaretPos(0, CurRow * 20);
        }
        else {
            TCHAR sub[200] = { 0 };
            TCHAR filteredSub[800] = { 0 };

            lstrcpyn(sub, MemoData[CurRow].str, Count + 1);
            Changer(sub, filteredSub);

            GetTextExtentPoint32(hDC, filteredSub, lstrlen(filteredSub), &size);
            SetCaretPos(size.cx, CurRow * 20);
        }

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_SIZE:
        height = HIWORD(lParam);
        wide = LOWORD(lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}