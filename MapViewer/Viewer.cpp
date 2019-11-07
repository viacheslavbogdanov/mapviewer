#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string>
#include <glew.h>
#include <wglew.h>
#include <sstream>

#include "VTZeroRead.h"
#include "Tesselator.h"
#include "ElevationMap.h"
#include "MeshSubdivision.h"
#include "MeshConstructor.h"
#include "GLRenderer.h"
#include "Scene.h"
#include "Utils.h"

HINSTANCE shInstance = NULL;
HWND shWnd = NULL;

const int TIMER_ID = 1;
const int TIMER_RATE = 30;

int ScrWidth = 800, ScrHeight = 800;

Scene g_Scene;


/// Application shutdown.
void Shutdown()
{
    PostQuitMessage(0);
    DestroyRenderer();
}


/// Application window procedure.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        Shutdown();
        break;

    case WM_KEYDOWN:
        if (wParam == '1')
        {
            SelectZoomLevel(12);
        }
        else if (wParam == '2')
        {
            SelectZoomLevel(13);
        }
        else if (wParam == '3')
        {
            SelectZoomLevel(14);
        }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


/// timer callback function
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    if (idEvent == TIMER_ID)
    {
        RenderFrame();
    }
}


/// Application window initialization.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    shInstance = hInstance;
    shWnd = FindWindowW(L"Viewer.MainWindow", L"Viewer");

    if (shWnd)
    {
        SetForegroundWindow((HWND)(((__int64)shWnd) | 0x01));
        return FALSE;
    }

    WNDCLASS	wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"Viewer.MainWindow";
    RegisterClass(&wc);

    int wndSizeX = ScrWidth + (GetSystemMetrics(SM_CXBORDER) * 2);
    int wndSizeY = ScrHeight + GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYBORDER);

    shWnd = CreateWindowW(L"Viewer.MainWindow", L"Viewer", WS_SYSMENU | WS_OVERLAPPED,
        (GetSystemMetrics(SM_CXSCREEN) - wndSizeX) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - wndSizeY) / 2,
        wndSizeX, wndSizeY, NULL, NULL, hInstance, NULL);
    if (!shWnd)
        return FALSE;

    if (SetTimer(shWnd, TIMER_ID, TIMER_RATE, (TIMERPROC)TimerProc) != TIMER_ID)
        return FALSE;

    ShowWindow(shWnd, nCmdShow);
    UpdateWindow(shWnd);

    InitRenderer(shWnd, ScrWidth, ScrHeight);

    return TRUE;
}


/// main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    std::string strPath = GetResourcePath() + std::string("/assets/");

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    g_Scene.Load(strPath);

    LoadSceneData(g_Scene.GetData());

    SelectZoomLevel(12);

    MSG msg;
    while (1)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            WaitMessage();
        }
    }

    return (int)msg.wParam;
}
