//--------------------------------------------------
//!
//!	\file filllightset.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _FILL_LIGHT_SET_H
#define _FILL_LIGHT_SET_H

#include "gfx/spatialinterpolator.h"
#include "gfx/lightingtemplates.h"

class SHContributor;
class SHEnvironment;

//--------------------------------------------------
//!
//!	FillLightNode 
//! takes a list of any SHContributor derived classes
//! and generates a set of SH coeffs for all of them...
//!
//--------------------------------------------------
class FillLightNode
{
public:
	FillLightNode() :
		m_reflectanceColour( 1.0f, 1.0f, 1.0f, 1.0f ),
		m_reflectanceCubeTexture( "reflect.dds" )
	{}

	typedef ntstd::List<SHContributor*, Mem::MC_GFX> SHContributorList;
	// fill light list
	SHContributorList m_SHLights;

	// access fill lights
	void GetContribution( SHEnvironment& environment ) const;

	// reflectance properties of this fill light node
	CDirection GetReflectanceColour() const { return CDirection(m_reflectanceColour)*m_reflectanceColour.W(); }
	CVector m_reflectanceColour;

	ntstd::String m_reflectanceCubeTexture;
};

//--------------------------------------------------
//!
//!	FillLightNode_TOD 
//!
//--------------------------------------------------
class FillLightNode_TOD
{
public:
	FillLightNode_TOD() : m_fTOD( 0.0f ) {};
	float GetSortingValue() const { return m_fTOD; }

	float			m_fTOD;
	FillLightNode	m_lights;
};




//--------------------------------------------------
//!
//!	Spatial_FillLightNode 
//! Virtual base class for fill light nodes
//!
//--------------------------------------------------
class Spatial_FillLightNode
{
public:
	Spatial_FillLightNode() : m_centerOfInfluence( CONSTRUCT_CLEAR ) {};
	virtual ~Spatial_FillLightNode() {};

	virtual void GetContribution( SHEnvironment& environment ) const = 0;
	virtual CDirection GetReflectanceColour() const = 0;
	virtual const char* GetReflectanceTexture() const = 0;

	CPoint	m_centerOfInfluence;
};

//--------------------------------------------------
//!
//!	Spatial_FillLightNode_Simple
//! Basic spatial fill light node, a wrapper around a
//! FillLightNode
//!
//--------------------------------------------------
class Spatial_FillLightNode_Simple : public Spatial_FillLightNode
{
public:
	virtual void GetContribution( SHEnvironment& environment ) const { m_lights.GetContribution( environment ); }
	virtual CDirection GetReflectanceColour() const { return m_lights.GetReflectanceColour(); }
	virtual const char* GetReflectanceTexture() const { return ntStr::GetString(m_lights.m_reflectanceCubeTexture); }

	FillLightNode	m_lights;
};

//--------------------------------------------------
//!
//!	Spatial_FillLightNode_TOD
//! Has a sub list of nodes based on time of day
//!
//--------------------------------------------------
class Spatial_FillLightNode_TOD : public Spatial_FillLightNode
{
public:
	void PostConstruct();
	void SanityCheck() const;

	mutable SortableList< FillLightNode_TOD, Mem::MC_GFX > m_fillNodes;

	virtual void GetContribution( SHEnvironment& environment ) const;
	virtual CDirection GetReflectanceColour() const;
	virtual const char* GetReflectanceTexture() const;
};

//--------------------------------------------------
//!
//!	FillLightSet 
//! Set of fill light nodes, linked by position
//!
//--------------------------------------------------
class FillLightSet
{
public:
	FillLightSet();
	~FillLightSet();
	void PostConstruct();
	bool EditorChangeValue(CallBackParameter,CallBackParameter);

	// get most important nodes at this time
	void CalcMostSignificantNodes( const CPoint& position );

	// get the current fill light values 
	// (must have called CalcMostSignificantNodes() before hand)
	void GetContribution( SHEnvironment& environment ) const;
	CDirection GetReflectanceColour() const;
	const char* GetReflectanceTexture() const;

	// the list of fill light nodes
	typedef ntstd::List< Spatial_FillLightNode*, Mem::MC_GFX > SpatialFillLightNodeList;
	SpatialFillLightNodeList m_nodes;

private:
	static const u_int		iMAX_CACHE_ENTRIES = 4;
	Spatial_FillLightNode*	m_apMostSignificant[iMAX_CACHE_ENTRIES];
	float					m_afMostSignificant[iMAX_CACHE_ENTRIES];
	SpatialInterpolator*	m_pSpatialInterpolator;

	class comparatorMoreThan
	{
	public:
		bool operator()( const PointIDandFraction* pFirst, const PointIDandFraction* pSecond ) const
		{
			return ( pFirst->fraction > pSecond->fraction );
		}
	};
};

#endif // _FILL_LIGHT_SET_H
