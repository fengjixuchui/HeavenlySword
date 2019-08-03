/***************************************************************************************************
*
*	$Header:: /game/curvetemplates.h 1     11/08/03 16:30 Wil                                      $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef TEMPLATE_CURVE_H
#define TEMPLATE_CURVE_H

#include "camera/camutils.h"

/***************************************************************************************************
*
*	CLASS			CTemplateBezier
*
*	DESCRIPTION		Representation of a bezier curve
*
*	Point(T) = [ t^3, t^2, t, 1 ] * [ -1  3 -3  1 ] * [ p0 ]
*									[  3 -6  3  0 ]   [ p1 ]
*									[ -3  3  0  0 ]   [ p2 ]
*									[  1  0  0  0 ]   [ p3 ]
*
*	NOTES			Publicly:
*						Provides constructors for Time and Normal curve types + from file
*						Provides a polynomial coefficent calculation function per segment
*						Provides CV accesors
*						Provides an info dump
*
***************************************************************************************************/
template<class CurveType, class NameTrait> class CTemplateBezier : public CurveType
{
public:
	// debug constructor intended for use with classes derived from CCurveInterface
	CTemplateBezier( bool bOpen, u_int uiNumCV, const CVector* pobVerts, bool bOwnsVerts = false, float fLength = -1.0f ) :
		CurveType( true, (uiNumCV - 1)/3, bOpen, fLength )
	{
		ConstructCore( pobVerts, bOwnsVerts );
	}

	// debug constructor intended for use with classes derived from CTimeCurveInterface
	CTemplateBezier( float* pfTimes, bool bOwnsTimes, bool bOpen, u_int uiNumCV, const CVector* pobVerts, bool bOwnsVerts = false, float fLength = -1.0f ) :
		CurveType( pfTimes, bOwnsTimes, true, (uiNumCV - 1)/3, bOpen, fLength )
	{
		ConstructCore( pobVerts, bOwnsVerts );
	}


	virtual ~CTemplateBezier( void ) { NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pobData ); }

	// calculate our polynomials from the control verts via our basis matrix
	virtual void GetCoeffs( int iSegment, CVector* pobCoeffs ) const
	{
		static const float gafBasisMat[4][4] =
		{
			{ -1.0f,  3.0f, -3.0f,  1.0f },
			{  3.0f, -6.0f,  3.0f,  0.0f },
			{ -3.0f,  3.0f,  0.0f,  0.0f },
			{  1.0f,  0.0f,  0.0f,  0.0f }
		};

		for (int i = 0; i < CUBIC_COEFFICIENTS; i++)
		{
			pobCoeffs[i].Clear();
			for (int j = 0; j < 4; j++)
				pobCoeffs[i] += GetCV((iSegment*3)+j) * gafBasisMat[i][j];
		}
	}
	
	virtual	u_int			GetNumCV( void )		const { return (CurveType::GetNumSpan()*3)+1; }
	virtual const CVector	GetCV( u_int uiIndex )	const { ntAssert(uiIndex<GetNumCV());	return m_pobData[uiIndex]; }
	
private:
	void	ConstructCore( const CVector* pobVerts, bool bOwnsVerts )
	{
		ntAssert( pobVerts );

		if(bOwnsVerts)
			m_pobData = pobVerts;
		else
		{
			CVector* pobData = NT_NEW_CHUNK( Mem::MC_CAMERA ) CVector[ GetNumCV() ];
			ntAssert( pobData );
			NT_MEMCPY( pobData, pobVerts, sizeof(CVector)*GetNumCV() );
			m_pobData = pobData;
		}
	}

	const CVector*			m_pobData;
};






/***************************************************************************************************
*
*	CLASS			CTemplateHermite
*
*	DESCRIPTION		Representation of a hermite curve
*
*	Point(T) = [ t^3, t^2, t, 1 ] * [  2 -2  3  3 ] * [ p0 ]
*									[ -3  3 -6 -3 ]   [ p1 ]
*									[  0  0  3  0 ]   [ t0 ]
*									[  1  0  0  0 ]   [ T0 ]
*
*	NOTES			Publicly:
*						Provides constructors for Time and Normal curve types + from file
*						Provides a polynomial coefficent calculation function per segment
*						Provides CV accesors
*						Provides Tangent accesors
*						Provides an info dump
*						Overides the debug CV render method
*
***************************************************************************************************/
template<class CurveType, class NameTrait> class CTemplateHermite : public CurveType
{
public:
	// debug constructor intended for use with classes derived from CCurveInterface
	CTemplateHermite(	bool bOpen, u_int uiNumCV, const CVector* pobVerts,
						const CVector* pobStarts, const CVector* pobEnds,
						bool bOwnsVerts = false, bool bCleanTans = true, float fLength = -1.0f ) :
		CurveType( true, uiNumCV - 1, bOpen, fLength )
	{
		ConstructCore(pobVerts,pobStarts,pobEnds,bOwnsVerts,bCleanTans);
	}

	// debug constructor intended for use with classes derived from CTimeCurveInterface
	CTemplateHermite(	float* pfTimes, bool bOwnsTimes, bool bOpen, u_int uiNumCV,
						const CVector* pobVerts, const CVector* pobStarts, const CVector* pobEnds,
						bool bOwnsVerts = false, bool bCleanTans = true, float fLength = -1.0f ) :
		CurveType( pfTimes, bOwnsTimes, true, uiNumCV - 1, bOpen, fLength )
	{
		ConstructCore(pobVerts,pobStarts,pobEnds,bOwnsVerts,bCleanTans);	
	}


	virtual ~CTemplateHermite( void )
	{
		NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pobData );
		if (m_bCleanTangents)
		{
			NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pobStarts );
			NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pobEnds );
		}
	}

	// calculate our polynomials from the control verts via our basis matrix
	virtual void GetCoeffs( int iSegment, CVector* pobCoeffs ) const
	{
		static const float gafBasisMat[4][4] =
		{
			{  2.0f, -2.0f,  3.0f,  3.0f },
			{ -3.0f,  3.0f, -6.0f, -3.0f },
			{  0.0f,  0.0f,  3.0f,  0.0f },
			{  1.0f,  0.0f,  0.0f,  0.0f }
		};

		for (int i = 0; i < CUBIC_COEFFICIENTS; i++)
		{
			pobCoeffs[i].Clear();
			pobCoeffs[i] += GetCV(iSegment) * gafBasisMat[i][0];
			pobCoeffs[i] += GetCV(iSegment+1) * gafBasisMat[i][1];
			pobCoeffs[i] += GetStartTan(iSegment) * gafBasisMat[i][2];
			pobCoeffs[i] += GetEndTan(iSegment) * gafBasisMat[i][3];
		}
	}
	
	virtual	u_int			GetNumCV( void )			const { return CurveType::GetNumSpan()+1; }
	virtual const CVector	GetCV( u_int uiIndex )		const { ntAssert(uiIndex<GetNumCV());		return m_pobData[uiIndex]; }
	u_int					GetNumTan( void )			const { return CurveType::GetNumSpan(); }
	const CVector			GetStartTan( u_int uiIndex )const { ntAssert(uiIndex<GetNumTan());	return m_pobStarts[uiIndex]; }
	const CVector			GetEndTan( u_int uiIndex )	const { ntAssert(uiIndex<GetNumTan());	return m_pobEnds[uiIndex]; }	
	
	virtual void			DebugRenderCVs( void )		const
	{
		#ifndef _RELEASE

		for (int i = 0; i < (int)CurveType::GetNumSpan(); i++)
		{
			CPoint obStart( GetCV(i) );
			CPoint obEnd( GetCV(i+1) );

			CPoint obMid1 = CPoint( GetStartTan(i) ) + obStart;
			CPoint obMid2 = obEnd - CPoint( GetEndTan(i) );

			CCamUtil::Render_Line( obStart, obMid1, 1.0,0.0f,1.0f,1.0f );
			CCamUtil::Render_Line( obMid1, obMid2, 1.0,0.0f,1.0f,1.0f );
			CCamUtil::Render_Line( obMid2, obEnd, 1.0,0.0f,1.0f,1.0f );
		}

		#endif
	}

private:
	void	ConstructCore(	const CVector* pobVerts,
							const CVector* pobStarts,
							const CVector* pobEnds,
							bool bOwnsVerts, bool bCleanTans )
	{
		ntAssert( pobVerts );
		ntAssert( pobStarts );
		ntAssert( pobEnds );

		if(bOwnsVerts)
		{
			m_pobData = pobVerts;
			m_pobStarts = pobStarts;
			m_pobEnds = pobEnds;
			m_bCleanTangents = bCleanTans;
		}
		else
		{
			CVector* pobData = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) CVector[ (GetNumTan() * 2) + GetNumCV() ];
			ntAssert( pobData );
			NT_MEMCPY( pobData, pobVerts, sizeof(CVector) * GetNumCV() );
			m_pobData = pobData;
			
			pobData += GetNumCV();
			NT_MEMCPY( pobData, pobStarts, sizeof(CVector) * GetNumTan() );
			m_pobStarts = pobData;
			
			pobData += GetNumTan();
			NT_MEMCPY( pobData, pobEnds, sizeof(CVector) * GetNumTan() );
			m_pobEnds = pobData;
			m_bCleanTangents = false;
		}
	}

	bool					m_bCleanTangents;
	const CVector*			m_pobData;
	const CVector*			m_pobStarts;
	const CVector*			m_pobEnds;
};







/***************************************************************************************************
*
*	CLASS			CTemplateCatmull
*
*	DESCRIPTION		Representation of a catmull-rom spline
*
*	Point(T) = (1/2) * [ t^3, t^2, t, 1 ] * [ -1  3 -3  1 ] * [ p0 ]
*											[  2 -5  4 -1 ]   [ p1 ]
*											[ -1  0  1  0 ]   [ p2 ]
*											[  0  2  0  0 ]   [ p3 ]	
*
*	NOTES			Publicly:
*						Provides constructors for Time and Normal curve types + from file
*						Provides a polynomial coefficent calculation function per segment
*						Provides CV accesors
*						Provides an info dump
*
***************************************************************************************************/
template<class CurveType, class NameTrait> class CTemplateCatmull : public CurveType
{
public:
	// debug constructor intended for use with classes derived from CCurveInterface
	CTemplateCatmull( bool bOpen, u_int uiNumCV, const CVector* pobVerts, bool bOwnsVerts = false, float fLength = -1.0f ) :
		CurveType( true, uiNumCV - 3, bOpen, fLength )
	{
		ConstructCore( pobVerts, bOwnsVerts );
	}

	// debug constructor intended for use with classes derived from CTimeCurveInterface
	CTemplateCatmull( float* pfTimes, bool bOwnsTimes, bool bOpen, u_int uiNumCV, const CVector* pobVerts, bool bOwnsVerts = false, float fLength = -1.0f ) :
		CurveType( pfTimes, bOwnsTimes, true, uiNumCV - 3, bOpen, fLength )
	{
		ConstructCore( pobVerts, bOwnsVerts );
	}


	virtual ~CTemplateCatmull( void ) { NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pobData ); }

	// calculate our polynomials from the control verts via our basis matrix
	virtual void GetCoeffs( int iSegment, CVector* pobCoeffs ) const
	{
		static const float gafBasisMat[4][4] = // NB has 0.5f scalar premultiplied
		{
			{ -0.5f,  1.5f, -1.5f,  0.5f },
			{  1.0f, -2.5f,  2.0f, -0.5f },
			{ -0.5f,  0.0f,  0.5f,  0.0f },
			{  0.0f,  1.0f,  0.0f,  0.0f }
		};

		for (int i = 0; i < CUBIC_COEFFICIENTS; i++)
		{
			pobCoeffs[i].Clear();
			for (int j = 0; j < 4; j++)
				pobCoeffs[i] += GetCV(iSegment+j) * gafBasisMat[i][j];
		}
	}

	virtual u_int			GetNumCV( void )		const { return CurveType::GetNumSpan()+3; }
	virtual const CVector	GetCV( u_int uiIndex )	const { ntAssert(uiIndex<GetNumCV());	return m_pobData[uiIndex]; }

private:
	void	ConstructCore( const CVector* pobVerts, bool bOwnsVerts )
	{
		ntAssert( pobVerts );

		if(bOwnsVerts)
			m_pobData = pobVerts;
		else
		{
			CVector* pobData = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) CVector[ GetNumCV() ];
			ntAssert( pobData );
			NT_MEMCPY( pobData, pobVerts, sizeof(CVector)*GetNumCV() );
			m_pobData = pobData;
		}
	}
	const CVector*		m_pobData;
};






/***************************************************************************************************
*
*	CLASS			CTemplateUBS
*
*	DESCRIPTION		Representation of a uniform non-rational basis spline
*
*	Point(T) = (1/6) * [ t^3, t^2, t, 1 ] * [ -1  3 -3  1 ] * [ p0 ]
*											[  3 -6  3  0 ]   [ p1 ]
*											[ -3  0  3  0 ]   [ p2 ]
*											[  1  4  1  0 ]   [ p3 ]	
*
*	NOTES			Publicly:
*						Provides constructors for Time and Normal curve types + from file
*						Provides a polynomial coefficent calculation function per segment
*						Provides CV accesors
*						Provides an info dump
*
***************************************************************************************************/
template<class CurveType, class NameTrait> class CTemplateUBS : public CurveType
{
public:
	// debug constructor intended for use with classes derived from CCurveInterface
	CTemplateUBS( bool bOpen, u_int uiNumCV, const CVector* pobVerts, bool bOwnsVerts = false, float fLength = -1.0f ) :
		CurveType( true, uiNumCV - 3, bOpen, fLength )
	{
		ConstructCore( pobVerts, bOwnsVerts );
	}

	// debug constructor intended for use with classes derived from CTimeCurveInterface
	CTemplateUBS( float* pfTimes, bool bOwnsTimes, bool bOpen, u_int uiNumCV, const CVector* pobVerts, bool bOwnsVerts = false, float fLength = -1.0f ) :
		CurveType( pfTimes, bOwnsTimes, true, uiNumCV - 3, bOpen, fLength )
	{
		ConstructCore( pobVerts, bOwnsVerts );
	}


	virtual ~CTemplateUBS( void ) { NT_DELETE_ARRAY_CHUNK(Mem::MC_CAMERA, m_pobData ); }

	// calculate our polynomials from the control verts via our basis matrix
	virtual void GetCoeffs( int iSegment, CVector* pobCoeffs ) const
	{
		#define I_DIV_6		0.16666666666666666666666666666667f
		#define II_DIV_3	0.66666666666666666666666666666667f

		static const float gafBasisMat[4][4] = // NB has 1.0f/6.0f scalar premultiplied
		{
			{ -I_DIV_6,  0.5f,		-0.5f,		I_DIV_6 },
			{  0.5f,	-1.0f,		 0.5f,		0.0f },
			{ -0.5f,	 0.0f,		 0.5f,		0.0f },
			{  I_DIV_6,  II_DIV_3,	 I_DIV_6,	0.0f }
		};

		for (int i = 0; i < CUBIC_COEFFICIENTS; i++)
		{
			pobCoeffs[i].Clear();
			for (int j = 0; j < 4; j++)
				pobCoeffs[i] += GetCV(iSegment+j) * gafBasisMat[i][j];
		}
	}

	virtual u_int			GetNumCV( void )		const { return CurveType::GetNumSpan()+3; }
	virtual const CVector	GetCV( u_int uiIndex )	const { ntAssert(uiIndex<GetNumCV());	return m_pobData[uiIndex]; }

private:
	void	ConstructCore( const CVector* pobVerts, bool bOwnsVerts )
	{
		ntAssert( pobVerts );

		if(bOwnsVerts)
			m_pobData = pobVerts;
		else
		{
			CVector* pobData = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) CVector[ GetNumCV() ];
			ntAssert( pobData );
			NT_MEMCPY( pobData, pobVerts, sizeof(CVector)*GetNumCV() );
			m_pobData = pobData;
		}
	}
	const CVector*		m_pobData;
};

#endif // TEMPLATE_CURVE_H
