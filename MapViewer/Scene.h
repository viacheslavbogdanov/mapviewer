#pragma once
#include <string>
#include <vector>
#include <map>

enum MeshTypes
{
    TERRAIN,
    WATER,
    LANDUSE,
};


struct ZoomLevelConfig
{
    int ZoomLevel = -1;
    int TilesInRow = -1;

    struct TileConfig
    {
        int TileCoordX;
        int TileCoordY;
    };
    std::vector<TileConfig > TileCoords;
};


struct SceneMeshes
{
    struct TileMeshes
    {
        int ZoomLevel = -1;
        int TileX;
        int TileY;

        struct MeshData
        {
            std::vector<uint32_t> Indices;
            std::vector<float> Vertices;
        };
        std::vector<MeshData> TerrainMeshes;
        std::vector<MeshData> WaterMeshes;
        std::vector<MeshData> LanduseMeshes;
    };

    struct ZoomLevel
    {
        int TilesInRow = -1;
        std::vector<SceneMeshes::TileMeshes> Tiles;
    };

    std::map<int, ZoomLevel> ZoomLevels;
};

class Scene
{
public:
    Scene();
    ~Scene();

    bool Load(const std::string& _AssetsPath);
    const SceneMeshes& GetData() const { return m_SceneMeshes; }

private:
    void SetupConfigs();
    void LoadZoomLevel(const ZoomLevelConfig& _Cfg);
    void LoadTile(SceneMeshes::TileMeshes& _CurTile, const ZoomLevelConfig::TileConfig& _Cfg);

private:
    std::vector<ZoomLevelConfig> g_ZoomLevelConfigs;
    std::map<std::string, MeshTypes> allowedTypes = { {"water", WATER}, {"earth", TERRAIN}/*, {"buildings", LANDUSE}*/ };
    std::string m_AssetsPath;

    SceneMeshes m_SceneMeshes;
};
