#include "MeshSubdivision.h"
#include <IOGMath.h>
#include <map>


inline bool FindEdges(
	const std::vector<unsigned int>& _Indices, 
	size_t _curTriangle, unsigned int _A, unsigned int _B, 
	unsigned int& _outAdjacentTriangle, size_t& _outAdjacentFaceId)
{
	size_t numIndices = _Indices.size();
	for (size_t i = (_curTriangle + 3); i <= numIndices - 3; i += 3)
	{
		for (size_t n = 0; n < 3; ++n)
		{
			size_t testAi = i + n;
			size_t testBi = (n == 2) ? i : (i + n + 1);
			unsigned int testA = _Indices[testAi];
			unsigned int testB = _Indices[testBi];
			if ((_A == testA && _B == testB) || (_B == testA && _A == testB))
			{
				// we found the adjacent triangle!
				// we also return which face (0-1, 1-2, 2-0) hits by returning it's index within the triangle
				_outAdjacentTriangle = i;
				_outAdjacentFaceId = n;
				// there can be only one adjacent in 2D :)
				return true;
			}
		}
	}
	return false;
}


enum TriSubdivision
{
	SUBD_NONE = 0,
	SUBD_AB = 1,	// A-B indices (0 -> 1)
	SUBD_BC = 2,	// B-C indices (1 -> 2)
	SUBD_CA = 4		// C-A indices (2 -> 0)
};


inline TriSubdivision FromEdgeId(unsigned int _EdgeId)
{
	switch (_EdgeId)
	{
	case 0: return SUBD_AB;
	case 1: return SUBD_BC;
	case 2: return SUBD_CA;
	}
	return SUBD_NONE;
}

struct SubdivInfo
{
	unsigned int SubdivType = SUBD_NONE;
	// 0: A->B 1: B->C 2: C->A
	unsigned int SubdivPtIds[3] = { 0xFFFFFFFF,  0xFFFFFFFF , 0xFFFFFFFF };
};


// Subdivision info:
// key is the original triangle id
// value is an info of an added odd verices for this triangle
using Subdivision = std::map<unsigned int, SubdivInfo>;

// Tri-Edge Adjacency: 
// key = edge id in triangle, 
// value is a pair of adjacent triangle id and index (0, 1, 2) of it's adjacent edge
using Adjacency = std::map<unsigned int, std::pair<unsigned int, unsigned int> >;


// Adjacency storage:
// key = triangle id
// value = it's adjacency info (id's of adjacent tri-s and edges)
using AdjacencyMap = std::map<unsigned int, Adjacency>;


void FindAdjacentTriangles(const std::vector<unsigned int>& _Indices, AdjacencyMap& _outAdjacencyInfo)
{
	_outAdjacencyInfo.clear();

	size_t numIndices = _Indices.size();
	for (size_t i = 0; i <= numIndices - 3; i += 3)
	{
		for (size_t n = 0; n < 3; ++n)
		{
			size_t ai = i + n;
			size_t bi = (n == 2) ? i : (i + n + 1);
			unsigned int a = _Indices[ai];
			unsigned int b = _Indices[bi];
			unsigned int adjacentTri = -1;
			size_t adjacentFace = -1;
			if (FindEdges(_Indices, i, a, b, adjacentTri, adjacentFace))
			{
				auto& curTA = _outAdjacencyInfo[i];
				curTA[n] = { adjacentTri, adjacentFace };
			}
		}
	}
}


inline void GetHalfVec(const OGVec2& _vA, const OGVec2& _vB, float _fDist, OGVec2& _Out)
{
	OGVec2 vDir = (_vB - _vA).normalize();
	_Out = _vA + vDir * _fDist;
}


inline void SubdivideEdge(
	unsigned int _TriangleId, unsigned int _EdgeId,
	const OGVec2& _v1, const OGVec2& _v2, float _MinDist,
	const AdjacencyMap& _adjacencyInfo, Subdivision& _subdivInfo, std::vector<OGVec2>& _oddVertices)
{
	// start subdividing edge only if it wasn't subdivided yet by an adjacent triangle
	auto s = _subdivInfo.find(_TriangleId);
	if (s == _subdivInfo.end() || (s->second.SubdivType & FromEdgeId(_EdgeId)) == 0)
	{
		// okay, this edge was not touched. Is it long enough for subdivision?
		float dist = Dist2D(_v1, _v2);
		if (dist >= _MinDist)
		{
			OGVec2 vOddPt;
			GetHalfVec(_v1, _v2, dist / 2.0f, vOddPt);
			_oddVertices.push_back(vOddPt);
			unsigned int oddVertId = (unsigned int)_oddVertices.size() - 1;
			auto& si = _subdivInfo[_TriangleId];
			si.SubdivType |= FromEdgeId(_EdgeId);
			si.SubdivPtIds[_EdgeId] = oddVertId;

			// notify adjacent triangles that they'll have to subdivide
			AdjacencyMap::const_iterator adjTris = _adjacencyInfo.find(_TriangleId);
			if (adjTris != _adjacencyInfo.end())
			{
				// we found info about our triangle in adjacency info
				// no check if this face is adjacent
				Adjacency::const_iterator adjF = adjTris->second.find(_EdgeId);
				if (adjF != adjTris->second.end())
				{
					// yep, this face is adjacent, now let's find with whom we share this edge
					auto& asi = _subdivInfo[adjF->second.first];
					// notify adjacent triangle, that it will get a new odd vector on a shared edge
					// by updating subdivision info
					asi.SubdivType |= FromEdgeId(adjF->second.second);
					asi.SubdivPtIds[adjF->second.second] = oddVertId;
				}
			}
		}
	}
}


bool SubdivideMesh(
	const std::vector<unsigned int>& _Indices, const std::vector<float>& _Vertices, float _MinDist,
	std::vector<unsigned int>& _OutIndices, std::vector<float>& _OutVertices)
{
	AdjacencyMap adjacencyInfo;
	FindAdjacentTriangles(_Indices, adjacencyInfo);
	std::vector<OGVec2> oddVertices;
	Subdivision subdivInfo;

	size_t numIndices = _Indices.size();
	for (size_t i = 0; i <= numIndices - 3; i += 3)
	{
		unsigned int a = _Indices[i + 0];
		unsigned int b = _Indices[i + 1];
		unsigned int c = _Indices[i + 2];
		
		OGVec2 vA(_Vertices[a * 2], _Vertices[a * 2 + 1]);
		OGVec2 vB(_Vertices[b * 2], _Vertices[b * 2 + 1]);
		OGVec2 vC(_Vertices[c * 2], _Vertices[c * 2 + 1]);

		SubdivideEdge(i, 0, vA, vB, _MinDist, adjacencyInfo, subdivInfo, oddVertices);
		SubdivideEdge(i, 1, vB, vC, _MinDist, adjacencyInfo, subdivInfo, oddVertices);
		SubdivideEdge(i, 2, vC, vA, _MinDist, adjacencyInfo, subdivInfo, oddVertices);
	}

	if (!oddVertices.empty())
	{
		_OutIndices.reserve(_Indices.size() + oddVertices.size() * 3);
		_OutVertices.reserve(_Vertices.size() + oddVertices.size());

		// Write the new vertices to the end of the existing vertex buffer
		// Result is extremely NOT vertex cache-friendly, but simple and works
		_OutVertices.assign(_Vertices.begin(), _Vertices.end());
		for (auto ov : oddVertices)
		{
			_OutVertices.push_back(ov.x);
			_OutVertices.push_back(ov.y);
		}

		// since new (odd) vertices will be placed after the old (even) ones,
		// it's quite easy to index them
		unsigned int firstOddVertexIndex = _Vertices.size() / 2;

		// go ver each triangle and generate new triangles if required
		for (size_t i = 0; i <= numIndices - 3; i += 3)
		{
			auto subdiv = subdivInfo.find(i);
			if (subdiv != subdivInfo.end())
			{
				switch (subdiv->second.SubdivType)
				{
				case SUBD_AB:
					// B C A'
					_OutIndices.push_back(_Indices[i + 1]);
					_OutIndices.push_back(_Indices[i + 2]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);

					// C A A'
					_OutIndices.push_back(_Indices[i + 2]);
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);
					break;

				case SUBD_BC:
					// B' A B
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(_Indices[i + 1]);

					// A B' C
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);
					_OutIndices.push_back(_Indices[i + 2]);
					break;

				case SUBD_CA:
					// C' A B
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[2]);
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(_Indices[i + 1]);

					// C' B C
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[2]);
					_OutIndices.push_back(_Indices[i + 1]);
					_OutIndices.push_back(_Indices[i + 2]);
					break;

				case SUBD_AB | SUBD_BC:
					// A A' B'
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);

					// A' B B'
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);
					_OutIndices.push_back(_Indices[i + 1]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);

					// C A B'
					_OutIndices.push_back(_Indices[i + 2]);
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);
					break;

				case SUBD_AB | SUBD_CA:
					// C A B'
					_OutIndices.push_back(_Indices[i + 2]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);
					_OutIndices.push_back(_Indices[i + 1]);

					// C C' A'
					_OutIndices.push_back(_Indices[i + 2]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[2]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);

					// C' A A'
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[2]);
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);
					break;

				case SUBD_BC | SUBD_CA:
					// A B B'
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(_Indices[i + 1]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);

					// C' A B'
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[2]);
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);

					// C C' B'
					_OutIndices.push_back(_Indices[i + 2]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[2]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);
					break;

				case SUBD_AB | SUBD_BC | SUBD_CA:
					// C' B' C
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[2]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);
					_OutIndices.push_back(_Indices[i + 2]);

					// C' A A'
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[2]);
					_OutIndices.push_back(_Indices[i + 0]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);

					// A' B B'
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);
					_OutIndices.push_back(_Indices[i + 1]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);

					// C' A' B'
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[2]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[0]);
					_OutIndices.push_back(firstOddVertexIndex + subdiv->second.SubdivPtIds[1]);
					break;

				default:
					// Other types are impossible!
					assert(0);
					break;
				}

			}
			else
			{
				// triangle was not touched, keep it as is
				_OutIndices.push_back(_Indices[i + 0]);
				_OutIndices.push_back(_Indices[i + 1]);
				_OutIndices.push_back(_Indices[i + 2]);
			}
		}
		return true;
	}
	return false;
}
