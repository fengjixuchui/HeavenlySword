//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Spherical harmonic basis function and coefficients.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_SPHERICAL_HARMONICS_H
#define GP_SPHERICAL_HARMONICS_H

#include <Fw/FwMaths/FwVector.h>
#include <Fw/FwStd/FwStdScopedPtr.h>

//--------------------------------------------------------------------------------------------------
//	TYPEDEFS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Allow the library to be built using single or double precision arithmetic.
**/
//--------------------------------------------------------------------------------------------------

typedef float sh_real;

//--------------------------------------------------------------------------------------------------
//	CLASS DECLARATIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Evaluates the spherical harmonic basis functions for any order.
**/
//--------------------------------------------------------------------------------------------------

class GpSphericalHarmonicBasis : FwNonCopyable
{
public:
	sh_real	operator()( int l, int m, sh_real phi, sh_real theta ) const;

private:
	static sh_real L( int l, int m, sh_real theta );
	static sh_real T( int l, int m, sh_real theta );
	static sh_real P( int m, sh_real phi );
	static sh_real factorial( int n );
};

//--------------------------------------------------------------------------------------------------
/**
	@brief		Stores spherical harmonic coefficients for any order.

	The coefficients are hard-coded to be of Vector4 type, since this is a most common usage. The
	polar coordinates are aligned to the Z axis, the relationship with cartesian coordinates is:

	( x, y, z ) = ( cos(phi)sin(theta), sin(phi)sin(theta), cos(theta) )
**/
//--------------------------------------------------------------------------------------------------

class GpSphericalHarmonicCoeffs
{
public:
	explicit GpSphericalHarmonicCoeffs( int order );

	int					GetOrder() const;

	void				SetCoefficients( FwVector4 const* pCoeffs );
	FwVector4 const*	GetCoefficients() const;

	FwVector4&			operator()( int l, int m );
	FwVector4_arg		operator()( int l, int m ) const;

	FwVector4			EvaluateDirection( sh_real phi, sh_real theta ) const;

	void				Clear();
	void				AddSample( sh_real phi, sh_real theta, FwVector4_arg value );
	void				AddSample( FwVector_arg direction, FwVector4_arg value );

private:
	int					GetIndex( int l, int m ) const;
	int					CheckIndex( int n ) const;

	int m_order;
	GpSphericalHarmonicBasis m_basis;
	FwStd::ScopedArrayPtr< FwVector4 > m_coeffs;
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline int GpSphericalHarmonicCoeffs::GetOrder() const
{
	return m_order;
}

inline int GpSphericalHarmonicCoeffs::GetIndex( int l, int m ) const
{
	FW_ASSERT( 0 <= l && l < m_order );
	FW_ASSERT( -l <= m && m <= l );
	return CheckIndex( l*l + m + l );
}

inline int GpSphericalHarmonicCoeffs::CheckIndex( int n ) const
{
	FW_ASSERT( 0 <= n && n < m_order*m_order );
	return n;
}

inline void GpSphericalHarmonicCoeffs::SetCoefficients( FwVector4 const* pCoeffs )
{
	FW_ASSERT( pCoeffs );
	FwMemcpy( m_coeffs.Get(), pCoeffs, m_order*m_order*sizeof( FwVector4 ) );
}

inline FwVector4 const* GpSphericalHarmonicCoeffs::GetCoefficients() const
{
	return m_coeffs.Get();
}

inline FwVector4& GpSphericalHarmonicCoeffs::operator()( int l, int m )
{
	return m_coeffs[GetIndex( l, m )];
}

inline FwVector4_arg GpSphericalHarmonicCoeffs::operator()( int l, int m ) const
{
	return m_coeffs[GetIndex( l, m )];
}

#endif // ndef GP_SPHERICAL_HARMONICS_H

