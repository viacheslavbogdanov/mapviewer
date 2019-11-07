#include "Scene.h"

#include "VTZeroRead.h"
#include "Tesselator.h"
#include "ElevationMap.h"
#include "MeshSubdivision.h"
#include "MeshConstructor.h"
#include "Utils.h"

#include "IOGMath.h"


Scene::Scene()
{
    SetupConfigs();
}


Scene::~Scene()
{
}


void Scene::SetupConfigs()
{
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
}


bool Scene::Load(const std::string& _AssetsPath)
{
    m_AssetsPath = _AssetsPath;
    for (auto zoomLevelCfg : g_ZoomLevelConfigs)
    {
        auto& CurZoomLevel = m_SceneMeshes.ZoomLevels[zoomLevelCfg.ZoomLevel];
        CurZoomLevel.TilesInRow = zoomLevelCfg.TilesInRow;
        LoadZoomLevel(zoomLevelCfg);
    }

    StitchTiles();

    return true;
}


void Scene::LoadZoomLevel(const ZoomLevelConfig& _Cfg)
{
    auto& CurZoomLevel = m_SceneMeshes.ZoomLevels[_Cfg.ZoomLevel];
    CurZoomLevel.TilesInRow = _Cfg.TilesInRow;

    for (size_t tileCfgId = 0; tileCfgId < _Cfg.TileCoords.size(); ++tileCfgId)
    {
        CurZoomLevel.Tiles.push_back(SceneMeshes::TileMeshes());
        auto& CurTile = CurZoomLevel.Tiles.at(tileCfgId);
        CurTile.ZoomLevel = _Cfg.ZoomLevel;
        CurTile.TileX = tileCfgId / CurZoomLevel.TilesInRow;
        CurTile.TileY = tileCfgId % CurZoomLevel.TilesInRow;
        LoadTile(CurTile, _Cfg.TileCoords[tileCfgId]);
    }
}


void Scene::LoadTile(SceneMeshes::TileMeshes& _CurTile, const ZoomLevelConfig::TileConfig& _Cfg)
{
    std::stringstream DemFileStr;
    DemFileStr << "dem/dem_" << _CurTile.ZoomLevel << "_" <<
        _Cfg.TileCoordX << "_" << _Cfg.TileCoordY << ".png";
    std::vector<float> elevationMap;
    unsigned int extents = 0;
    if (!LoadTerrariumElevationMap(m_AssetsPath + DemFileStr.str(), extents, elevationMap))
    {
        // TODO: better error handling here and further
        return;
    }

    std::stringstream MvtFileStr;
    MvtFileStr << "mvt/mvt_" << _CurTile.ZoomLevel << "_" <<
        _Cfg.TileCoordX << "_" << _Cfg.TileCoordY << ".mvt";
    Tile t;
    if (!ReadTile(m_AssetsPath + MvtFileStr.str(), t))
    {
        return;
    }

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
                                SceneMeshes::TileMeshes::MeshData* pMesh = nullptr;
                                switch (type->second)
                                {
                                case TERRAIN:
                                    {
                                        _CurTile.TerrainMeshes.push_back(SceneMeshes::TileMeshes::MeshData());
                                        pMesh = &(_CurTile.TerrainMeshes.at(_CurTile.TerrainMeshes.size() - 1));
                                    }
                                    break;
                                case WATER:
                                    {
                                        _CurTile.WaterMeshes.push_back(SceneMeshes::TileMeshes::MeshData());
                                        pMesh = &(_CurTile.WaterMeshes.at(_CurTile.WaterMeshes.size() - 1));
                                    }
                                    break;
                                case LANDUSE:
                                    {
                                        _CurTile.LanduseMeshes.push_back(SceneMeshes::TileMeshes::MeshData());
                                        pMesh = &(_CurTile.LanduseMeshes.at(_CurTile.LanduseMeshes.size() - 1));
                                    }
                                    break;
                                }
                                if (pMesh)
                                {
                                    ConstructMesh(_CurTile.ZoomLevel, *pIndicesIn, *pVerticesIn, elevationMap, pMesh->Vertices);
                                    pMesh->Indices.assign(pIndicesIn->begin(), pIndicesIn->end());
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}


struct StitchInfo
{
    int MeshA = -1;
    int MeshB = -1;
    StitchSide Side;
};

void Scene::StitchTiles()
{
    // TODO: calculate it dynamically or move it to a tile config
    std::map<int, std::vector<StitchInfo>> stitchSetup = {
        {13, {{0, 2, STITCH_VER}, {0, 1, STITCH_HOR}, {1, 3, STITCH_VER}, {2, 3, STITCH_HOR}}},
        {14, {{0, 4, STITCH_VER}, {4, 8, STITCH_VER},   {8, 12, STITCH_VER}, 
              {1, 5, STITCH_VER}, {5, 9, STITCH_VER},   {9, 13, STITCH_VER},
              {2, 6, STITCH_VER}, {6, 10, STITCH_VER},  {10, 14, STITCH_VER},
              {3, 7, STITCH_VER}, {7, 11, STITCH_VER},  {11, 15, STITCH_VER},
              {0, 1, STITCH_HOR}, {4, 5, STITCH_HOR}, {8, 9, STITCH_HOR},   {12, 13, STITCH_HOR},
              {1, 2, STITCH_HOR}, {5, 6, STITCH_HOR}, {9, 10, STITCH_HOR},  {13, 14, STITCH_HOR},
              {2, 3, STITCH_HOR}, {6, 7, STITCH_HOR}, {10, 11, STITCH_HOR}, {14, 15, STITCH_HOR}
        }},
    };

    for (auto& zl : m_SceneMeshes.ZoomLevels)
    {
        // nothing to stitch in a one-tile zoom level
        if (zl.first == 12)
        {
            continue;
        }

        auto stitchInfo = stitchSetup.find(zl.first);
        if (stitchInfo != stitchSetup.end())
        {
            for (auto si : stitchInfo->second)
            {
                StitchMeshes(zl.second.Tiles[si.MeshA].TerrainMeshes[0], zl.second.Tiles[si.MeshB].TerrainMeshes[0], si.Side);
            }
        }
    }
}


void Scene::StitchMeshes(SceneMeshes::TileMeshes::MeshData& _MeshA, SceneMeshes::TileMeshes::MeshData& _MeshB, StitchSide _Side)
{
    // since we're comparing an opposite edges (eg. right from the first tile and left from the second tile), let's 
    // introduce a shift (horizontal or vertical) to pretend that tiles overlap, so we can measure distance conveniently
    float TestShiftX = 0.f;
    float TestShiftY = 0.f;

    // Identify potential vertices to stitch from both tiles
    std::vector<uint32_t> StitchSideA;
    std::vector<uint32_t> StitchSideB;
    if (_Side == STITCH_VER)
    {
        TestShiftX = 8192.0f;

        for (size_t i = 0; i < _MeshA.Vertices.size(); i += 6)
        {
            float x = _MeshA.Vertices[i];
            if (x >= 8191.0f && x <= 8193.0f)
            {
                StitchSideA.push_back(i / 6);
            }
        }
        for (size_t i = 0; i < _MeshB.Vertices.size(); i += 6)
        {
            float x = _MeshB.Vertices[i];
            if (x >= 0.0f && x <= 1.0f)
            {
                StitchSideB.push_back(i / 6);
            }
        }
    }
    else
    {
        TestShiftY = 8192.0f;

        for (size_t i = 0; i < _MeshA.Vertices.size(); i += 6)
        {
            float y = _MeshA.Vertices[i + 1];
            if (y >= 8191.0f && y <= 8193.0f)
            {
                StitchSideA.push_back(i / 6);
            }
        }
        for (size_t i = 0; i < _MeshB.Vertices.size(); i += 6)
        {
            float y = _MeshB.Vertices[i + 1];
            if (y >= 0.0f && y <= 1.0f)
            {
                StitchSideB.push_back(i / 6);
            }
        }
    }

    // visit all candidates from both tiles and find pairs to stitch
    for (auto a : StitchSideA)
    {
        OGVec2 vA = OGVec2(_MeshA.Vertices[a * 6 + 0], _MeshA.Vertices[a * 6 + 1]);
        for (auto b : StitchSideB)
        {
            OGVec2 vB = OGVec2(_MeshB.Vertices[b * 6 + 0] + TestShiftX, _MeshB.Vertices[b * 6 + 1] + TestShiftY);

            // pair vertices should be close enough
            if (Dist2D(vA, vB) < 2.0f)
            {
                // the resulting normal will be an average of both normals
                OGVec3 vNormA = OGVec3(_MeshA.Vertices[a * 6 + 3], _MeshA.Vertices[a * 6 + 4], _MeshA.Vertices[a * 6 + 5]);
                OGVec3 vNormB = OGVec3(_MeshB.Vertices[b * 6 + 3], _MeshB.Vertices[b * 6 + 4], _MeshB.Vertices[b * 6 + 5]);
                OGVec3 vNorm = (vNormA + vNormB).normalize();

                _MeshA.Vertices[a * 6 + 3] = vNorm.x;
                _MeshA.Vertices[a * 6 + 4] = vNorm.y; 
                _MeshA.Vertices[a * 6 + 5] = vNorm.z;

                _MeshB.Vertices[b * 6 + 3] = vNorm.x;
                _MeshB.Vertices[b * 6 + 4] = vNorm.y;
                _MeshB.Vertices[b * 6 + 5] = vNorm.z;

                // second tile mesh vertex is replaced by the first mesh vertex
                _MeshB.Vertices[b * 6 + 0] = _MeshA.Vertices[a * 6 + 0] - TestShiftX;
                _MeshB.Vertices[b * 6 + 1] = _MeshA.Vertices[a * 6 + 1] - TestShiftY;
                _MeshB.Vertices[b * 6 + 2] = _MeshA.Vertices[a * 6 + 2];
            }
        }
    }
}
