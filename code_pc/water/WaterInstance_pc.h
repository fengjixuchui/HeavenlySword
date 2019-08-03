//--------------------------------------------------
//!
//!	\file WaterInstance_pc.h
//!
//!
//--------------------------------------------------


#ifndef _WATERINSTANCE_PC_H_
#define _WATERINSTANCE_PC_H_



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
	virtual void Reset( void ) {}
	virtual	void DebugRender( bool bRenderBuoys = false, bool bRenderSPULines = false ) { UNUSED(bRenderBuoys); UNUSED(bRenderSPULines);}

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
	virtual void RenderDepth() {}
	//! Renders the game material for this renderable.
	virtual void RenderMaterial() {}
	//! Renders the shadow map depths.
	virtual void RenderShadowMap() {}
	//! Renders with a shadow map compare only. 
	virtual void RenderShadowOnly() {}

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
};






#endif // end of _WATERINSTANCE_PC_H_
