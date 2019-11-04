#pragma once
#include <mapbox\earcut.hpp>
#include "VTZeroRead.h"
#include <vector>

bool TesselateRing(const Ring& _Ring, std::vector<uint32_t>& _OutIndices, std::vector<float>& _OutVertices);
