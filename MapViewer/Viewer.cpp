#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string>
#include <glew.h>
#include <wglew.h>

#include "VTZeroRead.h"
#include "Tesselator.h"
#include "ElevationMap.h"
#include "MeshSubdivision.h"
#include "MeshConstructor.h"
#include "GLRenderer.h"
#include "Utils.h"

HINSTANCE shInstance = NULL;
HWND shWnd = NULL;

#define TIMER_ID	1
#define TIMER_RATE	30

int ScrWidth = 800, ScrHeight = 600;


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
		if (wParam == VK_LEFT)
		{
		}
		else if (wParam == VK_RIGHT)
		{
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
	wc.hIcon = NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
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
int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	std::string strPath = GetResourcePath() + std::string("/../assets/");

	std::vector<float> elevationMap;
	unsigned int extents = 0;
	LoadTerrariumElevationMap(strPath + std::string("dem/dem_13_4236_2917.png"), extents, elevationMap);
	//LoadTerrariumElevationMap(strPath + std::string("dem/dem_14_8472_5835.png"), extents, elevationMap);

	Tile t;
	ReadTile(strPath + std::string("mvt/mvt_13_4236_2917.mvt"), t);
	//ReadTile(strPath + std::string("mvt/mvt_14_8472_5835.mvt"), t);

	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

#if 0
	std::vector<unsigned int> sampleIndices = { 3, 0, 1, 1, 2, 3 };
	std::vector<float> sampleVertices = { 8000.0f, 0.0f,   8000.0f, 8000.0f,   0.0f, 8000.0f,    0.0f, 0.0f};
	std::vector<unsigned int> outIndices;
	std::vector<float> outVertices;
	SubdivideMesh(sampleIndices, sampleVertices, 50.0f, outIndices, outVertices);
	COGVertexBuffers* newMesh = new COGVertexBuffers();
	ConstructMesh(outIndices, outVertices, *newMesh);
	//ConstructMesh(sampleIndices, sampleVertices, *newMesh);
	AddMesh(TERRAIN, newMesh);
#endif
	std::map<std::string, MeshTypes> allowedTypes = { {"water", WATER}, {"earth", TERRAIN} };

	for (auto l : t.m_Layers)
	{
		auto type = allowedTypes.find(l.m_Name);
		if (type != allowedTypes.end())
		{
			for (auto f : l.m_Features)
			{
				if (!f.m_Rings.empty())
				{
					for (auto r : f.m_Rings)
					{
						std::vector<float> verts2D;
						std::vector<uint32_t> indices;
						TesselateRing(r, indices, verts2D);

						std::vector<float> verts2DFine;
						std::vector<uint32_t> indicesFine;

						std::vector<uint32_t>* pIndicesIn = &indices;
						std::vector<float>* pVerticesIn = &verts2D;
						std::vector<uint32_t>* pIndicesOut = &indicesFine;
						std::vector<float>* pVerticesOut = &verts2DFine;
						bool needSubdivision = true;
						while (needSubdivision)
						{
							needSubdivision = SubdivideMesh(*pIndicesIn, *pVerticesIn, 500.0f, *pIndicesOut, *pVerticesOut);
							if (needSubdivision)
							{
								std::swap(pIndicesIn, pIndicesOut);
								std::swap(pVerticesIn, pVerticesOut);
								pIndicesOut->clear();
								pVerticesOut->clear();
							}
							else
							{
								COGVertexBuffers* newMesh = new COGVertexBuffers();
								ConstructMesh(*pIndicesIn, *pVerticesIn, elevationMap, *newMesh);
								AddMesh(13, 0, 0, type->second, newMesh);
								break;
							}
						}
					}
				}
			}
		}
	}

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
