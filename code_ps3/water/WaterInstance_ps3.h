//--------------------------------------------------
//!
//!	\file WaterInstance_ps3.h
//!
//!
//--------------------------------------------------


#ifndef _WATERINSTANCE_PS3_H_
#define _WATERINSTANCE_PS3_H_



#include "gfx/renderable.h"
#include "gfx/vertexdeclaration.h"
#include "gfx/proc_vertexbuffer.h"
#include "gfx/texturemanager.h"
#include "anim/transform.h"
#include "editable/enumlist.h"
#include "tbd/xmlfileinterface.h"


//
// Fwd class declarations
//
struct WaterInstanceDef;
struct WaterInstanceDma;
struct AttackDma;
struct WaveDma;
struct VertexDma;
struct BuoyDma;


class DebugShader;
class CEntity;
class SPUTask;

class WaterInstance : public CRenderable
{
public:
	WaterInstance( WaterInstanceDef* pobDef );
	virtual ~WaterInstance();

	virtual void Update( float fTimeStep );
	virtual void Reset( void ) { InitGridPositions(); }
	virtual	void DebugRender( bool bRenderBuoys = false, bool bRenderSPULines = false );

	uint32_t	GetNumOfWaveSlots( void ) const;
	WaveDma*	GetWaveSlot( uint32_t index );
	WaveDma*	GetFirstAvailableWaveSlot( void );

	uint32_t	GetNumOfBuoySlots( void ) const;
	BuoyDma*	GetBuoySlot( uint32_t index );
	BuoyDma*	GetFirstAvailableBuoySlot( void );	

	WaterInstanceDef& GetDefinition( void ) const { return *m_pobDef; }

	//! in local space!
	const VertexDma&	GetVertexAtAproxPos( float x, float z ) const;
	const VertexDma&	GetVertexAtCoords( uint32_t i, uint32_t j ) const;


	const CMatrix&		GetWaterToWorldMatrix( void ) const;
	const CMatrix&		GetWorldToWaterMatrix( void ) const;

	
	//
	//		CRenderable stuff
	//
	//! render depths for z pre-pass
	virtual void RenderDepth();
	//! Renders the game material for this renderable.
	virtual void RenderMaterial();
	//! Renders the shadow map depths.
	virtual void RenderShadowMap() { ntAssert_p( HasDmaResources(), ("WATER - water instance has no dma resources\n") ); }
	//! Renders with a shadow map compare only. 
	virtual void RenderShadowOnly() { ntAssert_p( HasDmaResources(), ("WATER - water instance has no dma resources\n") ); }

protected:
	//! sets the dma params for a given task
	void SetupDma( SPUTask& task ) const;

private:
	//
	// Area resource stuff
	//
	//! obvious
	void CreateDmaResources( void );
	//! obvious
	void DestroyDmaResources( void );
	//! duh
	bool HasDmaResources( void ) const;
	//!	inits grid positions to be evenly distributed in a flat grid of Width x Length
	void InitGridPositions( void );
	//! sets indices to produce triangle strips of a "transform cache friendly" size
	void InitIndexBuffer( void );


	//!	uses data from soft definitions to fill the wave array
	void ProcessEmitters( void );
	// tmp stuff
	void ProcessBuoys( void );
//	void InitBuoysTmp( void );
	void DebugRenderBuoys( void );

	void UpdateBuoyPPU( BuoyDma& buoy );

	void UpdateWavesPPU( float fTimeStep );

private:
	friend class WaterManager;
	/////////////////////////////////////////////////////
	//						DMA'ble part
	/////////////////////////////////////////////////////
	WaterInstanceDma*			m_pobWaterDma;				//!< actual water instance data
	VertexDma*					m_pVertexDmaArray;			//!< grid vertices. Aliased from ProceduralVB.m_data
	WaveDma*					m_pWaveDmaArray;			//!< standard wave array
	BuoyDma*					m_pBuoyDmaArray;			//!< buoy array
	/////////////////////////////////////////////////////

	WaterInstanceDef*			m_pobDef;


	ProceduralVB				m_hVB;
	IBHandle					m_hIB;
	u_int						m_iTriangleCount;
	u_int						m_iIndexCount;

	DebugShader*				m_pobVertexShader_Colour;
	DebugShader*				m_pobPixelShader_Colour;
	DebugShader*				m_pobVertexShader_Depth;
	DebugShader*				m_pobPixelShader_Depth;

	Texture::Ptr				m_pobNormalMap0;
	Texture::Ptr				m_pobNormalMap1;
};


#endif // end of _WATERINSTANCE_PS3_H_
