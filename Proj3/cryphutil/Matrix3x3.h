// Matrix3x3.h -- 3x3 Matrices
// This is OPEN SOURCE software developed by James R. Miller (jrmiller@ku.edu)
// Original version: ca. 1996. See README_*.txt for more information.

// This interface is designed for the specification and manipulation of
// 3x3 matrices as a part of Affine transformations specified on 3D
// vectors. As such, it provides facilities to create 3x3 matrices that
// rotate and mirror vectors, form tensor products of vectors, and provide
// other support required by Matrix4x4 in its role of representing
// transformations of 3D geometry. This interface is NOT intended to
// provide facilities for representing 2D transformations in homogeneous
// form.

#ifndef MATRIX3x3_H
#define MATRIX3x3_H

#include "AffPoint.h"
#include "AffVector.h"

namespace cryph
{

class Matrix3x3
{
public:
	Matrix3x3(); // Identity Matrix
	Matrix3x3(const Matrix3x3& M);
	Matrix3x3(
		double m11, double m12, double m13,
		double m21, double m22, double m23,
		double m31, double m32, double m33);
	virtual ~Matrix3x3();

	// class "Factory" methods to create 3x3 matrices:
	// 1. Rotation affine transformations
	static Matrix3x3 generalRotationRadians
					(const AffVector& rotationAxis, double angle);
	// 4. Miscellaneous
	static Matrix3x3 crossProductMatrix(const AffVector& u);
	static Matrix3x3 tensorProductMatrix
							(const AffVector& u, const AffVector& v);

	// Instance methods

	Matrix3x3 operator=(const Matrix3x3& rhs);

	Matrix3x3 operator*=(const Matrix3x3& rhs);
	Matrix3x3 operator*=(double f);
	Matrix3x3 operator+=(const Matrix3x3& rhs);

	AffPoint operator*(const AffPoint& p) const;
	AffVector operator*(const AffVector& v) const;

	Matrix3x3 operator*(const Matrix3x3& m2) const;
	Matrix3x3 operator+(const Matrix3x3& m2) const;
	Matrix3x3 operator-(const Matrix3x3& m2) const;
	
	friend Matrix3x3 operator*(double f, const Matrix3x3& m);
	friend std::ostream& operator<<(std::ostream& os, const Matrix3x3& m);
	friend std::istream& operator>>(std::istream& is, Matrix3x3& m);

    // ---------- Global constants

	// 1. Special Matrices
	static const Matrix3x3 IdentityMatrix;
	static const Matrix3x3 ZeroMatrix;

	friend class Matrix4x4;

protected:
	double	mElem[3][3];

private:
	void copy(const Matrix3x3& rhs);
};

}

#endif
