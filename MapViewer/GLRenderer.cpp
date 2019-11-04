#include "GLRenderer.h"
#include "Utils.h"
#include "ogshader.h"
#include "IOGMatrix.h"
#include "ogcamera.h"
#include "ogvertexbuffers.h"
#include <vector>
#include <map>


HDC g_hDC;
HGLRC g_hRC;
unsigned int g_VertShader = -1;
unsigned int g_FragShader = -1;
unsigned int g_ProgId = -1;
unsigned int g_MVPMatrixLoc = -1;
unsigned int g_MeshColorLoc = -1;
OGMatrix g_mProjection;
OGMatrix g_mView;
OGMatrix g_mMV;
OGMatrix g_mMVP;
COGCamera g_Camera;

OGVec3 g_TerrainColor = OGVec3(0.0f, 0.8f, 0.0f);
OGVec3 g_WaterColor = OGVec3(0.0f, 0.5f, 1.0f);

float g_dist = 14200.0f;
float g_baseX = -4000.0f;
float g_baseY = -4000.0f;


struct TileGeometry
{
	OGMatrix mTilePosition;
	OGMatrix mWorld;

	std::vector<COGVertexBuffers*> TerrainMeshes;
	std::vector<COGVertexBuffers*> WaterMeshes;
};


struct TileZoomLevel
{
	int ZoomLevel;
	OGMatrix mTileScale;
	std::vector<TileGeometry> Tiles;
};

struct ZoomLevelConfig
{
	int TilesInRow = 0;
	float Scale = 0.0f;
};

std::map<int, ZoomLevelConfig> g_Zoom2TileConfig = { {13, {1, 1.0f}}, {14, {2, 0.5f}}, {15, {4, 0.25f}} };

std::map<int, TileZoomLevel> g_ZoomLevels;



void InitRenderer(HWND _hWnd, int _ScrWidth, int _ScrHeight)
{
	GLuint PixelFormat;
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 16;
	pfd.cDepthBits = 16;
	g_hDC = GetDC(_hWnd);
	PixelFormat = ChoosePixelFormat(g_hDC, &pfd);
	SetPixelFormat(g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext(g_hDC);
	wglMakeCurrent(g_hDC, g_hRC);
	glewInit();

	std::string basePath = GetResourcePath() + std::string("/../assets/shaders/");
	if (ShaderLoadFromFile(basePath + std::string("model.vsh"), GL_VERTEX_SHADER, &g_VertShader) == false)
	{
		return;
	}
	if (ShaderLoadFromFile(basePath + std::string("model.fsh"), GL_FRAGMENT_SHADER, &g_FragShader) == false)
	{
		return;
	}

	const char* pszAttribs[] = { "inVertex", "inNormal" };
	if (CreateProgram(&g_ProgId, g_VertShader, g_FragShader, pszAttribs, 2) == false)
	{
		return;
	}

	g_MVPMatrixLoc = glGetUniformLocation(g_ProgId, "MVPMatrix");
	g_MeshColorLoc = glGetUniformLocation(g_ProgId, "MeshColor");

	MatrixPerspectiveFovRH(g_mProjection, 0.67f, float(_ScrWidth) / float(_ScrHeight), 1.0f, 50000.0f, false);

	// Camera setup
	OGVec3 vDir = OGVec3(0.0f, 0.0f, 1.0f);
	OGVec3 vTarget = OGVec3(0.0f, 0.0f, g_dist * -1.0f);
	OGVec3 vPos = vTarget + (vDir * g_dist);
	OGVec3 vUp = vDir.cross(OGVec3(1.0f, 0.0f, 0.0f));
	g_Camera.Setup(vPos, vTarget, vUp);
	g_Camera.SetupViewport(g_mProjection);
	g_Camera.Update();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glViewport(0, 0, _ScrWidth, _ScrHeight);
	glDisable(GL_CULL_FACE);
}


void DestroyRenderer()
{
	for (auto& z : g_ZoomLevels)
	{
		for (auto& t : z.second.Tiles)
		{
			for (auto& m : t.WaterMeshes)
			{
				delete m;
			}
			t.WaterMeshes.clear();

			for (auto& m : t.TerrainMeshes)
			{
				delete m;
			}
			t.TerrainMeshes.clear();
		}
	}

	glDeleteProgram(g_ProgId);
	glDeleteShader(g_VertShader);
	glDeleteShader(g_FragShader);

	wglDeleteContext(g_hRC);
}


void AddMesh(int _ZoomLevel, int _TileX, int _TileY, MeshTypes _Type, COGVertexBuffers* _Mesh)
{
	int tilesInRow = 0;
	auto zl = g_Zoom2TileConfig.find(_ZoomLevel);
	if (zl == g_Zoom2TileConfig.end())
	{
		// Unknown/unsupported zoom level
		return;
	}
	else
		tilesInRow = zl->second.TilesInRow;

	if (g_ZoomLevels.find(_ZoomLevel) == g_ZoomLevels.end())
	{
		// First time processing this zoom level, pre-allocate tiles array
		auto& newLevel = g_ZoomLevels[_ZoomLevel];
		newLevel.ZoomLevel = _ZoomLevel;
		newLevel.Tiles.resize(tilesInRow * tilesInRow);
		MatrixScaling(newLevel.mTileScale, zl->second.Scale, zl->second.Scale, 1.0f);
		
		for (int y = 0; y < tilesInRow; ++y)
		{
			for (int x = 0; x < tilesInRow; ++x)
			{
				auto& tile = newLevel.Tiles.at(tilesInRow * x + y);
				MatrixTranslation(tile.mTilePosition, g_baseX + x * g_baseX, g_baseY + y * g_baseY, g_dist * -1.0f);
				MatrixMultiply(tile.mWorld, tile.mTilePosition, newLevel.mTileScale);
			}
		}
	}
	auto& curLevel = g_ZoomLevels[_ZoomLevel];
	auto& curTile = curLevel.Tiles.at(tilesInRow * _TileX + _TileY);

	switch (_Type)
	{
	case WATER: curTile.WaterMeshes.push_back(_Mesh); break;
	case TERRAIN: curTile.TerrainMeshes.push_back(_Mesh); break;
	}
}


void RenderFrame()
{
	glClearColor(0.0f, 0.1f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_mView = g_Camera.GetViewMatrix();

	glUseProgram(g_ProgId);

	for (auto t : g_ZoomLevels[13].Tiles)
	{
		MatrixMultiply(g_mMV, t.mTilePosition, g_mView);
		MatrixMultiply(g_mMVP, g_mMV, g_mProjection);
		glUniformMatrix4fv(g_MVPMatrixLoc, 1, GL_FALSE, g_mMVP.f);

		for (auto m : t.TerrainMeshes)
		{
			glUniform3fv(g_MeshColorLoc, 1, g_TerrainColor.ptr());
			m->Apply();
			m->Render();
		}

		for (auto m : t.WaterMeshes)
		{
			glUniform3fv(g_MeshColorLoc, 1, g_WaterColor.ptr());
			m->Apply();
			m->Render();
		}
	}

	SwapBuffers(g_hDC);
}
