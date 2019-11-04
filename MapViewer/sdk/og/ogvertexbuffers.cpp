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
#include <OpenGL2.h>
#include "ogvertexbuffers.h"


COGVertexBuffers::~COGVertexBuffers ()
{
    if (m_pVertexData)
        free(m_pVertexData);
    if (m_pIndexData)
        free(m_pIndexData);
    if (m_VBO != 0)
        glDeleteBuffers(1, &m_VBO);
    if (m_IBO != 0)
        glDeleteBuffers(1, &m_IBO);
}


COGVertexBuffers::COGVertexBuffers (
    const void* _pVertexData, 
    unsigned int _NumVertices, 
    unsigned int _NumFaces,
    unsigned int _Stride, 
    const void* _pIndexData, 
    unsigned int _NumIndices)
{
    m_NumVertices = _NumVertices;
    m_Stride = _Stride;
    m_NumIndices = _NumIndices;
    m_NumFaces = _NumFaces;

    unsigned int VBOSize = m_NumVertices * m_Stride;
    m_pVertexData = malloc(VBOSize);
    memcpy(m_pVertexData, _pVertexData, VBOSize);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, VBOSize, m_pVertexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if(_pIndexData)
    {
		unsigned int IBOSize = _NumIndices * sizeof(GLuint);
		m_pIndexData = malloc(IBOSize);
        memcpy(m_pIndexData, _pIndexData, IBOSize);

        glGenBuffers(1, &m_IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, IBOSize, m_pIndexData, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}


void COGVertexBuffers::Fill(
	const void* _pVertexData,
	unsigned int _NumVertices,
	unsigned int _NumFaces,
	unsigned int _Stride,
	const void* _pIndexData,
	unsigned int _NumIndices)
{
	m_NumVertices = _NumVertices;
	m_Stride = _Stride;
	m_NumIndices = _NumIndices;
	m_NumFaces = _NumFaces;

	unsigned int VBOSize = m_NumVertices * m_Stride;
	m_pVertexData = malloc(VBOSize);
	memcpy(m_pVertexData, _pVertexData, VBOSize);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, VBOSize, m_pVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (_pIndexData)
	{
		//unsigned int IBOSize = _NumIndices * sizeof(GLshort);
		unsigned int IBOSize = _NumIndices * sizeof(GLuint);
		m_pIndexData = malloc(IBOSize);
		memcpy(m_pIndexData, _pIndexData, IBOSize);

		glGenBuffers(1, &m_IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IBOSize, m_pIndexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}


// apply buffers.
void COGVertexBuffers::Apply () const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_Stride, (const void*)(0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, m_Stride, (const void*)(0+sizeof(float)*3));
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, m_Stride, (const void*)(0+sizeof(float)*6));
}


// render buffer geometry.
void COGVertexBuffers::Render () const
{
    if(IsIndexed())
    {
		glDrawElements(GL_TRIANGLES, m_NumFaces * 3, GL_UNSIGNED_INT, 0);
	}
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, m_NumFaces * 3);
    }
}
