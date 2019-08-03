/***************************************************************************************************
*
*	$Header:: /game/meshinstance.h 8     24/07/03 11:53 Simonb                                     $
*
*	An instance of a mesh, with a given transform, material, and property list.
*
*	CHANGES
*
*	7/5/2003	SimonB	Created
*
***************************************************************************************************/

#ifndef _MESHINSTANCE_H
#define _MESHINSTANCE_H

#include "gfx/renderable.h"
#include "materialinstance.h"
#include "gfx/fxmaterial.h"
#include "gfx/gfxformat.h"

class CMaterial;

#ifdef PLATFORM_PS3
#define ERROR_MATERIAL	"error"
#else
#define ERROR_MATERIAL	"lambert"
#endif

// fake it up
#if defined( PLATFORM_PC )
struct Heresy_PushBuffer;
#endif

class CHeresyPushBuffers;

typedef FwStd::IntrusivePtr<CHeresyPushBuffers>	HeresyPushBuffersPtr;

/***************************************************************************************************
*
*	CLASS			CMeshInstance
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

class CMeshInstance : public CRenderable
{
	friend class CHeresyPushBuffers;
public:
	//! Creates a mesh instance.
	CMeshInstance(	Transform const* pobTransform, CMeshHeader const* pobMeshHeader,
					bool bRenderOpaque, bool bShadowRecieve, bool bShadowCast, bool bCreatePushBuffers = true, unsigned int iRenderableType = RT_MESH_INSTANCE );

	//! Destroys a mesh instance
	virtual ~CMeshInstance();

	//! Gets the mesh used for this instance.
	CMeshHeader const* GetMeshHeader() const { return m_pobMeshHeader; }
	
	//! Forces this mesh to use the given material.
	virtual void ForceMaterial( CMaterial const* pobMaterial );
	
	void ForceFXMaterial( const FXMaterial* pMaterial );

	//! Return the game material pointer for game related mesh manipulation
	MaterialInstanceBase*	GetMaterialInstance() { return m_pMaterial.Get(); }	

	//! render depths for z pre-pass
	virtual void RenderDepth();

	//! Renders the game material for this renderable.
	virtual void RenderMaterial();

	//! Renders the shadow map depths.
	virtual void RenderShadowMap();

	//! Renders with a shadow map compare only. 
	virtual void RenderShadowOnly();

	// use previous material
	void MaterialRollBack();

	//! new public interface for all area-related resources. A bit more general than the old create/delete push
	//! buffer methods
	virtual void ReleaseAreaResources( void );
	virtual void CreateAreaResources( void );

	const Heresy_PushBuffer* GetDepthPushBuffer(void);		
	const Heresy_PushBuffer* GetRenderPushBuffer(void);		
	const Heresy_PushBuffer* GetShadowMapPushBuffer(void);	
	
	const void* GetReconstructionMatrix() const;
	
	const u_int GetIndexCount(void)  const { return m_iIndexCount; }

protected:
	virtual void RenderMesh() const;
	void ForceFXMaterialFinalise();

	//! Forces the creation of our heresy push buffers
	void CreatePushBuffers();

	//! Force the deletion of our push buffers
	void DeletePushBuffers();

	virtual void GetVertexAndIndexBufferHandles( void );

	const CMeshHeader*	m_pobMeshHeader;
	VBHandle			m_hVertexBuffer[2];
	IBHandle			m_hIndexBuffer;
	
	void ResetMaterial(MaterialInstanceBase* pMaterial);
    CScopedPtr<MaterialInstanceBase> m_pMaterial;
    CScopedPtr<MaterialInstanceBase> m_pMaterialLast;
		
	PRIMTYPE	m_meshType;
	u_int		m_iPolyCount;
	u_int		m_iIndexCount;

	HeresyPushBuffersPtr	GetPushBuffers();
	void BuildDepthPB(Heresy_PushBuffer** pushBuffer);
	void BuildRenderPB(Heresy_PushBuffer** pushBuffer);
	void BuildShadowMapPB(Heresy_PushBuffer** pushBuffer);
	static void DestroyPushBuffer(Heresy_PushBuffer* pushBuffer);

	void RenderPB( restrict Heresy_PushBuffer* pPB );
	HeresyPushBuffersPtr	m_heresyPushBuffers;

};

#endif // ndef _MESHINSTANCE_H
