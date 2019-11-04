#pragma once
#include <string>
#include <map>
#include <clara.hpp>
#include <vtzero/vector_tile.hpp>

struct Ring
{
	std::vector<vtzero::point> m_OuterPoints;
	std::vector<vtzero::point> m_InnerPoints;
};

struct Linestring
{
	std::vector<vtzero::point> m_Points;
};

struct Points
{
	std::vector<vtzero::point> m_Points;
};

struct Feature
{
	std::vector<Linestring> m_Linestrings;
	std::vector<Points> m_Points;
	std::vector<Ring> m_Rings;

	std::string m_Name;
	std::string m_Type;
};

struct Layer
{
	std::vector<Feature> m_Features;
	std::string m_Name;
	uint32_t m_Extent;
};

struct Tile
{
	std::vector<Layer> m_Layers;
};

void ReadTile(const std::string& _TileFilename, Tile& _outTile);
