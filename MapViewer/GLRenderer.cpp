#include "GLRenderer.h"
#include "Utils.h"
#include "ogshader.h"
#include "IOGMatrix.h"
#include "ogcamera.h"
#include "ogvertexbuffers.h"
#include <vector>

HDC g_hDC;
HGLRC g_hRC;
unsigned int g_VertShader = -1;
unsigned int g_FragShader = -1;
unsigned int g_ProgId = -1;
unsigned int g_MVPMatrixLoc = -1;
unsigned int g_MeshColorLoc = -1;
OGMatrix g_mProjection;
OGMatrix g_mView;
OGMatrix g_mWorld;
OGMatrix g_mMV;
OGMatrix g_mMVP;
COGCamera g_Camera;

OGVec3 g_TerrainColor = OGVec3(0.0f, 0.8f, 0.0f);
OGVec3 g_WaterColor = OGVec3(0.0f, 0.5f, 1.0f);

std::vector<COGVertexBuffers*> g_TerrainMeshes;
std::vector<COGVertexBuffers*> g_WaterMeshes;


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

	float dist = 14200.0f;

	OGVec3 vDir = OGVec3(0.0f, 0.0f, 1.0f);
	OGVec3 vTarget = OGVec3(0.0f, 0.0f, dist * -1.0f);
	OGVec3 vPos = vTarget + (vDir * dist);
	OGVec3 vUp = vDir.cross(OGVec3(1.0f, 0.0f, 0.0f));

	g_Camera.Setup(vPos, vTarget, vUp);
	g_Camera.SetupViewport(g_mProjection);
	g_Camera.Update();

	MatrixTranslation(g_mWorld, -4000.0f, -4000.0f, dist * -1.0f);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glViewport(0, 0, _ScrWidth, _ScrHeight);
	glDisable(GL_CULL_FACE);
}


void DestroyRenderer()
{
	for (auto m : g_WaterMeshes)
	{
		delete m;
	}
	g_WaterMeshes.clear();

	for (auto m : g_TerrainMeshes)
	{
		delete m;
	}
	g_TerrainMeshes.clear();

	glDeleteProgram(g_ProgId);
	glDeleteShader(g_VertShader);
	glDeleteShader(g_FragShader);

	wglDeleteContext(g_hRC);
}


void AddMesh(MeshTypes _Type, COGVertexBuffers* _Mesh)
{
	switch (_Type)
	{
	case WATER: g_WaterMeshes.push_back(_Mesh); break;
	case TERRAIN: g_TerrainMeshes.push_back(_Mesh); break;
	}
}


void RenderFrame()
{
	glClearColor(0.0f, 0.1f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_mView = g_Camera.GetViewMatrix();

	glUseProgram(g_ProgId);

	MatrixMultiply(g_mMV, g_mWorld, g_mView);
	MatrixMultiply(g_mMVP, g_mMV, g_mProjection);
	glUniformMatrix4fv(g_MVPMatrixLoc, 1, GL_FALSE, g_mMVP.f);

	for (auto m : g_TerrainMeshes)
	{
		glUniform3fv(g_MeshColorLoc, 1, g_TerrainColor.ptr());
		m->Apply();
		m->Render();
	}

	for (auto m : g_WaterMeshes)
	{
		glUniform3fv(g_MeshColorLoc, 1, g_WaterColor.ptr());
		m->Apply();
		m->Render();
	}

	SwapBuffers(g_hDC);
}
