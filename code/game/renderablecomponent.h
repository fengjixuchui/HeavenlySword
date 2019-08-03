/***************************************************************************************************
*
*	DESCRIPTION		Renderable component that manages all things on an entity that are renderable.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _RENDERABLECOMPONENT_H
#define _RENDERABLECOMPONENT_H

#include "core/boundingvolumes.h"

class CHierarchy;
class Transform;
class CMeshInstance;
class CProjectedShadowCaster;
class COrientedSpringSystem;
class CMaterial;
class CRenderable;

/***************************************************************************************************
*
*	CLASS			CRenderableComponent
*
*	DESCRIPTION		This class can be seen as a set of renderables. Like the CHierarchy class, it is more
*                   or less an instance of a clump file (contained in the hierarchy passed in the constructor)
*
***************************************************************************************************/

class CRenderableComponent
{
public:
	//! Creates the component.
	CRenderableComponent(	CHierarchy* pobHierachy,
							bool bRenderOpaque,
							bool bShadowRecieve,
							bool bShadowCast,
							bool bCreatePushBuffers = true );

	//! Deletes all components owned by the component.
	~CRenderableComponent();

	//! ATTN! only use these if you know what your doing and you intend
	//! to restore their state afterwards
	void DisableAllRenderables();
	void EnableAllRenderables();
	void DisableAllShadows();
	void EnableRenderByMeshName( CHashedString obNameHash, bool bRender );
	void EnableShadowCastByMeshName( CHashedString obNameHash, bool bShadowCast );
	void EnableAllByTransform( const Transform* pobTransform, bool bEnable, bool bCheck = false );
	void EnableAllByMeshName( const CHashedString& obName, bool bEnable );

	//! Get the bounding box for this particular mesh
	const CMeshInstance* GetMeshByMeshName( CHashedString obNameHash ) const;


	typedef ntstd::List<CRenderable*, Mem::MC_GFX > RenderableList;
	typedef ntstd::List<CMeshInstance*, Mem::MC_GFX > MeshInstanceList;

	//! Accessor to CMeshInstances
	MeshInstanceList&	GetMeshInstances( void ) { return m_obMeshes; }
	const MeshInstanceList&	GetMeshInstances( void ) const { return m_obMeshes; }


	//! Forces all meshes to use the given material.
	void ForceMaterial( CMaterial const* pobMaterial );

	//! Allows effects to be added and owned by an entity
	void AddAddtionalRenderable( CRenderable* pobRenderable );

	const CRenderable* GetRenderable (const Transform* pobTransform);
	const CPoint &	GetPosition		()	const;
	CAABB GetWorldSpaceAABB() const;

	//! Add / remove components renderables to the level render list
	//! from a game or area system point of view.
	void AddRemoveAll_AreaSystem( bool bAdd );
	void AddRemoveAll_Game( bool bAdd );

	//! Accessors on flags that govern whether or not this renderable
	//! components are within the level render lists.
	bool DisabledByArea( void ) const { return m_bDisabledByArea; }
	bool DisabledByGame( void ) const { return m_bDisabledByGame; }

	//! Forces the creation of our heresy push buffers
	void CreateAreaResources();

	//! Force the deletion of our push buffers
	void ReleaseAreaResources();

	bool IsSuspended() const
	{
		return m_bProcessingSuspended;
	}

private:

	// flags govern render status
	bool	m_bProcessingSuspended;
	bool	m_bDisabledByArea;
	bool	m_bDisabledByGame;

	// sectoring hooks
	void	AddToLevelRenderables();
	void	RemoveFromLevelRenderables();
	void	SetProcessing();
	
	RenderableList							m_obRenderables;		//!< All renderables.
	MeshInstanceList						m_obMeshes;				//!< Duplicate list of just meshes.

	const CHierarchy *m_pobHierarchy;
};

#endif // _RENDERABLECOMPONENT_H
