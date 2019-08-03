//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Implementation of Euclidean to spherical harmonic rotation conversion.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_CHOI_ROTATION_H
#define GP_CHOI_ROTATION_H

#include <Fw/FwMaths/FwMatrix44.h>
#include <Fw/FwMaths/FwTransform.h>
#include <Fw/FwMaths/FwVector4.h>
#include <Fw/FwStd/FwStdScopedPtr.h>
#include <Gp/GpSphericalHarmonics/GpSphericalHarmonics.h>
#include <Gp/GpSphericalHarmonics/GpSphericalHarmonicRotation.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DECLARATIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Implementation of Euclidean to spherical harmonic rotation conversion by Choi et al.
**/
//--------------------------------------------------------------------------------------------------

class GpChoiRotation : FwNonCopyable
{
public:
	explicit GpChoiRotation( int order );

	void GenerateRotation( const FwTransform& rotation );
	void GetResult( GpSphericalHarmonicRotation& reals ) const;

private:
	static sh_real a( int l, int m, int n );
	static sh_real b( int l, int m, int n );
	static sh_real c( int l, int m, int n );
	static sh_real d( int l, int m, int n );

	sh_real H( int l, int m, int n, int i, int j ) const;
	sh_real K( int l, int m, int n, int i, int j ) const;

	void RecA( int l, int m, int n );
	void RecB( int l, int m, int n );
	void RecC( int l, int m, int n );

	static sh_real pow_1( int a );

	sh_real R( int l, int m, int n ) const;
	sh_real S( int l, int m, int n ) const;
	sh_real T( int l, int m, int n ) const;

	struct Rotation
	{
		sh_real xx;
		sh_real xy;
		sh_real xz;
		sh_real yx;
		sh_real yy;
		sh_real yz;
		sh_real zx;
		sh_real zy;
		sh_real zz;
	};

	int m_order;
	Rotation m_rotation;
	GpSphericalHarmonicRotation m_F, m_G;
};

#endif // ndef GP_CHOI_ROTATION_H

