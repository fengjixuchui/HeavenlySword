//--------------------------------------------------
//!
//!	\file colour_function.h
//!	Assorted editable objects that represent colour
//! functions used by effects
//!
//--------------------------------------------------

#ifndef _COLOUR_FUNCTION_H
#define _COLOUR_FUNCTION_H

#include "gfx/texture.h"
#include "camera/curves.h"

class FunctionCurve_User;
class DataInterfaceField;
class DataObject;

//--------------------------------------------------
//!
//! Virtual base class that implemnents a colour / alpha
//! table of some description:
//!		LERP
//!		PALETTE
//!		RGBA_CURVE
//!
//--------------------------------------------------
class ColourFunction
{
public:
	ColourFunction() : m_bHasChanged(false), m_bDebugRangesCalced(false)
	{
		m_iLastGenerationTick = NTCOLOUR_ARGB(0xff,0xff,0xff,0xff);
		m_aDebugCurveCol[0] = NTCOLOUR_ARGB(0xff,0xff,0,0);;
		m_aDebugCurveCol[1] = NTCOLOUR_ARGB(0xff,0,0xff,0);;
		m_aDebugCurveCol[2] = NTCOLOUR_ARGB(0xff,0,0,0xff);;
		m_aDebugCurveCol[3] = NTCOLOUR_ARGB(0xff,0xff,0xff,0xff);;
		m_borderCol = NTCOLOUR_ARGB(0xff,0,0,0);
	};
	virtual ~ColourFunction() {};

	enum COLFUNC_TYPE
	{
		COLFUNC_LERP,
		COLFUNC_PALETTE,
		COLFUNC_RGBA_CURVE,
	};

	enum TEXGEN_MODE
	{
		TEXGEN_NORMAL = 0,
		TEXGEN_GREYSCALE_R,
		TEXGEN_GREYSCALE_G,
		TEXGEN_GREYSCALE_B,
		TEXGEN_GREYSCALE_A,
	};

	virtual CVector	GetColour( float fU ) const = 0;
	virtual COLFUNC_TYPE GetType() const = 0;

	void RenderDebugTex( float fWidth, float fHeight, float fStartX, float fStartY );
	void RenderDebugGraph( float fminX, float fmaxX, float fminY, float fmaxY );

	Texture::Ptr GenerateTexture( TEXGEN_MODE eMODE, u_int iResolution, bool bHDR, bool bSaveToDisk, const char* pName = 0 );
	virtual bool HasChanged() const
	{
		if (m_bHasChanged)
		{
			m_bHasChanged = false;
			return true;
		}
		return false;
	};

protected:
	void	GenerateDebugTextures();
	uint32_t	m_aDebugCurveCol[4];
	uint32_t	m_borderCol;
	mutable bool m_bHasChanged;

	void	GetDebugBounds( float& fMin, float& fMax )
	{
		CalcDebugBounds();
		fMin = 0.0f;
		fMax = m_fMax;
	}

#ifdef _DEBUG
	static const int iDEBUG_RESOLUTION = 16;
#else
	static const int iDEBUG_RESOLUTION = 256;
#endif

private:
	void	CalcDebugBounds();

	u_int			m_iLastGenerationTick;
	Texture::Ptr	m_pDebugHDRTexture;
	Texture::Ptr	m_pDebugLDRTexture;
	Texture::Ptr	m_pDebugAlphaTexture;

	bool	m_bDebugRangesCalced;
	float	m_fMax;
};

//--------------------------------------------------
//!
//!	ColourFunction_Lerp
//!
//--------------------------------------------------
class ColourFunction_Lerp : public ColourFunction
{
public:
	ColourFunction_Lerp();
	~ColourFunction_Lerp();

	CVector			GetStart() const
	{
		return CVector(	m_obColourStart.X() * m_obColourStart.W(),
						m_obColourStart.Y() * m_obColourStart.W(),
						m_obColourStart.Z() * m_obColourStart.W(),
						m_fAlphaStart );
	}

	CVector			GetEnd() const
	{
		return CVector(	m_obColourEnd.X() * m_obColourEnd.W(),
						m_obColourEnd.Y() * m_obColourEnd.W(),
						m_obColourEnd.Z() * m_obColourEnd.W(),
						m_fAlphaEnd );
	}

	virtual CVector	GetColour( float fU ) const
	{
		CVector start( GetStart() );
		CVector gradient( GetEnd() - start );
		CVector result(start + (gradient * fU));
		
		result.W() = ntstd::Clamp( result.W(), 0.0f, 1.0f );
		return result;
	}

	virtual COLFUNC_TYPE GetType() const { return COLFUNC_LERP; }

	void PostConstruct( void );
	bool EditorChangeValue( CallBackParameter, CallBackParameter );
	void EditorChangeParent();
	void DebugRender( void );

	CVector		m_obColourStart;
	CVector		m_obColourEnd;

	float		m_fAlphaStart;
	float		m_fAlphaEnd;
};

//--------------------------------------------------
//!
//!	ColourFunction_Palette
//!
//--------------------------------------------------
class ColourFunction_Palette : public ColourFunction
{
public:
	ColourFunction_Palette();
	~ColourFunction_Palette();

	virtual CVector	GetColour( float fU ) const;
	virtual COLFUNC_TYPE GetType() const { return COLFUNC_PALETTE; }
	
	void PostConstruct( void );
	bool EditorChangeValue( CallBackParameter, CallBackParameter );
	void EditorChangeParent();
	void DebugRender( void );

	// uggg. this is because i cant have a static list or array in
	// a serialisable object
	CVector		m_obColour0;
	float		m_fAlpha0;
	float		m_fTime0;

	CVector		m_obColour1;
	float		m_fAlpha1;
	float		m_fTime1;

	CVector		m_obColour2;
	float		m_fAlpha2;
	float		m_fTime2;

	CVector		m_obColour3;
	float		m_fAlpha3;
	float		m_fTime3;

	CVector		m_obColour4;
	float		m_fAlpha4;
	float		m_fTime4;

	CVector		m_obColour5;
	float		m_fAlpha5;
	float		m_fTime5;

	CVector		m_obColour6;
	float		m_fAlpha6;
	float		m_fTime6;

	CVector		m_obColour7;
	float		m_fAlpha7;
	float		m_fTime7;

	CVector		m_obColour8;
	float		m_fAlpha8;
	float		m_fTime8;

	CVector		m_obColour9;
	float		m_fAlpha9;
	float		m_fTime9;

private:
	struct EntryFixup
	{
		void Set( CVector* pLight, float* pAlpha, float* pTime )
		{ m_pLight = pLight; m_pAlpha = pAlpha; m_pTime = pTime; }
	
		CVector	GetColour() const 
		{
			float fAlpha = ntstd::Clamp( *m_pAlpha, 0.0f, 1.0f );
			return CVector( m_pLight->X() * m_pLight->W(),
							m_pLight->Y() * m_pLight->W(),
							m_pLight->Z() * m_pLight->W(),
							fAlpha );
		}

		CVector*	m_pLight;
		float*		m_pAlpha;
		float*		m_pTime;
	};

	#define MAX_PALETTE_ENTRIES (10)

	EntryFixup	m_data[MAX_PALETTE_ENTRIES];

	struct PaletteNode
	{
		PaletteNode( const CVector& rgba, float fTime ) : m_rgba( rgba ), m_fTime( fTime ) {}

		CVector m_rgba;
		float m_fTime;
	};

	class CComparator_PaletteNode_LT
	{
	public:
		bool operator()( const PaletteNode* pFirst, const PaletteNode* pSecond ) const
		{
			return pFirst->m_fTime < pSecond->m_fTime;
		}
	};

	CCubicTimeHermite*	m_pGenerated;				//!< allocated in MC_EFFECTS
};

//--------------------------------------------------
//!
//!	ColourFunction_Curves
//!
//--------------------------------------------------
class ColourFunction_Curves : public ColourFunction
{
public:
	ColourFunction_Curves();
	~ColourFunction_Curves();

	virtual CVector	GetColour( float fU ) const;
	virtual COLFUNC_TYPE GetType() const { return COLFUNC_RGBA_CURVE; }
	
	void PostConstruct( void );
	bool EditorChangeValue( CallBackParameter, CallBackParameter );
	void EditorChangeParent();
	void DebugRender( void );
	void AutoConstruct( const DataInterfaceField* pField );

	FunctionCurve_User*	m_pobRFunction;
	FunctionCurve_User*	m_pobGFunction;
	FunctionCurve_User*	m_pobBFunction;
	FunctionCurve_User*	m_pobAFunction;

	ntstd::List<void*>	m_obObjects;	// storage for auto created thingumys

	virtual bool HasChanged() const
	{
		if (SubCurvesChanged())
		{
			m_bHasChanged= true;
			m_bForceNewDebugTex= true;
		}

		if (m_bHasChanged)
		{
			m_bHasChanged = false;
			return true;
		}
		return false;
	};

private:
	// check to see if any of our child curves have changed
	bool SubCurvesChanged() const;

	enum OVERIDE_STATUS
	{
		OS_NONE,
		OS_RED,
		OS_GREEN,
		OS_BLUE,
		OS_ALPHA,
	};
	void StartOveride( OVERIDE_STATUS eType );
	void UpdateOveride();
	void CleanOveride();

	OVERIDE_STATUS		m_eOveride;
	CCubicTimeHermite*	m_pOverideTemp;			//!< allocated in MC_EFFECTS
	FunctionCurve_User*	m_pOverideFunc;
	ntstd::List<CVector*, Mem::MC_EFFECTS>		m_overideSamples;
	float				m_fLastTime;
	mutable bool		m_bForceNewDebugTex;
};

#endif
