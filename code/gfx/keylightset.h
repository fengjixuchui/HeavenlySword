//--------------------------------------------------
//!
//!	\file keylightset.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef GFX_KEYLIGHT_SET_H
#define GFX_KEYLIGHT_SET_H

#include "gfx/lightingtemplates.h"
#include "gfx/spatialinterpolator.h"

//--------------------------------------------------
//!
//!	KeyLightNode 
//! takes a list of any of the above contributors
//! and generates a set of SH coeffs for all of them...
//!
//--------------------------------------------------
class KeyLightNode
{
public:
	static const CVector s_RaylieghCoEff;
	static const CVector s_MieCoEffatT2;
	static const CVector s_MieCoEffatT16;

	KeyLightNode();
	void Clear() { memset( this, 0, sizeof(KeyLightNode) ); }

	CDirection GetColour() const	{ return CDirection(m_colour) * m_colour.W(); }
	CDirection GetSkyColour() const	{ return CDirection(m_skyColour) * m_skyColour.W(); }

	CVector m_colour;
	CVector m_skyColour;
	CVector m_rayleighScattering;
	CVector m_mieScattering;
	float m_fInscatterMultiplier;
	float m_fHenleyGreensteinEccentricity;
	float m_fSunPower;
	float m_fSunMultiplier;
	float m_fWorldSunPower;
	float m_fWorldSunMultiplier;

	//! opereators to aide lerping between nodes
	KeyLightNode& operator+=( const KeyLightNode& keyLight )
	{
		// get new colour
		CDirection temp( GetColour() + keyLight.GetColour() );
		float fMax = max( max( temp.X(), temp.Y() ), temp.Z() );

		if (fMax > EPSILON)
			m_colour = CVector( temp ) * (1.0f / fMax);
		m_colour.W() = fMax;

		// get new sky colour
		temp = GetSkyColour() + keyLight.GetSkyColour();
		fMax = max( max( temp.X(), temp.Y() ), temp.Z() );

		if (fMax > EPSILON)
			m_skyColour = CVector( temp ) * (1.0f / fMax);
		m_skyColour.W() = fMax;
		
		// get everything else
		m_rayleighScattering += keyLight.m_rayleighScattering;
		m_mieScattering += keyLight.m_mieScattering;
		m_fInscatterMultiplier += keyLight.m_fInscatterMultiplier;
		m_fHenleyGreensteinEccentricity += keyLight.m_fHenleyGreensteinEccentricity;
		m_fSunPower += keyLight.m_fSunPower;
		m_fSunMultiplier += keyLight.m_fSunMultiplier;
		m_fWorldSunPower += keyLight.m_fWorldSunPower;
		m_fWorldSunMultiplier += keyLight.m_fWorldSunMultiplier;

		return *this;
	}

	KeyLightNode& operator*=( float fScale )
	{
		// vectors with W() scalars are special
		m_colour.W() *= fScale;
		m_skyColour.W() *= fScale;

		// everything else scaled normally
		m_rayleighScattering *= fScale;
		m_mieScattering *= fScale;
		m_fInscatterMultiplier *= fScale;
		m_fHenleyGreensteinEccentricity *= fScale;
		m_fSunPower *= fScale;
		m_fSunMultiplier *= fScale;
		m_fWorldSunPower *= fScale;
		m_fWorldSunMultiplier *= fScale;

		return *this;
	}
};

//--------------------------------------------------
//!
//!	KeyLightNode_TOD 
//!
//--------------------------------------------------
class KeyLightNode_TOD
{
public:
	KeyLightNode_TOD() : m_fTOD( 0.0f ) {};
	float GetSortingValue() const { return m_fTOD; }

	float			m_fTOD;
	KeyLightNode	m_light;
};





//--------------------------------------------------
//!
//!	Spatial_KeyLightNode 
//! Virtual base class for Key light nodes
//!
//--------------------------------------------------
class Spatial_KeyLightNode
{
public:
	Spatial_KeyLightNode() : m_centerOfInfluence( CONSTRUCT_CLEAR ) {};
	virtual ~Spatial_KeyLightNode() {};

	virtual void GetKeyLight( KeyLightNode& keyLight ) const = 0;
	CPoint	m_centerOfInfluence;
};

//--------------------------------------------------
//!
//!	Spatial_KeyLightNode_Simple
//! Basic spatial Key light node, a wrapper around a
//! KeyLightNode
//!
//--------------------------------------------------
class Spatial_KeyLightNode_Simple : public Spatial_KeyLightNode
{
public:
	virtual void GetKeyLight( KeyLightNode& keyLight ) const { keyLight = m_light; }
	KeyLightNode m_light;
};

//--------------------------------------------------
//!
//!	Spatial_KeyLightNode_TOD
//! Has a sub list of nodes based on time of day
//!
//--------------------------------------------------
class Spatial_KeyLightNode_TOD : public Spatial_KeyLightNode
{
public:
	void PostConstruct();
	void SanityCheck() const;

	mutable SortableList< KeyLightNode_TOD, Mem::MC_GFX > m_keyNodes;
	virtual void GetKeyLight( KeyLightNode& keyLight ) const;
};




//--------------------------------------------------
//!
//!	KeyLightSet 
//! Set of Key light nodes, linked by position
//!
//--------------------------------------------------
class KeyLightSet
{
public:
	KeyLightSet();
	~KeyLightSet();
	void PostConstruct();
	bool EditorChangeValue(CallBackParameter,CallBackParameter);

	// get the current Key light values 
	void GetKeyLight( const CPoint& position, KeyLightNode& keyLight ) const;

	// the list of Key light nodes
	typedef ntstd::List< Spatial_KeyLightNode*, Mem::MC_GFX > SpatialKeyLightNodeList;
	SpatialKeyLightNodeList m_nodes;

private:
	SpatialInterpolator* m_pSpatialInterpolator;

	class comparatorMoreThan
	{
	public:
		bool operator()( const PointIDandFraction* pFirst, const PointIDandFraction* pSecond ) const
		{
			return ( pFirst->fraction > pSecond->fraction );
		}
	};
};

#endif // GFX_KEYLIGHT_SET_H
