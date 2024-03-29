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
#ifndef IOGMATH_H_
#define IOGMATH_H_

#include <math.h>
#include <stdlib.h>
#include "IOGCoreHelpers.h"

#include "IOGVector.h"
#include "IOGMatrix.h"
#include "IOGQuaternion.h"


// Get distance between two points
inline float Dist ( int _x1, int _y1, int _x2, int _y2 )
{
	float xdiff = (float)(_x1 - _x2);
	float ydiff = (float)(_y1 - _y2);
	return sqrtf (xdiff*xdiff + ydiff*ydiff);
}


// Get distance between two points (2D)
inline float Dist2D(const OGVec2& _p1, const OGVec2& _p2)
{
	float xdiff = _p1.x - _p2.x;
	float ydiff = _p1.y - _p2.y;
	return sqrtf(xdiff * xdiff + ydiff * ydiff);
}


// Get distance between two points (2D)
inline float Dist2D ( const OGVec3& _p1, const OGVec3& _p2 )
{
	float xdiff = _p1.x - _p2.x;
	float zdiff = _p1.z - _p2.z;
	return sqrtf (xdiff*xdiff + zdiff*zdiff);
}


// Get square distance between two points (2D)
inline float Dist2DSq ( const OGVec3& _p1, const OGVec3& _p2 )
{
	float xdiff = _p1.x - _p2.x;
	float zdiff = _p1.z - _p2.z;
	return (xdiff*xdiff + zdiff*zdiff);
}


// Get distance between two points
inline float Dist3D ( const OGVec3& _p1, const OGVec3& _p2 )
{
	float xdiff = _p1.x - _p2.x;
	float ydiff = _p1.y - _p2.y;
	float zdiff = _p1.z - _p2.z;
	return sqrtf (xdiff*xdiff + ydiff*ydiff + zdiff*zdiff);
}


// Get square distance between two points
inline float Dist3DSq ( const OGVec3& _p1, const OGVec3& _p2 )
{
	float xdiff = _p1.x - _p2.x;
	float ydiff = _p1.y - _p2.y;
	float zdiff = _p1.z - _p2.z;
	return (xdiff*xdiff + ydiff*ydiff + zdiff*zdiff);
}


// Check ray intersection with the triangle
inline bool CheckTriangleIntersection (	
    const OGVec3& _orig, 
    const OGVec3& _dir, 
    const OGVec3& _p0, 
    const OGVec3& _p1, 
    const OGVec3& _p2,
    float* _fT, 
    float* _fU, 
    float* _fV)
{
	static const float CTI_Epsilon = 0.0001f;
	OGVec3 edge1, edge2, tvec, pvec, qvec;
	float det, inv_det;

	edge1 = _p1 - _p0;
	edge2 = _p2 - _p0;

	Vec3CrossProduct(pvec, _dir, edge2);
	det = Vec3DotProduct(edge1, pvec);

	if (det > -CTI_Epsilon && det < CTI_Epsilon)
		return false;
	inv_det = 1.0f / det;

	tvec = _orig - _p0;

	*_fU = Vec3DotProduct(tvec, pvec) * inv_det;
	if (*_fU < 0.0f || *_fU > 1.0f)
		return false;

	Vec3CrossProduct(qvec, tvec, edge1);

	*_fV = Vec3DotProduct(_dir, qvec) * inv_det;
	if (*_fV < 0.0f || *_fU + *_fV > 1.0f)
		return false;

	*_fT = Vec3DotProduct(edge2, qvec) * inv_det;
	return true;
}


// Converting barycentric coords. to world coords.
inline OGVec3 Barycentric2World (
    float _fU, 
    float _fV,
    const OGVec3& _p0, 
    const OGVec3& _p1, 
    const OGVec3& _p2)
{
	float w = 1.0f - ( _fU + _fV);
	float x = w * _p0.x + _fU * _p1.x + _fV * _p2.x;
	float y = w * _p0.y + _fU * _p1.y + _fV * _p2.y;
	float z = w * _p0.z + _fU * _p1.z + _fV * _p2.z;
	return OGVec3(x, y, z);
}


// Converting barycentric coords. to world coords.
inline OGVec3 Vec3Lerp (float _fFactor, const OGVec3& _p0, const OGVec3& _p1)
{
	OGVec3 out;
	out.x = _p0.x + _fFactor * (_p1.x - _p0.x);
	out.y = _p0.y + _fFactor * (_p1.y - _p0.y);
	out.z = _p0.z + _fFactor * (_p1.z - _p0.z);
	return out;
}


// Make a world transformation matrix from the position, rotation and scling matrices
inline void WorldMatrixFromTransforms (
    OGMatrix& _mWorld, 
    const OGVec3& _vPos, 
    const OGVec3& _vRot,
    const OGVec3& _vScale)
{
    OGMatrix mX, mY, mZ, mS, mT;
    MatrixTranslation(mT, _vPos.x, _vPos.y, _vPos.z);
    MatrixRotationX(mX, _vRot.x);
    MatrixRotationY(mY, _vRot.y);
    MatrixRotationZ(mZ, _vRot.z);
    MatrixScaling(mS, _vScale.x, _vScale.y, _vScale.z);
    MatrixMultiply(_mWorld, mS, mY);
    MatrixMultiply(_mWorld, _mWorld, mZ);
    MatrixMultiply(_mWorld, _mWorld, mX);
    MatrixMultiply(_mWorld, _mWorld, mT);
}


// Clips the segment by the given axis
// Returns  negative value if no intersection
//			0 if front-side intersection
//			1 if rear-side intersection
inline int ClipAxialLine ( 
    OGVec3& _Vec0, 
    OGVec3& _Vec1, 
    int	_Sign,
    int	_Axis,
    float _BoxCoordValue)
{
    // calculate the distance to the point
    float d0 = _Vec0[ _Axis ] * _Sign - _BoxCoordValue;
    float d1 = _Vec1[ _Axis ] * _Sign - _BoxCoordValue;

    // get the sign
    unsigned long sign0 = FP_SIGN_BIT ( d0 );
    unsigned long sign1 = FP_SIGN_BIT ( d1 );

    // on the same side
    if ( sign0 == sign1 )
    {
        return - ( (int) ( sign0 >> FP_SIGN_BIT_SHIFT ) + 1 );
    }

    // on the different sides
    sign0 >>= FP_SIGN_BIT_SHIFT;
    if (sign0 == 0)
        _Vec0 = Vec3Lerp ( d0 / ( d0 - d1 ), _Vec0, _Vec1 );
    else
        _Vec1 = Vec3Lerp ( d0 / ( d0 - d1 ), _Vec0, _Vec1 );

    return sign0;	
}


// Find intersection with plane
inline OGVec3 FindIntersectionWithPlane ( 
    float _fHeight,
    const OGVec3& _vRayOrig,
    const OGVec3& _vRayDir)
{
    OGVec3 vIntersectionPoint;
    float fFactorLine = (float)( ( _fHeight - _vRayOrig.y ) / _vRayDir.y );
    vIntersectionPoint.x = _vRayDir.x * fFactorLine + _vRayOrig.x;
    vIntersectionPoint.y = _vRayDir.y * fFactorLine + _vRayOrig.y;
    vIntersectionPoint.z = _vRayDir.z * fFactorLine + _vRayOrig.z;
    return vIntersectionPoint;
}


// CW or CCW
inline bool IsCCW (const OGVec2& _vV1, const OGVec2& _vV2)
{
    return (_vV1.x * _vV2.y - _vV1.y * _vV2.x > 0.0f);
}


// Get angle between two vectors
inline float GetAngle (const OGVec3& _vV1, const OGVec3& _vV2)
{
    float fDot = _vV2.dot(_vV1) / (_vV1.length() * _vV2.length());
    OG_CLAMP(fDot, -1.0f, 1.0f);
    float fAngle = acosf( fDot );
    if ( fAngle < 0 )
        return -PI;
    bool bSign = IsCCW (OGVec2(_vV2.x, _vV2.z), OGVec2(_vV1.x, _vV1.z));
    return bSign ? fAngle : -fAngle;
}


// Rotate 2D point
inline void Rotate2DPoint ( 
    float& _X, 
    float& _Y, 
    float _Angle, 
    float _CenterX, 
    float _CenterY )
{
    float sin = sinf ( _Angle );
    float cos = cosf ( _Angle );

    float RotX = _X - _CenterX;
    float RotY = _Y - _CenterY;

    float RetX = RotX * cos - RotY * sin;
    float RetY = RotX * sin + RotY * cos;

    _X = RetX + _CenterX;
    _Y = RetY + _CenterY;
}


// check if number is power of 2
inline bool IsPowerOf2 (unsigned int input)
{
    unsigned int minus1;

    if( !input ) 
        return 0;

    minus1 = input - 1;
    return ( (input | minus1) == (input ^ minus1) );
}


// Transform point from world coords. to screen coords.
inline bool Project(
    const OGVec3& _Pos,
    const OGMatrix& _mModelView, 
    const OGMatrix& _mProj,
    int ViewWidth, 
    int ViewHeight,
    int& _ScrX, 
    int& _ScrY)
{
    OGVec4 in;
	in.x = _Pos.x;
	in.y = _Pos.y;
	in.z = _Pos.z;
	in.w = 1.0f;

    MatrixVec4Multiply(in, in, _mModelView);
    MatrixVec4Multiply(in, in, _mProj);

	if (in.w == 0)
		return false;

	in.x /= in.w;
	in.y /= in.w;
	in.z /= in.w;

	_ScrX = (int)((1 + in.x) * ViewWidth / 2);
	_ScrY = (int)((1 + in.y) * ViewHeight / 2);
    return true;
}


// Transform point from screen coords. to world coords.
inline bool UnProject(
    int _ScrX, 
    int _ScrY,
    float _ScrZ,
    const OGMatrix& _mModelView, 
    const OGMatrix& _mProj,
    int ViewWidth, 
    int ViewHeight,
    OGVec3& _outPos)
{
	// transformation matrices
	OGMatrix m, A;
	OGVec4 in, out;

	// normalize coords. from -1 to 1
	in.x = (float)_ScrX * 2 / ViewWidth - 1.0f;
	in.y = (float)_ScrY * 2 / ViewHeight - 1.0f;
	in.z = 2 * _ScrZ - 1.0f;
	in.w = 1.0f;

	// calculate inverse transformation
	MatrixMultiply(A, _mModelView, _mProj);
    MatrixInverseEx2(m, A);

	// transform point with the unprojection matrix
	MatrixVec4Multiply(out, in, m);
	if (out.w == 0.0f)
		return false;

    _outPos.x = out.x / out.w;
	_outPos.y = out.y / out.w;
	_outPos.z = out.z / out.w;
	return true;
}


// get random number in range
inline int GetRandomRange (int _Min, int _Max)
{
	return _Min + abs ( rand() % _Max );
}


#endif