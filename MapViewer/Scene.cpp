#include "Scene.h"

#include "VTZeroRead.h"
#include "Tesselator.h"
#include "ElevationMap.h"
#include "MeshSubdivision.h"
#include "MeshConstructor.h"
#include "Utils.h"


Scene::Scene()
{
    SetupConfigs();
}


Scene::~Scene()
{
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
    LoadTerrariumElevationMap(m_AssetsPath + DemFileStr.str(), extents, elevationMap);

    std::stringstream MvtFileStr;
    MvtFileStr << "mvt/mvt_" << _CurTile.ZoomLevel << "_" <<
        _Cfg.TileCoordX << "_" << _Cfg.TileCoordY << ".mvt";
    Tile t;
    ReadTile(m_AssetsPath + MvtFileStr.str(), t);

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
