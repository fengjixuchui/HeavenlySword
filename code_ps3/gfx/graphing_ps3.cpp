/***************************************************************************************************
*
*	DESCRIPTION		Debug Graphing Module (Implementation)
*
*	NOTES	
*
***************************************************************************************************/

#include "core/visualdebugger.h"

#include "gfx/graphing.h"
#include "gfx/debugshader_ps3.h"
#include "gfx/renderer.h"

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
	GraphRenderer();
	~GraphRenderer()
	{
		while (!m_sampleDataSets.empty())
		{
			VBHandle* pHandle = m_sampleDataSets.back();
			NT_DELETE_CHUNK(Mem::MC_GFX, pHandle );
			m_sampleDataSets.pop_back();
		}
	}

	void Render( const CGraph* m_pThis, const CMatrix& toScreen, const CMatrix& screenToView ) const;

private:
	// mutables are due to the fact that the render method changes this class. sigh.
	mutable DebugShader	*	m_vertexShader;
	mutable DebugShader	*	m_pixelShader;

	VBHandle							m_backgroundData;
	typedef ntstd::List<VBHandle*, Mem::MC_GFX> VBHandleList;
	mutable VBHandleList		m_sampleDataSets;
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
	m_pRenderer = NT_NEW_CHUNK( Mem::MC_GFX ) GraphRenderer();
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
	NT_DELETE_CHUNK(Mem::MC_GFX, m_pRenderer );
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
	m_pRenderer->Render( this, toScreen, screenToView );
}


/***************************************************************************************************
*
*	FUNCTION		GraphRenderer::GraphRenderer
*
*	DESCRIPTION		allocates vertex shaders and geometry streams
*
***************************************************************************************************/

GraphRenderer::GraphRenderer()
{
	m_vertexShader = DebugShaderCache::Get().LoadShader( "debuggraph_vp.sho" );
	m_pixelShader = DebugShaderCache::Get().LoadShader( "simplecolour_fp.sho" );
		
	GcStreamField simpleDesc( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 );

	m_backgroundData = RendererPlatform::CreateVertexStream( 4, sizeof( float ) * 2, 1, &simpleDesc, Gc::kScratchBuffer );
};

/***************************************************************************************************
*
*	FUNCTION		GraphRenderer::Render
*
*	DESCRIPTION		Platform specific graph renderer, called by the PS3 debug visualiser
*
*	INPUTS			transform			-	A matrix supplied by Simon that is used to transform
*											graph sample values into the coordinate space of the
*											graph display.
*
***************************************************************************************************/

void	GraphRenderer::Render( const CGraph* m_pThis, const CMatrix& toScreen, const CMatrix& screenToView ) const
{
#ifndef _GOLD_MASTER

//	CMatrix transform = (toScreen * screenToView).GetTranspose();
	CMatrix transform = toScreen * screenToView;

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

	// Do anything that can be drawn with simple line render calls first
	//--------------------------------------------------------------------------------
	{
		float	fMarker;
		CPoint start, end;

		// Y scale markers
		fMarker = fXAxisPosition;
		while ( fMarker > 0.0f )
		{
			start = CPoint( 0.0f, fMarker, 0.0f ) * toScreen;
			end = CPoint( 1.0f, fMarker, 0.0f ) * toScreen;

			g_VisualDebug->RenderLine( start, end, m_pThis->m_iMarkColour, DPF_DISPLAYSPACE);
			fMarker -= fYAxisMarkStep;
		}

		fMarker = fXAxisPosition;
		while ( fMarker < 1.0f )
		{
			start = CPoint( 0.0f, fMarker, 0.0f ) * toScreen;
			end = CPoint( 1.0f, fMarker, 0.0f ) * toScreen;
			
			g_VisualDebug->RenderLine( start, end, m_pThis->m_iMarkColour, DPF_DISPLAYSPACE);
			fMarker += fYAxisMarkStep;
		}

		// X scale markers
		fMarker = fYAxisPosition;
		while ( fMarker > 0.0f )
		{
			start = CPoint( fMarker, 0.0f, 0.0f ) * toScreen;
			end = CPoint( fMarker, 1.0f, 0.0f ) * toScreen;

			g_VisualDebug->RenderLine( start, end, m_pThis->m_iMarkColour, DPF_DISPLAYSPACE);
			fMarker -= fXAxisMarkStep;
		}

		fMarker = fYAxisPosition;
		while ( fMarker < 1.0f )
		{
			start = CPoint( fMarker, 0.0f, 0.0f ) * toScreen;
			end = CPoint( fMarker, 1.0f, 0.0f ) * toScreen;
			
			g_VisualDebug->RenderLine( start, end, m_pThis->m_iMarkColour, DPF_DISPLAYSPACE);
			fMarker += fXAxisMarkStep;
		}

		// If we're a rolling graph, then we have a marker showing upper limit in Y axis (it's only shown if it's > 0.0f!)
		if ( ( m_pThis->GetGraphType() == GRAPH_TYPE_ROLLING ) && ( m_pThis->m_fLimitValue > 0.0f ) )
		{
			float fLimitPos = m_pThis->m_fLimitValue / m_pThis->m_fMaxYAxis;

			start = CPoint( 0.0f, fLimitPos, 0.0f ) * toScreen;
			end = CPoint( 1.0f, fLimitPos, 0.0f ) * toScreen;

			g_VisualDebug->RenderLine( start, end, m_pThis->m_iLimitColour, DPF_DISPLAYSPACE);
		}

		// Now do our axes
		{
			start = CPoint( 0.0f, fXAxisPosition, 0.0f ) * toScreen;
			end = CPoint( 1.0f, fXAxisPosition, 0.0f ) * toScreen;

			g_VisualDebug->RenderLine( start, end, m_pThis->m_iAxisColour, DPF_DISPLAYSPACE);

			start = CPoint( fYAxisPosition, 0.0f, 0.0f ) * toScreen;
			end = CPoint( fYAxisPosition, 1.0f, 0.0f ) * toScreen;

			g_VisualDebug->RenderLine( start, end, m_pThis->m_iAxisColour, DPF_DISPLAYSPACE);
		}

		// If we're a rolling graph, then it'd be a good idea for us to display a marker showing where our buffer position
		// is, kinda like the one you get in Microsoft's performance monitoring software in Windows 2000/XP. Do you see?
		if ( m_pThis->GetGraphType() == GRAPH_TYPE_ROLLING )
		{
			CGraphSampleSet* pSampleSet = ( *m_pThis->m_sampleSets.begin() );

			// Remember that, at this point, our area coordinates go from 0.0f to 1.0f (not in graph sample space anymore).
			float	fCurrentMarkPosition = ( float )pSampleSet->GetUsedSamples() / ( float )( pSampleSet->GetMaxSamples() );

			start = CPoint( fCurrentMarkPosition, 0.0f, 0.0f ) * toScreen;
			end = CPoint( fCurrentMarkPosition, 1.0f, 0.0f ) * toScreen;
				
			g_VisualDebug->RenderLine( start, end, m_pThis->m_iAxisColour, DPF_DISPLAYSPACE);
		}
	}

	// Now do the rest of the custom graph rendering
	//--------------------------------------------------------------------------------
	Renderer::Get().SetVertexShader( m_vertexShader );
	Renderer::Get().SetPixelShader( m_pixelShader );

	int iTransformIndex = m_vertexShader->GetConstantIndex("transform");
	int iColourIndex = m_vertexShader->GetConstantIndex("colour");

	// set the render states
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
	Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

	// Do our background
	{
		CVector backColour;
		backColour.SetFromNTColor( m_pThis->m_iBackColour );
		m_vertexShader->SetVSConstant( iColourIndex, &backColour, 1);
		m_vertexShader->SetVSConstant( iTransformIndex, &transform, 4 );

		static CGraphSample	vertices[] =
						{
							CGraphSample( 0.0f, 0.0f ),
							CGraphSample( 0.0f, 1.0f ),
							CGraphSample( 1.0f, 1.0f ),
							CGraphSample( 1.0f, 0.0f )
						};

		if (m_backgroundData->QueryGetNewScratchMemory())
		{
			m_backgroundData->GetNewScratchMemory();
			m_backgroundData->Write( vertices );

			Renderer::Get().m_Platform.SetStream( m_backgroundData );
			Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, 4 );
			Renderer::Get().m_Platform.ClearStreams();
		}
	}

	// Make sure we transform the sample set data into the graph coordinate system
	CMatrix graphCoordTransform( CONSTRUCT_IDENTITY );
	graphCoordTransform[0][0] = 1.0f / fXAxisLength;
	graphCoordTransform[0][3] = -m_pThis->m_fMinXAxis * graphCoordTransform[0][0];
	graphCoordTransform[1][1] = 1.0f / fYAxisLength;
	graphCoordTransform[1][3] = -m_pThis->m_fMinYAxis * graphCoordTransform[1][1];
	graphCoordTransform = graphCoordTransform*transform;

	m_vertexShader->SetVSConstant( iTransformIndex, &graphCoordTransform, 4 );

	// make sure we have enough sample data sets to render with
	while (m_sampleDataSets.size() < m_pThis->m_sampleSets.size())
	{
		VBHandle* pNewHandle = NT_NEW_CHUNK( Mem::MC_GFX ) VBHandle;
		m_sampleDataSets.push_back( pNewHandle );
	}

	// Iterate through each sample set in the graph (in reverse order, so the first one added is the last one rendered)
	int iCount = 0;
	for (	CGraph::GraphSampleSetList::const_reverse_iterator obIt = m_pThis->m_sampleSets.rbegin();
			obIt != m_pThis->m_sampleSets.rend(); ++obIt, iCount++ )
	{
		// retrieve the sample set
		CGraphSampleSet*	pSampleSet	= *obIt;

		// retrive the dynamic render buffer for this set
		VBHandleList::iterator dataIt( m_sampleDataSets.begin() );
		ntstd::advance( dataIt, iCount );
		ntAssert( dataIt != m_sampleDataSets.end() );

		VBHandle* ppSampleData = *dataIt;
		ntAssert( ppSampleData );
			
		// we have to allocate a new dynamic buffer if we havent already
		// we always allocate 4 times as much, incase we're swapped to histogram mode
		if (!ppSampleData->Get())
		{
			GcStreamField simpleDesc( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 );
			*ppSampleData = RendererPlatform::CreateVertexStream( pSampleSet->GetMaxSamples() * 4, sizeof( float ) * 2, 1, &simpleDesc, Gc::kScratchBuffer );
		}

		VBHandle& sampleData = *ppSampleData;
		ntAssert( sampleData );

		// Set the colour to that of the sample set..
		CVector sampleColour;
		sampleColour.SetFromNTColor( pSampleSet->GetColour() );
		m_vertexShader->SetVSConstant( iColourIndex, &sampleColour, 1);

		switch	( m_pThis->m_eGraphType )
		{
			// It's a standard graph.. this means that all we're going to do is blat out a linestrip.
			case	GRAPH_TYPE_STANDARD:
			{
				if ( pSampleSet->GetUsedSamples() > 1 )
				{
					if ( !m_pThis->m_bHistogramMode )
					{
						if (sampleData->QueryGetNewScratchMemory())
						{
							sampleData->GetNewScratchMemory();
							sampleData->Write( pSampleSet->GetSampleData(), 0, pSampleSet->GetUsedSamples() * sizeof(CGraphSample) );

							Renderer::Get().m_Platform.SetStream( sampleData );
							Renderer::Get().m_Platform.DrawPrimitives( Gc::kLineStrip, 0, pSampleSet->GetUsedSamples() );
							Renderer::Get().m_Platform.ClearStreams();
						}
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
												CGraphSample( pSample->X() + fHistWidth, fBaseY ),
												CGraphSample( pSample->X(), fBaseY )
											};

							if (sampleData->QueryGetNewScratchMemory())
							{
								sampleData->GetNewScratchMemory();
								sampleData->Write( &histogramVertices, iLoop * sizeof(CGraphSample) * 4, sizeof(CGraphSample) * 4 );
							}
						}

						Renderer::Get().m_Platform.SetStream( sampleData );
						Renderer::Get().m_Platform.DrawPrimitives( Gc::kQuads, 0, (pSampleSet->GetUsedSamples() - 1) * 4 );
						Renderer::Get().m_Platform.ClearStreams();
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
						if (sampleData->QueryGetNewScratchMemory())
						{
							sampleData->GetNewScratchMemory();
							sampleData->Write( pSampleSet->GetSampleData(), 0, pSampleSet->GetMaxSamples() * sizeof(CGraphSample) );

							Renderer::Get().m_Platform.SetStream( sampleData );
							Renderer::Get().m_Platform.DrawPrimitives( Gc::kLineStrip, 0, pSampleSet->GetMaxSamples() );
							Renderer::Get().m_Platform.ClearStreams();
						}
					}
					else
					{
						// Hello! If you're here, then it's obvious that we have to display the graph data in two discrete chunks, so, compute
						// the primitive count for each chunk, and then kick off the two separate line strips. 
						int	iPrimCount1 = max( 1, ( pSampleSet->GetUsedSamples() - 1 ) );
						int	iPrimCount2 = max( 0, ( pSampleSet->GetMaxSamples() - pSampleSet->GetUsedSamples() ) - 1 );

						if (sampleData->QueryGetNewScratchMemory())
						{
							sampleData->GetNewScratchMemory();
							sampleData->Write(	pSampleSet->GetSampleData(),				// where from (addr)
												0,											// where to (in bytes)
												(iPrimCount1+1) * sizeof(CGraphSample) );	// how much (in bytes)

							Renderer::Get().m_Platform.SetStream( sampleData );
							Renderer::Get().m_Platform.DrawPrimitives( Gc::kLineStrip, 0, iPrimCount1 );

							if ( iPrimCount2 > 0 )
							{
								if (sampleData->QueryGetNewScratchMemory())
								{
									sampleData->GetNewScratchMemory();
									sampleData->Write(	pSampleSet->GetSampleData() + pSampleSet->GetUsedSamples(),	// where from (addr)
														(iPrimCount1+1) * sizeof(CGraphSample),						// where to (in bytes)
														(iPrimCount2+1) * sizeof(CGraphSample) );					// how much (in bytes)

									Renderer::Get().m_Platform.SetStream( sampleData );
									Renderer::Get().m_Platform.DrawPrimitives( Gc::kLineStrip, iPrimCount1+1, iPrimCount2 );
								}
							}

							Renderer::Get().m_Platform.ClearStreams();
						}
					}
					
				break;
			}
		}
	}

	// set the render states back
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_NORMAL );
	Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );

#endif
}
