#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include "VTZeroRead.h"
#include "Utils.h"


vtzero::layer get_layer(const vtzero::vector_tile& tile, const std::string& layer_name_or_num)
{
	vtzero::layer layer;
	char* str_end = nullptr;
	const long num = std::strtol(layer_name_or_num.c_str(), &str_end, 10);

	if (str_end == layer_name_or_num.data() + layer_name_or_num.size())
	{
		if (num >= 0 && num < std::numeric_limits<long>::max()) 
		{
			layer = tile.get_layer(static_cast<std::size_t>(num));
			if (!layer)
			{
				std::cerr << "No such layer: " << num << '\n';
				std::exit(1);
			}
			return layer;
		}
	}

	layer = tile.get_layer_by_name(layer_name_or_num);
	if (!layer)
	{
		std::cerr << "No layer named '" << layer_name_or_num << "'.\n";
		std::exit(1);
	}
	return layer;
}


class geom_handler 
{
	std::vector<vtzero::point> m_TmpPointsInRing;

public:
	std::vector<Linestring> m_Linestrings;
	std::vector<Points> m_Points;
	std::vector<Ring> m_Rings;

public:
	void points_begin(const uint32_t count) noexcept
	{
		m_Points.push_back(Points());
		m_Points.rbegin()->m_Points.reserve(count);
	}

	void points_point(const vtzero::point point) 
	{
		assert(!m_Points.empty());
		m_Points.rbegin()->m_Points.push_back(point);
	}

	void points_end() const noexcept 
	{
	}

	void linestring_begin(const uint32_t count) 
	{
		m_Linestrings.push_back(Linestring());
		m_Linestrings.rbegin()->m_Points.reserve(count);
	}

	void linestring_point(const vtzero::point point) 
	{
		assert(!m_Linestrings.empty());
		m_Linestrings.rbegin()->m_Points.push_back(point);
	}

	void linestring_end() 
	{
	}

	void ring_begin(const uint32_t count) 
	{
		m_TmpPointsInRing.reserve(count);
	}

	void ring_point(const vtzero::point point) 
	{
		m_TmpPointsInRing.push_back(point);
	}

	void ring_end(const vtzero::ring_type rt) 
	{
		if (m_Rings.empty())
		{
			m_Rings.push_back(Ring());
		}
		switch (rt)
		{
		case vtzero::ring_type::outer:
			m_Rings.rbegin()->m_OuterPoints = m_TmpPointsInRing;
			m_TmpPointsInRing.clear();
			break;
		case vtzero::ring_type::inner:
			m_Rings.rbegin()->m_InnerPoints = m_TmpPointsInRing;
			m_TmpPointsInRing.clear();
			break;
		default:
			m_TmpPointsInRing.clear();
			break;
		}
	}
};


static void print_layer(vtzero::layer& layer, Layer& _outLayer) 
{
	_outLayer.m_Name = std::string(layer.name());
	_outLayer.m_Extent = layer.extent();

	int feature_num = 0;
	while (auto feature = layer.next_feature()) 
	{
		Feature f;
		geom_handler gh;
		vtzero::decode_geometry(feature.geometry(), gh);
		f.m_Linestrings = gh.m_Linestrings;
		f.m_Points = gh.m_Points;
		f.m_Rings = gh.m_Rings;
		while (auto property = feature.next_property()) 
		{
			if (std::string(property.key()) == std::string("name"))
			{
				f.m_Name = std::string(property.value().data());
				break;
			}
		}
		++feature_num;
		_outLayer.m_Features.push_back(f);
	}
}


void ReadLayers(vtzero::vector_tile& _Tile, Tile& _outTile)
{
	int layer_num = 0;
	int feature_num = 0;
	std::string layer_num_or_name;
	try
	{
		if (layer_num_or_name.empty())
		{
			while (auto layer = _Tile.next_layer())
			{
				Layer l;
				print_layer(layer, l);
				++layer_num;
				_outTile.m_Layers.push_back(l);
			}
		}
		else 
		{
			auto layer = get_layer(_Tile, layer_num_or_name);
			Layer l;
			print_layer(layer, l);
			_outTile.m_Layers.push_back(l);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error in layer " << layer_num << " (feature " << feature_num << "): " << e.what() << '\n';
	}
}


void ReadTile(const std::string& _TileFilename, Tile& _outTile)
{
	int layer_num = 0;
	int feature_num = 0;
	try
	{
		const auto data = ReadFile(_TileFilename);
		vtzero::vector_tile vtzTile{ data };
		ReadLayers(vtzTile, _outTile);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error in layer " << layer_num << " (feature " << feature_num << "): " << e.what() << '\n';
	}
}
