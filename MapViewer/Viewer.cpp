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
#include "Utils.h"

HINSTANCE shInstance = NULL;
HWND shWnd = NULL;

#define TIMER_ID	1
#define TIMER_RATE	30

int ScrWidth = 800, ScrHeight = 800;


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


struct ZoomLevelConfig
{
	int ZoomLevel = -1;
	int TilesInRow = -1;
	std::vector<std::pair<int, int> > TileCoords;
};


std::vector<ZoomLevelConfig> g_ZoomLevelConfigs;


/// main function
int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	std::string strPath = GetResourcePath() + std::string("/assets/");

	std::map<std::string, MeshTypes> allowedTypes = { {"water", WATER}, {"earth", TERRAIN}/*, {"buildings", LANDUSE}*/ };

	g_ZoomLevelConfigs.push_back(ZoomLevelConfig());
	g_ZoomLevelConfigs.at(0).ZoomLevel = 12;
	g_ZoomLevelConfigs.at(0).TilesInRow = 1;
	g_ZoomLevelConfigs.at(0).TileCoords = { {2118, 1458} };

	g_ZoomLevelConfigs.push_back(ZoomLevelConfig());
	g_ZoomLevelConfigs.at(1).ZoomLevel = 13;
	g_ZoomLevelConfigs.at(1).TilesInRow = 2;
	g_ZoomLevelConfigs.at(1).TileCoords = { {4236, 2916}, {4236, 2917}, {4237, 2916}, {4237, 2917} };
	
	g_ZoomLevelConfigs.push_back(ZoomLevelConfig());
	g_ZoomLevelConfigs.at(2).ZoomLevel = 14;
	g_ZoomLevelConfigs.at(2).TilesInRow = 4;
	g_ZoomLevelConfigs.at(2).TileCoords 
		= { {8472, 5832}, {8472, 5833}, {8472, 5834}, {8472, 5835},
			{8473, 5832}, {8473, 5833}, {8473, 5834}, {8473, 5835},
			{8474, 5832}, {8474, 5833}, {8474, 5834}, {8474, 5835},
			{8475, 5832}, {8475, 5833}, {8475, 5834}, {8475, 5835} };

	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	for (auto zoomLevelCfg : g_ZoomLevelConfigs)
	{
		for (size_t tileCfgId = 0; tileCfgId < zoomLevelCfg.TileCoords.size(); ++tileCfgId)
		{
			std::stringstream DemFileStr;
			DemFileStr << "dem/dem_" << zoomLevelCfg.ZoomLevel << "_" << 
				zoomLevelCfg.TileCoords[tileCfgId].first << "_" << zoomLevelCfg.TileCoords[tileCfgId].second << ".png";
			std::vector<float> elevationMap;
			unsigned int extents = 0;
			LoadTerrariumElevationMap(strPath + DemFileStr.str(), extents, elevationMap);

			std::stringstream MvtFileStr;
			MvtFileStr << "mvt/mvt_" << zoomLevelCfg.ZoomLevel << "_" <<
				zoomLevelCfg.TileCoords[tileCfgId].first << "_" << zoomLevelCfg.TileCoords[tileCfgId].second << ".mvt";
			Tile t;
			ReadTile(strPath + MvtFileStr.str(), t);

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
										ConstructMesh(zoomLevelCfg.ZoomLevel, *pIndicesIn, *pVerticesIn, elevationMap, *newMesh);
										int y = tileCfgId / zoomLevelCfg.TilesInRow;
										int x = tileCfgId % zoomLevelCfg.TilesInRow;
										AddMesh(zoomLevelCfg.ZoomLevel, x, y, type->second, newMesh);
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

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
