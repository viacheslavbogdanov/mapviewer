#pragma once
#include <glew.h>
#include <wglew.h>
#include <ogvertexbuffers.h>

enum MeshTypes
{
	TERRAIN,
	WATER,
};

void InitRenderer(HWND _hWnd, int _ScrWidth, int _ScrHeight);
void DestroyRenderer();
void RenderFrame();
void AddMesh(MeshTypes _Type, COGVertexBuffers* _Mesh);
