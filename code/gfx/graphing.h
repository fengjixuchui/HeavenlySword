/***************************************************************************************************
*
*	DESCRIPTION		Debug Graphing Module (Definition)
*
*	NOTES
*
***************************************************************************************************/

#ifndef	_GRAPHING_H
#define	_GRAPHING_H

#if !defined(CORE_HASH_H)
#include "core/hash.h"
#endif

class	CGraphSample;
class	CGraphSampleSet;
class	CGraph;
class	CDebugPrimitives;

/***************************************************************************************************
*
*	CLASS			CGraphSample
*
*	DESCRIPTION		This is the data for a single graph sample. 
*
***************************************************************************************************/

class	CGraphSample
{
public:
	CGraphSample( float fX, float fY )	{ X() = fX; Y() = fY; };

	// Float Accessors
	const float&	X() const							{ return m_fX; };
	const float&	Y() const							{ return m_fY; };
	float&	X()											{ return m_fX; };
	float&	Y() 										{ return m_fY; };

protected:
	CGraphSample();

	float	m_fX;
	float	m_fY;
};


/***************************************************************************************************
*
*	CLASS			CGraphSampleSet
*
*	DESCRIPTION		This is an object that manages a group of samples.
*
***************************************************************************************************/

class	CGraphSampleSet
{
	friend class CGraph;

public:
	CGraphSample*	GetSampleData( void ) const			{ return m_pSampleData; };
	void			SetUsedSamples( int iUsedSamples )	{ ntAssert( iUsedSamples <= m_iMaxSamples ); m_iUsedSamples = iUsedSamples; };
	int				GetUsedSamples( void ) const		{ return m_iUsedSamples; };
	int				GetMaxSamples( void ) const			{ return m_iMaxSamples; };
	uint32_t		GetColour( void ) const				{ return m_iColour; };
	void			AddSample( float fValue );

protected:
	CGraphSampleSet();
	~CGraphSampleSet();

	static	CGraphSampleSet*	Create( const char* pcName, int iMaxSamples, uint32_t iColour, CGraph* pobGraph );
	static	void				Destroy( CGraphSampleSet* pSampleSet );
	void						Invalidate( void );
	CHashedString				GetNameHash( void ) const { return m_name; };

	CHashedString	m_name;
	CGraph*			m_pGraph;
	int				m_iUsedSamples;
	int				m_iMaxSamples;
	uint32_t		m_iColour;
	CGraphSample*	m_pSampleData;
};

/***************************************************************************************************
*
*	ENUMERATION		GRAPH_TYPE
*
*	DESCRIPTION		We have a number of graph types.. this enumeration defines them..
*
***************************************************************************************************/

enum	GRAPH_TYPE
{
	GRAPH_TYPE_STANDARD,
	GRAPH_TYPE_ROLLING,
};

/***************************************************************************************************
*
*	CLASS			GraphRenderer
*
*	DESCRIPTION		Platform specific graphing class, friend of CGraph
*
*	NOTES			Defined in CPP of platform specific graph implementation.
*
***************************************************************************************************/
class GraphRenderer;

/***************************************************************************************************
*
*	CLASS			CGraph
*
*	DESCRIPTION		This is the main graphing object. It effectively 'owns' a number of sample sets
*					and is the object that's actually rendered on a flush of the debug primitive
*					buffer.
*
***************************************************************************************************/

class	CGraph
{
	friend class GraphRenderer;

public:
	CGraph( GRAPH_TYPE eGraphType ); 
	~CGraph();

	//DGF
	bool DeleteSampleSet(const char* pcName);

	void				SetHistogramMode( bool bHistogramMode )				{ ntAssert( m_eGraphType == GRAPH_TYPE_STANDARD ); m_bHistogramMode = bHistogramMode; };
	void				SetBackColour( uint32_t iBackColour )					{ m_iBackColour = iBackColour; };
	void				SetAxisColour( uint32_t iAxisColour )					{ m_iAxisColour = iAxisColour; };
	void				SetMarkColour( uint32_t iMarkColour )					{ m_iMarkColour = iMarkColour; };
	void				SetLimitColour( uint32_t iLimitColour )
	{
		ntAssert( m_eGraphType == GRAPH_TYPE_ROLLING );
		m_iLimitColour = iLimitColour;
	};

	void				SetLimitValue( float fLimitValue )
	{	
		ntAssert( ( m_eGraphType == GRAPH_TYPE_ROLLING ) && ( fLimitValue >= 0.0f ) && ( fLimitValue <= m_fMaxYAxis ) );
		m_fLimitValue = fLimitValue;
	};

	void				SetXAxis( float fMinX, float fMaxX, float fXStep )
	{
		if ( m_eGraphType == GRAPH_TYPE_ROLLING )
		{
			ntAssert( fMinX == 0.0f );
			Invalidate();
		}

		ntAssert( fMinX < fMaxX );
		m_fMinXAxis = fMinX;
		m_fMaxXAxis = fMaxX;
		m_fXAxisMarkStep = fXStep;
	};

	void				SetYAxis( float fMinY, float fMaxY, float fYStep )
	{
		if ( m_eGraphType == GRAPH_TYPE_ROLLING )
		{
			//ntAssert( fMinY == 0.0f );
			Invalidate();
		}

		ntAssert( fMinY < fMaxY );
		m_fMinYAxis = fMinY;
		m_fMaxYAxis = fMaxY;
		m_fYAxisMarkStep = fYStep;
	};
	
	CGraphSampleSet*	AddSampleSet( const char* pcName, int iSampleCount, uint32_t iSetColour );
	CGraphSampleSet*	GetSampleSet( const char* pcName );

	GRAPH_TYPE			GetGraphType( void ) const { return m_eGraphType; };

	float				GetMinXAxis( void ) const { return m_fMinXAxis; };
	float				GetMaxXAxis( void ) const { return m_fMaxXAxis; };
	float				GetMinYAxis( void ) const { return m_fMinYAxis; };
	float				GetMaxYAxis( void ) const { return m_fMaxYAxis; };

	void				RecomputeAxes( void );
	void				Render( const CMatrix& toScreen, const CMatrix& screenToView ) const;

protected:
	void				InternalCtor( GRAPH_TYPE eGraphType );
	void				InternalDtor();
	typedef ntstd::List<CGraphSampleSet*, Mem::MC_GFX> GraphSampleSetList;
	void				Invalidate( void )
	{
		// Iterate through each sample set in the graph, invalidating data.
		for ( GraphSampleSetList::const_iterator obIt = m_sampleSets.begin(); obIt != m_sampleSets.end(); ++obIt )
		{
			( *obIt )->Invalidate();
		}
	}

	GraphSampleSetList	m_sampleSets;

	GRAPH_TYPE		m_eGraphType;
	bool			m_bHistogramMode;
	
	// Colour information
	uint32_t		m_iBackColour;
	uint32_t		m_iAxisColour;
	uint32_t		m_iMarkColour;
	uint32_t		m_iLimitColour;

	// Limit value for rolling graph usage
	float			m_fLimitValue;

	// X Axis information
	float			m_fMinXAxis;
	float			m_fMaxXAxis;
	float			m_fXAxisMarkStep;

	// Y Axis information
	float			m_fMinYAxis;
	float			m_fMaxYAxis;
	float			m_fYAxisMarkStep;

	GraphRenderer*	m_pRenderer;
};


#endif	//_GRAPHING_H
