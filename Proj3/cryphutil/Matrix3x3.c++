// Matrix3x3.c++ -- 3x3 transformation matrices
// This is OPEN SOURCE software developed by James R. Miller (jrmiller@ku.edu)
// Original version: ca. 1996. See README_*.txt for more information.

#include <stdlib.h>
#include <math.h>
#include <iomanip>
using namespace std;

#include "Inline.h"
#include "Tolerances.h"

#include "Matrix3x3.h"
#include "Matrix4x4.h"

namespace cryph
{
// 1. Special Matrices
const Matrix3x3 Matrix3x3::IdentityMatrix =
	Matrix3x3(
		1.0 , 0.0 , 0.0 ,
		0.0 , 1.0 , 0.0 ,
		0.0 , 0.0 , 1.0
		);

const Matrix3x3 Matrix3x3::ZeroMatrix =
	Matrix3x3(
		0.0 , 0.0 , 0.0 ,
		0.0 , 0.0 , 0.0 ,
		0.0 , 0.0 , 0.0
		);

// END: public Global constants

// Other local private constants
static const int	DIM = 3;

Matrix3x3::Matrix3x3()
{
	this->copy(Matrix3x3::IdentityMatrix);
}

Matrix3x3::Matrix3x3(const Matrix3x3& M)
{
	this->copy(M);
}

Matrix3x3::Matrix3x3(
	double m11, double m12, double m13,
	double m21, double m22, double m23,
	double m31, double m32, double m33)
{
	mElem[0][0] = m11; mElem[0][1] = m12; mElem[0][2] = m13;
	mElem[1][0] = m21; mElem[1][1] = m22; mElem[1][2] = m23;
	mElem[2][0] = m31; mElem[2][1] = m32; mElem[2][2] = m33;
}

Matrix3x3::~Matrix3x3()
{
}

void Matrix3x3::copy(const Matrix3x3& rhs)
{
	for (int i=0 ; i<DIM ; i++)
		for (int j=0 ; j<DIM ; j++)
			this->mElem[i][j] = rhs.mElem[i][j];
}

Matrix3x3 Matrix3x3::crossProductMatrix(const AffVector& u)
{
	Matrix3x3	m = Matrix3x3::ZeroMatrix;

	/*********************/ m.mElem[0][1] = -u[DZ]; m.mElem[0][2] =  u[DY];
	m.mElem[1][0] =  u[DZ]; /*********************/ m.mElem[1][2] = -u[DX];
	m.mElem[2][0] = -u[DY]; m.mElem[2][1] =  u[DX]; /*********************/

	return m;
}

Matrix3x3 Matrix3x3::generalRotationRadians(const AffVector& rotationAxis, double angle)
{
	AffVector	w;
	if (rotationAxis.normalizeToCopy(w) < BasicDistanceTol)
		// zero-length vector -- cannot proceed
		return Matrix3x3::IdentityMatrix;

	Matrix3x3 T = tensorProductMatrix(w,w);
	Matrix3x3	Xprod = crossProductMatrix(w);

	double c = cos(angle); double s = sin(angle);

	return c*Matrix3x3::IdentityMatrix + (1.0-c)*T + s*Xprod;
}

Matrix3x3 Matrix3x3::operator=(const Matrix3x3& rhs)
{
	this->copy(rhs);
	return *this;
}

Matrix3x3 Matrix3x3::operator*=(const Matrix3x3& rhs)
{
	*this = (*this) * rhs;
	return *this;
}

Matrix3x3 Matrix3x3::operator*=(double f)
{
	for (int i=0 ; i<DIM ; i++)
		for (int j=0 ; j<DIM ; j++)
			this->mElem[i][j] *= f;
	return *this;
}

Matrix3x3 Matrix3x3::operator+=(const Matrix3x3& rhs)
{
	for (int i=0 ; i<DIM ; i++)
		for (int j=0 ; j<DIM ; j++)
			this->mElem[i][j] += rhs.mElem[i][j];
	return *this;
}

AffPoint Matrix3x3::operator*(const AffPoint& p) const
{
	double	c[DIM] = { p[X] , p[Y] , p[Z] };
	double	d[DIM];

	for (int i=0 ; i<DIM ; i++)
	{
		double sum = 0.0;
		for (int j=0 ; j<DIM ; j++)
			sum += (this->mElem[i][j] * c[j]);
		d[i] = sum;
	}
	return AffPoint(d[0],d[1],d[2]);
}

AffVector Matrix3x3::operator*(const AffVector& v) const
{
	double	c[DIM] = { v[DX] , v[DY] , v[DZ] };
	double	d[DIM];

	for (int i=0 ; i<DIM ; i++)
	{
		double sum = 0.0;
		for (int j=0 ; j<DIM ; j++)
			sum += (this->mElem[i][j] * c[j]);
		d[i] = sum;
	}
	return AffVector(d[0],d[1],d[2]);
}

Matrix3x3 Matrix3x3::operator*(const Matrix3x3& m2) const
{
	Matrix3x3 mOut;

	for (int i=0 ; i<DIM ; i++)
		for (int j=0 ; j<DIM ; j++)
		{
			double sum = 0.0;
			for (int k=0 ; k<DIM ; k++)
				sum += mElem[i][k]*m2.mElem[k][j];
			mOut.mElem[i][j] = sum;
		}

	return mOut;
}

Matrix3x3 Matrix3x3::operator+(const Matrix3x3& m2) const
{
	Matrix3x3	result(*this);

	for (int i=0 ; i<DIM ; i++)
		for (int j=0 ; j<DIM ; j++)
			result.mElem[i][j] += m2.mElem[i][j];
	return result;
}

Matrix3x3 Matrix3x3::operator-(const Matrix3x3& m2) const
{
	Matrix3x3	result(*this);

	for (int i=0 ; i<DIM ; i++)
		for (int j=0 ; j<DIM ; j++)
			result.mElem[i][j] -= m2.mElem[i][j];
	return result;
}

Matrix3x3 operator*(double f, const Matrix3x3& m) // friend function; not a method
{
	Matrix3x3	result(m);

	for (int i=0 ; i<DIM ; i++)
		for (int j=0 ; j<DIM ; j++)
			result.mElem[i][j] *= f;
	return result;
}

ostream& operator<<(ostream& os, const Matrix3x3& m)
{
	for (int i=0 ; i<DIM ; i++)
	{
		for (int j=0 ; j<DIM ; j++)
		{
			os << m.mElem[i][j] << ' ';
		}
	}
	return os;
}

istream& operator>>(istream& is, Matrix3x3& m)
{
	for (int i=0 ; i<DIM ; i++)
	{
		for (int j=0 ; j<DIM ; j++)
		{
			is >> m.mElem[i][j];
		}
	}

	return is;
}

Matrix3x3 Matrix3x3::tensorProductMatrix(const AffVector& u, const AffVector& v)
{
	double U[DIM] = { u[DX] , u[DY] , u[DZ] };
	double V[DIM] = { v[DX] , v[DY] , v[DZ] };

	Matrix3x3	m(Matrix3x3::ZeroMatrix);

	for (int i=0 ; i<DIM ; i++)
		for (int j=0 ; j<DIM ; j++)
			m.mElem[i][j] = U[i]*V[j];

	return m;
}

}
