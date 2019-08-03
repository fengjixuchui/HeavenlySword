#ifndef _SHADERID_H_
#define _SHADERID_H_



//--------------------------------------------------
//!
//!	shader simple description.
//!
//--------------------------------------------------
struct ShaderId
{
public:
	typedef enum
	{
		WRITE_DEPTH = 0,
		SHADOW_MAP = 1,
		RECEIVE_SHADOW = 2,
		MATERIAL = 3,
		_NB_RENDER_MODE = 4,
	} RenderMode;

	typedef enum
	{
		VSTT_BASIC = 0,
		VSTT_SKINNED = 1,
		VSTT_BATCHED = 2,
		_VSTT_NB_VSTT_MODE = 3,
	} VertexMode;
private:
	unsigned m_renderMode : 6;
	unsigned m_vertexMode : 6;
	unsigned m_depthHazeOn : 1;
	unsigned m_shadowOn : 1;
public:
	// ctor
	ShaderId(RenderMode renderMode, VertexMode vertexMode, bool depthHazeOn, bool shadowOn)
		:m_renderMode(renderMode)
		,m_vertexMode(vertexMode)
		,m_depthHazeOn(depthHazeOn?1:0)
		,m_shadowOn(shadowOn?1:0)
	{
		// nothing
	}

	// invalid ctor
	ShaderId()
		:m_renderMode(_NB_RENDER_MODE)
		,m_vertexMode(_VSTT_NB_VSTT_MODE)
		,m_depthHazeOn(false)
		,m_shadowOn(false)
	{
		// nothing
	}

	RenderMode GetRenderMode() const
	{
		return RenderMode(m_renderMode);
	}
	VertexMode GetVertexMode() const
	{
		return VertexMode(m_vertexMode);
	}
	bool UseDepthHaze() const
	{
		return m_depthHazeOn==1;
	}
	bool UseShadow() const
	{
		return m_shadowOn==1;
	}

	int GetTechniqueIndex() const
	{
		// compute the technique index
		int iTechnique = 0, iPower = 1;

		iTechnique += iPower * ( UseShadow() ? 1 : 0 );
		iPower *= 2; // disabled/recieve screen aligned shadow map

		iTechnique += iPower * ( UseDepthHaze() ? 1 : 0 );
		iPower *= 2; // enabled/disabled

		return iTechnique;
	}
};


#endif // end of _SHADERID_H_
