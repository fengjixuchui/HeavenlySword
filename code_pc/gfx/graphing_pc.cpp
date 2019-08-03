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
*	CLASS			GraphRenderer
*
*	DESCRIPTION		Implements PC side rendering of graphs
*
***************************************************************************************************/
class	GraphRenderer
{
public:
	GraphRenderer() {};
	~GraphRenderer() {};

	void Render( const CGraph* m_pThis, const CMatrix& transform ) const;
};



/***************************************************************************************************
*
*	FUNCTION		CGraph::CGraph
*
*	DESCRIPTION		calls InternalCtor, allocates renderer
*
*	INPUTS			eGraphType		-	Type of graph (GRAPH_TYPE_STANDARD or GRAPH_TYPE_ROLLING)
*
***************************************************************************************************/

CGraph::CGraph( GRAPH_TYPE eGraphType )
{
	InternalCtor( eGraphType );
	m_pRenderer = NT_NEW GraphRenderer();
}

/***************************************************************************************************
*
*	FUNCTION		CGraph::~CGraph
*
*	DESCRIPTION		calls InternalDtor, destroys renderer
*
***************************************************************************************************/

CGraph::~CGraph()
{
	InternalDtor();
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pRenderer );
}

/***************************************************************************************************
*
*	FUNCTION		CGraph::Render
*
*	DESCRIPTION		calls platform specific render method
*
***************************************************************************************************/

void CGraph::Render( const CMatrix& toScreen, const CMatrix& screenToView ) const
{
	ntAssert( m_pRenderer );
	CMatrix transform = (toScreen*screenToView).GetTranspose();
	m_pRenderer->Render( this, transform );
}

/***************************************************************************************************
*
*	FUNCTION		GraphRenderer::Render
*
*	DESCRIPTION		Called only by CDebugPrimitives, this is responsible for actually getting the
*					graph onto screen. 
*
*	INPUTS			transform			-	A matrix supplied by Simon that is used to transform
*											graph sample values into the coordinate space of the
*											graph display.
*
***************************************************************************************************/

void	GraphRenderer::Render( const CGraph* m_pThis, const CMatrix& transform ) const
{
	// Nabbed from the core debug primitive code.. any chance these could be defines, eh, Simon?? 
	static const int iColourRegister = 6;
	static const int iProjectionRegister = 1;

	// Commonly used scalars
	float	fXAxisLength	= m_pThis->m_fMaxXAxis - m_pThis->m_fMinXAxis;
	float	fYAxisLength	= m_pThis->m_fMaxYAxis - m_pThis->m_fMinYAxis;

	// Compute axis positions
	float	fYAxisPosition	= ( 1.0f / fXAxisLength ) * ( -m_pThis->m_fMinXAxis );
	float	fXAxisPosition	= ( 1.0f / fYAxisLength ) * ( -m_pThis->m_fMinYAxis );

	// Compute the mark step for X and Y axes.
	float	fXAxisMarkStep	= 1.0f / ( fXAxisLength / m_pThis->m_fXAxisMarkStep );
	float	fYAxisMarkStep	= 1.0f / ( fYAxisLength / m_pThis->m_fYAxisMarkStep );

	// Axes always have to be visible..
	ntAssert( ( fXAxisPosition >= 0.0f ) && ( fXAxisPosition <= 1.0f ) );
	ntAssert( ( fYAxisPosition >= 0.0f ) && ( fYAxisPosition <= 1.0f ) );

	// Do our background
	{
		CVector backColour;
		backColour.SetFromNTColor( m_pThis->m_iBackColour );
		Renderer::Get().SetVertexShaderConstant( iColourRegister, &backColour, 1);
	
		CGraphSample	vertices[] =
						{
							CGraphSample( 0.0f, 1.0f ),
							CGraphSample( 1.0f, 1.0f ),
							CGraphSample( 0.0f, 0.0f ),
							CGraphSample( 1.0f, 0.0f )
						};

		GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertices[0], sizeof(CGraphSample));
	}

	// Do our markers
	{
		CVector markerColour;
		markerColour.SetFromNTColor( m_pThis->m_iMarkColour );
		Renderer::Get().SetVertexShaderConstant( iColourRegister, &markerColour, 1);

		float	fMarker;

		// First do our positive Y markers
		fMarker = fXAxisPosition;
		while ( fMarker > 0.0f )
		{
			CGraphSample	markVertices[] = 
							{
								CGraphSample( 0.0f, fMarker ),	
								CGraphSample( 1.0f, fMarker ),
							};
			
			GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINELIST, 1, &markVertices[0], sizeof(CGraphSample));

			fMarker -= fYAxisMarkStep;
		}

		// First do our negative Y markers
		fMarker = fXAxisPosition;
		while ( fMarker < 1.0f )
		{
			CGraphSample	markVertices[] = 
							{
								CGraphSample( 0.0f, fMarker ),	
								CGraphSample( 1.0f, fMarker ),
							};
			
			GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINELIST, 1, &markVertices[0], sizeof(CGraphSample));

			fMarker += fYAxisMarkStep;
		}

		// First do our negative X markers
		fMarker = fYAxisPosition;
		while ( fMarker > 0.0f )
		{
			CGraphSample	markVertices[] = 
							{
								CGraphSample( fMarker, 0.0f ),	
								CGraphSample( fMarker, 1.0f ),
							};
			
			GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINELIST, 1, &markVertices[0], sizeof(CGraphSample));

			fMarker -= fXAxisMarkStep;
		}

		// First do our positive X markers
		fMarker = fYAxisPosition;
		while ( fMarker < 1.0f )
		{
			CGraphSample	markVertices[] = 
							{
								CGraphSample( fMarker, 0.0f ),	
								CGraphSample( fMarker, 1.0f ),
							};
			
			GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINELIST, 1, &markVertices[0], sizeof(CGraphSample));

			fMarker += fXAxisMarkStep;
		}

		// If we're a rolling graph, then we have a marker showing upper limit in Y axis (it's only shown if it's > 0.0f!)
		if ( ( m_pThis->GetGraphType() == GRAPH_TYPE_ROLLING ) && ( m_pThis->m_fLimitValue > 0.0f ) )
		{
			CVector limitColour;
			limitColour.SetFromNTColor( m_pThis->m_iLimitColour );
			Renderer::Get().SetVertexShaderConstant( iColourRegister, &limitColour, 1);

			float fLimitPos = m_pThis->m_fLimitValue / m_pThis->m_fMaxYAxis;

			CGraphSample	limitVertices[] = 
							{
								CGraphSample( 0.0f, fLimitPos ),	
								CGraphSample( 1.0f, fLimitPos ),
							};

			GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINELIST, 1, &limitVertices[0], sizeof(CGraphSample));
		}
	}

	// Make sure we transform the sample set data into the graph coordinate system
	CMatrix graphCoordTransform( CONSTRUCT_IDENTITY );
	graphCoordTransform[0][0] = 1.0f / fXAxisLength;
	graphCoordTransform[0][3] = -m_pThis->m_fMinXAxis * graphCoordTransform[0][0];
	graphCoordTransform[1][1] = 1.0f / fYAxisLength;
	graphCoordTransform[1][3] = -m_pThis->m_fMinYAxis * graphCoordTransform[1][1];
	graphCoordTransform = transform*graphCoordTransform;
	Renderer::Get().SetVertexShaderConstant( iProjectionRegister, &graphCoordTransform, 4 );

	// Iterate through each sample set in the graph (in reverse order, so the first one added is the last one rendered)
	for ( CGraph::GraphSampleSetList::const_reverse_iterator obIt = m_pThis->m_sampleSets.rbegin(); obIt != m_pThis->m_sampleSets.rend(); ++obIt )
	{
		CGraphSampleSet*	pSampleSet	= *obIt;

		// Set the colour to that of the sample set..
		CVector sampleColour;
		sampleColour.SetFromNTColor( pSampleSet->GetColour() );
		Renderer::Get().SetVertexShaderConstant( iColourRegister, &sampleColour, 1);

		switch	( m_pThis->m_eGraphType )
		{
			// It's a standard graph.. this means that all we're going to do is blat out a linestrip.
			case	GRAPH_TYPE_STANDARD:
			{
				if ( pSampleSet->GetUsedSamples() > 1 )
				{
					if ( !m_pThis->m_bHistogramMode )
					{
						GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINESTRIP, pSampleSet->GetUsedSamples() - 1, pSampleSet->GetSampleData(), sizeof( CGraphSample ) );
					}
					else
					{
						CGraphSample*	pSample		= pSampleSet->GetSampleData();
						float			fHistWidth	= ( ( pSample + 1 )->X() - pSample->X() ) * 0.75f;
	
						for ( int iLoop = 0; iLoop < pSampleSet->GetUsedSamples() - 1 ; iLoop++, pSample++ )
						{
							float	fBaseY;
							float	fTopY;

							if ( pSample->Y() >= 0.0f )
							{
								fBaseY = 0.0f;
								fTopY = pSample->Y();
							}
							else
							{
								fBaseY = pSample->Y();
								fTopY = 0.0f;
							}

							CGraphSample	histogramVertices[] =
											{
												CGraphSample( pSample->X(), fTopY ),
												CGraphSample( pSample->X() + fHistWidth, fTopY ),
												CGraphSample( pSample->X(), fBaseY ),
												CGraphSample( pSample->X() + fHistWidth, fBaseY )
											};

							GetD3DDevice()->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &histogramVertices[0], sizeof(CGraphSample));
						}
					}
				}
				break;
			}

			// It's a rolling graph.. and with this comes a load more chuff that we have to deal with.
			case	GRAPH_TYPE_ROLLING:
			{
					// Ok, if we can display the entire line strip in one chunk, then do so. It'll save us a few cycles, I'm sure.
					if ( ( pSampleSet->GetUsedSamples() == 0 ) || ( pSampleSet->GetUsedSamples() == pSampleSet->GetMaxSamples() ) )
					{
						GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINESTRIP, pSampleSet->GetMaxSamples() - 1, pSampleSet->GetSampleData(), sizeof( CGraphSample ) );
					}
					else
					{
						// Hello! If you're here, then it's obvious that we have to display the graph data in two discrete chunks, so, compute
						// the primitive count for each chunk, and then kick off the two separate line strips. 
						int	iPrimCount1 = max( 1, ( pSampleSet->GetUsedSamples() - 1 ) );
						int	iPrimCount2 = max( 0, ( pSampleSet->GetMaxSamples() - pSampleSet->GetUsedSamples() ) - 1 );

						GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINESTRIP, iPrimCount1, pSampleSet->GetSampleData(), sizeof( CGraphSample ) );
						if ( iPrimCount2 > 0 )
							GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINESTRIP, iPrimCount2, pSampleSet->GetSampleData() + pSampleSet->GetUsedSamples(), sizeof( CGraphSample ) );
					}
					
				break;
			}
		}
	}

	// Ok.. we're all done with graph samples, so we put our projection matrix back to the one that
	// Simon generously set up for us, going from 0.0f to 1.0f in each axis..
	Renderer::Get().SetVertexShaderConstant( iProjectionRegister, &transform, 4 );

	// Now do our axes.. we do 'em last, so the sample data doesn't blat over the same space.
	{
		// We'd better put our axis colour back..
		CVector axisColour;
		axisColour.SetFromNTColor( m_pThis->m_iAxisColour );
		Renderer::Get().SetVertexShaderConstant( iColourRegister, &axisColour, 1);

		CGraphSample	axisVertices[] = 
						{
							CGraphSample( 0.0f, fXAxisPosition ),	
							CGraphSample( 1.0f, fXAxisPosition ),
							CGraphSample( fYAxisPosition, 0.0f ),	
							CGraphSample( fYAxisPosition, 1.0f ),
						};

		GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINELIST, 2, &axisVertices[0], sizeof(CGraphSample));

		// If we're a rolling graph, then it'd be a good idea for us to display a marker showing where our buffer position
		// is, kinda like the one you get in Microsoft's performance monitoring software in Windows 2000/XP. Do you see?
		if ( m_pThis->GetGraphType() == GRAPH_TYPE_ROLLING )
		{
			CGraphSampleSet*	pSampleSet = ( *m_pThis->m_sampleSets.begin() );

			// Remember that, at this point, our area coordinates go from 0.0f to 1.0f (not in graph sample space anymore).
			float	fCurrentMarkPosition = ( float )pSampleSet->GetUsedSamples() / ( float )( pSampleSet->GetMaxSamples() );

			CGraphSample	aobCurrentMarkVertices[] = 
							{
								CGraphSample( fCurrentMarkPosition, 0.0f ),	
								CGraphSample( fCurrentMarkPosition, 1.0f ),
							};
				
			GetD3DDevice()->DrawPrimitiveUP( D3DPT_LINELIST, 1, &aobCurrentMarkVertices[0], sizeof(CGraphSample));
		}
	}
}
