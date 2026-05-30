#define _CRT_SECURE_NO_WARNINGS

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment (lib, "msimg32.lib")

#include <windows.h>
#include <tchar.h>
#include <random>    
#include <iostream>
#include "resource.h"

using namespace std;
int wide{ 1200 };
int height{ 800 };

// --------------------------------------------------------
// 난수 생성기 세팅
// --------------------------------------------------------
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> colorDist(0, 255);
uniform_int_distribution<int> speedDist(5, 12);    // 몬스터 속도 범위 적정 조절
uniform_int_distribution<int> typeDist(0, 1);     // 0: 몬스터 종류 1, 1: 몬스터 종류 2 (바닥/공중 배정용)

// --------------------------------------------------------
// 윈 메인
// --------------------------------------------------------
HINSTANCE g_hInst;
LPCTSTR IpszClass = L"My Window Class 3";
LPCTSTR IpszWindowName = L"실습 5-7: 점프하고 엎드리는 캐릭터";
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
// 구조체 선언 구간
// --------------------------------------------------------
struct ch
{
    int x;
    int y;
    int fream;
    int speed;
    bool see = false;

    // [기능 추가] 몬스터 속성 및 충돌 관리를 위한 변수 확장
    int type;        // 0: 바닥 몬스터(g_Monster 0~2번 사용), 1: 공중 몬스터(g_Monster 3~5번 사용)
    bool isExplode;  // 현재 폭발 애니메이션 중인지 여부
    int explodeFrame;// 폭발 프레임카운터 (0~4)
};

// --------------------------------------------------------
// 전역 함수 선언 구간
// --------------------------------------------------------
void Draw(HDC memDC);
void DrawGround(HDC memDC);
void DrawSky(HDC memDC);
void DrawCh(HDC memDC);
void DrawMon(HDC memDC);
void ResetMap();

// --------------------------------------------------------
// 전역 변수 선언 구간
// --------------------------------------------------------
HBITMAP g_hImg;
BITMAP bmpInfo;

HBITMAP g_Sky;
HBITMAP g_Ground;

ch c;
HBITMAP g_ch[8];

ch mon[10];
HBITMAP g_Monster[6];

bool isFail = false;
bool isSlide = false; // 납작하게 엎드린 상태 [cite: 112]

// [기능 추가] 무한 횡스크롤 배경을 위한 스크롤 오프셋 변수 
int skyOffset = 0;
int groundOffset = 0;

// [기능 추가] 점프 상태 처리를 위한 물리 변수
bool isJumping = false;
int jumpY = 0;          // 점프 높이 오프셋
int jumpVelocity = 0;   // 점프 속도(중력 가속도 영향)

// [기능 추가] 캐릭터 좌우 연속 이동을 위한 비동기 키토글 변수 [cite: 111]
bool isKeyLeft = false;
bool isKeyRight = false;

// --------------------------------------------------------
// 메시지 처리 함수
// --------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    static RECT rectView;

    switch (uMsg) {
    case WM_CREATE:
        GetClientRect(hWnd, &rectView);

        // 배경 이미지 세팅
        g_Sky = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SKY));
        g_Ground = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_GROUND));

        // 캐릭터 런 애니메이션 (0~6) 및 엎드리기 이미지 (7) [cite: 114]
        g_ch[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CH01));
        g_ch[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CH02));
        g_ch[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CH03));
        g_ch[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CH04));
        g_ch[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CH05));
        g_ch[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CH06));
        g_ch[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CH07));
        g_ch[7] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_Slide));

        // 몬스터 이미지 6종 (0~2: 바닥주행형 애니메이션 / 3~5: 공중비행형 애니메이션) [cite: 107, 108, 114]
        g_Monster[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_MONSTER1));
        g_Monster[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_MONSTER2));
        g_Monster[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_MONSTER3));
        g_Monster[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_MONSTER4));
        g_Monster[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_MONSTER5));
        g_Monster[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_MONSTER6));

        ResetMap();

        // 주기적 업데이트를 위한 실시간 타이머 작동
        SetTimer(hWnd, 1, 32, NULL);
        break;

    case WM_KEYDOWN:
        if (wParam == 'Q' || wParam == 'q') {
            PostQuitMessage(0);
        }
        else if (wParam == 'R' || wParam == 'r') {
            ResetMap();
        }
        // 캐릭터 좌우 이동 트리거 키 활성화 [cite: 111]
        else if (wParam == VK_LEFT) {
            isKeyLeft = true;
        }
        else if (wParam == VK_RIGHT) {
            isKeyRight = true;
        }
        // [점프 기능] J 또는 j 키를 입력 받았을 때 (엎드린 상태가 아닐 때만 가능) [cite: 112]
        else if ((wParam == 'J' || wParam == 'j') && !isSlide) {
            // 위쪽 화살표 키나 점프 플래그가 꺼져 있을 때만 점프 개시
            if (!isJumping) {
                isJumping = true;
                jumpVelocity = 24; // 초기 점프 추진력
            }
        }
        // [엎드리기 기능] 아래쪽 화살표 키 혹은 다운 트리거 시 납작해짐 [cite: 112]
        else if (wParam == VK_DOWN) {
            if (!isJumping) { // 점프 중이 아닐 때만 엎드리기 가능
                isSlide = true;
            }
        }
        break;

    case WM_KEYUP:
        if (wParam == VK_LEFT) {
            isKeyLeft = false;
        }
        else if (wParam == VK_RIGHT) {
            isKeyRight = false;
        }
        else if (wParam == VK_DOWN) {
            isSlide = false; // 아래 키를 떼면 다시 일어섬 [cite: 112]
        }
        break;

    case WM_TIMER: {
        if (wParam == 1) {
            // 1. 배경 무한 스크롤 처리 (하늘은 빠르게 -6px, 땅은 천천히 -2px) 
            skyOffset -= 6;
            if (skyOffset <= -wide) skyOffset = 0;

            groundOffset -= 2;
            if (groundOffset <= -wide) groundOffset = 0;

            // 2. 캐릭터 애니메이션 프레임 업데이트 [cite: 114]
            if (!isSlide) {
                c.fream = (c.fream + 1) % 7; // 달리기 프레임 (0~6)
            }
            else {
                c.fream = 7; // 엎드리기 프레임 고정 [cite: 112]
            }

            // 3. 캐릭터 이동 처리 (실시간 비동기 조작 입력 반영) [cite: 111]
            if (isKeyLeft)  c.x -= c.speed;
            if (isKeyRight) c.x += c.speed;

            // 화면 밖 무단 이탈 방지 벽 스크립트
            if (c.x < 0) c.x = 0;
            if (c.x > wide - 80) c.x = wide - 80;

            // 4. 점프 물리 로직 갱신 (가상 중력 가속도 반영)
            if (isJumping) {
                jumpY += jumpVelocity;
                jumpVelocity -= 2; // 중력 가속도 수치 차감

                if (jumpY <= 0) { // 지면 도달 시 점프 안전 해제
                    jumpY = 0;
                    jumpVelocity = 0;
                    isJumping = false;
                }
            }

            // 5. 몬스터 이동, 애니메이션 갱신 및 화면 끝 리스폰 처리 [cite: 107, 109, 114]
            for (int i = 0; i < 10; ++i) {
                if (mon[i].see) {
                    if (!mon[i].isExplode) {
                        // 정상 상태: 우측으로 계속 전진 이동 [cite: 107]
                        mon[i].x += mon[i].speed;

                        // 타입별 애니메이션 프레임 순환 (각 3프레임씩 쪼개어 연동) [cite: 114]
                        if (mon[i].type == 0) {
                            mon[i].fream = (mon[i].fream + 1) % 3;       // 바닥 몬스터: 0 -> 1 -> 2
                        }
                        else {
                            mon[i].fream = 3 + ((mon[i].fream - 3 + 1) % 3); // 공중 몬스터: 3 -> 4 -> 5
                        }

                        // 화면 우측 경계를 완전히 넘어가면 다시 왼쪽에서 리스폰 스폰 [cite: 109]
                        if (mon[i].x > wide + 50) {
                            mon[i].see = false;
                        }
                    }
                    else {
                        // 폭발 진행 상태: 이동을 멈추고 이펙트 카운트 업 
                        mon[i].explodeFrame++;
                        if (mon[i].explodeFrame > 4) {
                            mon[i].see = false; // 이펙트가 끝나면 몬스터 소멸
                        }
                    }
                }
            }

            // 6. 새로운 몬스터 무작위 등장 스케줄러 (일정 확률로 휴면 상태인 몬스터 가동) [cite: 107]
            if (rand() % 40 == 0) {
                for (int i = 0; i < 10; ++i) {
                    if (!mon[i].see) {
                        mon[i].see = true;
                        mon[i].isExplode = false;
                        mon[i].explodeFrame = 0;
                        mon[i].x = -80; // 화면 왼쪽 구석 바깥 스폰 
                        mon[i].speed = speedDist(gen);
                        mon[i].type = typeDist(gen); // 바닥 혹은 공중 무작위 성격 부여 

                        if (mon[i].type == 0) {
                            mon[i].y = height * 2 / 3 + 40; // 지면 바닥 밀착 좌표 
                            mon[i].fream = 0;
                        }
                        else {
                            mon[i].y = height * 2 / 3 - 30; // 공중 부유 좌표 
                            mon[i].fream = 3;
                        }
                        break; // 한 번에 한 마리씩만 깨우고 탈출
                    }
                }
            }

            // 7. 실시간 박스 충돌 체크 처리 (주인공 vs 모든 활성화된 몬스터) [cite: 110, 115]
            int cxLeft = c.x + 15;
            int cxRight = c.x + 65;
            // 점프 및 슬라이딩 상태에 의거하여 정밀한 실시간 히트박스 영역 가공
            int cyTop = (c.y - jumpY) + (isSlide ? 50 : 10);
            int cyBottom = (c.y - jumpY) + 80;

            for (int i = 0; i < 10; ++i) {
                if (mon[i].see && !mon[i].isExplode) {
                    int mxLeft = mon[i].x + 10;
                    int mxRight = mon[i].x + 70;
                    int myTop = mon[i].y + 10;
                    int myBottom = mon[i].y + 70;

                    // AABB 사각형 영역 충돌 중첩 체크 검사
                    if (cxRight > mxLeft && cxLeft < mxRight && cyBottom > myTop && cyTop < myBottom) {
                        mon[i].isExplode = true; // 폭발 모드 셋업 
                        mon[i].explodeFrame = 0;

                        // 기능 추가: 충돌 시 피격 체감용 캐릭터 뒤로 살짝 넉백 효과
                        c.x += 40;
                    }
                }
            }
        }

        InvalidateRect(hWnd, NULL, false);
        break;
    }
    case WM_PAINT: {
        hDC = BeginPaint(hWnd, &ps);

        // 더블 버퍼링 기법 캔버스 세팅
        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, wide, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        // 레이어 계층 순서 보장을 위한 통합 그리기 제어 단일 호출
        Draw(memDC);

        // 최종 완성본 도화지 백버퍼에서 프론트 hDC로 일괄 전송 고속 복사
        BitBlt(hDC, 0, 0, wide, height, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_SIZE:
        height = HIWORD(lParam);
        wide = LOWORD(lParam);
        InvalidateRect(hWnd, NULL, true);
        break;
    case WM_DESTROY:
        // 프로그램 종료 시 로드되었던 모든 비트맵 커널 오브젝트 자원 안전 해제
        DeleteObject(g_Sky);
        DeleteObject(g_Ground);
        for (int i = 0; i < 8; i++) DeleteObject(g_ch[i]);
        for (int i = 0; i < 6; i++) DeleteObject(g_Monster[i]);

        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void ResetMap()
{
    // 주인공 배치 초기값 (화면 우측 배치 성격 유지) [cite: 110]
    c.x = wide * 45 / 60;
    c.y = height * 2 / 3;
    c.fream = 0;
    c.speed = 12;

    // 점프 및 행동 상태 플래그 완전 초기 청소
    isJumping = false;
    jumpY = 0;
    jumpVelocity = 0;
    isSlide = false;
    isKeyLeft = false;
    isKeyRight = false;
    skyOffset = 0;
    groundOffset = 0;

    // 풀링 객체 배열 비활성 클리어
    for (int i = 0; i < 10; ++i) {
        mon[i].see = false;
        mon[i].isExplode = false;
        mon[i].explodeFrame = 0;
        mon[i].speed = speedDist(gen);
        mon[i].x = -100;
        mon[i].y = height * 2 / 3;
    }
}

void Draw(HDC memDC)
{
    // 레이어 백그라운드 우선순위 계층 순차 드로잉 제어 
    DrawSky(memDC);    // Layer 1: 원경 하늘 배경 
    DrawGround(memDC); // Layer 2: 근경 땅 지면 배경 
    DrawMon(memDC);    // Layer 3: 필드 서식 크리처 [cite: 114]
    DrawCh(memDC);     // Layer 4: 최종 최상단 플레이어 액터 [cite: 114]
}

void DrawSky(HDC memDC)
{
    if (g_Sky == NULL) return;
    HDC imgDC = CreateCompatibleDC(memDC);
    SelectObject(imgDC, g_Sky);
    GetObject(g_Sky, sizeof(BITMAP), &bmpInfo);

    // [무한 스크롤 스크립트] 스카이 원경 파트 좌측 이동분 2장 이어붙이기 가공 
    StretchBlt(memDC, skyOffset, 0, wide, height, imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
    StretchBlt(memDC, skyOffset + wide, 0, wide, height, imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);

    DeleteDC(imgDC);
}

void DrawGround(HDC memDC)
{
    if (g_Ground == NULL) return;
    HDC imgDC = CreateCompatibleDC(memDC);
    SelectObject(imgDC, g_Ground);
    GetObject(g_Ground, sizeof(BITMAP), &bmpInfo);

    // [무한 스크롤 스크립트] 그라운드 근경 파트 좌측 이동분 2장 이어붙이기 가공 (마젠타 혹은 좌상단 픽셀 투명 처리) 
    COLORREF transColor = GetPixel(imgDC, 0, 0);
    TransparentBlt(memDC, groundOffset, 0, wide, height, imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, transColor);
    TransparentBlt(memDC, groundOffset + wide, 0, wide, height, imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, transColor);

    DeleteDC(imgDC);
}

void DrawCh(HDC memDC)
{
    if (g_ch[c.fream] == NULL) return;
    HDC imgDC = CreateCompatibleDC(memDC);
    SelectObject(imgDC, g_ch[c.fream]);
    GetObject(g_ch[c.fream], sizeof(BITMAP), &bmpInfo);

    COLORREF transColor = GetPixel(imgDC, 0, 0);

    // 슬라이딩 모드 토글에 따라 세로 길이를 압축 스케일링 가공 출력 (80x80 -> 엎드릴 시 80x40 구조) [cite: 112]
    int renderHeight = isSlide ? 40 : 80;
    int renderY = isSlide ? (c.y - jumpY + 40) : (c.y - jumpY);

    TransparentBlt(memDC, c.x, renderY, 80, renderHeight,
        imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, transColor);

    DeleteDC(imgDC);
}

void DrawMon(HDC memDC)
{
    HDC imgDC = CreateCompatibleDC(memDC);

    for (int i = 0; i < 10; ++i) {
        if (mon[i].see) {
            if (!mon[i].isExplode) {
                // 정상 주행 패턴 렌더링 [cite: 114]
                SelectObject(imgDC, g_Monster[mon[i].fream]);
                GetObject(g_Monster[mon[i].fream], sizeof(BITMAP), &bmpInfo);
                COLORREF transColor = GetPixel(imgDC, 0, 0);

                TransparentBlt(memDC, mon[i].x, mon[i].y, 80, 80,
                    imgDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, transColor);
            }
            else {
                // 부딪혔을 때 발생하는 폭발 애니메이션 GDI 벡터 효과 기법 
                // 붉은 계열의 파편 스파크가 원형 방사형태로 유동적으로 뻗어나가는 구조
                HBRUSH expBrush = CreateSolidBrush(RGB(255, 50 * mon[i].explodeFrame, 0));
                HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, expBrush);

                int radius = mon[i].explodeFrame * 15; // 프레임 진행에 따라 폭발 반경 확산
                Ellipse(memDC, mon[i].x + 40 - radius, mon[i].y + 40 - radius, mon[i].x + 40 + radius, mon[i].y + 40 + radius);

                SelectObject(memDC, oldBrush);
                DeleteObject(expBrush);
            }
        }
    }
    DeleteDC(imgDC);
}