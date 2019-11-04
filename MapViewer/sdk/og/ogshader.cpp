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
#include <string>
#include "ogshader.h"
#include "IOGCoreHelpers.h"


/*****************************************************************************
 @Function		ShaderLoadSourceFromMemory
 @Input         pszShaderCode	shader source code
 @Input         Type			GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
 @Output        pObject		    the resulting shader object
 @Returns       OG_SUCCESS on success and OG_FAIL on failure
 @Description   Loads a shader source file into memory and passes it to the GL.
 ****************************************************************************/
bool ShaderLoadSourceFromMemory(	
    const char* pszShaderCode, 
    GLenum Type, 
    GLuint* const pObject)
{
	// Create the shader object.
    *pObject = glCreateShader(Type);
	
	// Load the source code into it.
    glShaderSource(*pObject, 1, &pszShaderCode, NULL);
	
	// Compile the source code.
    glCompileShader(*pObject);

	// Test if compilation succeeded.
	GLint bShaderCompiled;
    glGetShaderiv(*pObject, GL_COMPILE_STATUS, &bShaderCompiled);
	if (!bShaderCompiled)
	{
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten;
		glGetShaderiv(*pObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		
		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(*pObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		
		OG_LOG_ERROR("ShaderLoadSourceFromMemory: Failed to compile fragment shader: %s", pszInfoLog);
		delete [] pszInfoLog;
		
		// Delete shader.
		glDeleteShader(*pObject);
		
		// Return false, couldn't compile.
		return false;
	}
	return true;
}


/*!***************************************************************************
 @Function		ShaderLoadFromFile
 @Input			pszSrcFile	source shader filename
 @Input			Type		type of shader (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
 @Output		pObject		the resulting shader object
 @Returns       OG_SUCCESS on success and OG_FAIL on failure
 @Description   Loads a shader file into memory and passes it to the GL.
 ****************************************************************************/
bool ShaderLoadFromFile(	
	const std::string& _Filename,
    GLenum Type,
    GLuint* const pObject)
{
	char* pData = nullptr;
	FILE* pFile = fopen(_Filename.c_str(), "rb");
	if (pFile)
	{
		// Get the file size
		fseek(pFile, 0, SEEK_END);
		long size = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		// read the data, append a 0 byte as the data might represent a string
		pData = new char[size + 1];
		pData[size] = '\0';
		size_t BytesRead = fread(pData, 1, size, pFile);

		if (BytesRead != size)
		{
			delete[] pData;
			pData = nullptr;
			size = 0;
		}
		fclose(pFile);
	}
	else
	{
		OG_LOG_ERROR("COGResourceFile::OpenForRead: cannot find resource file %s at filesystem.", _Filename.c_str());
	}

	unsigned int result = ShaderLoadSourceFromMemory(pData, Type, pObject);
    if (result == false)
    {
		OG_LOG_ERROR("ShaderLoadFromFile: Failed to load source shader %s", _Filename.c_str());
    }
    return result;
}


/*!***************************************************************************
 @Function		CreateProgram
 @Output		pProgramObject			the created program object
 @Input			VertexShader			the vertex shader to link
 @Input			FragmentShader			the fragment shader to link
 @Input			pszAttribs				an array of attribute names
 @Input			i32NumAttribs			the number of attributes to bind
 @Returns		OG_SUCCESS on success, OG_FAIL if failure
 @Description	Links a shader program.
 ****************************************************************************/
bool CreateProgram(	
    GLuint* const pProgramObject, 
    GLuint VertexShader, 
    GLuint FragmentShader, 
    const char** const pszAttribs,
    int i32NumAttribs)
{
	// Create the shader program.
	*pProgramObject = glCreateProgram();

	// Attach the fragment and vertex shaders to it.
	glAttachShader(*pProgramObject, FragmentShader);
	glAttachShader(*pProgramObject, VertexShader);

	// For every member in pszAttribs, bind the proper attributes.
	for (int i = 0; i < i32NumAttribs; ++i)
	{
		glBindAttribLocation(*pProgramObject, i, pszAttribs[i]);
	}

	// Link the program object
	glLinkProgram(*pProgramObject);
	
	// Check if linking succeeded.
	GLint bLinked;
	glGetProgramiv(*pProgramObject, GL_LINK_STATUS, &bLinked);
	if (!bLinked)
	{
		int i32InfoLogLength, i32CharsWritten;
		glGetProgramiv(*pProgramObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog(*pProgramObject, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		OG_LOG_ERROR("CreateProgram: Failed to link: %s", pszInfoLog);
		delete [] pszInfoLog;
		return false;
	}

	// Actually use the created program.
	glUseProgram(*pProgramObject);
	return true;
}
