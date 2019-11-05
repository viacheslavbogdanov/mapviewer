#include "Tesselator.h"
#include <array>
#include <vector>

//namespace zzz
//{
//	template <typename T> using Polygon = std::vector<std::vector<T>>;
//	template <typename T> using Triangles = std::vector<T>;
//};
//
//template <typename Coord, typename Polygon>
//class EarcutTesselator {
//public:
//	using Vertex = std::array<Coord, 2>;
//	using Vertices = std::vector<Vertex>;
//
//	EarcutTesselator(const Polygon& polygon_)
//		: polygon(polygon_)
//	{
//		for (const auto& ring : polygon_) {
//			for (const auto& vertex : ring) {
//				vertices_.emplace_back(Vertex{ { Coord(std::get<0>(vertex)),
//												 Coord(std::get<1>(vertex)) } });
//			}
//		}
//	}
//
//	EarcutTesselator& operator=(const EarcutTesselator&) = delete;
//
//	void run() {
//		indices_ = mapbox::earcut(polygon);
//	}
//
//	std::vector<uint32_t> const& indices() const {
//		return indices_;
//	}
//
//	Vertices const& vertices() const {
//		return vertices_;
//	}
//
//private:
//	const Polygon& polygon;
//	Vertices vertices_;
//	std::vector<uint32_t> indices_;
//};


bool TesselateRing(const Ring& _Ring, std::vector<uint32_t>& _OutIndices, std::vector<float>& _OutVertices)
{
	if (_Ring.m_OuterPoints.empty())
		return false;

	using Point = std::pair<int, int>;
	std::vector<std::vector<Point>> polygon;

	if (!_Ring.m_InnerPoints.empty())
	{
		polygon.resize(2);
	}
	else
	{
		polygon.resize(1);
	}

	auto& outer = polygon.at(0);
	//for (size_t oi = 0; oi < _Ring.m_OuterPoints.size() - 1; ++oi)
	//{
	//	Point newP = { _Ring.m_OuterPoints[oi].x, _Ring.m_OuterPoints[oi].y };
	//	outer.push_back(newP);
	//}
	for (auto p : _Ring.m_OuterPoints)
	{
		Point newP = { p.x, p.y };
		outer.push_back(newP);
	}
	if (polygon.size() == 2)
	{
		auto& inner = polygon.at(1);
		for (auto p : _Ring.m_InnerPoints)
		{
			Point newP = { p.x, p.y };
			inner.push_back(newP);
		}
	}


	//auto pppp = zzz::Polygon<std::pair<int, int>>{};
	//EarcutTesselator<int, decltype(pppp)> tesselator(pppp);
	//tesselator.run();

	_OutIndices = mapbox::earcut<uint32_t>(polygon);
	if (_OutIndices.empty())
	{
		return false;
	}

	if (polygon.empty())
		return false;

	_OutVertices.reserve((_Ring.m_OuterPoints.size()) * 2);
	for (size_t oi = 0; oi < _Ring.m_OuterPoints.size(); ++oi)
	{
		_OutVertices.push_back((float)_Ring.m_OuterPoints[oi].x);
		_OutVertices.push_back((float)_Ring.m_OuterPoints[oi].y);
	}
	if (polygon.size() == 2)
	{
		//_OutVertices.reserve((_Ring.m_OuterPoints.size() - 1) * 2);
		for (size_t oi = 0; oi < _Ring.m_InnerPoints.size(); ++oi)
		{
			_OutVertices.push_back((float)_Ring.m_InnerPoints[oi].x);
			_OutVertices.push_back((float)_Ring.m_InnerPoints[oi].y);
		}
	}
	return true;
}
