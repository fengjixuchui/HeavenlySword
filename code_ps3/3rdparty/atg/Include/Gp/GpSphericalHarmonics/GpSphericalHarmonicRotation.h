//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Spherical harmonic coefficient rotation.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_SPHERICAL_HARMONIC_ROTATION_H
#define GP_SPHERICAL_HARMONIC_ROTATION_H

#include <Fw/FwMaths/FwVector.h>
#include <Fw/FwStd/FwStdScopedPtr.h>
#include <Gp/GpSphericalHarmonics/GpSphericalHarmonics.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DECLARATIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Stores a spherical harmonic rotation blockwise by band.
**/
//--------------------------------------------------------------------------------------------------

class GpSphericalHarmonicRotation : FwNonCopyable
{
public:
	explicit GpSphericalHarmonicRotation( int order );

	int		GetOrder() const;

	sh_real&	operator()( int l, int i, int j );
	sh_real	operator()( int l, int i, int j ) const;

	void	Transform( GpSphericalHarmonicCoeffs const& source, 
					   GpSphericalHarmonicCoeffs& dest ) const;

private:
	//! Band matrix with +/- lookup.
	class Band
	{
	public:
		void SetElements( sh_real* elements, int width );

		sh_real& operator()( int i, int j );
		sh_real operator()( int i, int j ) const;

	private:
		int GetIndex( int i, int j ) const;
		int CheckIndex( int n ) const;

		sh_real* m_elements;
		int m_width;
		int m_offset;
	};

	int		CheckBand( int l ) const;

	int m_order;
	FwStd::ScopedArrayPtr< Band > m_bands;
	FwStd::ScopedArrayPtr< sh_real > m_elements;
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline int GpSphericalHarmonicRotation::GetOrder() const
{
	return m_order;
}

inline sh_real& GpSphericalHarmonicRotation::operator()( int l, int i, int j )		
{ 
	return m_bands[CheckBand(l)]( i, j ); 
}

inline sh_real GpSphericalHarmonicRotation::operator()( int l, int i, int j ) const
{ 
	return m_bands[CheckBand(l)]( i, j ); 
}

inline sh_real& GpSphericalHarmonicRotation::Band::operator()( int i, int j )		
{ 
	return m_elements[GetIndex( i, j )]; 
}

inline sh_real GpSphericalHarmonicRotation::Band::operator()( int i, int j ) const
{ 
	return m_elements[GetIndex( i, j )]; 
}

inline int GpSphericalHarmonicRotation::Band::GetIndex( int i, int j ) const
{
	FW_ASSERT( -m_offset <= i && i <= m_offset );
	FW_ASSERT( -m_offset <= j && j <= m_offset );
	return CheckIndex( ( i + m_offset )*m_width + j + m_offset );
}

inline int GpSphericalHarmonicRotation::Band::CheckIndex( int n ) const
{
	FW_ASSERT( 0 <= n && n < m_width*m_width );
	return n;
}

inline int GpSphericalHarmonicRotation::CheckBand( int l ) const 
{
	FW_ASSERT( 0 <= l && l < m_order );
	return l;
}

#endif // ndef GP_SPHERICAL_HARMONIC_ROTATION_H

