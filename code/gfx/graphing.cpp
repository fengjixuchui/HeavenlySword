/***************************************************************************************************
*
*	DESCRIPTION		Debug Graphing Module (Implementation)
*
*	NOTES	
*
***************************************************************************************************/

#include "gfx/graphing.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"


/***************************************************************************************************
*
*	FUNCTION		CGraphSampleSet::Create
*
*	DESCRIPTION		Creates a CGraphSampleSet object, 
*
*	INPUTS			pcName		-	Name of the sample set
*
*					iMaxSamples	-	Number of samples this set can contain
*
*					iSetColour	-	The colour that this sets samples should be displayed in
*
*					pobGraph	-	A pointer to the CGraph that owns this sample set.
*
***************************************************************************************************/

CGraphSampleSet*	CGraphSampleSet::Create( const char* pcName, int iMaxSamples, uint32_t iSetColour, CGraph* pobGraph )
{
	ntAssert( iMaxSamples );

	// Allocate memory for a CGraphSampleSet object, and all the required samples
	int	iSampleSetSize	= ROUND_POW2( sizeof( CGraphSampleSet ), 16 );
	int iAllocateSize	= iSampleSetSize + ( iMaxSamples * sizeof( CGraphSample ) );
	CGraphSampleSet*	pSampleSet = ( CGraphSampleSet* )( NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) char[ iAllocateSize ] );

	// Fill in the core of the sample set object
	pSampleSet->m_name			=	CHashedString( pcName );
	pSampleSet->m_pGraph		=	pobGraph;
	pSampleSet->m_iUsedSamples	=	0;	
	pSampleSet->m_iMaxSamples		=	iMaxSamples;
	pSampleSet->m_iColour		=	iSetColour;
	pSampleSet->m_pSampleData	=	( CGraphSample* )( ( ( char* )pSampleSet ) + iSampleSetSize );

	// Make sure all samples in the set are initialised..
	pSampleSet->Invalidate();

	// Now return a pointer to the CGraphSampleSet object..
	return	pSampleSet;
}


/***************************************************************************************************
*
*	FUNCTION		CGraphSampleSet::Destroy
*
*	DESCRIPTION		Destroys the specified sample set.
*
*	INPUTS			pSampleSet		-	Pointer to CGraphSampleSet object that needs destroying.
*
*	NOTES			This is only to be called from within CGraph.
*
***************************************************************************************************/

void	CGraphSampleSet::Destroy( CGraphSampleSet* pSampleSet )
{
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, ( char* )pSampleSet );
}


/***************************************************************************************************
*
*	FUNCTION		CGraphSampleSet::Invalidate
*
*	DESCRIPTION		There are some cases where changes in graph properties must invalidate any
*					existing data within our sample data buffer. Well, actually, at the time of
*					writing there's only one case - and that's when axis lengths are changed.
*
***************************************************************************************************/

void	CGraphSampleSet::Invalidate( void )
{
	for ( int iLoop = 0; iLoop < GetMaxSamples(); iLoop++ )
	{	
		if ( m_pGraph->GetGraphType() == GRAPH_TYPE_ROLLING )
			m_pSampleData[ iLoop ].X() = ( m_pGraph->GetMaxXAxis() / ( float )( GetMaxSamples() - 1 ) ) * ( float )iLoop;
		else
			m_pSampleData[ iLoop ].X() = 0.0f;

		m_pSampleData[ iLoop ].Y() = 0.0f;
	}

	m_iUsedSamples = 0;
}


/***************************************************************************************************
*
*	FUNCTION		CGraphSampleSet::AddSample
*
*	DESCRIPTION		Rolling graphs needs samples adding at regular intervals.. and this is the
*					way they're added. 
*
*	INPUTS			fValue		-		Value to add as a sample
*
*	NOTES			The values are clamped to the minimum & maximum Y axis ranges..
*
***************************************************************************************************/

void	CGraphSampleSet::AddSample( float fValue )
{
	// We can only add samples to rolling graphs
	ntAssert( m_pGraph->GetGraphType() == GRAPH_TYPE_ROLLING );

	fValue = clamp( fValue, m_pGraph->GetMinYAxis(), m_pGraph->GetMaxYAxis() );
	
	// We use the 'm_iUsedSamples' variable as our current marker.
	if ( m_iUsedSamples >= m_iMaxSamples )
		m_iUsedSamples = 0;

	m_pSampleData[ m_iUsedSamples ].X() = ( m_pGraph->GetMaxXAxis() / ( float )( GetMaxSamples() - 1 ) ) * ( float )m_iUsedSamples;
	m_pSampleData[ m_iUsedSamples ].Y() = fValue;
	
	m_iUsedSamples++;
}


/***************************************************************************************************
*
*	FUNCTION		CGraph::InternalCtor
*
*	DESCRIPTION		Constructs a CGraph object of the type specified in 'eGraphType'. At the time
*					of writing, we support two graph types. One ('STANDARD') is used to display
*					function evaluation graphs.. it might be used to display things like exposure
*					ramps, gamma ramps and so on. The other ('ROLLING') is a roll-over based graph
*					that accumulates samples over time. This kind of graph is suitable for Gatso
*					display (or memory usage). Rolling graphs also support the display of a 'limit
*					marker' that highlights an important value on the 'Y' axis. For the gatso case,
*					this might highlight a CPU time of 100%..
*
*					If you want to change the properties of the graph (minimum/maximum axis lengths,
*					marker step values, limit marker values and so on), please do so through the 
*					accessors that are defined in 'graphing.h'. 
*
*	INPUTS			eGraphType		-	Type of graph (GRAPH_TYPE_STANDARD or GRAPH_TYPE_ROLLING)
*
*	NOTES			Each type of graph has different default settings. Look at the code if you want
*					to know what they are, as they might change with time..
*
*					Please be aware that the axes at (X=0, Y=0) must be visible in all cases. 
*
*					Also, you should know that rolling graphs can only represent positive ranges.
*					So, don't try setting minimum axis lengths of < 0.0f, as you'll only trip an
*					assertion. 
*
***************************************************************************************************/

void CGraph::InternalCtor( GRAPH_TYPE eGraphType )
{
	// Graph Type
	m_eGraphType		=	eGraphType;
	m_bHistogramMode	=	false;

	// Colour information
	m_iBackColour		=	NTCOLOUR_ARGB( 0x20, 0x00, 0x00, 0x00 );
	m_iAxisColour		=	NTCOLOUR_ARGB( 0xff, 0xff, 0xff, 0xff );
	m_iMarkColour		=	NTCOLOUR_ARGB( 0x60, 0x80, 0x80, 0x80 );
	m_iLimitColour		=	NTCOLOUR_ARGB( 0xff, 0xff, 0x00, 0x00 );

	switch ( eGraphType )
	{
		case	GRAPH_TYPE_STANDARD:
		{
			// Standard-mode default X Axis information
			m_fMinXAxis			=	-1.0f;
			m_fMaxXAxis			=	1.0f;	
			m_fXAxisMarkStep	=	0.5f;
	
			// Standard-mode default Y Axis information
			m_fMinYAxis			=	-1.0f;
			m_fMaxYAxis			=	1.0f;
			m_fYAxisMarkStep	=	0.5f;

			// Default is a limit value of 0.0f, 'cos it's for rolling types only
			m_fLimitValue		=	0.0f;

			break;
		}

		case	GRAPH_TYPE_ROLLING:
		{
			// Rolling-mode default X Axis information
			m_fMinXAxis			=	0.0f;
			m_fMaxXAxis			=	3.0f;
			m_fXAxisMarkStep	=	0.5f;

			// Rolling-mode default Y Axis information
			m_fMinYAxis			=	0.0f;
			m_fMaxYAxis			=	2.0f;
			m_fYAxisMarkStep	=	0.5f;

			// Default is a limit value of 1.0f
			m_fLimitValue		=	1.0f;
			break;
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGraph::InternalDtor
*
*	DESCRIPTION		Destroys the CGraph object, along with any sample sets that it manages. This is
*					actually the only way CGraphSampleSet objects are destroyed.
*
***************************************************************************************************/

void CGraph::InternalDtor()
{
	// Iterating through our sample sets, deleting them.
	for ( GraphSampleSetList::iterator obIt = m_sampleSets.begin(); obIt != m_sampleSets.end(); )
	{
		CGraphSampleSet::Destroy( *obIt );
		obIt = m_sampleSets.erase( obIt );
	}
}

//DGF
bool CGraph::DeleteSampleSet(const char* pcName)
{
	CHashedString	obName( pcName );

	// Lets go through all our sample sets
	for ( GraphSampleSetList::iterator obIt = m_sampleSets.begin(); obIt != m_sampleSets.end(); ++obIt )
	{
		// When we find our definition, return it to the caller
		if ( ( *obIt )->GetNameHash() == obName )
		{
			CGraphSampleSet::Destroy( *obIt );
			obIt = m_sampleSets.erase( obIt );
			return true;
		}
	}

	// If we got here, then we didn't find the sample sets in our list.
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CGraph::AddSampleSet
*
*	DESCRIPTION		Adds a new CGraphSampleSet object to the current CGraph object. Sample sets are
*					defined by name (so we can find them in CGraph::GetSampleSet()). 
*
*	INPUTS			pcName			-	Name of the sample set
*
*					iSampleCount	-	How many samples this set contains (see note!)
*
*					iSetColour		-	The colour associated with the new sample set
*
*	NOTES			If this is being called for a rolling graph (like the one used by the gatso, for
*					example), then be aware that *all* sets have to have the same number of samples.
*
*					This is because the marker bar that moves as samples are accumulated wouldn't
*					make any sense if each sample set moved at a different rate. Of course, if 
*					samples aren't added to each sample set at the same frequency (eg once per frame)
*					then it's gonna look slightly iffy anyway, but consider yourself warned.
*
***************************************************************************************************/

CGraphSampleSet*	CGraph::AddSampleSet( const char* pcName, int iSampleCount, uint32_t iSetColour )
{
	CGraphSampleSet*	pSampleSet = CGraphSampleSet::Create( pcName, iSampleCount, iSetColour, this );
	ntAssert( pSampleSet );

	// If we're a rolling graph, then all sample sets need the same number of samples..
	if ( ( GetGraphType() == GRAPH_TYPE_ROLLING ) && ( !m_sampleSets.empty() ) )
	{
		ntAssert( (*m_sampleSets.begin())->GetMaxSamples() == iSampleCount );
	}

	m_sampleSets.push_back( pSampleSet );
	return pSampleSet;
}


/***************************************************************************************************
*
*	FUNCTION		CGraph::GetSampleSet
*
*	DESCRIPTION		Obtains a pointer to a CGraphSampleSet object using the name of it as a search
*					key.
*
*	INPUTS			pcName			-		Name of the sample set.
*
***************************************************************************************************/

CGraphSampleSet*	CGraph::GetSampleSet( const char* pcName )
{
	CHashedString	obName( pcName );

	// Lets go through all our sample sets
	for ( GraphSampleSetList::const_iterator obIt = m_sampleSets.begin(); obIt != m_sampleSets.end(); ++obIt )
	{
		// When we find our definition, return it to the caller
		if ( ( *obIt )->GetNameHash() == obName )
			return ( *obIt );
	}

	// If we got here, then we didn't find the sample sets in our list.
	return NULL;
}

/***************************************************************************************************
*
*	FUNCTION		CGraph::RecomputeAxes
*
*	DESCRIPTION		Given a graph with active sample sets, this function recomputes suitable minimum
*					and maximum axis extents based on the currently active samples. 
*
*	NOTES			The X and Y axes (that pass through X=0.0f and Y=0.0f) are always present on a
*					graph..
*
***************************************************************************************************/

void	CGraph::RecomputeAxes( void )
{
	float	fMinYAxis = 0.0f;
	float	fMaxYAxis = 0.0f;
	float	fMinXAxis = 0.0f;
	float	fMaxXAxis = 0.0f;

	for ( GraphSampleSetList::const_iterator obIt = m_sampleSets.begin(); obIt != m_sampleSets.end(); ++obIt )
	{
		CGraphSampleSet*	pSampleSet	= *obIt;
		CGraphSample*		pSample = pSampleSet->GetSampleData();
		
		for ( int iLoop = 0; iLoop < pSampleSet->GetUsedSamples(); iLoop++, pSample++ )
		{
			fMinYAxis = min( fMinYAxis, pSample->Y() );
			fMaxYAxis = max( fMaxYAxis, pSample->Y() );
			fMinXAxis = min( fMinXAxis, pSample->X() );
			fMaxXAxis = max( fMaxXAxis, pSample->X() );
		}
	}
	
	if ( ( fMinYAxis < fMaxYAxis ) && ( fMinXAxis < fMaxXAxis ) )
	{
		m_fMinYAxis = fMinYAxis;
		m_fMaxYAxis	= fMaxYAxis;
		m_fMinXAxis = fMinXAxis;
		m_fMaxXAxis	= fMaxXAxis;
	}
}
