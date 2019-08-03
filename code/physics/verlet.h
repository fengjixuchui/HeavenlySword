#ifndef	_DYNAMICS_VERLET_H
#define	_DYNAMICS_VERLET_H

#include "anim/transform.h"
#include "core/bitmask.h"
#include "gfx/renderable.h"
#include "core/explicittemplate.h"
#include "core/rotationnalindex.h"

#ifdef PLATFORM_PS3
#include "spu/flags/data.h"
//#include "exec/ppu/spuprogram_ps3.h"
#include "gfx/proceduralMaterial_ps3.h"
#include "physics/verletinstance_ps3.h"
#include "physics/verletdef_ps3.h"
#endif

class CRenderable;
class Template_Flag;
class SPUProgram;
class CEntity;

namespace Physics
{
	class VerletInstance;
	class VerletInstanceDef;
	class VerletRenderableInstance;
	class VerletMaterialInstanceDef;
	class VerletGameLink;

	//--------------------------------------------------
	//!
	//!	just a wrapper around VerletInstance.
	//!
	//--------------------------------------------------
	class RenderableVerletSystem
	{
	public:
		VerletInstance* m_pInstance;
	public:
		RenderableVerletSystem(VerletInstance* pNewData);
		~RenderableVerletSystem();
		void Update(float fTime);
		void SetupRenderingData() {};
		CRenderable* GetRenderable();
		void SetAllParticleMass(float) {} // TODO
	};
	
	//--------------------------------------------------
	//!
	//!	One verlet instance
	//!
	//--------------------------------------------------
	class VerletInstance
	{
	public:
		typedef enum
		{
			FIRST_UPDATE = BITFLAG(0),
			ISVISIBLE_FOR_GAMEUPDATE = BITFLAG(1),
		} Flags;

		typedef enum
		{
			CURRENT = 2,
			BEFORE = 1,
			EVENBEFORE = 0,
		} TimeIndex;

		typedef enum
		{
			FLAGIN = 0, // init
			CONSTRAINTS,
			TANGEANT_INDICES,
			STATIC,
			EXTERNAL_STATIC,
			EXTERNAL_DYNAMIC,
			EXTERNAL_CONSTRAINTS,
			DYNAMIC_0,
			DYNAMIC_1,
			DYNAMIC_2,
			FLAGOUT,
			MEM_SIZE,
			// nb elem in this enum:
			NB_ELEMS,
		} LOCATION;

		// nb buffer
		static const uint32_t g_uiNbBuffers = 3; 

#ifdef PLATFORM_PS3
		// value for dma
		FlagBinding::FlagIn* m_pFlagIn;
		FlagBinding::FlagOut* m_pFlagOut;
		FlagBinding::PointDynamic* m_timeArray[g_uiNbBuffers];

		// rotational index
		RotationnalIndex<uint8_t,g_uiNbBuffers> m_rotationnalIndex;
		CScopedArray<CVector> m_pDefaultPosition; // debug purpose
		
		// attachment to non static object
		uint32_t m_uiNbAttachment;
		CScopedArray<CPoint> m_attachment;
#endif
		// renderable (just for deletion)
		VerletRenderableInstance* m_pRenderable;
		
		// transform
		Transform* m_pTransform;
		CMatrix m_oldWorldMatrix;

#ifdef PLATFORM_PS3
		// for placement new
		uint32_t m_uiDmaLocation[NB_ELEMS];
		CScopedArray<uint8_t> m_pDmaMemory;
		const SPUProgram* m_pElf;
#endif

		// offset
		CMatrix m_offset;
		CPoint m_fixup;

		//  flags
		BitMask<Flags> m_flags;

		Template_Flag*	m_pobTemplate;
		Template_Flag*	GetTemplate() const {return m_pobTemplate;}
		void			SetTemplate(Template_Flag* pobTemplate) {m_pobTemplate = pobTemplate;}
		

	public:

		///////////////////////////
		VerletInstance(Transform* pTransform, const VerletInstanceDef& def);

		///////////////////////////
		~VerletInstance();
	
		// per frame update
		VerletRenderableInstance* GetRenderable() {return m_pRenderable;}

		
		///////////////////////////
		void SetRenderable(VerletRenderableInstance* pRenderable);

		///////////////////////////
		void SetDefaultPosition(const VerletInstanceDef& def, bool bReset = true);

		///////////////////////////
		void ResetToDefaultPosition();

		///////////////////////////
		void ComputeLocation(const VerletInstanceDef& def);

		///////////////////////////
		void AllocateDmaMemory(const VerletInstanceDef& def);

		///////////////////////////
		void UpdateExternalConstraint();

		// init attachment
		void InitAttachment(const VerletInstanceDef& def);

		///////////////////////////
		void Update(float fTimeDelta);
			
		///////////////////////////
		void CopyStatic(const VerletInstanceDef& def);

		///////////////////////////
		void DebugRender();

		/////////////////////////// NOTE just a helper for AttachToTransform now
		void AttachToEntity(CEntity* pobParent, CKeyString& obTransform);

		///////////////////////////
		void AttachToTransform(Transform* pobParentTransform );

		///////////////////////////
		void DetachFromParent();

	}; // end of class VerletInstance







}

#endif // _DYNAMICS_VERLET_H
