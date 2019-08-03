//--------------------------------------------------
//!
//!	\file rangestancechain.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _ARMY_IMPOSTORS_H
#define _ARMY_IMPOSTORS_H

#ifndef _RENDERABLE_H
#include "gfx/renderable.h"
#endif

#ifndef IMPOSTOR_H
#include "effect/impostor.h"
#endif

#ifndef	TRANSFORM_H_
#include "anim/transform.h"
#endif

//--------------------------------------------------
//!
//!	ArmyImpostors
//!	Renderable that represents the army impostors
//!
//--------------------------------------------------
class ArmyImpostors : public CRenderable, IFrameFlagsUpdateCallback
{
public:
	ArmyImpostors( int iMaxImpostors, const ImpostorDef& def );
	~ArmyImpostors();

	//! render depths for z pre-pass
	virtual void RenderDepth();

	//! Renders the game material for this renderable.
	virtual void RenderMaterial();

	//! Renders the shadow map depths.
	virtual void RenderShadowMap();

	//! Unused on PS3 now
	virtual void RenderShadowOnly(){};

    // IFrameFlagsUpdateCallback
	virtual void PreUpdate() {};
    virtual void PostUpdate();

    virtual IFrameFlagsUpdateCallback* GetUpdateFrameFlagsCallback() const;

	// Accessor to fiddle with the sprites
	PointImpostor& GetImpostors() { return *m_pImpostors; }

	CAABB& GetBound()
	{
		return m_obBounds;
	}
private:
	PointImpostor* m_pImpostors;
	Transform m_transform;
};


#endif // _ARMY_IMPOSTORS_H
