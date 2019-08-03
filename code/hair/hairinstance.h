#ifndef _HAIRINSTANCE_H_
#define _HAIRINSTANCE_H_

#include "gfx/meshinstance.h"


class CHairInstance: public CMeshInstance
{
public:
	//! constructor
	CHairInstance(Transform const* pobTransform, CMeshHeader const* pobMeshHeader,bool bRenderOpaque, bool bShadowRecieve, bool bShadowCast);

	//! Renders the game material for this renderable.
	//virtual void RenderMesh() const;
}; // end of class CHairInstance

#endif // end of _HAIRINSTANCE_H_
