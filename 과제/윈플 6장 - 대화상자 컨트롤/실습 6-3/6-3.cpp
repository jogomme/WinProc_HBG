#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include "resource.h"

using namespace std;

// 전역 변수
wstring g_expr = L"";
double  g_lastResult = 0.0;
bool    g_binaryMode = false;
bool    g_justCalced = false;

BOOL CALLBACK Dialog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

// 에디트박스 서브클래싱 (키보드 입력 가로채기)
WNDPROC g_OldEditProc = nullptr;
HWND    g_hDlg = nullptr;

LRESULT CALLBACK EditSubclassProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CHAR) {
        // 에디트박스로 가는 문자를 다이얼로그로 전달
        SendMessage(g_hDlg, WM_CHAR, wParam, lParam);
        return 0; // 에디트박스 기본 처리 막음
    }
    if (msg == WM_KEYDOWN) {
        int vk = (int)wParam;
        if (vk == VK_DELETE || vk == VK_ESCAPE ||
            vk == VK_ADD || vk == VK_SUBTRACT ||
            vk == VK_MULTIPLY || vk == VK_DIVIDE ||
            (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)) {
            SendMessage(g_hDlg, WM_KEYDOWN, wParam, lParam);
            return 0;
        }
    }
    return CallWindowProc(g_OldEditProc, hEdit, msg, wParam, lParam);
}


void    UpdateDisplay(HWND hDlg, const wstring& text);
wstring EvalExpr(const wstring& expr);
wstring ReverseTokens(const wstring& expr);
wstring ClearEntry(const wstring& expr);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    HWND hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1),
        NULL, (DLGPROC)Dialog_Proc);
    if (!hDlg) return 0;
    ShowWindow(hDlg, nCmdShow);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

void UpdateDisplay(HWND hDlg, const wstring& text)
{
    SetDlgItemTextW(hDlg, IDC_EDIT_RESULT, text.c_str());
}

// 수식 계산 (+, -, *, / 우선순위 지원)
wstring EvalExpr(const wstring& expr)
{
    if (expr.empty()) return L"0";

    vector<double>  nums;
    vector<wchar_t> ops;
    wstring cur;

    for (size_t i = 0; i < expr.size(); i++) {
        wchar_t c = expr[i];
        bool isOp = (c == L'+' || c == L'-' || c == L'*' || c == L'/');
        bool isSign = (c == L'-') && (i == 0 || (!nums.empty() && ops.size() == nums.size()));
        if (isOp && !isSign) {
            if (!cur.empty()) { nums.push_back(_wtof(cur.c_str())); cur = L""; }
            ops.push_back(c);
        }
        else {
            cur += c;
        }
    }
    if (!cur.empty()) nums.push_back(_wtof(cur.c_str()));
    if (nums.empty()) return L"0";

    // 1패스: * /
    vector<double>  n2; n2.push_back(nums[0]);
    vector<wchar_t> o2;
    for (size_t i = 0; i < ops.size(); i++) {
        if (ops[i] == L'*') n2.back() *= nums[i + 1];
        else if (ops[i] == L'/') {
            if (nums[i + 1] == 0.0) return L"Error(0/)";
            n2.back() /= nums[i + 1];
        }
        else { o2.push_back(ops[i]); n2.push_back(nums[i + 1]); }
    }
    // 2패스: + -
    double result = n2[0];
    for (size_t i = 0; i < o2.size(); i++)
        result = (o2[i] == L'+') ? result + n2[i + 1] : result - n2[i + 1];

    wchar_t buf[64];
    if (result == (long long)result)
        swprintf_s(buf, L"%lld", (long long)result);
    else
        swprintf_s(buf, L"%g", result);
    return wstring(buf);
}

// R 버튼: 각 숫자 토큰 자릿수 역순  예) "12+34" -> "21+43"
wstring ReverseTokens(const wstring& expr)
{
    if (expr.empty()) return expr;
    struct Token { wstring val; bool isOp; };
    vector<Token> tokens;
    wstring cur;
    for (size_t i = 0; i < expr.size(); i++) {
        wchar_t c = expr[i];
        bool isOp = (c == L'+' || c == L'-' || c == L'*' || c == L'/');
        bool isSgn = (c == L'-') && cur.empty() && tokens.empty();
        if (isOp && !isSgn) {
            if (!cur.empty()) { tokens.push_back({ cur,false }); cur = L""; }
            tokens.push_back({ wstring(1,c),true });
        }
        else cur += c;
    }
    if (!cur.empty()) tokens.push_back({ cur,false });

    for (auto& t : tokens) {
        if (!t.isOp) {
            size_t dot = t.val.find(L'.');
            wstring ip = (dot == wstring::npos) ? t.val : t.val.substr(0, dot);
            wstring fp = (dot == wstring::npos) ? L"" : t.val.substr(dot);
            reverse(ip.begin(), ip.end());
            t.val = ip + fp;
        }
    }
    wstring res;
    for (auto& t : tokens) res += t.val;
    return res;
}

// CE 버튼: 마지막 피연산자만 삭제
wstring ClearEntry(const wstring& expr)
{
    if (expr.empty()) return expr;
    int lastOp = -1;
    for (int i = (int)expr.size() - 1; i >= 1; i--) {
        wchar_t c = expr[i];
        if (c == L'+' || c == L'-' || c == L'*' || c == L'/') { lastOp = i; break; }
    }
    return (lastOp == -1) ? L"" : expr.substr(0, lastOp + 1);
}

void AppendDigit(HWND hDlg, wchar_t c)
{
    if (g_justCalced) { g_expr = L""; g_justCalced = false; g_binaryMode = false; }
    g_expr += c;
    UpdateDisplay(hDlg, g_expr);
}

void AppendOp(HWND hDlg, wchar_t op)
{
    g_justCalced = false; g_binaryMode = false;
    if (g_expr.empty()) {
        if (op == L'-') g_expr += op;
    }
    else {
        wchar_t last = g_expr.back();
        if (last == L'+' || last == L'-' || last == L'*' || last == L'/')
            g_expr.back() = op;
        else
            g_expr += op;
    }
    UpdateDisplay(hDlg, g_expr);
}

void ApplyUnary(HWND hDlg, double val)
{
    wchar_t buf[64];
    if (val == (long long)val) swprintf_s(buf, L"%lld", (long long)val);
    else                     swprintf_s(buf, L"%g", val);
    g_expr = buf; g_lastResult = val; g_justCalced = true; g_binaryMode = false;
    UpdateDisplay(hDlg, g_expr);
}

BOOL CALLBACK Dialog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg) {
    case WM_INITDIALOG: {
        g_hDlg = hDlg;
        HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_RESULT);
        g_OldEditProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
        UpdateDisplay(hDlg, L"");
        return TRUE;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        switch (id) {
            // 숫자 버튼 (ID가 비연속이므로 개별 case 처리)
        case IDC_BTN0: AppendDigit(hDlg, L'0'); break;
        case IDC_BTN1: AppendDigit(hDlg, L'1'); break;
        case IDC_BTN2: AppendDigit(hDlg, L'2'); break;
        case IDC_BTN3: AppendDigit(hDlg, L'3'); break;
        case IDC_BTN4: AppendDigit(hDlg, L'4'); break;
        case IDC_BTN5: AppendDigit(hDlg, L'5'); break;
        case IDC_BTN6: AppendDigit(hDlg, L'6'); break;
        case IDC_BTN7: AppendDigit(hDlg, L'7'); break;
        case IDC_BTN8: AppendDigit(hDlg, L'8'); break;
        case IDC_BTN9: AppendDigit(hDlg, L'9'); break;

            // 연산자 버튼
        case IDC_BTN_PLUS:  AppendOp(hDlg, L'+'); break;
        case IDC_BTN_MINUS: AppendOp(hDlg, L'-'); break;
        case IDC_BTN_MUL:   AppendOp(hDlg, L'*'); break;
        case IDC_BTN_DIV:   AppendOp(hDlg, L'/'); break;

            // = 버튼: 계산
        case IDC_BTN_EQUAL: {
            g_binaryMode = false;
            wstring res = EvalExpr(g_expr);
            g_lastResult = _wtof(res.c_str());
            g_expr = res; g_justCalced = true;
            UpdateDisplay(hDlg, g_expr);
            break;
        }

                          // C: 전체 초기화
        case IDC_BTN_C:
            g_expr = L""; g_lastResult = 0.0; g_justCalced = false; g_binaryMode = false;
            UpdateDisplay(hDlg, L"");
            break;

            // CE: 마지막 피연산자 삭제
        case IDC_BTN_CE:
            g_expr = ClearEntry(g_expr); g_justCalced = false; g_binaryMode = false;
            UpdateDisplay(hDlg, g_expr);
            break;

            // Backspace: 한 글자 삭제
        case IDC_BTN_BACK:
            if (!g_expr.empty()) g_expr.pop_back();
            g_justCalced = false; g_binaryMode = false;
            UpdateDisplay(hDlg, g_expr);
            break;

            // R: 숫자 자릿수 역순
        case IDC_BTN_REVERS:
            g_expr = ReverseTokens(g_expr); g_binaryMode = false;
            UpdateDisplay(hDlg, g_expr);
            break;

            // Binary: 2진수 <-> 10진수 토글
        case IDC_BTN_BINARY: {
            if (!g_binaryMode) {
                wstring res = EvalExpr(g_expr);
                g_lastResult = _wtof(res.c_str());
                long long val = (long long)g_lastResult;
                wstring bin;
                if (val == 0) bin = L"0";
                else {
                    long long tmp = (val < 0) ? -val : val;
                    while (tmp > 0) { bin = (wchar_t)(L'0' + tmp % 2) + bin; tmp /= 2; }
                    if (val < 0) bin = L"-" + bin;
                }
                UpdateDisplay(hDlg, bin);
                g_binaryMode = true;
            }
            else {
                wchar_t buf[256] = {};
                GetDlgItemTextW(hDlg, IDC_EDIT_RESULT, buf, 255);
                wstring binStr = buf;
                bool neg = (!binStr.empty() && binStr[0] == L'-');
                wstring digits = neg ? binStr.substr(1) : binStr;
                long long val = 0;
                for (wchar_t d : digits) val = val * 2 + (d - L'0');
                if (neg) val = -val;
                wchar_t out[64]; swprintf_s(out, L"%lld", val);
                g_expr = out; UpdateDisplay(hDlg, g_expr); g_binaryMode = false;
            }
            break;
        }

                           // 1/2: 현재값 / 2
        case IDC_BTN_HALF: {
            double v = _wtof(EvalExpr(g_expr).c_str());
            ApplyUnary(hDlg, v / 2.0);
            break;
        }

                         // *10: 현재값 * 10
        case IDC_BTN_MUL10: {
            double v = _wtof(EvalExpr(g_expr).c_str());
            ApplyUnary(hDlg, v * 10.0);
            break;
        }

                          // x²: 현재값 제곱
        case IDC_BTN_SQUARE: {
            double v = _wtof(EvalExpr(g_expr).c_str());
            ApplyUnary(hDlg, v * v);
            break;
        }

                           // 종료
        case IDC_BTN_FINISH:
            PostQuitMessage(0);
            break;

        default: break;
        }
        return TRUE;
    }


                   // 키보드 문자 입력 처리 (숫자, 연산자, Enter, Backspace)
    case WM_CHAR: {
        wchar_t c = (wchar_t)wParam;
        if (c >= L'0' && c <= L'9') AppendDigit(hDlg, c);
        else if (c == L'+')              AppendOp(hDlg, L'+');
        else if (c == L'-')              AppendOp(hDlg, L'-');
        else if (c == L'*')              AppendOp(hDlg, L'*');
        else if (c == L'/')              AppendOp(hDlg, L'/');
        else if (c == L'=' || c == L'\r') {
            g_binaryMode = false;
            wstring res = EvalExpr(g_expr);
            g_lastResult = _wtof(res.c_str());
            g_expr = res; g_justCalced = true;
            UpdateDisplay(hDlg, g_expr);
        }
        else if (c == L'\b') {
            if (!g_expr.empty()) g_expr.pop_back();
            g_justCalced = false; g_binaryMode = false;
            UpdateDisplay(hDlg, g_expr);
        }
        return TRUE;
    }

                // 특수키 처리 (Delete=C, Escape=C, 넘패드 연산자)
    case WM_KEYDOWN: {
        int vk = (int)wParam;
        if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
            AppendDigit(hDlg, (wchar_t)(L'0' + vk - VK_NUMPAD0));
        else if (vk == VK_ADD)      AppendOp(hDlg, L'+');
        else if (vk == VK_SUBTRACT) AppendOp(hDlg, L'-');
        else if (vk == VK_MULTIPLY) AppendOp(hDlg, L'*');
        else if (vk == VK_DIVIDE)   AppendOp(hDlg, L'/');
        else if (vk == VK_DELETE || vk == VK_ESCAPE) {
            g_expr = L""; g_lastResult = 0.0; g_justCalced = false; g_binaryMode = false;
            UpdateDisplay(hDlg, L"");
        }
        return TRUE;
    }

    case WM_CLOSE:
        PostQuitMessage(0);
        return TRUE;
    }
    return FALSE;
}
