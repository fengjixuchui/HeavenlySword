//--------------------------------------------------------------------------------------------------
/**
	@file		GpMultiGraph.h

	@brief		Graph drawing helper.

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_MULTI_GRAPH_H
#define GP_MULTI_GRAPH_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gp/GpProfiler/GpGraph.h>

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class	GpMultiGraph

	@brief	Provides drawing of multiple overlaid graph-plots within a single graph.
	
	GpMultiGraph is essentially a container of GpGraph objects, hence the familiar interface.
**/
//--------------------------------------------------------------------------------------------------

class GpMultiGraph : public FwNonCopyable
{
public:
	
	// Operations
	
	void		Initialise(	float						x,
							float						y,
							float						width,
							float						height,
							uint						numGraphPlots,
							float						minY = -1.0f,
							float						maxY = 1.0f,
							uint						maxSamples = 640,
							GpGraph::DrawMode			drawMode = GpGraph::kDrawModeDefault,
							float						markerLineDeltaY = 0.0f,
							GpGraph::MultisampleMode	multisampleMode = GpGraph::kMultisampleNone,
							uint						multisampleRate = 2,
							float						transparency = 1.0f);
	
	void		Destroy();
	
	void		ResetSamples();
	
	void		AddSample(uint plotIndex, float sample);
	
	void		Draw(GcContext& context = GcKernel::GetContext());
	
	
	// Accessors
	
	GpGraph&	GetGraph(uint plotIndex);
	
	void		SetVisibility(bool isVisible);
	bool		GetVisibility() const;
	
	void		SetDrawOrigin(float x, float y);
	void		GetDrawOrigin(float* pX, float* pY) const;
	
	void		SetDrawDimensions(float width, float height);
	void		GetDrawDimensions(float* pWidth, float* pHeight) const;
	
	void		SetTransparency(float alpha);
	float		GetTransparency() const;
	
	void		SetClampFactor(float clampFactor);
	float		GetClampFactor() const;
	

private:

	// Constants
	
	static const uint		kMaxGraphPlots = 8;
	
	static const GcColour	kDefaultColours[kMaxGraphPlots];
	
	
	// Attributes
	
	uint			m_numGraphs;					///< No. graph plots
	GpGraph*		m_pGraphs;						///< Graphs - one per graph plot
	
	bool			m_isVisible;					///< True if visible, false otherwise
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gp/GpProfiler/GpMultiGraph.inl>

//--------------------------------------------------------------------------------------------------

#endif // GP_MULTI_GRAPH_H
