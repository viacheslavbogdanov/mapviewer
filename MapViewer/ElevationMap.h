#pragma once
#include <string>
#include <vector>

bool LoadTerrariumElevationMap(const std::string& _Filename, unsigned int& _OutExtent, std::vector<float>& _OutElevationMap);
float GetElevation(const std::vector<float>& _ElevationMap, int _X, int _Y);
float GetElevation(const std::vector<float>& _ElevationMap, float _normalizedX, float _normalizedY);
