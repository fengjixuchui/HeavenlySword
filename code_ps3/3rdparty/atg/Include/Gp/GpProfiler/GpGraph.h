//--------------------------------------------------------------------------------------------------
/**
	@file		GpGraph.h

	@brief		Graph drawing helper.

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_GRAPH_H
#define GP_GRAPH_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/GcColour.h>
#include <Gc/GcKernel.h>

#include <Gp/GpProfiler/GpGraph.h>

#include <Gp/GpProfiler/Internal/GpProfilerDrawHelpers.h>
#include <Gp/GpProfiler/Internal/GpProfilerDrawVertexSet.h>

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class	GpGraph

	@brief	Graph drawing helper.
**/
//--------------------------------------------------------------------------------------------------

class GpGraph : public FwNonCopyable
{
public:
	
	// Enumerations
	
	enum DrawMode
	{
		kDrawModeBackgroundBit	= 0x01,
		kDrawModeAxesBit		= 0x02,
		kDrawModeSamplesBit		= 0x04,
		kDrawModeLabelsBit		= 0x08,
		kDrawModeMarkerLinesBit	= 0x10,
		
		kDrawModeSamplesOnly	= kDrawModeSamplesBit,
		kDrawModeDefault		= kDrawModeBackgroundBit | kDrawModeAxesBit | kDrawModeSamplesBit,
		kDrawModeAll			= kDrawModeDefault | kDrawModeLabelsBit | kDrawModeMarkerLinesBit
	};
	
	// Multisampling method used for processing newly added samples.
	enum MultisampleMode
	{
		kMultisampleNone,				///< Multisampling off
		kMultisampleStoreMax,			///< Store maximum of all multisamples
		kMultisampleStoreMin,			///< Store minimum of all multisamples
		kMultisampleStoreAverage,		///< Store average of all multisamples
	
		kNumMultisampleModes
	};
	
	
	// Operations
	
	void		Initialise(	float				x,
							float				y,
							float				width,
							float				height,
							float				minY = -1.0f,
							float				maxY = 1.0f,
							uint				maxSamples = 640,
							GcColour_arg		colour = Gc::kColourWhite,
							DrawMode			drawMode = kDrawModeDefault,
							float				markerLineDeltaY = 0.0f,
							MultisampleMode		multisampleMode = kMultisampleNone,
							uint				multisampleRate = 2,
							float				transparency = 1.0f);
	
	void		Destroy();
	
	void		ResetSamples();
	
	void		AddSample(float sample);
	
	void		Draw(GcContext& context = GcKernel::GetContext());
	
	
	// Accessors
	
	void		SetVisibility(bool isVisible);
	bool		GetVisibility() const;
	
	void		SetDrawOrigin(float x, float y);
	void		GetDrawOrigin(float* pX, float* pY) const;
	
	void		SetDrawDimensions(float width, float height);
	void		GetDrawDimensions(float* pWidth, float* pHeight) const;
	
	void		SetColour(GcColour_arg colour);
	GcColour	GetColour() const;
	
	void		SetTransparency(float alpha);
	float		GetTransparency() const;
	
	void		SetClampFactor(float clampFactor);
	float		GetClampFactor() const;
	

private:

	// Constants
	
	static const float	kInitialMultisampleValues[kNumMultisampleModes];
	 
	
	// Attributes
	
	float			m_x;							///< Draw origin (in NDC)
	float			m_y;
	float			m_width;						///< Draw dimensions (in NDC)
	float			m_height;
	float			m_minY;							///< Minimum sample (and y-axis) value
	float			m_maxY;							///< Maximum sample (and y-axis) value
	uint			m_maxSamples;					///< Maximum no. of stored samples (ring-buffer size)
	GcColour		m_colour;						///< Graph plot colour
	DrawMode		m_drawMode;						///< Draw mode
	uint			m_numMarkerLines;				///< No. of dashed graph marker lines
	float			m_markerLineDeltaY;				///< Delta between marker lines
	MultisampleMode	m_multiSampleMode;				///< Multisample mode for processing newly added samples
	uint			m_multisampleRate;				///< No. of multisamples to one stored sample
	uint			m_numMultisamples;				///< Current no. of multisamples
	float			m_multisample;					///< Work storage for the multisampled value
	float			m_transparency;					///< [0..1] => [fully transparent..opaque]
	
	bool			m_isVisible;					///< True if visible, false otherwise
	
	float			m_clampSampleFactor;			///< Clamp sample factor (0.0 implies no clamping)

	uint			m_numSamples;					///< Current no. of samples [0..m_maxSamples]
	float*			m_pSamples;						///< Samples ring-buffer (size m_maxSamples)
	float*			m_pNextFreeSample;				///< Ptr to the next free sample in the ring-buffer
	
	bool			m_fullRefreshRequested;			///< True implies a full graphics refresh
	
	GpProfilerDrawHelpers	m_staticPrims;			///< Static primitives - compute once, draw many frames
	GpProfilerDrawHelpers	m_markerLinePrims;		///< Static primitives - compute once, draw many frames
	GpProfilerDrawVertexSet	m_plotPrims;			///< Static primitives - compute once, draw many frames


	// Operations

	void	RequestFullRefresh()	{ m_fullRefreshRequested = true; }
	
	void	DrawStaticPrims();
	void	DrawMarkerLinePrims();
	void	DrawGraphPlot();
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gp/GpProfiler/GpGraph.inl>

//--------------------------------------------------------------------------------------------------

#endif // GP_GRAPH_H
