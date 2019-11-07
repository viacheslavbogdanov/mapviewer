#include "GLRenderer.h"
#include "Utils.h"
#include "ogshader.h"
#include "IOGMatrix.h"
#include "ogcamera.h"
#include "ogvertexbuffers.h"
#include "Scene.h"
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

const OGVec3 g_TerrainColor = OGVec3(0.0f, 0.8f, 0.0f);
const OGVec3 g_WaterColor = OGVec3(0.0f, 0.5f, 1.0f);
const OGVec3 g_LanduseColor = OGVec3(0.0f, 0.5f, 0.0f);
const int g_TileLength = 8192;


struct TileGeometry
{
    OGMatrix mTilePosition;
    OGMatrix mWorld;

    std::vector<COGVertexBuffers*> TerrainMeshes;
    std::vector<COGVertexBuffers*> WaterMeshes;
    std::vector<COGVertexBuffers*> LanduseMeshes;
};


struct TileZoomLevel
{
    int ZoomLevel;
    OGMatrix mTileScale;
    float CameraDistance = 0.0f;
    std::vector<TileGeometry> Tiles;
};


std::map<int, TileZoomLevel> g_ZoomLevels;

int g_SelectedZoomLevel = 12;


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

    std::string basePath = GetResourcePath() + std::string("/assets/shaders/");
    if (ShaderLoadFromFile(basePath + std::string("model.vsh"), GL_VERTEX_SHADER, &g_VertShader) == false)
    {
        // TODO: better error handling here and further
        ::MessageBoxA(NULL, "Assets folder was not found", "Error", 0);
        return;
    }
    if (ShaderLoadFromFile(basePath + std::string("model.fsh"), GL_FRAGMENT_SHADER, &g_FragShader) == false)
    {
        ::MessageBoxA(NULL, "Assets folder was not found", "Error", 0);
        return;
    }

    const char* pszAttribs[] = { "inVertex", "inNormal" };
    if (CreateProgram(&g_ProgId, g_VertShader, g_FragShader, pszAttribs, 2) == false)
    {
        ::MessageBoxA(NULL, "GL Program link error", "Error", 0);
        return;
    }

    g_MVPMatrixLoc = glGetUniformLocation(g_ProgId, "MVPMatrix");
    g_MeshColorLoc = glGetUniformLocation(g_ProgId, "MeshColor");

    MatrixPerspectiveFovRH(g_mProjection, 0.67f, float(_ScrWidth) / float(_ScrHeight), 1.0f, 50000.0f, false);

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

            for (auto& m : t.LanduseMeshes)
            {
                delete m;
            }
            t.LanduseMeshes.clear();
        }
    }

    glDeleteProgram(g_ProgId);
    glDeleteShader(g_VertShader);
    glDeleteShader(g_FragShader);

    wglDeleteContext(g_hRC);
}


void LoadSceneData(const SceneMeshes& _SceneData)
{
    // positioning offsets
    // TODO: either calculate them on the flight or move to tile config
    std::map<int, float> TileOffsets = { {12, -1.0f * g_TileLength / 2}, {13, -1.0f * g_TileLength}, {14, -1.0f * g_TileLength - g_TileLength} };

    for (auto l : _SceneData.ZoomLevels)
    {
        if (g_ZoomLevels.find(l.first) == g_ZoomLevels.end())
        {
            // First time processing this zoom level, pre-allocate tiles array
            auto& newLevel = g_ZoomLevels[l.first];
            newLevel.ZoomLevel = l.first;
            newLevel.Tiles.resize(l.second.TilesInRow * l.second.TilesInRow);
            MatrixScaling(newLevel.mTileScale, 1.0f, 1.0f, 1.0f);

            newLevel.CameraDistance = ((g_TileLength * l.second.TilesInRow) * 0.5f) / tanf(0.67f * 0.5f);

            float fOffset = TileOffsets[l.first];

            for (int y = 0; y < l.second.TilesInRow; ++y)
            {
                for (int x = 0; x < l.second.TilesInRow; ++x)
                {
                    auto& tile = newLevel.Tiles.at(l.second.TilesInRow * y + x);
                    MatrixTranslation(tile.mTilePosition, x * g_TileLength + fOffset, y * g_TileLength + fOffset, newLevel.CameraDistance * -1.0f);
                    MatrixMultiply(tile.mWorld, tile.mTilePosition, newLevel.mTileScale);
                }
            }
        }

        // fill meshes (GL index+vertex buffs) for the appropriate tiles
        auto& curLevel = g_ZoomLevels[l.first];
        for (auto t : l.second.Tiles)
        {
            auto& curTile = curLevel.Tiles.at(l.second.TilesInRow * t.TileY + t.TileX);
            for (auto mt : t.TerrainMeshes)
            {
                COGVertexBuffers* pNewMesh = new COGVertexBuffers();
                pNewMesh->Fill(mt.Vertices.data(), mt.Vertices.size() / 6, mt.Indices.size() / 3, sizeof(float) * 6, mt.Indices.data(), mt.Indices.size());
                curTile.TerrainMeshes.push_back(pNewMesh);
            }
            for (auto mt : t.WaterMeshes)
            {
                COGVertexBuffers* pNewMesh = new COGVertexBuffers();
                pNewMesh->Fill(mt.Vertices.data(), mt.Vertices.size() / 6, mt.Indices.size() / 3, sizeof(float) * 6, mt.Indices.data(), mt.Indices.size());
                curTile.WaterMeshes.push_back(pNewMesh);
            }
            for (auto mt : t.LanduseMeshes)
            {
                COGVertexBuffers* pNewMesh = new COGVertexBuffers();
                pNewMesh->Fill(mt.Vertices.data(), mt.Vertices.size() / 6, mt.Indices.size() / 3, sizeof(float) * 6, mt.Indices.data(), mt.Indices.size());
                curTile.LanduseMeshes.push_back(pNewMesh);
            }
        }
    }
}


void SelectZoomLevel(int _ZoomLevel)
{
    auto zoomLevel = g_ZoomLevels.find(_ZoomLevel);
    if (zoomLevel == g_ZoomLevels.end())
        return;

    g_SelectedZoomLevel = _ZoomLevel;

    // Camera setup
    OGVec3 vDir = OGVec3(0.0f, 0.0f, 1.0f);
    OGVec3 vTarget = OGVec3(0.0f, 0.0f, zoomLevel->second.CameraDistance * -1.0f);
    OGVec3 vPos = vTarget + (vDir * zoomLevel->second.CameraDistance);
    OGVec3 vUp = vDir.cross(OGVec3(1.0f, 0.0f, 0.0f));
    g_Camera.Setup(vPos, vTarget, vUp);
    g_Camera.SetupViewport(g_mProjection);
    g_Camera.Update();
}


void RenderFrame()
{
    glClearColor(0.0f, 0.1f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    g_mView = g_Camera.GetViewMatrix();

    glUseProgram(g_ProgId);

    for (auto t : g_ZoomLevels[g_SelectedZoomLevel].Tiles)
    {
        MatrixMultiply(g_mMV, t.mWorld, g_mView);
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

        for (auto m : t.LanduseMeshes)
        {
            glUniform3fv(g_MeshColorLoc, 1, g_LanduseColor.ptr());
            m->Apply();
            m->Render();
        }
    }

    SwapBuffers(g_hDC);
}
