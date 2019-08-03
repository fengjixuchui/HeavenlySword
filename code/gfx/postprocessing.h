//--------------------------------------------------
//!
//!	\file postprocessing.h
//!	Deals with post processing details 
//!
//--------------------------------------------------

#ifndef _POSTPROCESSING_H
#define _POSTPROCESSING_H

#include "lightingtemplates.h"

class SpatialInterpolator;

//--------------------------------------------------
//!
//!	ColourTransformMatrix 
//! Defines the data required for a colour transform matrix
//!
//--------------------------------------------------
class ColourTransformMatrix
{
public:
	ColourTransformMatrix() :
		m_col1( 1.0f, 0.0f, 0.0f ),
		m_col2( 0.0f, 1.0f, 0.0f ),
		m_col3( 0.0f, 0.0f, 1.0f ),
		m_row4( 0.0f, 0.0f, 0.0f ) {}

	// The single accessor to the data
	void GetMatrix( CMatrix& result ) const
	{
		result[0] = CVector( m_col1.X(), m_col2.X(), m_col3.X(), 0.0f );
		result[1] = CVector( m_col1.Y(), m_col2.Y(), m_col3.Y(), 0.0f );
		result[2] = CVector( m_col1.Z(), m_col2.Z(), m_col3.Z(), 0.0f );
		result[3] = CVector( m_row4.X(), m_row4.Y(), m_row4.Z(), 1.0f );
	}

	// The 'rotation' of the colours
	CPoint	m_col1;
	CPoint	m_col2;
	CPoint	m_col3;

	// The 'transform' of the colours
	CPoint	m_row4;
};

//--------------------------------------------------
//!
//!	ColourTransformNode 
//! Holds a list of colour transform matrices and 
//! a time of day
//!
//--------------------------------------------------
class ColourTransformNode
{
public:
	ColourTransformNode() : m_fTOD(0.0f) {}

	// The colour transformation
	typedef ntstd::List< ColourTransformMatrix*, Mem::MC_GFX> ColourTransformMatrixList;
	ColourTransformMatrixList m_colourMats;

	// The single accessor to the data
	void GetMatrix( CMatrix& result ) const
	{
		result.SetIdentity();

		// Loop through all the matrices in our list and mulitply
		for (	ColourTransformMatrixList::const_iterator it = m_colourMats.begin();
				it != m_colourMats.end(); ++it )
		{
			CMatrix temp;
			(*it)->GetMatrix(temp);
			result *= temp;
		}
	}


	// Time of day we correspond to
	float GetSortingValue() const { return m_fTOD; }
	float m_fTOD;
};




//--------------------------------------------------
//!
//!	Spatial_PostProcessingNode 
//! Virtual base class for post processing nodes
//!
//--------------------------------------------------
class Spatial_PostProcessingNode
{
public:
	Spatial_PostProcessingNode() : m_centerOfInfluence( CONSTRUCT_CLEAR ) {};
	virtual ~Spatial_PostProcessingNode() {};

	virtual void GetMatrix( CMatrix& result ) = 0;
	virtual void DebugRender();
	CPoint	m_centerOfInfluence;
};

//--------------------------------------------------
//!
//!	Spatial_PostProcessingNode_Simple
//! Basic post processing node, takes a list of colour
//! matrices to produce a single colour matrix
//!
//--------------------------------------------------
class Spatial_PostProcessingNode_Simple : public Spatial_PostProcessingNode
{
public:
	// Retrive a matrix based on all our sub matrices
	virtual void GetMatrix( CMatrix& result )
	{
		result.SetIdentity();

		// Loop through all the matrices in our list and mulitply
		for (	ColourTransformMatrixList::const_iterator it = m_colourMats.begin();
				it != m_colourMats.end(); ++it )
		{
			CMatrix temp;
			(*it)->GetMatrix(temp);
			result *= temp;
		}
	}

	typedef ntstd::List< ColourTransformMatrix*, Mem::MC_GFX > ColourTransformMatrixList;
	ColourTransformMatrixList m_colourMats;
};

//--------------------------------------------------
//!
//!	Spatial_PostProcessingNode_TOD
//! Has a sub list of nodes based on time of day
//!
//--------------------------------------------------
class Spatial_PostProcessingNode_TOD : public Spatial_PostProcessingNode
{
public:
	void PostConstruct();
	void SanityCheck();

	virtual void GetMatrix( CMatrix& result );
	virtual void DebugRender();
	SortableList< ColourTransformNode, Mem::MC_GFX > m_colourNodes;
};




//--------------------------------------------------
//!
//!	PostProcessingSet 
//! Set of post processing nodes, linked by position
//!
//--------------------------------------------------
class PostProcessingSet
{
public:
	PostProcessingSet();
	~PostProcessingSet();
	void PostConstruct();
	bool EditorChangeValue(CallBackParameter,CallBackParameter);

	// get the current post processing value
	void GetMatrix( const CPoint& position, CMatrix& result ) const ;

	// the list of post processing nodes
	typedef ntstd::List< Spatial_PostProcessingNode*, Mem::MC_GFX > SpatialPostProcessingNodeList;
	SpatialPostProcessingNodeList m_nodes;

private:
	SpatialInterpolator* m_pSpatialInterpolator;
};

#endif // _POSTPROCESSING_H
