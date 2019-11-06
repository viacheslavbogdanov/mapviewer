#include "Tesselator.h"
#include <vector>


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

	_OutIndices = mapbox::earcut<uint32_t>(polygon);
	if (_OutIndices.empty())
	{
		return false;
	}

	if (polygon.empty())
		return false;

	size_t NumOuterPoints = _Ring.m_OuterPoints.size();
	size_t NumInnerPoints = _Ring.m_InnerPoints.size();
	size_t OutSize = NumOuterPoints * 2;
	if (polygon.size() == 2)
	{
		OutSize += NumInnerPoints * 2;
	}
	_OutVertices.reserve(OutSize);
	for (size_t oi = 0; oi < NumOuterPoints; ++oi)
	{
		_OutVertices.push_back((float)_Ring.m_OuterPoints[oi].x);
		_OutVertices.push_back((float)_Ring.m_OuterPoints[oi].y);
	}
	if (polygon.size() == 2)
	{
		for (size_t oi = 0; oi < NumInnerPoints; ++oi)
		{
			_OutVertices.push_back((float)_Ring.m_InnerPoints[oi].x);
			_OutVertices.push_back((float)_Ring.m_InnerPoints[oi].y);
		}
	}
	return true;
}
