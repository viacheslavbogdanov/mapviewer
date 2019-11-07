#pragma once
#include <glew.h>
#include <wglew.h>
#include <ogvertexbuffers.h>
#include "Scene.h"

void InitRenderer(HWND _hWnd, int _ScrWidth, int _ScrHeight);
void DestroyRenderer();
void RenderFrame();

void LoadSceneData(const SceneMeshes& _SceneData);

void SelectZoomLevel(int _ZoomLevel);
