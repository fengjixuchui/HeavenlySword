//--------------------------------------------------
//!
//!	\file verlet.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------
#include "verlet.h"

#ifdef PLATFORM_PS3
#include "physics/verletManager_ps3.h"
#include "physics/verletdef_ps3.h"
#include "physics/verletinstance_ps3.h"
#include "physics/verletrenderable_ps3.h"

#include "core/boundingvolumes.h"
#include "core/frustum.h"
#include "game/entitymanager.h"
#include "core/visualdebugger.h"
#include "core/timer.h"
#include "anim/transform.h"
#include <limits>

#include "exec/ppu/dmabuffer_ps3.h"
#include "exec/ppu/exec_ps3.h"
#include "exec/ppu/spuprogram_ps3.h"
#include "exec/ppu/sputask_ps3.h"
#include "exec/ppu/ElfManager.h"
#include "gfx/renderstates.h"
#include "gfx/renderer_ps3.h"
#include "gfx/graphicsdevice.h"

#include "physics/softflag.h"
#include "physics/globalwind.h"

#include "gfx/lightingtemplates.h"
#include "gfx/pictureinpicture.h"

#include "core/gatso.h"

namespace 
{
	template<typename UINT>
	UINT GetNextAlignment(UINT loc,UINT alignment)
	{
		UINT mod = (loc % alignment);
		if(mod > 0)
		{
			return loc-mod+alignment;
		}
		else
		{
			return loc;
		}
	}


	bool IsVisible(const CAABB& flagBB)
	{
		CPoint dec(5.0f,5.0f,5.0f);
		CAABB bb = flagBB;
		bb.Min() -= dec;
		bb.Max() += dec;

		SortableList<PIPView> views;
		Renderer::Get().m_pPIPManager->GetValidViews(views);
		for(SortableList<PIPView>::listType::iterator it = views.m_list.begin();
			it != views.m_list.end();
			++it)
		{
			PIPView& view = *(*it);
			const CCamera* pCamera = view.GetCamera();
			if(pCamera)
			{
				CMatrix viewToScreen;
				pCamera->GetProjection(16.f/9.f, viewToScreen ); // fake aspect ratio
				CMatrix worldToView = pCamera->GetViewTransform()->GetWorldMatrix().GetAffineInverse();
				CMatrix worldToScreen = worldToView * viewToScreen;
				
				CullingFrustum frustum( worldToScreen );
				
				if(frustum.FastTestBox(&bb)!=CullingFrustum::COMPLETELY_OUTSIDE)
				{
					return true;
				}
			}
		}
		
		return false;
	}
} // end of namespace 



namespace Physics
{

	RenderableVerletSystem::RenderableVerletSystem(VerletInstance* pInstance)
		:m_pInstance(pInstance)
	{
		// nothing
	}

	RenderableVerletSystem::~RenderableVerletSystem()
	{
		VerletManager::Get().Unregister(m_pInstance);
		NT_DELETE(m_pInstance);
	}

	void RenderableVerletSystem::Update(float fTime)
	{
		m_pInstance->Update(fTime);
	}

	CRenderable* RenderableVerletSystem::GetRenderable()
	{
		return m_pInstance->GetRenderable();
	}

	///////////////////////////
	VerletInstance::VerletInstance(Transform* pTransform, const VerletInstanceDef& def)
		:m_pFlagIn(0)
		,m_pFlagOut(0)
		,m_pRenderable(0)
		,m_pTransform(pTransform)
		,m_pDmaMemory(0)
		,m_pElf( ElfManager::Get().GetProgram("flags_spu_ps3.mod") )
	{
		using namespace FlagBinding;
		
		// allocating memory
		AllocateDmaMemory(def);

		// goto default positon
		SetDefaultPosition(def);

		// mass
		CopyStatic(def);

		// init attach to static
		InitAttachment(def);

		// first update to come
		m_flags.Set(FIRST_UPDATE);
	}

	///////////////////////////
	void VerletInstance::SetRenderable(VerletRenderableInstance* pRenderable)
	{
		m_pRenderable=pRenderable;
	}

	void VerletInstance::InitAttachment(const VerletInstanceDef& def)
	{
		m_uiNbAttachment = def.m_staticPoints.size();
		m_attachment.Reset( NT_NEW CPoint[m_uiNbAttachment] );
		for(uint32_t uiPoint = 0 ; uiPoint < m_uiNbAttachment ; ++uiPoint )
		{
			uint16_t usIndex = def.m_staticPoints[uiPoint];
			m_attachment[uiPoint]= def.m_particles[usIndex].m_position;
		}
	}

	///////////////////////////
	VerletInstance::~VerletInstance() 
	{
		DetachFromParent();
	}

	void VerletInstance::DetachFromParent()
	{
		if (m_pTransform && m_pTransform->GetParent() )
			m_pTransform->RemoveFromParent();

	}


	void VerletInstance::SetDefaultPosition(const VerletInstanceDef& def, bool bReset)
	{
		// init
		m_offset = def.m_offset;
		m_fixup = CPoint(0.f,def.m_realSize[1],def.m_realSize[0]*0.5f);;
		
		// compute transform
		CMatrix trf = m_offset * m_pTransform->GetWorldMatrix();
		CMatrix rot = trf;
		rot.SetTranslation(CPoint(0.f,0.f,0.f));
		CPoint fixup = m_fixup * rot;

		// set default position
		for(uint16_t usFlatIndex = 0 ; usFlatIndex < m_pFlagIn->m_nbPoints ; ++usFlatIndex)
		{
			CPoint particle = def.m_particles[usFlatIndex].m_position * trf - fixup;
			m_pDefaultPosition[usFlatIndex]=CVector(particle);
		}
		if(bReset)
		{
			ResetToDefaultPosition();
		}
	}
	void VerletInstance::ResetToDefaultPosition()
	{
		for(uint16_t index = 0 ; index < m_pFlagIn->m_nbPoints ; ++index)
		{
			m_timeArray[m_rotationnalIndex[CURRENT]][index].m_position=m_pDefaultPosition[index];
			m_timeArray[m_rotationnalIndex[BEFORE]][index].m_position=m_pDefaultPosition[index];
			m_timeArray[m_rotationnalIndex[EVENBEFORE]][index].m_position=m_pDefaultPosition[index];
		}
	}

	void VerletInstance::ComputeLocation(const VerletInstanceDef& def)
	{
		using namespace FlagBinding;

		uint32_t uiNbPoints = def.m_particles.size();
		uint32_t uiNbConstraints = def.m_constraints.size();
		uint32_t uiNbStaticPoints = def.m_staticPoints.size();

		// compute location
		uint32_t uiCurrent = 0;
		
		m_uiDmaLocation[FLAGIN] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + sizeof(FlagIn),16);

		m_uiDmaLocation[CONSTRAINTS] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbConstraints*sizeof(Constraint),16);

		m_uiDmaLocation[TANGEANT_INDICES] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbPoints*sizeof(TangeantPlaneIndices),16);

		m_uiDmaLocation[STATIC] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbPoints*sizeof(PointStatic),16);

		m_uiDmaLocation[EXTERNAL_STATIC] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbStaticPoints*sizeof(PointStatic),16);

		m_uiDmaLocation[EXTERNAL_DYNAMIC] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbStaticPoints*sizeof(PointDynamic),16);
		
		m_uiDmaLocation[EXTERNAL_CONSTRAINTS] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbStaticPoints*sizeof(Constraint),16);
		
		m_uiDmaLocation[DYNAMIC_0] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbPoints*sizeof(PointDynamic),16);

		m_uiDmaLocation[DYNAMIC_1] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbPoints*sizeof(PointDynamic),16);

		m_uiDmaLocation[DYNAMIC_2] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbPoints*sizeof(PointDynamic),16);

		m_uiDmaLocation[FLAGOUT] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + sizeof(FlagOut),16);

		m_uiDmaLocation[MEM_SIZE] = uiCurrent;
	}

	///////////////////////////
	void VerletInstance::AllocateDmaMemory(const VerletInstanceDef& def)
	{
		uint32_t nbPoints = def.m_particles.size();
		uint32_t nbConstraints = def.m_constraints.size();
		uint32_t nbStaticPoints = def.m_staticPoints.size();

		ntError_p(nbPoints<0x7fff , ("Too many points"));
		ntError_p(nbConstraints<0x7fff , ("Too many constraints"));
		using namespace FlagBinding;
		ComputeLocation(def);

		// total size and allocation
		m_pDmaMemory.Reset( NT_NEW uint8_t[ m_uiDmaLocation[MEM_SIZE] ] );

		// flags in
		m_pFlagIn = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[FLAGIN]) FlagIn;
		m_pFlagIn->m_global.m_fDeltaTime = 0.1f;
		//wind
		//
		m_pFlagIn->m_oldToNewWorld = CMatrix(CONSTRUCT_IDENTITY);
		m_pFlagIn->m_worldToObject = CMatrix(CONSTRUCT_IDENTITY);
		// static param
		m_pFlagIn->m_nbPoints=nbPoints;
		m_pFlagIn->m_pGridStatic = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[STATIC]) PointStatic;
		// constraints
		m_pFlagIn->m_nbConstraints=nbConstraints;
		m_pFlagIn->m_pConstraints = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[CONSTRAINTS]) Constraint;
		// mesh
		m_pFlagIn->m_nbMeshElements=def.m_staticBuffer.size();
		m_pFlagIn->m_pTangeantPlaneIndices = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[TANGEANT_INDICES]) TangeantPlaneIndices;
		
		// external constraint
		m_pFlagIn->m_nbExternalConstraints = nbStaticPoints;
		m_pFlagIn->m_pExternalConstraints = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[EXTERNAL_CONSTRAINTS]) Constraint;;
		m_pFlagIn->m_pExternalStatic = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[EXTERNAL_STATIC]) PointStatic;;
		m_pFlagIn->m_pExternalDynamic = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[EXTERNAL_DYNAMIC]) PointDynamic;;

		// flags in
		m_pFlagOut = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[FLAGOUT]) FlagOut;
		
		// dynamic arrays
		m_timeArray[ m_rotationnalIndex[CURRENT] ] = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[DYNAMIC_0]) PointDynamic;
		m_timeArray[ m_rotationnalIndex[BEFORE] ] = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[DYNAMIC_1]) PointDynamic;
		m_timeArray[ m_rotationnalIndex[EVENBEFORE] ] = NT_PLACEMENT_NEW(m_pDmaMemory.Get() + m_uiDmaLocation[DYNAMIC_2]) PointDynamic;

		// also allocate default position here
		m_pDefaultPosition.Reset( NT_NEW CVector[nbPoints] );
	}


	void VerletInstance::UpdateExternalConstraint()
	{
		using namespace FlagBinding;
		
		// compute transform
		CMatrix trf = m_offset * m_pTransform->GetWorldMatrix();
		CMatrix rot = trf;
		rot.SetTranslation(CPoint(0.f,0.f,0.f));
		CPoint fixup = m_fixup * rot;

		// update position
		PointDynamic* pExternalDynamic = m_pFlagIn->m_pExternalDynamic.Get();
		//CMatrix local2World = m_pTransform->GetWorldMatrix();
		for(uint32_t uiConstraint = 0 ; uiConstraint < m_uiNbAttachment ; ++uiConstraint )
		{
			// attachment constraint
			CPoint particle = m_attachment[uiConstraint] * trf - fixup;
			CVector worldPosition = CVector(particle);
			worldPosition.W() = 1.0f;
			pExternalDynamic[uiConstraint].m_position = worldPosition;
		}
	}


	///////////////////////////
	void VerletInstance::Update(float fTimeDelta)
	{
		using namespace FlagBinding;
		
		// deprectaed soon:
		if(m_flags[FIRST_UPDATE])
		{
			m_pFlagIn->m_oldToNewWorld = CMatrix(CONSTRUCT_IDENTITY);
			m_oldWorldMatrix = m_pTransform->GetWorldMatrix();
			//m_oldWorldMatrix.SetTranslation(CEntityManager::Get().GetPlayer()->GetPosition()); // debug
		}
		else
		{
			CMatrix worldMatrix = m_pTransform->GetWorldMatrix();
			//worldMatrix.SetTranslation(CEntityManager::Get().GetPlayer()->GetPosition()); // debug
			m_pFlagIn->m_oldToNewWorld = m_oldWorldMatrix.GetAffineInverse() * worldMatrix;
			m_oldWorldMatrix = worldMatrix;
		}

		// update external constraint
		UpdateExternalConstraint();

		// set visibility flag, because we don't want to animate a non-visible flags
		// not that the first update will consider everything visible, just to make sure everything is initialised ok
		m_flags.Set(ISVISIBLE_FOR_GAMEUPDATE,m_flags[FIRST_UPDATE] || IsVisible(m_pRenderable->GetWorldSpaceAABB()));
		m_flags.Unset(FIRST_UPDATE); // unset first update here

		m_flags.Set(ISVISIBLE_FOR_GAMEUPDATE, true); //MBREMOVE

		// set breakpoint flag
		m_pFlagIn->m_flags.Reset();
		if( VerletManager::Get().GetFlags(VerletManager::SPU_BREAKPOINT_AUX))
		{
			m_pFlagIn->m_flags.Set(FlagBinding::BREAKPOINT);
		}
		
		/////////////////////////////////////////
		/////////////////////////////////////////
		// update per-frame value:

		GlobalWind& obWind = *m_pobTemplate->GetWind();
		SoftMaterial& obSoft = *m_pobTemplate->GetSoftMaterial();

		// world to object matrix
		m_pFlagIn->m_worldToObject = m_pTransform->GetWorldMatrix().GetAffineInverse();
		
		// time
		m_pFlagIn->m_global.m_fTime = CTimer::Get().GetGameTime();
		m_pFlagIn->m_global.m_fDeltaTime = fTimeDelta;
		m_pFlagIn->m_global.m_obGravity = VerletManager::Get().GetGravity(); 
		m_pFlagIn->m_obGlobalWind = obWind.GetForce(m_pTransform->GetWorldMatrix().GetTranslation());
		m_pFlagIn->m_fDrag = obSoft.m_fDrag;
		m_pFlagIn->m_obLocalTurbFreq = obSoft.m_obLocalTurbulenceFreq;
		m_pFlagIn->m_obLocalTurbVel = obSoft.m_obLocalTurbulenceVel;
		m_pFlagIn->m_fLocalPower = obSoft.m_fLocalPower;
		m_pFlagIn->m_fInvMass = 1.0f / obSoft.m_fMass;
		m_pFlagIn->m_uiNbStep = obSoft.m_iIterations;

		/////////////////////////////////////////
		/////////////////////////////////////////

		// incrementing rotationnal index
		VerletRenderableInstance::VertexBuffer pDynamicMesh = m_pRenderable->GetNextBufferPointer();

		DMABuffer flagIn( m_pFlagIn, m_uiDmaLocation[DYNAMIC_0]-m_uiDmaLocation[FLAGIN] );
		uint32_t uiOneDynamicArraySize = m_pFlagIn->m_nbPoints*sizeof(PointDynamic);
		DMABuffer current( m_timeArray[m_rotationnalIndex[CURRENT]], uiOneDynamicArraySize );
		DMABuffer before( m_timeArray[m_rotationnalIndex[BEFORE]], uiOneDynamicArraySize );
		DMABuffer evenbefore( m_timeArray[m_rotationnalIndex[EVENBEFORE]], uiOneDynamicArraySize );
		DMABuffer flagOut( m_pFlagOut, sizeof(FlagOut) );
		DMABuffer mesh( pDynamicMesh, m_pFlagIn->m_nbMeshElements *sizeof(FlagBinding::VertexDynamic) );

		uint32_t uiCurrent = 0;
		SPUTask spu_task( m_pElf );
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, flagIn ), uiCurrent++ );
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_OutputOnly, current ), uiCurrent++ );
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, before ), uiCurrent++ );
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, evenbefore ), uiCurrent++ );
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_OutputOnly, flagOut ), uiCurrent++ );
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_OutputOnly, mesh ), uiCurrent++ );

	
		//CGatso::Start( "SPU flag (and DMA)" );
		Exec::RunTask( &spu_task );

//		spu_task.StallForJobToFinish();


//		Exec::FrameEnd();
//		Exec::FrameReset();
		//CGatso::Stop( "SPU flag (and DMA)" );

		m_rotationnalIndex++;
	}


	///////////////////////////
	void VerletInstance::CopyStatic(const VerletInstanceDef& def)
	{
		using namespace FlagBinding;
		
		// mass
		PointStatic* pParam = m_pFlagIn->m_pGridStatic.Get();
		for(uint16_t usFlatIndex = 0 ; usFlatIndex < m_pFlagIn->m_nbPoints ; ++usFlatIndex)
		{
			pParam[usFlatIndex].m_fInvMass = def.m_particles[usFlatIndex].m_fInvMass;
		}

		// constraint
		Constraint* pConstraints = m_pFlagIn->m_pConstraints.Get();
		for(uint32_t uiConstraint = 0 ; uiConstraint < m_pFlagIn->m_nbConstraints ; ++uiConstraint )
		{
			pConstraints[uiConstraint].m_index_1=def.m_constraints[uiConstraint][0];
			pConstraints[uiConstraint].m_index_2=def.m_constraints[uiConstraint][1];
			CPoint a=CPoint(m_pDefaultPosition[pConstraints[uiConstraint].m_index_1]);
			CPoint b=CPoint(m_pDefaultPosition[pConstraints[uiConstraint].m_index_2]);
			pConstraints[uiConstraint].m_fRestLength=(b-a).Length();
		}

		// tangeant plabe indices
		TangeantPlaneIndices* pTangeantPlaneIndices=m_pFlagIn->m_pTangeantPlaneIndices.Get();
		for(uint16_t usFlatIndex = 0 ; usFlatIndex < m_pFlagIn->m_nbPoints ; ++usFlatIndex)
		{
			TangeantPlaneIndices& tmp = pTangeantPlaneIndices[usFlatIndex];
			ntAssert((def.m_tangeantPlaneIndices[usFlatIndex][0]>=SCHAR_MIN) && (def.m_tangeantPlaneIndices[usFlatIndex][0]<=SCHAR_MAX));
			ntAssert((def.m_tangeantPlaneIndices[usFlatIndex][1]>=SCHAR_MIN) && (def.m_tangeantPlaneIndices[usFlatIndex][1]<=SCHAR_MAX));
			tmp.m_iBinormal = def.m_tangeantPlaneIndices[usFlatIndex][0];
			tmp.m_iTangeant =  def.m_tangeantPlaneIndices[usFlatIndex][1];
		}

		// external constraint
		Constraint* pExternalConstraints = m_pFlagIn->m_pExternalConstraints.Get();
		PointStatic* pExternalStatic = m_pFlagIn->m_pExternalStatic.Get();
		for(uint32_t uiConstraint = 0 ; uiConstraint < def.m_staticPoints.size() ; ++uiConstraint )
		{
			// attachment constraint
			pExternalConstraints[uiConstraint].m_index_1=uiConstraint;
			pExternalConstraints[uiConstraint].m_index_2=def.m_staticPoints[uiConstraint];
			pExternalConstraints[uiConstraint].m_fRestLength=0.0f;
			
			// null mass
			pExternalStatic[uiConstraint].m_fInvMass =0.0f;
		}
	}

	/////////////////////////////
	//void VerletInstance::CopyMusToSPU(const CPoint* pBefore, const CPoint* pEvenbefore)
	//{
	//	using namespace FlagBinding;
	//	PointDynamic* pBeforeSPU = m_timeArray[m_rotationnalIndex[BEFORE]];
	//	PointDynamic* pEvenBeforeSPU = m_timeArray[m_rotationnalIndex[EVENBEFORE]];
	//
	//	for(uint16_t usFlatIndex = 0 ; usFlatIndex < m_pFlagIn->m_nbPoints ; ++usFlatIndex)
	//	{
	//		pBeforeSPU[usFlatIndex].m_position=CVector(pBefore[usFlatIndex]);
	//		pEvenBeforeSPU[usFlatIndex].m_position=CVector(pEvenbefore[usFlatIndex]);
	//	}
	//}
	//
	/////////////////////////////
	//void VerletInstance::CopySPUToMus(CPoint* pCurrent)
	//{
	//	using namespace FlagBinding;
	//	PointDynamic* pCurrentSPU = m_timeArray[m_rotationnalIndex[CURRENT]];
	//
	//	for(uint16_t usFlatIndex = 0 ; usFlatIndex < m_pFlagIn->m_nbPoints ; ++usFlatIndex)
	//	{
	//		pCurrent[usFlatIndex]=CPoint(pCurrentSPU[usFlatIndex].m_position);
	//	}	
	//}


	void VerletInstance::DebugRender()
	{
#ifndef _GOLD_MASTER
		// if visible, just ignore the debug rendering
		if(!m_flags[ISVISIBLE_FOR_GAMEUPDATE])
		{
			return;
		}

		using namespace FlagBinding;
		PointDynamic* pCurrent = m_timeArray[m_rotationnalIndex[CURRENT]];
		float fPointSize = 0.02f;
		
		// points
		if(true) //MBREMOVE
		{
			for(uint16_t usFlatIndex = 0 ; usFlatIndex < m_pFlagIn->m_nbPoints ; ++usFlatIndex)
			{
				CMatrix obTemp;
				obTemp.SetIdentity();
				obTemp[0][0]=fPointSize;
				obTemp[1][1]=fPointSize;
				obTemp[2][2]=fPointSize;
				obTemp.SetTranslation(CPoint(pCurrent[usFlatIndex].m_position));
				g_VisualDebug->RenderCube(obTemp, DC_PURPLE );
			}
		}
		
		// constraint
		if(false)
		{
			Constraint* pConstraints = m_pFlagIn->m_pConstraints.Get();
			for(uint32_t uiConstraint = 0 ; uiConstraint < m_pFlagIn->m_nbConstraints ; ++uiConstraint )
			{
				CPoint m_constraint[2];
				m_constraint[0]=CPoint(pCurrent[pConstraints[uiConstraint].m_index_1].m_position);
				m_constraint[1]=CPoint(pCurrent[pConstraints[uiConstraint].m_index_2].m_position);
				g_VisualDebug->RenderLine(m_constraint[0], m_constraint[1], DC_BLUE );
			}
		}
#endif
	}


	void VerletInstance::AttachToEntity(CEntity* pobParent, CKeyString& obTransform)
	{
		Transform* pobParentTransform;

		if (obTransform == "WORLD")
		{
			pobParentTransform = pobParent->GetHierarchy()->GetRootTransform();
		}
		else
		{
			pobParentTransform = pobParent->GetHierarchy()->GetTransformFromHash(obTransform.GetHash());
		}
		AttachToTransform( pobParentTransform );
	}

	void VerletInstance::AttachToTransform( Transform* pobParentTransform )
	{
		pobParentTransform->AddChild(m_pTransform);
	}



}

#else
#include "physics/verletManager_pc.h"

	Physics::VerletInstance::VerletInstance(Transform*, const VerletInstanceDef&)
	{
	}

	Physics::VerletInstance::~VerletInstance()
	{
	}

	void Physics::VerletInstance::Update(float)
	{
	}

	void Physics::VerletInstance::AttachToEntity(CEntity*, CKeyString&)
	{
	}

	void Physics::VerletInstance::AttachToTransform( Transform* )
	{
	}

	void Physics::VerletInstance::DetachFromParent()
	{
	}

#endif









