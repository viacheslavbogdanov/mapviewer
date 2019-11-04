#pragma once
#include <ogvertexbuffers.h>
#include <vector>

void ConstructMesh(
	const std::vector<uint32_t>& _Indices, const std::vector<float>& _Vertices2D, const std::vector<float>& _ElevationMap,
	COGVertexBuffers& _OutMesh);
