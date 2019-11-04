/*
 * OrangeGrass
 * Copyright (C) 2009 Vyacheslav Bogdanov.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/lgpl-3.0-standalone.html>.
 */
#ifndef OGVERTEXBUFFERS_H_
#define OGVERTEXBUFFERS_H_
#include "IOGVertexBuffers.h"


class COGVertexBuffers : public IOGVertexBuffers
{
public:
	COGVertexBuffers() {}

	COGVertexBuffers (
        const void* _pVertexData, 
        unsigned int _NumVertices,
        unsigned int _NumFaces,
        unsigned int _Stride, 
        const void* _pIndexData, 
        unsigned int _NumIndices);

    virtual ~COGVertexBuffers ();

	virtual void Fill(
		const void* _pVertexData,
		unsigned int _NumVertices,
		unsigned int _NumFaces,
		unsigned int _Stride,
		const void* _pIndexData,
		unsigned int _NumIndices);

    // apply buffers.
    virtual void Apply () const;

    // render buffer geometry.
    virtual void Render () const;

    // is indexed
    virtual bool IsIndexed() const { return (m_pIndexData != nullptr); }

    // number of vertices
    virtual unsigned int GetNumVertices () const { return m_NumVertices; }

    // number of indices
    virtual unsigned int GetNumIndices () const { return m_NumIndices; }

    // number of faces
    virtual unsigned int GetNumFaces () const { return m_NumFaces; }

    // stride
    virtual unsigned int GetStride () const { return m_Stride; }

    // vertex data
    virtual const void* GetVertexData () const { return m_pVertexData; }

    // index data
    virtual const void* GetIndexData () const { return m_pIndexData; }

    // map buffer geometry.
    virtual void Map () {}

    // unmap buffer geometry.
    virtual void Unmap () {}

    // update buffer geometry.
    virtual void Update (unsigned int _Offset, const void* _pBuff, unsigned int _Size) {}

    // is dynamic
    virtual bool IsDynamic() const { return false; }

private:

    unsigned int m_VBO = 0;
    unsigned int m_IBO = 0;
    unsigned int m_NumVertices = 0;
    unsigned int m_NumIndices = 0;
    unsigned int m_NumFaces = 0;
    unsigned int m_Stride = 0;
	void* m_pVertexData = nullptr;;
    void* m_pIndexData = nullptr;
};

#endif
