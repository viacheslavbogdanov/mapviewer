#include "MeshConstructor.h"
#include "ElevationMap.h"
#include "IOGVector.h"

void ConstructMesh(
	int _ZoomLevel,
	const std::vector<uint32_t>& _Indices, const std::vector<float>& _Vertices2D, const std::vector<float>& _ElevationMap,
	COGVertexBuffers& _OutMesh)
{
	float fMult = 1.0f;
	switch (_ZoomLevel)
	{
	case 12: fMult = 1.0f; break;
	case 13: fMult = 2.0f; break;
	case 14: fMult = 4.0f; break;
	}
	std::vector<float> vertices3D;
	size_t vertsSize2D = _Vertices2D.size();
	vertices3D.reserve(vertsSize2D + vertsSize2D / 2);
	for (size_t i = 0; i < vertsSize2D; i += 2)
	{
		// position
		vertices3D.push_back(_Vertices2D[i + 0]);
		vertices3D.push_back(_Vertices2D[i + 1]);
		float z = GetElevation(_ElevationMap, (float)(_Vertices2D[i + 0]), (float)(_Vertices2D[i + 1]));
		vertices3D.push_back(z * fMult);

		// normal
		vertices3D.push_back(0.0f);
		vertices3D.push_back(0.0f);
		vertices3D.push_back(1.0f);
	}

	//for (size_t i = 0; i < _Indices.size(); i += 3)
	//{
	//	if ((i % 3 == 0))
	//	{
	//		OGVec3 vA = OGVec3(vertices3D[(_Indices[i + 0] * 6) + 0], vertices3D[(_Indices[i + 0] * 6) + 1], vertices3D[(_Indices[i + 0] * 6) + 2]);
	//		OGVec3 vB = OGVec3(vertices3D[(_Indices[i + 1] * 6) + 0], vertices3D[(_Indices[i + 1] * 6) + 1], vertices3D[(_Indices[i + 1] * 6) + 2]);
	//		OGVec3 vC = OGVec3(vertices3D[(_Indices[i + 2] * 6) + 0], vertices3D[(_Indices[i + 2] * 6) + 1], vertices3D[(_Indices[i + 2] * 6) + 2]);
	//		OGVec3 vAB = (vA - vB).normalize();
	//		OGVec3 vAC = (vC - vA).normalize();
	//		OGVec3 vNorm = (vAB.cross(vAC)).normalize();
	//		if (vNorm.z < 0.0f)
	//			vNorm *= -1.0f;
	//
	//		vertices3D[(_Indices[i + 0] * 6) + 3] = vNorm.x;
	//		vertices3D[(_Indices[i + 0] * 6) + 4] = vNorm.y;
	//		vertices3D[(_Indices[i + 0] * 6) + 5] = vNorm.z;
	//
	//		vertices3D[(_Indices[i + 1] * 6) + 3] = vNorm.x;
	//		vertices3D[(_Indices[i + 1] * 6) + 4] = vNorm.y;
	//		vertices3D[(_Indices[i + 1] * 6) + 5] = vNorm.z;
	//
	//		vertices3D[(_Indices[i + 2] * 6) + 3] = vNorm.x;
	//		vertices3D[(_Indices[i + 2] * 6) + 4] = vNorm.y;
	//		vertices3D[(_Indices[i + 2] * 6) + 5] = vNorm.z;
	//	}
	//}

	for (size_t i = 0; i < vertices3D.size(); i += 6)
	{
		OGVec3 vPos = OGVec3(vertices3D[i], vertices3D[i+1], vertices3D[i+2]);
		std::vector<int> trianglesShare;
		for (size_t ii = 0; ii < _Indices.size(); ++ii)
		{
			if (_Indices[ii] == i / 6)
				trianglesShare.push_back((ii / 3) * 3);
		}
		if (trianglesShare.empty())
			continue;
		OGVec3 vNorm = OGVec3(0.0f);
		for (auto tri : trianglesShare)
		{
			OGVec3 vA = OGVec3(vertices3D[(_Indices[tri + 0] * 6) + 0], vertices3D[(_Indices[tri + 0] * 6) + 1], vertices3D[(_Indices[tri + 0] * 6) + 2]);
			OGVec3 vB = OGVec3(vertices3D[(_Indices[tri + 1] * 6) + 0], vertices3D[(_Indices[tri + 1] * 6) + 1], vertices3D[(_Indices[tri + 1] * 6) + 2]);
			OGVec3 vC = OGVec3(vertices3D[(_Indices[tri + 2] * 6) + 0], vertices3D[(_Indices[tri + 2] * 6) + 1], vertices3D[(_Indices[tri + 2] * 6) + 2]);
			//if (vA == vB || vA == vC || vB == vC)
			//	continue;
			OGVec3 vAB = (vB - vA).normalize();
			OGVec3 vAC = (vC - vA).normalize();

			OGVec3 vN = (vAB.cross(vAC)).normalize();
			if (vN.z < 0.0f)
				vN *= -1.0f;
			vNorm += vN;
		}
		vNorm.normalize();

		vertices3D[i + 3] = vNorm.x;
		vertices3D[i + 4] = vNorm.y;
		vertices3D[i + 5] = vNorm.z;
	}

	_OutMesh.Fill(vertices3D.data(), vertices3D.size() / 6, _Indices.size() / 3, sizeof(float) * 6, _Indices.data(), _Indices.size());
}
