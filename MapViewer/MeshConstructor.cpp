#include "MeshConstructor.h"
#include "ElevationMap.h"
#include "IOGVector.h"

void ConstructMesh(
    int _ZoomLevel,
    const std::vector<uint32_t>& _Indices, const std::vector<float>& _Vertices2D, const std::vector<float>& _ElevationMap,
    std::vector<float>& _OutVertices)
{
    // calculate elevated position
    float fMult = 1.0f;
    switch (_ZoomLevel)
    {
    case 12: fMult = 1.0f; break;
    case 13: fMult = 2.0f; break;
    case 14: fMult = 4.0f; break;
    }
     size_t vertsSize2D = _Vertices2D.size();
     _OutVertices.reserve(vertsSize2D + vertsSize2D / 2);
    for (size_t i = 0; i < vertsSize2D; i += 2)
    {
        // position
        _OutVertices.push_back(_Vertices2D[i + 0]);
        _OutVertices.push_back(_Vertices2D[i + 1]);
        float z = GetElevation(_ElevationMap, (float)(_Vertices2D[i + 0]), (float)(_Vertices2D[i + 1]));
        _OutVertices.push_back(z * fMult);

        // normal
        _OutVertices.push_back(0.0f);
        _OutVertices.push_back(0.0f);
        _OutVertices.push_back(1.0f);
    }

    // calculate vertex normals
    for (size_t i = 0; i < _OutVertices.size(); i += 6)
    {
        OGVec3 vPos = OGVec3(_OutVertices[i], _OutVertices[i + 1], _OutVertices[i + 2]);

        // search for triangles that share this vector
        std::vector<int> trianglesShare;
        for (size_t ii = 0; ii < _Indices.size(); ++ii)
        {
            if (_Indices[ii] == i / 6)
                trianglesShare.push_back((ii / 3) * 3);
        }
        if (trianglesShare.empty())
            continue;

        // Get normals of all triangles, that share our vertex and produce the averege one.
        // TODO: consider using weighted intepolation using triangle square as a weight
        OGVec3 vNorm = OGVec3(0.0f);
        for (auto tri : trianglesShare)
        {
            OGVec3 vA = OGVec3(_OutVertices[(_Indices[tri + 0] * 6) + 0], _OutVertices[(_Indices[tri + 0] * 6) + 1], _OutVertices[(_Indices[tri + 0] * 6) + 2]);
            OGVec3 vB = OGVec3(_OutVertices[(_Indices[tri + 1] * 6) + 0], _OutVertices[(_Indices[tri + 1] * 6) + 1], _OutVertices[(_Indices[tri + 1] * 6) + 2]);
            OGVec3 vC = OGVec3(_OutVertices[(_Indices[tri + 2] * 6) + 0], _OutVertices[(_Indices[tri + 2] * 6) + 1], _OutVertices[(_Indices[tri + 2] * 6) + 2]);
            OGVec3 vAB = (vB - vA).normalize();
            OGVec3 vAC = (vC - vA).normalize();

            OGVec3 vN = (vAB.cross(vAC)).normalize();
            if (vN.z < 0.0f)
                vN *= -1.0f;
            vNorm += vN;
        }
        vNorm.normalize();

        _OutVertices[i + 3] = vNorm.x;
        _OutVertices[i + 4] = vNorm.y;
        _OutVertices[i + 5] = vNorm.z;
    }
}
