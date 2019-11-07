#pragma once
#include <ogvertexbuffers.h>
#include <vector>

void ConstructMesh(
	int _ZoomLevel,
	const std::vector<uint32_t>& _Indices, const std::vector<float>& _Vertices2D, const std::vector<float>& _ElevationMap,
    std::vector<float>& _OutVertices);
