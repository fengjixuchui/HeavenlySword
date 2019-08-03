/***************************************************************************************************
*
*	DESCRIPTION		The core renderable base class.
*
*	NOTES			Anything to be automatically rendered by a renderer spatial hierarchy needs
*					to derive itself from the class CRenderable.
*
***************************************************************************************************/

#ifndef _RENDERABLE_H
#define _RENDERABLE_H

#include "core/boundingvolumes.h"

class CullingFrustum;
class Transform;

struct IFrameFlagsUpdateCallback
{
	virtual ~IFrameFlagsUpdateCallback() {};
    virtual void PreUpdate() = 0;
    virtual void PostUpdate() = 0;
};

/***************************************************************************************************
*
*	CLASS			CRenderable
*
*	DESCRIPTION		A renderable object base class. Currently testing revision number 3 billion.
*
***************************************************************************************************/

class CRenderable : CNonCopyable
{
public:
	//! These get updated every frame for every renderable in SetVisibleFrustum Don't change the numbers without checking the entire codebase first!
	enum RENDERABLE_FRAME_FLAGS
	{
		RFF_CLEAR				= 0,		//!< no frame flags not visible, not casting 
		RFF_VISIBLE				= (1 << 0),	//!< the renderable is visible this frame
		RFF_CLIPPING			= (1 << 1), //!< this renderable clips (intersects) at least one clip plane of the frustum.

		RFF_CAST_SHADOWMAP0		= (1 << 2), //!< this renderable casts shadows on shadowmap0
		RFF_CAST_SHADOWMAP1		= (1 << 3), //!< this renderable casts shadows on shadowmap1
		RFF_CAST_SHADOWMAP2		= (1 << 4), //!< this renderable casts shadows on shadowmap2
		RFF_CAST_SHADOWMAP3		= (1 << 5), //!< this renderable casts shadows on shadowmap3

		RFF_RECIEVES_SHADOWMAP0	= (1 << 6), //!< this renderable recieves shadows on shadowmap0
		RFF_RECIEVES_SHADOWMAP1	= (1 << 7), //!< this renderable recieves shadows on shadowmap1
		RFF_RECIEVES_SHADOWMAP2	= (1 << 8), //!< this renderable recieves shadows on shadowmap2
		RFF_RECIEVES_SHADOWMAP3	= (1 << 9), //!< this renderable recieves shadows on shadowmap3

		RFF_CLIPS_SHADOWMAP0	= (1 << 10), //!< this renderable receiving clips the shadows frustumon shadowmap0
		RFF_CLIPS_SHADOWMAP1	= (1 << 11), //!< this renderable receiving clips the shadows frustumon shadowmap1
		RFF_CLIPS_SHADOWMAP2	= (1 << 12), //!< this renderable receiving clips the shadows frustumon shadowmap2
		RFF_CLIPS_SHADOWMAP3	= (1 << 13), //!< this renderable receiving clips the shadows frustumon shadowmap3
	};

	enum RENDERABLE_TYPES
	{
		RT_BASE = 0,
		RT_MESH_INSTANCE,
		RT_SPEED_TREE,
		RT_SPEED_TREE_CELL,
		RT_SPEED_GRASS,
		RT_VERLET,
		RT_CHAINMAN_CHAINS,
		RT_COMMANDER_CHAINS,
		RT_RANGESTANCE_CHAIN,
		RT_WATER,
		RT_BS_MESH_INSTANCE,
		RT_ARMYIMPOSTOR,
		RT_END
	};

	CAABB m_obBounds;							//!< A transform aligned bounding box.

	unsigned int m_FrameFlags; //!< reset every frame with the relevant data from above

	CRenderable( const Transform* pobTransform, bool bRenderOpaque, bool bShadowRecieve, bool bShadowCast, unsigned int iRenderableType = RT_BASE );
	virtual ~CRenderable();

	// inc / dec feature semaphores here.
	void DisableRendering( bool bDisabled )
	{
		m_iDisableCount_Rendering = bDisabled ? (m_iDisableCount_Rendering+1) : (m_iDisableCount_Rendering-1);
		ntAssert_p( m_iDisableCount_Rendering >=0, ("Rendering enabled too many times") );
		ntAssert_p( m_iDisableCount_Rendering <100, ("Rendering disabled ALOT, likely you're disabling without enabling") );
	}

	void DisableShadowCasting( bool bDisabled )
	{
		m_iDisableCount_SCasting = bDisabled ? (m_iDisableCount_SCasting+1) : (m_iDisableCount_SCasting-1);
		ntAssert_p( m_iDisableCount_SCasting >=0, ("Shadow Casting enabled too many times") );
		ntAssert_p( m_iDisableCount_SCasting <100, ("Shadow Casting disabled ALOT, likely you're disabling without enabling") );
	}

	void DisableShadowRecieving( bool bDisabled )
	{
		m_iDisableCount_SRecieving = bDisabled ? (m_iDisableCount_SRecieving+1) : (m_iDisableCount_SRecieving-1);
		ntAssert_p( m_iDisableCount_SRecieving >=0, ("Shadow Recieving enabled too many times") );
		ntAssert_p( m_iDisableCount_SRecieving <100, ("Shadow Recieving disabled ALOT, likely you're disabling without enabling") );
	}

	bool IsRendering() const		{ return (m_iDisableCount_Rendering == 0); }
	bool IsShadowCasting() const	{ return (m_iDisableCount_SCasting == 0); }
	bool IsShadowRecieving() const	{ return (m_iDisableCount_SRecieving == 0); }

	void SetAlphaBlended( bool bEnabled )	{ m_bIsAlphaBlended = bEnabled; }
	bool IsAlphaBlended() const				{ return m_bIsAlphaBlended; }

	void SetIsSkinned( bool bEnabled )	{ m_bIsSkinned = bEnabled; }
	bool IsSkinned() const				{ return m_bIsSkinned; }

	void SetUseHeresy( bool bEnabled )	{ m_bUseHeresy = bEnabled; }
	bool UseHeresy() const				{ return m_bUseHeresy; }

	void SetIsShapeBlended( bool bEnabled )	{ m_bIsShapeBlended = bEnabled; }
	bool IsShapeBlended() const { return m_bIsShapeBlended; }

	unsigned int GetRenderableType( void ) { return m_iRenderableType; }

	Transform const* GetTransform() const;
	CAABB const& GetBounds() const;

	CAABB GetWorldSpaceAABB() const;

	//! Tests this renderable's AABB against a view frustum. Also has optional shadow culling planes
	bool UpdateFrameFlags( CullingFrustum const* pFrustum, const unsigned int iNumShadowVolumes, CVector* pShadowPlanes = 0, CullingFrustum* pShadowFrustums = 0 );

	//! render depths for z pre-pass
	virtual void RenderDepth() = 0;

	//! Renders the game material for this renderable.
	virtual void RenderMaterial() = 0;

	//! Renders the shadow map depths.
	virtual void RenderShadowMap() = 0;

	//! Renders with a shadow map compare only. 
	virtual void RenderShadowOnly() = 0;

	//! return sort key for this renderable
	unsigned int GetSortKey() const { return m_iSortKey; }

	//! calculate sort key, overrideable per sub-class if nessecary
	virtual void CalculateSortKey(const CMatrix* pTransform);

    virtual IFrameFlagsUpdateCallback* GetUpdateFrameFlagsCallback() const;

	void DisplayBounds();

protected:
	Transform const* m_pobTransform;			//!< The transform for this renderable.


	int m_iDisableCount_Rendering;		//!< Semaphore counting Disable() requests of rendering
	int m_iDisableCount_SCasting;		//!< Semaphore counting Disable() requests of shadow casting
	int m_iDisableCount_SRecieving;		//!< Semaphore counting Disable() requests of shadow recieving
	
	bool m_bIsAlphaBlended;						//!< Is this renderable alpha-blended.
	bool m_bIsSkinned;							//!< Does this renderable needs skinning
	bool m_bIsShapeBlended;						//! needs vertex shapeblending
	bool m_bUseHeresy;

	unsigned int m_iSortKey;					//!< Sorted so 0 render before +
	unsigned int m_iRenderableType;

private:
	CRenderable(){};
};

#endif // ndef _RENDERABLE_H
