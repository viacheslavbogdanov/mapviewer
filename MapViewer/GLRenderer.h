#pragma once
#include <glew.h>
#include <wglew.h>
#include <ogvertexbuffers.h>

enum MeshTypes
{
	TERRAIN,
	WATER,
	LANDUSE,
};

void InitRenderer(HWND _hWnd, int _ScrWidth, int _ScrHeight);
void DestroyRenderer();
void RenderFrame();

void AddMesh(int _ZoomLevel, int _TileX, int _TileY, MeshTypes _Type, COGVertexBuffers* _Mesh);

void SelectZoomLevel(int _ZoomLevel);
