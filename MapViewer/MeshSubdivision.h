#pragma once
#include <vector>

bool SubdivideMesh(const std::vector<unsigned int>& _Indices, const std::vector<float>& _Vertices, float _MinDist,
	std::vector<unsigned int>& _OutIndices, std::vector<float>& _OutVertices);
