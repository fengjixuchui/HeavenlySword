/***************************************************************************************************
*
*	DESCRIPTION		Core Entity System Implementation
*
*	NOTES
*
***************************************************************************************************/

#include "game/entitymanager.h"
#include "game/entitybrowser.h"
#include "game/entityinfo.h"
#include "game/attacks.h"
#include "game/movement.h"
#include "game/renderablecomponent.h"
#include "game/interactioncomponent.h"
#include "game/messagehandler.h"
#include "game/aicomponent.h"
#include "game/luaattrtable.h"
#include "game/awareness.h"
#include "game/fsm.h"
#include "game/entityanimcontainer.h"

#include "camera/camman_public.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camutils.h"

#include "ai/aitask.h"
#include "ai/aistates.h"
#include "ai/aibehaviourpool.h"
#include "ai/aistatemachine.h"
#include "anim/animator.h"
#include "anim/hierarchy.h"
#include "core/visualdebugger.h"
#include "gfx/meshinstance.h"
#include "input/mouse.h"
#include "objectdatabase/dataobject.h"
#include "core/exportstruct_anim.h"

#include "physics/system.h"
#include "physics/world.h"
#include "physics/advancedcharactercontroller.h"
#include "physics/physicsmaterial.h"
#include "physics/physicstools.h"

#include "editable/enums_ai.h"

#include "ai/ainavigationsystem/ainavigsystemman.h" // Render vision cones
#include "ai/aiuseobjectqueueman.h"
#include "game/aimcontroller.h"
#include "game/entityinteractableturretweapon.h"

#include "camera/debugcam.h"

#include <hkcollide\castutil\hkWorldRayCastInput.h>
#include <hkcollide\castutil\hkWorldRayCastOutput.h>

static const float fV_SPACING = 11.0f;

//#define _SHOW_PARENT_ENTITY
//#define _SHOW_ENTITY_AABB



/***************************************************************************************************
*
*	FUNCTION		CEntityBrowser::CEntityBrowser
*
*	DESCRIPTION		
*
***************************************************************************************************/
CEntityBrowser::CEntityBrowser () :
	m_bMoveInterface(false),
	m_fInterfaceX(280.0f),
	m_fInterfaceY(10.0f)
{
	m_pobCurrentEntity=NULL;
	m_pobCurrentTemplateAnim=NULL;
	m_bEnabled=false;
	m_bShowNames = false;
	m_iCurrentAnim=1;
	m_iCurrentEntity=1;

	m_bHelp=false;

	m_iLuaStateDebugMode=0;
	m_bRenderTransforms=false;
	m_bRenderUsePoints=false;
	m_bRenderCollision=false;

	m_bDebugAxis=false;
	m_bGridInit=false;
	m_bGridEnabled=false;
	m_bCircleEnabled=false;
	m_obGridMatrix.SetIdentity();
	m_bMaterialRayCast=false;

	m_bDebugAnimevents=false;
	
	m_fLockonTimer=0.0f;
	m_iLockonMode=0;
    
	m_pobPlayer=NULL;

	m_bDisplayAnimOnly = false;
}


/***************************************************************************************************
*
*	FUNCTION		CEntityBrowser::DebugRender
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntityBrowser::DebugRender ()
{
#ifndef _RELEASE

	if (!m_pobPlayer)
	{
		// Get a pointer to the player
		m_pobPlayer=CEntityManager::Get().GetPlayer();
	}

	UpdateBrowser();

	UpdateAxis();
	RenderLockon();
	UpdateLuaStateDisplay();
	RenderHelp();

#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!  public  ToggleEntityBrowser
//!
//!  @return COMMAND_RESULT
//!
//!  @author GavB @date 27/11/2006
//------------------------------------------------------------------------------------------
COMMAND_RESULT CEntityBrowser::ToggleEntityBrowser()
{
	// Toggle the entity browser. 
	m_bEnabled = !m_bEnabled;

	// Find the best entity in frame
	if( m_bEnabled )
	{
		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
										Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
										Physics::RAGDOLL_BIT				 
										);

		CPoint obCameraPosition		= CamMan_Public::GetCurrMatrix().GetTranslation();
		CPoint obEndPosition		= obCameraPosition + (CamMan_Public::GetCurrMatrix().GetZAxis() * 30.0f );

		Physics::TRACE_LINE_QUERY stQuery;
		
		// Perform a raycast that ignores static entities
		Physics::CPhysicsWorld::Get().TraceLine(obCameraPosition, obEndPosition, 0, stQuery, obFlag); 

		m_pobCurrentEntity = stQuery.pobEntity;
	}

	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!  global static  FindAttackHelper
//!
//!  @param [in, out]  pobContainer ObjectContainer * 
//!  @param [in]       pcAttackAnimName const CHashedString & 
//!
//!  @return CAttackData * 
//!
//!  @author GavB @date 27/11/2006
//------------------------------------------------------------------------------------------
static CAttackData* FindAttackHelper(ObjectContainer* pobContainer, const CHashedString& pcAttackAnimName)
{
	CAttackData* pobResult = 0;

	// Iterate through the contents of the list
	for (ObjectContainer::ObjectList::iterator obIt = pobContainer->m_ContainedObjects.begin(); obIt != pobContainer->m_ContainedObjects.end(); obIt++ )
	{
		// Check for a CAttackData
		if (0 == strcmp((*obIt)->GetClassName(), "CAttackData"))
		{
			CAttackData* pobAttack = (CAttackData*)((*obIt)->GetBasePtr());
			if (pobAttack->m_obAttackAnimName == pcAttackAnimName)
			{
				return pobAttack;
			}
		}

		// Check for container
		if (0 == strcmp((*obIt)->GetClassName(), "ObjectContainer"))
		{
			// Recurse into the child container
			pobResult = FindAttackHelper((ObjectContainer*)((*obIt)->GetBasePtr()), pcAttackAnimName);
			if (pobResult) return pobResult;
		}

	}
	// Failed to find
	return pobResult;
}



/***************************************************************************************************
*
*	FUNCTION		CEntityBrowser::DebugRender
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntityBrowser::UpdateBrowser ()
{
#ifndef _RELEASE

	CInputKeyboard &obKeyboard=CInputHardware::Get().GetKeyboard();

	// CTRL-N toggles the name viewer
//	if (obKeyboard.IsKeyPressed(KEYC_N,KEYM_CTRL)) 
//	{
//		m_bShowNames=!m_bShowNames;
//	}

	DisplayNames();


	if (MouseInput::Exists())
	{
		if (!m_bMoveInterface)
		{
			CVector obMousePos(MouseInput::Get().GetMousePos());

			obMousePos.X()*=g_VisualDebug->GetDebugDisplayWidth();
			obMousePos.Y()*=g_VisualDebug->GetDebugDisplayHeight();
			
			if (MouseInput::Get().GetButtonState(MOUSE_LEFT).GetPressed() &&
				obMousePos.X()>=m_fInterfaceX &&
				obMousePos.X()<m_fInterfaceX+12.0f &&
				obMousePos.Y()>=m_fInterfaceY &&
				obMousePos.Y()<m_fInterfaceY+12.0f)
			{
				m_bMoveInterface=true;
			}
		}
		else
		{
			CVector obMousePos(MouseInput::Get().GetMousePos());

			obMousePos.X()*=g_VisualDebug->GetDebugDisplayWidth();
			obMousePos.Y()*=g_VisualDebug->GetDebugDisplayHeight();

			m_fInterfaceX=obMousePos.X()-4.0f;
			m_fInterfaceY=obMousePos.Y()-4.0f;

			if (MouseInput::Get().GetButtonState(MOUSE_LEFT).GetReleased())
				m_bMoveInterface=false;
		}
	}

	// ----- Material checker -----
	if (obKeyboard.IsKeyPressed(KEYC_X,KEYM_CTRL))
	{
		m_bMaterialRayCast=!m_bMaterialRayCast;
	}

	if (m_bMaterialRayCast)
	{
		const CMatrix& obCamMatrix=CamMan::Get().GetPrimaryView()->GetCurrMatrix();

		CDirection obDir=CDirection(0.0f,0.0f,1000.0f) * obCamMatrix;
		CPoint obEnd(obCamMatrix.GetTranslation() + obDir);

		hkWorldRayCastInput input;
		hkWorldRayCastOutput output;
		
		input.m_from(0)=obCamMatrix.GetTranslation().X();
		input.m_from(1)=obCamMatrix.GetTranslation().Y();
		input.m_from(2)=obCamMatrix.GetTranslation().Z();

		input.m_to(0)=obEnd.X();
		input.m_to(1)=obEnd.Y();
		input.m_to(2)=obEnd.Z();

		Physics::RaycastCollisionFlag flags; 
		flags.base = 0;
		flags.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		flags.flags.i_collide_with = Physics::LARGE_INTERACTABLE_BIT;

		input.m_filterInfo = flags.base;

		//ntPrintf("input.m_filterInfo=%d\n",input.m_filterInfo);

		g_VisualDebug->RenderLine(obCamMatrix.GetTranslation(),obEnd,0xffffffff);

		Physics::CPhysicsWorld::Get().GetHavokWorldP()->castRay(input,output);

		// Calculate the intersect point

		if (output.hasHit())
		{
			obDir*=output.m_hitFraction;
			CPoint obIntersect(obDir);
			obIntersect+=obCamMatrix.GetTranslation();

			g_VisualDebug->RenderLine( CPoint(obIntersect.X()-1.0f,obIntersect.Y(),obIntersect.Z()), CPoint(obIntersect.X()+1.0f,obIntersect.Y(),obIntersect.Z()), 0xffffffff); // X-axis
			g_VisualDebug->RenderLine( CPoint(obIntersect.X(),obIntersect.Y()-1.0f,obIntersect.Z()), CPoint(obIntersect.X(),obIntersect.Y()+1.0f,obIntersect.Z()), 0xffffffff); // Y-axis
			g_VisualDebug->RenderLine( CPoint(obIntersect.X(),obIntersect.Y(),obIntersect.Z()-1.0f), CPoint(obIntersect.X(),obIntersect.Y(),obIntersect.Z()+1.0f), 0xffffffff); // Z-axis

			Physics::psPhysicsMaterial* pMaterial=Physics::Tools::GetMaterial(output);

			CDirection obDiff(obIntersect - obCamMatrix.GetTranslation());

			float fTY=-38.0f;

			g_VisualDebug->Printf3D(obIntersect,0.0f,fTY,0xffffffff,DTF_ALIGN_HCENTRE,"%.2f %.2f %.2f",obIntersect.X(),obIntersect.Y(),obIntersect.Z()); fTY+=12.0f;
			g_VisualDebug->Printf3D(obIntersect,0.0f,fTY,0xffffffff,DTF_ALIGN_HCENTRE,"Dist: %.2f",obDiff.Length()); fTY+=12.0f;

			if (pMaterial)
			{
				g_VisualDebug->Printf3D(obIntersect,0.0f,fTY,0xffffffff,DTF_ALIGN_HCENTRE,"Material:%s",pMaterial->GetMaterialName());
			}
			else
			{
				g_VisualDebug->Printf3D(obIntersect,0.0f,fTY,0xffffffff,DTF_ALIGN_HCENTRE,"No material assigned to this surface");
			}
		}
	}

	if ((!m_bEnabled) || (CEntityManager::Get().m_entities.empty()))
		return;

	CEntity* pobFirstEntity=CEntityManager::Get().m_entities.front();

	if (!m_pobCurrentEntity)
	{
		if (!CEntityManager::Get().m_entities.empty())
		{
			m_pobCurrentEntity=pobFirstEntity;
			m_pobCurrentTemplateAnim=NULL;
			m_iCurrentAnim=1;
		}
	}
	else
	{
		if (m_pobCurrentEntity->IsToDestroy()) // This entity is flagged for removal, it is about to be deleted!
		{
			m_pobCurrentEntity=pobFirstEntity;
			m_pobCurrentTemplateAnim=NULL;
			m_iCurrentAnim=1;
		}
	}

	if (m_pobCurrentEntity)
	{
		/*
		// Simulate collision message
		if (obKeyboard.IsKeyPressed(KEYC_F11) && m_pobCurrentEntity->GetMessageHandler())
		{
			CMessageSender::SendEmptyMessage("msg_collision", m_pobCurrentEntity->GetMessageHandler());
		}
		*/

		// Look at current entity? Bound to triangle in free cam mode
		if ( CamMan::Get().GetPrimaryView()->GetDebugCameraMode() == CamView::DM_FREE && CInputHardware::Get().GetPad(CamMan::Get().GetDebugCameraPadNumber()).GetHeld() & PAD_FACE_1 )
		{
			CPoint obCamPos = CamMan::Get().GetPrimaryView()->GetDebugControllerMatrix().GetTranslation();
			CamMan::Get().GetPrimaryView()->SetDebugCameraPlacement( obCamPos, m_pobCurrentEntity->GetPosition() );
		}

		// Find nearest entity to camera? Bound to square in free cam mode
		if ( CamMan::Get().GetPrimaryView()->GetDebugCameraMode() == CamView::DM_FREE && CInputHardware::Get().GetPad(CamMan::Get().GetDebugCameraPadNumber()).GetHeld() & PAD_FACE_4 )
		{
			CPoint obCamPos = CamMan::Get().GetPrimaryView()->GetDebugControllerMatrix().GetTranslation();

			CEntity* pobClosest = 0;
			float fClosest = 9999999.0f;
			QuickPtrList<CEntity>::iterator obIt = CEntityManager::Get().m_entities.begin();
			while (obIt != CEntityManager::Get().m_entities.end())
			{
				CDirection obTo((*obIt)->GetPosition() - obCamPos);
				float fLengthSquared = obTo.LengthSquared();
				if (fLengthSquared < fClosest)
				{	
					fClosest = fLengthSquared;
					pobClosest = (*obIt);
				}

				++obIt;
			}
	
			if (pobClosest)
				m_pobCurrentEntity = pobClosest;
		}		

		// Pick an entity?
		if ( !m_bMoveInterface && ( MouseInput::Exists() ) && ( MouseInput::Get().GetButtonState( MOUSE_LEFT ).GetHeld() ) )
		{
			if (m_pobCurrentEntity->GetAnimator())
			{
				m_pobCurrentEntity->GetAnimator()->GetAnimEventHandler().SetDebugMode(false);	
			}

			// Find a ray that the mouse position is describing
			CPoint obCameraPosition( CONSTRUCT_CLEAR );
			CDirection obWorldRay( CONSTRUCT_CLEAR );
			MouseInput::Get().GetWorldRayFromMousePos( obWorldRay, obCameraPosition );

			// [Mus] - What settings for this cast ?
			Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
			obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
			obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
											Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
											Physics::RAGDOLL_BIT						|
											Physics::SMALL_INTERACTABLE_BIT				|
											Physics::LARGE_INTERACTABLE_BIT				);

			CPoint obEndPosition(obCameraPosition);
			obEndPosition+=obWorldRay * 100.0f;

			ntstd::List<CEntity*> obIgnoreList;//=(ntstd::List<CEntity*>)CEntityManager::Get().m_obStaticEntities;

			for(ntstd::List<Static*>::iterator obStaticIt=CEntityManager::Get().GetStaticEntityList().begin(); obStaticIt!=CEntityManager::Get().GetStaticEntityList().end(); ++obStaticIt)
			{
				obIgnoreList.push_back((CEntity*)*obStaticIt);
			}

			Physics::TRACE_LINE_QUERY stQuery;
			
			Physics::CPhysicsWorld::Get().TraceLine(obCameraPosition,obEndPosition,obIgnoreList,stQuery,obFlag); // Perform a raycast that ignores static entities

			const CEntity* pobClickedOnEntity=stQuery.pobEntity;
			


			// I'm going to have to cast away constness here for the debug code
			if ( pobClickedOnEntity )
			{
				u_int iCurrentEntity=1; 

				// Determine the entity index
				for(QuickPtrList<CEntity>::iterator obIt=CEntityManager::Get().m_entities.begin(); obIt!=CEntityManager::Get().m_entities.end(); ++obIt)
				{
					if ((*obIt)==pobClickedOnEntity)
					{
						break;
					}

					++iCurrentEntity;
				}

				if (iCurrentEntity<=CEntityManager::Get().m_entities.size())
				{
					m_pobCurrentEntity = const_cast<CEntity*>( pobClickedOnEntity );
					m_pobCurrentTemplateAnim = NULL;
					m_iCurrentAnim=1;
					m_iCurrentEntity=iCurrentEntity;
				}
			}

			if (m_pobCurrentEntity->GetAnimator())
			{
				m_pobCurrentEntity->GetAnimator()->GetAnimEventHandler().SetDebugMode(m_bDebugAnimevents);	
			}
		}

		// ----- Cycle entities -----
		else if (obKeyboard.IsKeyPressed(KEYC_LEFT_ARROW) || obKeyboard.IsKeyHeld(KEYC_LEFT_ARROW,KEYM_SHIFT))
		{

		}
		else if (obKeyboard.IsKeyPressed(KEYC_RIGHT_ARROW) || obKeyboard.IsKeyHeld(KEYC_RIGHT_ARROW,KEYM_SHIFT))
		{
		}

		/*
		if	( obKeyboard.IsKeyPressed( KEYC_PERIOD, KEYM_CTRL ) )
		{
			CMessage* pobMsg = CMessage::Create(MESSAGE_TYPE_PLAYER,
												MESSAGE_PLAYER_KO,
												CMessageDataPtr(0),
												m_pobCurrentEntity->GetEntityInfo(),
												false );
			m_pobCurrentEntity->GetMessageStore()->ReceiveMessage( pobMsg );
		}
		*/


		/*
		// Camera focus on entity
		if (obKeyboard.IsKeyPressed(KEYC_F))
		{
			if (m_pobCurrentEntity)
				CamMan::Get().GetElementManager().DebugSwapLookat(m_pobCurrentEntity->GetName().c_str());
		}

		// Restore camera focus to its default
		if (obKeyboard.IsKeyPressed(KEYC_R))
		{
			if (m_pobCurrentEntity)
				CamMan::Get().GetElementManager().DebugSwapLookat();
		}

		// Enable the movement controller
		if (obKeyboard.IsKeyPressed(KEYC_Z))
		{
			if (m_pobCurrentEntity->GetMovement())
				m_pobCurrentEntity->GetMovement()->SetEnabled( true );
		}

		// Disable the movement controller
		if (obKeyboard.IsKeyPressed(KEYC_X))
		{
			if (m_pobCurrentEntity->GetMovement())
				m_pobCurrentEntity->GetMovement()->SetEnabled( false );
		}
		*/

		// Debug rendering of transforms
//		if (obKeyboard.IsKeyPressed(KEYC_T))
//		{
//			m_bRenderTransforms=!m_bRenderTransforms;
//		}
	}

	// Do not render the entity browser if capturing movie
	{
		const u_long ulENTITY_COLOUR	= 0xff99ff99;
		const u_long ulINFO_COLOUR		= 0xffeeffee;

#ifdef JUST_COLLISION_RENDER
		if (m_pobCurrentEntity && m_pobCurrentEntity->GetPhysicsSystem() )
		{
			// Debug collision rendering
			m_pobCurrentEntity->GetPhysicsSystem()->Debug_RenderAllCollisionInfo();
			
			g_VisualDebug->Printf2D(m_fInterfaceX,m_fInterfaceY, ulENTITY_COLOUR, 0, "Entity:");
			g_VisualDebug->Printf2D(m_fInterfaceX+72.0f,m_fInterfaceY, ulINFO_COLOUR, 0, "%d/%d %s",
				m_iCurrentEntity,
				CEntityManager::Get().m_entities.size(),
				m_pobCurrentEntity->GetName().c_str());
			return;
		}
#endif
		// Debug rendering stuff
		const u_long ulTITLE_COLOUR		= 0xff99ff99;
		//const u_long ulHELP_COLOUR		= 0xff77ff77;
		const u_long ulANIMATION_COLOUR = 0xff99ff99;
		const u_long ulSTATE_COLOUR		= 0xff77ffff;
		const u_long ulMOVEMENT_COLOUR	= 0xffff66ff;
		const u_long ulNAVIGTITLE_COLOUR= 0xffff0000; // Dario

		float fX=m_fInterfaceX;
		float fY=m_fInterfaceY;

		char acString [1024];

		g_VisualDebug->Printf2D(fX,fY, ulTITLE_COLOUR, 0, "ENTITY DEBUGGER [Press I for help]");
		fY+=fV_SPACING;
		/*
		g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "L/R ARROW cycles entity  U/D cycles anim");
		fY+=fV_SPACING;
		g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "F focus camera  R restore camera focus");
		fY+=fV_SPACING;
		g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "A play currently selected anim");
		fY+=fV_SPACING;
		g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "Z/X enable/disable movmentment controller");
		fY+=fV_SPACING;
		*/

		if (m_pobCurrentEntity)
		{
			// Entity name
			g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Entity:");
			g_VisualDebug->Printf2D(fX+72.0f,fY, ulINFO_COLOUR, 0, "%d/%d %s",
				m_iCurrentEntity,
				CEntityManager::Get().m_entities.size(),
				m_pobCurrentEntity->GetName().c_str());
			fY+=fV_SPACING;

			// Show position/rotation
			if (!m_bDisplayAnimOnly)
			{
				if (m_pobCurrentEntity->GetHierarchy())
				{
					#ifdef _SHOW_PARENT_ENTITY
					if (m_pobCurrentEntity->GetParentEntity())
					{
						g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "ParentEntity: %s",m_pobCurrentEntity->GetParentEntity()->GetName().c_str());
						fY+=fV_SPACING;
					}
					#endif // _SHOW_PARENT_ENTITY

					const CMatrix& obWorldMatrix=m_pobCurrentEntity->GetMatrix();

					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Position:");
					g_VisualDebug->Printf2D(fX+88.0f,fY, ulINFO_COLOUR, 0, "%.3f %.3f %.3f",
						obWorldMatrix.GetTranslation().X(),
						obWorldMatrix.GetTranslation().Y(),
						obWorldMatrix.GetTranslation().Z());

					fY+=fV_SPACING;

					CQuat obRotation(obWorldMatrix);

					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Rotation:");
					g_VisualDebug->Printf2D(fX+88.0f,fY, ulINFO_COLOUR, 0, "%.3f %.3f %.3f %.3f",
						obRotation.X(),obRotation.Y(),obRotation.Z(),obRotation.W());

					fY+=fV_SPACING;

					/*
					if (m_pobCurrentEntity->GetParentEntity())
					{
						Transform* pobRootTransform=m_pobCurrentEntity->GetHierarchy()->GetRootTransform();
						
						
						CMatrix obLocalMatrix=pobRootTransform->GetWorldMatrix() * m_pobCurrentEntity->GetParentEntity()->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetAffineInverse();

						CQuat obLocalRotation(obLocalMatrix);

						g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Local Rotation:");
						g_VisualDebug->Printf2D(fX+136.0f,fY, ulINFO_COLOUR, 0, "%.3f %.3f %.3f %.3f",obLocalRotation.X(),obLocalRotation.Y(),obLocalRotation.Z(),obLocalRotation.W());

						fY+=fV_SPACING;
					}
					//*/

					/*
					// Velocity
					if (m_pobCurrentEntity->GetEntityInfo())
					{
						sprintf(acString,"Velocity: %.3f  %.3f  %.3f",
							m_pobCurrentEntity->GetEntityInfo()->GetCalcVelocity().X(),
							m_pobCurrentEntity->GetEntityInfo()->GetCalcVelocity().Y(),
							m_pobCurrentEntity->GetEntityInfo()->GetCalcVelocity().Z());

						g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, acString);
						fY+=fV_SPACING;
					}
					*/

					if (m_pobCurrentEntity->GetPhysicsSystem())
					{
						Physics::AdvancedCharacterController* ctrl = (Physics::AdvancedCharacterController*) m_pobCurrentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );

						if (ctrl)
						{
							CDirection obLinearVelocity(ctrl->GetLinearVelocity());
							sprintf(acString,"Velocity: %.3f  %.3f  %.3f  Speed: %0.3f",obLinearVelocity.X(),obLinearVelocity.Y(),obLinearVelocity.Z(),obLinearVelocity.Length());
							g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, acString);
							fY+=fV_SPACING;
						}
					}
	

					// Display an axis at the root of the entity

					const CPoint& obWorldTranslation(obWorldMatrix.GetTranslation());

					CDirection obXOffset = CVecMath::GetXAxis() * obWorldMatrix;
					CDirection obYOffset = CVecMath::GetYAxis() * obWorldMatrix;
					CDirection obZOffset = CVecMath::GetZAxis() * obWorldMatrix;

					CPoint obXEnd(	obWorldTranslation.X()+obXOffset.X(),
									obWorldTranslation.Y()+obXOffset.Y(),
									obWorldTranslation.Z()+obXOffset.Z());

					CPoint obYEnd(	obWorldTranslation.X()+obYOffset.X(),
									obWorldTranslation.Y()+obYOffset.Y(),
									obWorldTranslation.Z()+obYOffset.Z());

					CPoint obZEnd(	obWorldTranslation.X()+obZOffset.X(),
									obWorldTranslation.Y()+obZOffset.Y(),
									obWorldTranslation.Z()+obZOffset.Z());

					g_VisualDebug->RenderLine(obWorldTranslation,obXEnd,0xffff0000);
					g_VisualDebug->RenderLine(obWorldTranslation,obYEnd,0xff00ff00);
					g_VisualDebug->RenderLine(obWorldTranslation,obZEnd,0xff0000ff);

					// Render use-points if required
					if (m_bRenderUsePoints)
					{
						m_pobCurrentEntity->GetInteractionComponent()->DebugRenderUsePoints();
					}

					// Render other transforms if applicable
					if (m_bRenderTransforms && m_pobCurrentEntity->GetHierarchy()->GetTransformCount()>1)
					{
						for(int iIndex=1; iIndex<m_pobCurrentEntity->GetHierarchy()->GetTransformCount(); ++iIndex)
						{
							Transform* pobTransform=m_pobCurrentEntity->GetHierarchy()->GetTransform(iIndex);
							const CMatrix& obMatrix(pobTransform->GetWorldMatrix());

							// Do an inverse look up to find the bone index on characters
							int iBone = CHARACTER_BONE_INVALID;
							if ( m_pobCurrentEntity->GetHierarchy()->GetCharacterBoneToIndexArray() )
							{
								for ( int i = 0; i < NUM_CHARACTER_BONES; ++i )
								{
									if ( m_pobCurrentEntity->GetHierarchy()->GetCharacterBoneTransformIndex( static_cast<CHARACTER_BONE_ID>( i ) ) == iIndex )
										iBone = i;
								}
							}

							const CPoint& obCentre(obMatrix.GetTranslation());

							CDirection obXOffset = CVecMath::GetXAxis() * obMatrix;
							CDirection obYOffset = CVecMath::GetYAxis() * obMatrix;
							CDirection obZOffset = CVecMath::GetZAxis() * obMatrix;

							obXOffset*=0.2f;
							obYOffset*=0.2f;
							obZOffset*=0.2f;
							
							CPoint obXEnd(	obCentre.X()+obXOffset.X(),
											obCentre.Y()+obXOffset.Y(),
											obCentre.Z()+obXOffset.Z());

							CPoint obYEnd(	obCentre.X()+obYOffset.X(),
											obCentre.Y()+obYOffset.Y(),
											obCentre.Z()+obYOffset.Z());

							CPoint obZEnd(	obCentre.X()+obZOffset.X(),
											obCentre.Y()+obZOffset.Y(),
											obCentre.Z()+obZOffset.Z());

							g_VisualDebug->RenderLine(obCentre,obXEnd,0xffff0000);
							g_VisualDebug->RenderLine(obCentre,obYEnd,0xff00ff00);
							g_VisualDebug->RenderLine(obCentre,obZEnd,0xff0000ff);
							if ( iBone != CHARACTER_BONE_INVALID )
								g_VisualDebug->Printf3D( obCentre, 0xffffffff,0,"%d (%d)",iIndex, iBone );
							else
							g_VisualDebug->Printf3D( obCentre, 0xffffffff,0,"%d",iIndex );
						}
					}
				}
				else
				{
					sprintf(acString,"No hierarchy");
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, acString);
					fY+=fV_SPACING;
				}

				// Debug collision rendering
				if (m_bRenderCollision)
				{
					if (m_pobCurrentEntity && m_pobCurrentEntity->GetPhysicsSystem())
					{
						m_pobCurrentEntity->GetPhysicsSystem()->Debug_RenderAllCollisionInfo();
					}
				}
	
				// Entity Health
				if( m_pobCurrentEntity->IsCharacter() )
				{
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Health:" );
					g_VisualDebug->Printf2D(fX+72.0f,fY, ulINFO_COLOUR, 0, "%.2f", m_pobCurrentEntity->ToCharacter()->GetCurrHealth() );
					if(m_pobCurrentEntity->ToCharacter()->IsDeadMessageSent())
					{
						g_VisualDebug->Printf2D(fX+120.0f,fY, DC_RED, 0, "*Killed*");
					}

					fY+=fV_SPACING;
				}

				// Show combat state - if applicable
				extern char* g_apcCombatStateTable[];
				if( m_pobCurrentEntity->GetAttackComponent() )
				{
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "CombatState: %s", g_apcCombatStateTable[(int)m_pobCurrentEntity->GetAttackComponent()->AI_Access_GetState()] );
					fY+=fV_SPACING;
				}
				
				// Show ragdoll/character astate, if applicable
				extern char* g_apcRagdollStateTable[];
				extern char* g_apcCharacterControllerTypeTable[];
				if( m_pobCurrentEntity->GetPhysicsSystem() )
				{
					Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*)m_pobCurrentEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
					if ( pobCharacterState && pobCharacterState->IsRagdollActive() )
					{
						g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "RagdollState: %s", g_apcRagdollStateTable[(int)pobCharacterState->GetRagdollState()] );
						fY+=fV_SPACING;
					}
					else
					{
						g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "RagdollState: No ragdoll.");
						fY+=fV_SPACING;
					}
				
					if ( pobCharacterState && pobCharacterState->IsCharacterControllerActive() )
					{
						char pcGrav;
						if (pobCharacterState->GetApplyCharacterControllerGravity())
							pcGrav = 'Y';
						else
							pcGrav = 'N';
						char pcAbs;
						if (pobCharacterState->GetDoCharacterControllerMovementAbsolutely())
							pcAbs = 'Y';
						else
							pcAbs = 'N';
						g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "CharacterType: %s, Gravity: %c, Absolute: %c", g_apcCharacterControllerTypeTable[(int)pobCharacterState->GetCharacterControllerType()], pcGrav, pcAbs );
						fY+=fV_SPACING;
					}
					else
					{
						g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "CharacterType: No character.");
						fY+=fV_SPACING;
					}
				}

				// Show AI state - if applicable
				if( m_pobCurrentEntity->GetFSM() )
				{
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "FSM State: %s", m_pobCurrentEntity->GetFSM()->GetStateName() );
					fY+=fV_SPACING;

					m_pobCurrentEntity->GetFSM()->DebugRender(fX, fY);
				}

				// Show AI state - if applicable
				if( m_pobCurrentEntity->IsAI() )
				{
					CAIStateMachine* pCurrentStateMachine = AIBehaviourPool::Get().GetCurrentBehaviour( m_pobCurrentEntity );

					if( pCurrentStateMachine )
					{
						g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Brain State: %s-%s:%s", ((AI*)m_pobCurrentEntity)->GetAIComponent()->GetCurrentState(),
																									pCurrentStateMachine->GetBehaviourName().c_str(),
																									pCurrentStateMachine->GetBehaviourState().c_str() );
					}
					else
					{
						g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Brain State: %s", ((AI*)m_pobCurrentEntity)->GetAIComponent()->GetCurrentState() );
					}
					
					fY+=fV_SPACING;

					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "AI Combat Move State: %s", ((AI*)m_pobCurrentEntity)->GetAIComponent()->GetCombatComponent().GetMovementState() );
					fY+=fV_SPACING;
				}

				// Sector info
				sprintf( acString, ("Within Areas: ") );
				for ( int32_t i = 1; i <= AreaSystem::NUM_AREAS; i++ )
				{
					if ( m_pobCurrentEntity->m_areaInfo.Within(i)  )
					{
						char acTemp[32];
						sprintf( acTemp, "%d ", i );
						strcat( acString, acTemp );
					}
				}

				if ( m_pobCurrentEntity->GetRenderableComponent() && !m_pobCurrentEntity->GetRenderableComponent()->DisabledByArea() )
					strcat( acString, "VISIBLE " );

				if(!m_pobCurrentEntity->IsPaused())
					strcat(acString, "ACTIVE");
				else
					strcat(acString, "PAUSED");

				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, acString);
				fY+=fV_SPACING;

				// If this entity has a message component then we will print out the history of those states
				if ( m_pobCurrentEntity->GetMessageHandler() )
				{
					// Loop through the state history and print out the information	
					int iNumStateHistory = m_pobCurrentEntity->GetMessageHandler()->GetSizeOfDebugStateHistory();
					for ( int iState = 0; iState < iNumStateHistory; ++iState )
					{
						if (m_pobCurrentEntity->GetMessageHandler()->DebugPrintStateAndMessageData( iState, fX, fY, ulSTATE_COLOUR ))
						{
							fY += fV_SPACING;																		
						}
					}
				}
			}
/*
			if( m_pobCurrentEntity->GetAIComponent() )
			{
				LuaObject obDebugState = CLuaGlobal::Get().GetState().GetGlobal("AI_Debug_GetState");

				if( obDebugState.IsFunction() )
				{
					LuaFunction<const char*> obFn = obDebugState;

					if (!m_pobCurrentEntity->GetAIComponent()->GetLuaState().IsNil())
					{
						const char* pcString = obFn( m_pobCurrentEntity->GetAIComponent()->GetLuaState() );
						char acStringBuf[256];
						sprintf( acStringBuf, "Entity State: %s", pcString );
						g_VisualDebug->Printf2D(fX, fY, ulINFO_COLOUR, 0, acStringBuf );
						fY += fV_SPACING;
					}
				}
			}
*/

			// HC: I've removed this nasty bit of code because it doesn't work for all entities.
			// The problem is centred around the controller debug details which takes a pointer to a movement controller definition.
			// This is very bad because not all movement controller definitions are serialised, some are created on a stack and then destroyed
			// meaning the pointer is no longer valid and causing this debugging stuff to crash the game.
			// If we want debugging for movement controller stuff, this stuff will need to be refactored.

			// If this entity has a movement component then print out the details of the current controllers
			if ( m_pobCurrentEntity->GetMovement() && (m_pobCurrentEntity->IsPlayer() || m_pobCurrentEntity->IsEnemy() || m_pobCurrentEntity->IsBoss())  ) // This will only work on players and enemies
			{
				// Loop through the controllers and get the details
				int iNumberOfControllers = 3;
				for ( int iController = 0; iController < iNumberOfControllers; ++iController )
				{
					// Set up the data
					const char*	pcInstanceName = 0;
					const char*	pcTypeName = 0;
					float		fWeight = 0.0f;
			
					// Get the details from the controller
					m_pobCurrentEntity->GetMovement()->GetControllerDebugDetails(	iController, 
																					pcInstanceName,
																					pcTypeName, 
																					fWeight );

					//It's useful to see unknown names too just so we know that they're there!!! - JML
					//if ( ( pcInstanceName ) && ( pcTypeName ) )
					{
						// If we have anything useful write our some info
						if ( ( pcInstanceName ) || ( pcTypeName ) || ( fWeight > 0.0f ) )
							g_VisualDebug->Printf2D(fX, fY, ulMOVEMENT_COLOUR, 0, "%s(%s),%.3f% \n",
													(pcTypeName ? pcTypeName : "Type Unknown"),
													(pcInstanceName ? pcInstanceName : "Name Unknown"),
													fWeight );

						// Move down a line whether there was useful information or not
						fY += fV_SPACING;																		
					}
				}
			}

			// (Dario)
			
			#define IS_ACTIVE_NAVIG_FLAG(x) (x & uiFlags)
			#define SHOW_NAVIG_FLAG_STATUS(x) ((x & uiFlags)? "On" : "Off")
			#define SHOW_YES_NO(x) (x==true ? "Yes" : "No")
			#define SHOW_ON_OFF(x) (x==true ? "ON" : "OFF")
			
			CAIComponent* pCAIComp = ((AI*)m_pobCurrentEntity)->GetAIComponent(); 
			
			if( m_pobCurrentEntity->IsAI() && pCAIComp &&
				pCAIComp->GetCAIMovement() )
			{

				CAIVision*   pVis = pCAIComp->GetAIVision();
				CAIMovement* pMov = pCAIComp->GetCAIMovement();
				unsigned int uiFlags = pMov->GetSteeringFlags();

				Player* pPlayer = CEntityManager::Get().GetPlayer();
				if ( pPlayer && pPlayer->IsArcher())
				{
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Is Valid Cover Time?: [%s] - [%.3f/%.3f]", 
												SHOW_YES_NO(CAINavigationSystemMan::Get().IsValidCoverTime()),
												CAINavigationSystemMan::Get().GetTimeFromLastValidBolt(),
												CAINavigationSystemMan::Get().GetBoltCoverTimer()
												);
					fY+=fV_SPACING;
				}
				
				g_VisualDebug->Printf2D(fX,fY, ulNAVIGTITLE_COLOUR, 0, "-- Navigation Information --");
				fY+=fV_SPACING;

				if (pCAIComp->GetDisabled() == true )
				{
					g_VisualDebug->Printf2D(fX,fY, DC_RED, 0, "ENTITY HAS AI DISABLED!!!!!!!!!!!!");
					fY+=fV_SPACING;
				}
				if (m_pobCurrentEntity->ToCharacter()->IsDead() == true )
				{
					g_VisualDebug->Printf2D(fX,fY, DC_RED, 0, "ENTITY IS DEAD!!!!!!!!!!!!");
					fY+=fV_SPACING;
				}
				if (pCAIComp && pCAIComp->IsRecovering() )
				{
					g_VisualDebug->Printf2D(fX,fY, DC_GREEN, 0, "ENTITY IS RECOVERING!!");
					fY+=fV_SPACING;
				}

				// AI Intentions
				char sAIIName[128];
				char sNodeName[128];
				char sStartNodeName[128];
				char sGEName[128];
				char sAEName[128];
				char sFEName[128];
				char sCannonTarget[128];

				const CEntity* pGE, * pAE, * pFE, *pCannonTgt;

				SAIIntentions* pAII = pMov->GetIntention();

				pGE = pMov->GetEntityToGoTo(); if (pGE) strcpy(sGEName,pGE->GetName().c_str()); else strcpy(sGEName,"NONE");
				pAE = (CEntity*)pMov->GetEntityToAttack(); if (pAE) strcpy(sAEName,pAE->GetName().c_str()); else strcpy(sAEName,"NONE");
				pFE = pMov->GetEntityToFollow(); if (pFE) strcpy(sFEName,pFE->GetName().c_str()); else strcpy(sFEName,"NONE");
				pCannonTgt = pMov->GetCannonTarget(); if (pCannonTgt) strcpy(sCannonTarget,ntStr::GetString(pCannonTgt->GetName())); else strcpy(sCannonTarget,"NONE");
				strcpy(sNodeName,ntStr::GetString(pAII->ksDestNodeName));
				strcpy(sStartNodeName,ntStr::GetString(pAII->ksStartNodeName));

				if (!strcmp(sStartNodeName,"NULL")) strcpy(sStartNodeName,"AI Position");

				// Gathering Vision data

				char sVision[64];
				if (pVis)
				{
					if (!pVis->IsActive())
						strcpy(sVision,"INACTIVE");
					else
					{
						if (pVis->DoISeeEnemyThroughGoAroundVolumes())
							strcpy(sVision,"Enemy Seen Through Go Around Volumes");
						else if (pVis->DoISeeNothing())
							strcpy(sVision,"I DO NOT see anything");
						else if (pVis->IsTargetInShootingRange())
							strcpy(sVision,"I see my Target in Shooting Range");
						else if (pVis->DoISeeTheEnemy())
							strcpy(sVision,"I see the Enemy, my Target");
						else if (pVis->DoISeeSomething())
							strcpy(sVision,"I see Something...");
						else
							strcpy(sVision,"UNKNOWN VISUAL STATUS");
					}
				}

				switch (pAII->eNavIntentions) { 
					case NAVINT_NONE					: strcpy(sAIIName,"NONE ["); break;
					case NAVINT_USE_CANNON				: strcpy(sAIIName,"USE CANNON against [");
														  strcat(sAIIName,sCannonTarget); break;
					case NAVINT_GO_AROUND_VOLUMES		: strcpy(sAIIName,"GO_AROUND_VOLUME [");
														  strcat(sAIIName,sAEName); break;
					case NAVINT_WHACKAMOLE				: strcpy(sAIIName,"WHACKAMOLE/TIME-CRISIS Shooting to [");
														  strcat(sAIIName,sAEName); break;
					case NAVINT_FOLLOWPATHTONODE		: strcpy(sAIIName,"Use waypoints to go from ["); strcat(sAIIName,sStartNodeName);
														  strcat(sAIIName,"] to node ["); strcat(sAIIName,sNodeName); break;
					case NAVINT_FOLLOWPATHCOVER_TO_NODE	: strcpy(sAIIName,"Use WP and cover to go from ["); strcat(sAIIName,sStartNodeName);
														  strcat(sAIIName,"] to ["); strcat(sAIIName,sNodeName); break;
					case NAVINT_FOLLOWPATHCOVER_MINMAX	: strcpy(sAIIName,"Use WP, cover, MINMAX, to go from ["); strcat(sAIIName,sStartNodeName);
														  strcat(sAIIName,"] to ["); strcat(sAIIName,sNodeName); break;
					case NAVINT_FOLLOWPATHCOVER_TO_ENEMY: strcpy(sAIIName,"Use WP, cover, to go to Player from ["); strcat(sAIIName,sStartNodeName);
														  strcat(sAIIName,"] to ["); strcat(sAIIName,sNodeName); break;
					case NAVINT_GOTOENTITY				: strcpy(sAIIName,"To move towards entity ["); strcat(sAIIName,sGEName); break;
					case NAVINT_ATTACK_ENTITY			: strcpy(sAIIName,"To attack entity ["); strcat(sAIIName,sAEName); break;
					case NAVINT_FOLLOW_ENTITY			: strcpy(sAIIName,"To follow entity ["); strcat(sAIIName,sFEName); break;
					case NAVINT_GOTOLOCATORNODE			: strcpy(sAIIName,"To move to ["); 
														  if (pMov->IsDestinationNodeValid()) strcat(sAIIName,"Valid]"); else  strcat(sAIIName,"Invalid]");
														  strcat(sAIIName," locator node ["); strcat(sAIIName,sNodeName); break;
					default								: strcpy(sAIIName,"Strange Fellow... dunno what he thinks... [");
				}
				strcat(sAIIName,"]");
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Intentions:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "%s",sAIIName);
				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "VISION:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "%s", sVision);
				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Move-To Targets:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "Entity[%s], Node[%s]", sGEName, sNodeName);
				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Action Targets:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "Attack[%s], Follow[%s]",sAEName, sFEName);
				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Dest.Points:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "To Combat Point[%s] - To Destination Point[%s]", 
									SHOW_NAVIG_FLAG_STATUS(NF_TO_COMBAT_POINT),
									SHOW_NAVIG_FLAG_STATUS(NF_ARRIVE_AT_POINT));
				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Move/Anim Flags:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "Facing Entity[%s] - Single Animation[%s] - Facing Enemy[%s] - Ext.Control[%s]", 
									SHOW_ON_OFF(pCAIComp->IsPlayingFacingAction()),
									SHOW_ON_OFF(!pCAIComp->IsSimpleActionComplete()),
									SHOW_ON_OFF(pMov->IsMovingWhileFacingTgt()),
									SHOW_ON_OFF(pMov->GetExternalControlState()));
				fY+=fV_SPACING;
				
				// Render Location of gotoentity
				if (pAII->eNavIntentions == NAVINT_GOTOENTITY )
				{
					float fMaxRad = sqrtf(pMov->GetDestinationRadiusSQR());
					static float fRad = 0.0f;

					CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
					ArcMatrix.SetTranslation(pGE->GetPosition()+CPoint(0,0.5,0));
					g_VisualDebug->RenderArc(ArcMatrix, fMaxRad , TWO_PI,  DC_RED);
					fRad +=0.1f;
					if ( fRad>1.5f ) fRad = 0.0f;
					g_VisualDebug->RenderArc(ArcMatrix, fRad , TWO_PI,  DC_WHITE);
				}
				
				// Render Destination Point
				
				if (IS_ACTIVE_NAVIG_FLAG(NF_ARRIVE_AT_POINT))
				{
					static float fDestPointRadius = 0.0f;
					CPoint p3DDestPos = pMov->GetDestinationPos();
					CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
					ArcMatrix.SetTranslation(p3DDestPos+CPoint(0,0.2f,0));
					fDestPointRadius +=0.1f;
					if ( fDestPointRadius>sqrt(pMov->GetDestinationRadiusSQR()) ) fDestPointRadius = 0.0f;
					g_VisualDebug->RenderArc(ArcMatrix, fDestPointRadius , TWO_PI, DC_GREEN );
					g_VisualDebug->Printf3D(p3DDestPos,DC_GREEN,0,"Dest.Pos[%s]:(%.3f,%.3f,%.3f)",
												ntStr::GetString(m_pobCurrentEntity->GetName()),
												p3DDestPos.X(),
												p3DDestPos.Y(),
												p3DDestPos.Z());
				}

				// WHACK-A-MOLE info

				bool bRangeEnemy = (pAE && (((Character*)pAE)->GetRangedWeapon() != NULL)) ? true : false;
				bool bAmIARangeAI= ((Character*)m_pobCurrentEntity)->GetRangedWeapon() == NULL ? false : true;

				if (bAmIARangeAI && (pAII->eNavIntentions == NAVINT_WHACKAMOLE) )
				{
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Whack-a-Mole:");
					g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "Cover Cycle [%d/%d], Shot Nr: [%d/%d], Type: [%s], Accuracy[%.3f], OffsetRadius[%.3f] %s, ",
															pMov->GetCurrentCoverCycle(), 
															pMov->GetTotalCoverCycles(),
															pMov->GetShotsCount(),
															pMov->GetNumberOfConsecShoots(false),
															pMov->GetIsAccurateShot() ? "HIT" : "MISS",
															pMov->GetShootingAccuracy(false),
															pMov->GetOffsetRadius(),
															pMov->GetAlwaysMiss() ? ", - Always Miss" : "");
					fY+=fV_SPACING;
				}

				// Render shooting point
				if (bAmIARangeAI)
				{
					// Render the Shooting Target Spot
					static float fRadiusShootingSpot = 0.0f;
					unsigned int uiColorShootingSpot = pMov->GetAlwaysMiss() ? DC_YELLOW : DC_RED;

					CPoint obShootingSpot = pMov->GetShootingPoint();
					CMatrix ArcMatrixSpot(CONSTRUCT_IDENTITY);
					ArcMatrixSpot.SetTranslation(obShootingSpot);
					g_VisualDebug->RenderArc(ArcMatrixSpot, fRadiusShootingSpot , TWO_PI,  uiColorShootingSpot);
					g_VisualDebug->RenderArc(ArcMatrixSpot, 0.4f , TWO_PI,  uiColorShootingSpot);
					g_VisualDebug->Printf3D(obShootingSpot,uiColorShootingSpot,0,"Pos: (%.3f,%.3f,%.3f)-Shooter:%s", obShootingSpot.X(),
																													 obShootingSpot.Y(),
																													 obShootingSpot.Z(),
																													 ntStr::GetString(m_pobCurrentEntity->GetName()));
					if (pAE)
						g_VisualDebug->RenderLine(obShootingSpot,pAE->GetPosition(),uiColorShootingSpot);

					fRadiusShootingSpot += 0.1f;
					if (fRadiusShootingSpot>0.4f)
						fRadiusShootingSpot = 0.0f;

					CAINavigNode* pTCNode = pMov->GetWhackAMoleNode();
					if (pTCNode)
					{
						CDirection obDir = pTCNode->GetWhackAMoleDir();
						CPoint obPos = m_pobCurrentEntity->GetPosition() + CPoint(0,1.0f,0);
						g_VisualDebug->RenderLine(obPos,CPoint(obPos+obDir*2.0f),DC_BLUE);
					}

				}

				if (bRangeEnemy || bAmIARangeAI)
				{
					// MIN-MAX area
					CPoint obEnemyrPos	= pAE->GetPosition()+CPoint(0,0.5,0);
					CPoint obAIPos		= pMov->GetPosition()+CPoint(0,0.5,0);

					float fMax = sqrtf(pMov->GetRangeMaxDistSQR());
					float fMin = sqrtf(pMov->GetRangeMinDistSQR());

					CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
					ArcMatrix.SetTranslation(obEnemyrPos);
					g_VisualDebug->RenderArc(ArcMatrix, fMin , TWO_PI,  DC_GREEN);
					g_VisualDebug->RenderArc(ArcMatrix, fMax , TWO_PI,  DC_GREEN);

					CDirection Heading		= pAE->GetMatrix().GetZAxis();
					CDirection SideDir		= pAE->GetMatrix().GetXAxis();
					g_VisualDebug->RenderLine(obEnemyrPos+CPoint(Heading*fMin),obEnemyrPos+CPoint(Heading*fMax),DC_GREEN);
					g_VisualDebug->RenderLine(obEnemyrPos-CPoint(Heading*fMin),obEnemyrPos-CPoint(Heading*fMax),DC_GREEN);
					g_VisualDebug->RenderLine(obEnemyrPos+CPoint(SideDir*fMin),obEnemyrPos+CPoint(SideDir*fMax),DC_GREEN);
					g_VisualDebug->RenderLine(obEnemyrPos-CPoint(SideDir*fMin),obEnemyrPos-CPoint(SideDir*fMax),DC_GREEN);

					// Is AI whithin the area?

					CDirection Line2Enemy( obAIPos - obEnemyrPos );
					float fDist2Enemy = Line2Enemy.Length();
					
					if ( fDist2Enemy < fMin ||
							fDist2Enemy > fMax )
					{
						// Highlight it
						static float fRadiusH = 0.0f;
						CMatrix ArcMatrixAI(CONSTRUCT_IDENTITY);
						ArcMatrixAI.SetTranslation(obAIPos);
						fRadiusH +=0.1f;
						if ( fRadiusH>1.5f ) fRadiusH = 0.0f;
						g_VisualDebug->RenderArc(ArcMatrixAI, fRadiusH , TWO_PI,  DC_GREEN);
						g_VisualDebug->Printf3D(obAIPos,DC_YELLOW,0,"Moving to MIN-MAX");

					}
				}
		
				// Leader
				//CEntity* pLeader = pMov->GetLeader();
				//g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Leader:");
				//g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "%s", (pLeader ? pLeader->GetName().c_str() : "No Leader" ));
				//fY+=fV_SPACING;
				// Am I Leader?
				//if (pMov->IsLeader()) g_VisualDebug->Printf3D(m_pobCurrentEntity->GetPosition(),DC_YELLOW,0,"LEADER");
				// Intentions and Destination
				//g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Actions:");
				//SDestination* pD = pMov->GetDestination();
				//switch (pD->eType)
				//{
				//	case DEST_IDLE	: g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0,"None"); break;
				//	case DEST_LEADER: g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0,"Follows Leader (%s)",pD->pLeader->GetName().c_str()); break;
				//	case DEST_NODE  : g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0,"Goes to Node (%s)",*(*(pD->pNode->GetName()))); break;
				//	default			: g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0,"Unknown Intentions!!!");
				//}	
				//fY+=fV_SPACING;
				// Navigation Flags
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Follow Completed:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "Combat Point(%s), Path(%s), Target(%s), Entity(%s)",
					SHOW_YES_NO(pMov->IsMoveToCombatPointCompleted()),
					SHOW_YES_NO(pMov->IsFollowPathCompleted()),
					SHOW_YES_NO(pMov->IsChaseTargetCompleted()),
					SHOW_YES_NO(pMov->IsFollowEntityCompleted()));
				fY+=fV_SPACING;

				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Steering Flags:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "GA(%s) FPCv(%s) FPMM(%s) AAP(%s) ChME(%s), Ptrl(%s), F_Ld(%s), Avoid[S(%s), D(%s), AI(%s)]", 
							//SHOW_NAVIG_FLAG_STATUS(NF_CHASE_ENEMY),
							SHOW_NAVIG_FLAG_STATUS(NF_GO_AROUND_VOLUMES),
							SHOW_NAVIG_FLAG_STATUS(NF_FOLLOW_PATH_COVER),
							SHOW_NAVIG_FLAG_STATUS(NF_FOLLOW_PATH_DYNCOVER),
							SHOW_NAVIG_FLAG_STATUS(NF_ARRIVE_AT_POINT),
							SHOW_NAVIG_FLAG_STATUS(NF_CHASE_MOVING_ENTITY),
							SHOW_NAVIG_FLAG_STATUS(NF_FOLLOW_PATROL_PATH),
							SHOW_NAVIG_FLAG_STATUS(NF_FOLLOW_LEADER),
							SHOW_NAVIG_FLAG_STATUS(NF_S_OBSTACLE),
							SHOW_NAVIG_FLAG_STATUS(NF_D_OBSTACLE),
							SHOW_NAVIG_FLAG_STATUS(NF_AI_OBSTACLE));
				fY+=fV_SPACING;
				// Patrolling?
				if (NF_FOLLOW_PATROL_PATH & uiFlags) g_VisualDebug->Printf3D(m_pobCurrentEntity->GetPosition(),DC_YELLOW,0,"Patrolling");
				// Speed
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Speed / Max Speed:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "%f / %f", pMov->GetSpeedMagnitude(), pMov->GetMaxSpeed() );
				fY+=fV_SPACING;
				// Playing Single Anim?
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Idle Flag:");
				g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "Clear Intention[%s]", SHOW_YES_NO(pMov->GetIdleClearsIntention()) );
				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulNAVIGTITLE_COLOUR, 0, "----------------------------");
				fY+=fV_SPACING;
				// Cover Point
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Entering Cover Point:");
				if (pMov->GetCoverPoint() && pMov->GetCoverPoint()->GetClaimer())
				{
					g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "Booked:[%s]", ntStr::GetString(pMov->GetCoverPoint()->GetClaimer()->GetName()) );
				}
				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Clamed Cover Point:");
				if (pMov->GetClaimedCoverPoint())
				{
					g_VisualDebug->Printf2D(fX+160.0f,fY, ulINFO_COLOUR, 0, "Booked:[%s]", ntStr::GetString(pMov->GetClaimedCoverPoint()->GetName()) );
				}
				fY+=fV_SPACING;
				
				// Show Current node
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Current Node (Path):");
				CAINavigPath* pPath = pMov->GetPathContainer();
				CAINavigNode* pNode = pPath->GetCurrentNode();
				static float fRadiusCurrentNode = 0.2f;
				if (pNode)
				{
					pPath->RenderPath();
					g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "%s - Pos: %.3f,%.3f,%.3f", ntStr::GetString(pNode->GetName()),
																									pNode->GetPos().X(),
																									pNode->GetPos().Y(),
																									pNode->GetPos().Z());
					CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
					fRadiusCurrentNode +=0.1f;
					if ( fRadiusCurrentNode>1.5f ) fRadiusCurrentNode = 0.0f;
					ArcMatrix.SetTranslation(pNode->GetPos()+CPoint(0.0f,0.1f,0.0f));
					g_VisualDebug->RenderArc(ArcMatrix, fRadiusCurrentNode , TWO_PI,  DC_YELLOW);
					g_VisualDebug->Printf3D( pNode->GetPos()+CPoint(0.0f,0.1f,0.0f), DC_YELLOW, 0, "Node:%s",ntStr::GetString(pNode->GetName()));
					// Draw target point within node
					g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), pPath->GetPointWithinCurrentNode(), 0.2f, DC_BLUE );
				}
				else
				{
					g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "%s", pPath->empty() ? "EMPTY PATH" : "NULL - N/A" );
				}

				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Start Node (Path):");
				pNode = pPath->GetStartNode();
				g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "%s", pNode ? ntStr::GetString(pNode->GetName()) : "[]" );
				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Goal Node (Path):");
				pNode = pPath->GetGoalNode();
				g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "%s", pNode ? ntStr::GetString(pNode->GetName()) : "[]" );
				fY+=fV_SPACING;

				// Patrol Path
				//CAINavigGraph* pNGPatroller = CAINavigationSystemMan::Get().GetPatrollersGraph(m_pobCurrentEntity);
				CAIPatrolGraph* pPG = pMov->GetPatrolGraph();
				if (pPG) 
				{
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Patrol (Path):");
					g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "%s", ntStr::GetString(pPG->GetName() ));
					fY+=fV_SPACING;

					CAINavigNode* pCurrentPatrolNode = pPG->GetCurrentNode(m_pobCurrentEntity);
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Current Patrol Node:");
					g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "%s",	pCurrentPatrolNode ? 
																					ntStr::GetString(pCurrentPatrolNode->GetName()) : "NULL!" );
					fY+=fV_SPACING;
					if (pCurrentPatrolNode)
					{
						CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
						fRadiusCurrentNode +=0.1f;
						if ( fRadiusCurrentNode>1.5f ) fRadiusCurrentNode = 0.0f;
						ArcMatrix.SetTranslation(pCurrentPatrolNode->GetPos()+CPoint(0.0f,0.1f,0.0f));
						g_VisualDebug->RenderArc(ArcMatrix, fRadiusCurrentNode , TWO_PI,  DC_RED);
						g_VisualDebug->Printf3D( pCurrentPatrolNode->GetPos()+CPoint(0.0f,0.1f,0.0f), DC_YELLOW, 0, "Patrol Node:%s",ntStr::GetString(pCurrentPatrolNode->GetName()));
					}

				}
				// View Cones
				if(	(CAINavigationSystemMan::Get().m_bRenderViewCones) && m_pobCurrentEntity->ToCharacter() && !m_pobCurrentEntity->ToCharacter()->IsDead() )
				{
					pCAIComp->GetAIVision()->DebugRender();
				}
				g_VisualDebug->Printf2D(fX,fY, ulNAVIGTITLE_COLOUR, 0, "----------------------------");
				fY+=fV_SPACING;
				g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Hearing:");
				CAIHearing* pHear = pCAIComp->GetAIHearing();
				if (pHear)
					g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "%s - Vol. Threshold: %.3f - Last Source: {%.3f, %.3f, %.3f}", 
										pHear->IsDeaf() ? "Deaf!" : "On", 
										pHear->GetVolumeThreshold(),
										pHear->GetSoundSourceLocation().X(),pHear->GetSoundSourceLocation().Y(), pHear->GetSoundSourceLocation().Z()
										);
				else
					g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "NUL!!");
				fY+=fV_SPACING;
				CAISingleQueue* pSQ = pMov->GetSingleQueue();
				if (pSQ)
				{
					g_VisualDebug->Printf2D(fX,fY, ulNAVIGTITLE_COLOUR, 0, "----------------------------");
					fY+=fV_SPACING;
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "Queue:");
					g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "Object (%s) - SQ (%s) - Index: %d",ntStr::GetString(pSQ->GetObject()->GetName()), 
																												ntStr::GetString(pSQ->GetName()),
																												pMov->GetQueueIndex());
					fY+=fV_SPACING;
				}
				g_VisualDebug->Printf2D(fX,fY, ulNAVIGTITLE_COLOUR, 0, "----------------------------");
				fY+=fV_SPACING;
		
				// Cannons ========================================================================
				// ================================================================================

				CEntity* pIntTarget = ((Character*)m_pobCurrentEntity)->GetInteractionTarget();

				if ( pIntTarget && pIntTarget->IsInteractable() &&
					((Interactable*)pIntTarget)->GetInteractableType()==Interactable::EntTypeInteractable_TurretWeapon
					)
				{
					Interactable* pCannon = (Interactable*)pIntTarget;
					int iNumUsePoints = pCannon->GetInteractionComponent()->GetNumberOfUsePoints();
					CPoint obUsePoint(CONSTRUCT_CLEAR);
					if (iNumUsePoints>0)
						obUsePoint = pCannon->GetInteractionComponent()->GetUsePoint(0)->GetLocalPosition();
					CPoint obCannonRoot = pCannon->GetPosition()+ obUsePoint;
					CPoint obCannonTgt	= CPoint (obCannonRoot+ pMov->GetFacingAction() + CPoint(0,1.0f,0));
					g_VisualDebug->RenderLine(obCannonRoot,obCannonTgt,DC_GREEN);
					g_VisualDebug->RenderPoint(obCannonRoot,15.0f,DC_GREEN);
					g_VisualDebug->RenderPoint(obCannonTgt,15.0f,DC_BLUE);
					
					// Draw Axis
					
					Interactable_TurretWeapon* pCannonTurret = (Interactable_TurretWeapon*)pCannon;
					CHashedString sHash(pCannonTurret->GetDrivingSeat());
					Transform*    pTransform = pCannonTurret->GetHierarchy()->GetTransform(sHash);

					CDirection CannonHeading = pTransform->GetWorldMatrix().GetZAxis();
					CDirection CannonSideDir = pTransform->GetWorldMatrix().GetXAxis();
					CDirection CannonPitchDir = pTransform->GetWorldMatrix().GetYAxis();

					g_VisualDebug->RenderLine(obCannonRoot,CPoint(obCannonRoot+CDirection(0.0f,100.0f,0.0f)),DC_WHITE);
					g_VisualDebug->RenderLine(obCannonRoot,CPoint(obCannonRoot+CDirection(0.0f,0.0f,100.0f)),DC_WHITE);
					g_VisualDebug->RenderLine(obCannonRoot,CPoint(obCannonRoot+CDirection(100.0f,0.0f,0.0f)),DC_WHITE);
					g_VisualDebug->Printf3D(CPoint(obCannonRoot+CDirection(0.0f,50.0f,0.0f)),DC_YELLOW,0,"WORLD Y-AXIS");
					g_VisualDebug->Printf3D(CPoint(obCannonRoot+CDirection(50.0f,0.0f,0.0f)),DC_YELLOW,0,"WORLD Z-AXIS");
					g_VisualDebug->Printf3D(CPoint(obCannonRoot+CDirection(0.0f,0.0f,50.0f)),DC_YELLOW,0,"WORLD X-AXIS");
					g_VisualDebug->RenderLine(obCannonRoot,CPoint(obCannonRoot+20.0f*CannonPitchDir),DC_CYAN);
					g_VisualDebug->RenderLine(obCannonRoot,CPoint(obCannonRoot+20.0f*CannonHeading),DC_CYAN);
					g_VisualDebug->RenderLine(obCannonRoot,CPoint(obCannonRoot+20.0f*CannonSideDir),DC_CYAN);
					g_VisualDebug->Printf3D(CPoint(obCannonRoot+10.0f*CannonPitchDir),DC_CYAN,0,"%s Y-AXIS", ntStr::GetString(pIntTarget->GetName()));
					g_VisualDebug->Printf3D(CPoint(obCannonRoot+10.0f*CannonHeading),DC_CYAN,0,"%s Z-AXIS", ntStr::GetString(pIntTarget->GetName()));
					g_VisualDebug->Printf3D(CPoint(obCannonRoot+10.0f*CannonSideDir),DC_CYAN,0,"%s X-AXIS", ntStr::GetString(pIntTarget->GetName()));


					// Draw Parabola
					CAIComponent* pCAI = pCAIComp;
					
					float fVo  = pMov->GetCannonBallV0();
					float fVox = fVo*cosf(-pCAI->GetUseObjectPitch());
					float fVoy = fVo*sinf(-pCAI->GetUseObjectPitch());
					float fVo_SQR = fVo*fVo;

					float fMaxDist	= pMov->GetCannonBallV0_SQR()/pMov->GetCannonBallG();
					float fMaxHight	= (fVoy*fVoy)/(2*pMov->GetCannonBallG());
					float fPeakDist = 0.5f*fVo_SQR*fsinf(-2*pCAIComp->GetUseObjectPitch())/pMov->GetCannonBallG();
					float fShotDist = 2.0f*fPeakDist;
	
					CPoint obParabolicPos	= obCannonRoot + CPoint(0.0f,1.5f,0.0f);
					CPoint obParabolicPosRoot = obParabolicPos;
					CDirection obDirParabola = pMov->GetFacingAction();
					obDirParabola.Normalise();
					CDirection dirPerpend =CDirection(obDirParabola.Z(),obDirParabola.Y(),-obDirParabola.X());

					CPoint obShotDistPos = CPoint(obParabolicPosRoot + obDirParabola*fShotDist);
					CPoint obMaxDistPos = CPoint(obParabolicPosRoot + obDirParabola*fMaxDist);
					CPoint obPeakDistPos = CPoint(obParabolicPosRoot + obDirParabola*fPeakDist);
					CPoint obPeakPos	= CPoint(obParabolicPosRoot + obDirParabola*fPeakDist + CDirection(0,1.0f,0)*fMaxHight);

					for (float t = 0.0f; t<100.0f; t=t+0.05f)
					{
						obParabolicPos = CPoint(obParabolicPosRoot + obDirParabola*fVox*t);
						float fY = fVoy*t-0.5f*pMov->GetCannonBallG()*t*t + obParabolicPosRoot.Y();
						obParabolicPos.Y() = fY;
						g_VisualDebug->RenderPoint(obParabolicPos,10.0f,DC_RED);
						if (!pMov->GetCannonTarget())
							break;
						if ( fY < pMov->GetCannonTarget()->GetPosition().Y() )
							break;
					}
					
					g_VisualDebug->RenderLine(obPeakDistPos,obPeakPos,DC_RED);
					g_VisualDebug->Printf3D(CPoint(obPeakDistPos+obPeakPos)*0.5f,DC_RED,0,"Max. Hight = %.3f",	fMaxHight);
					g_VisualDebug->RenderLine(obPeakDistPos,CPoint(obPeakDistPos+dirPerpend*5.0f),DC_RED);
					g_VisualDebug->RenderLine(obPeakDistPos,CPoint(obPeakDistPos-dirPerpend*5.0f),DC_RED);
					g_VisualDebug->Printf3D(obPeakDistPos,DC_RED,0,"Distance = %.3f",	fPeakDist);

					g_VisualDebug->RenderLine(obShotDistPos,CPoint(obShotDistPos+dirPerpend*5.0f),DC_RED);
					g_VisualDebug->RenderLine(obShotDistPos,CPoint(obShotDistPos-dirPerpend*5.0f),DC_RED);
					g_VisualDebug->Printf3D(obShotDistPos,DC_RED,0,"Distance = %.3f",	fShotDist);

					g_VisualDebug->RenderLine(obMaxDistPos,CPoint(obMaxDistPos+dirPerpend*5.0f),DC_RED);
					g_VisualDebug->RenderLine(obMaxDistPos,CPoint(obMaxDistPos-dirPerpend*5.0f),DC_RED);
					g_VisualDebug->Printf3D(obMaxDistPos,DC_RED,0,"Max. Distance = %.3f (%.3f,%.3f,%.3f)",	fMaxDist,
																										 	obMaxDistPos.X(),
																											obMaxDistPos.Y(),
																											obMaxDistPos.Z());
					g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "CANNON INFO:");
					g_VisualDebug->Printf2D(fX+180.0f,fY, ulINFO_COLOUR, 0, "V0(%.3f), G(%.3f), Pitch(%.3f Deg), Yaw(%.3f Deg)",pMov->GetCannonBallV0(),
																														pMov->GetCannonBallG(),
																														RAD_TO_DEG_VALUE*pCAIComp->GetUseObjectPitch(),
																														2*RAD_TO_DEG_VALUE*pCAIComp->GetUseObjectYaw());
					fY+=fV_SPACING;
					g_VisualDebug->Printf2D(fX,fY, ulNAVIGTITLE_COLOUR, 0, "----------------------------");
					fY+=fV_SPACING;
				}

				// Varius Radii
				if ( pAE && pAE->IsPlayer() && pAE->ToPlayer()->IsArcher() )
				{
					unsigned int uiColor = DC_WHITE;
					CDirection Line2Enemy	= CDirection(m_pobCurrentEntity->GetPosition() - pAE->GetPosition());
					float fDist2EnemySQR	= Line2Enemy.LengthSquared();
					if (fDist2EnemySQR < pMov->GetRangeMinDistSQR()) //GetResidentEvilRadiusSQR())
					{
						uiColor = DC_BLUE;
							
					}
					CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
					ArcMatrix.SetTranslation(pAE->GetPosition()+CPoint(0.0f,0.1f,0.0f));
					g_VisualDebug->RenderArc(ArcMatrix, sqrt(pMov->GetRangeMinDistSQR()) , TWO_PI,  uiColor); //GetResidentEvilRadiusSQR()
					//g_VisualDebug->Printf3D( pCurrentPatrolNode->GetPos()+CPoint(0.0f,0.1f,0.0f), DC_YELLOW, 0, "Patrol Node:%s",ntStr::GetString(pCurrentPatrolNode->GetName()));
					uiColor =  (fDist2EnemySQR < pMov->GetAttackRangeSQR()) ? DC_BLUE : DC_WHITE;
					ArcMatrix = CMatrix(CONSTRUCT_IDENTITY);
					ArcMatrix.SetTranslation(pAE->GetPosition()+CPoint(0.0f,0.1f,0.0f));
					g_VisualDebug->RenderArc(ArcMatrix, sqrt(pMov->GetAttackRangeSQR()) , TWO_PI,  uiColor);
				}

				if (CAINavigationSystemMan::Get().m_bRenderAIAvoidance)
				{
					DisplaySpeeds();
				}
			}

			// (Dario - End)
			if( (m_pobCurrentEntity)->IsAI() && ((AI*)m_pobCurrentEntity)->GetAIComponent()->GetAIState() )
			{
				CPoint obDest = ((AI*)m_pobCurrentEntity)->GetAIComponent()->GetAIState()->GetActionDest();
				g_VisualDebug->Printf2D(fX,fY, ulINFO_COLOUR, 0, "AI action dest, %.2f,%.2f,%.2f", obDest.X(), obDest.Y(), obDest.Z() );
				fY+=fV_SPACING;
			}

			// Selected anim
			if (m_pobCurrentTemplateAnim && m_pobCurrentEntity && m_pobCurrentEntity->g_pAnimationMap && m_pobCurrentTemplateAnim )
			{
				g_VisualDebug->Printf2D(fX,fY, ulANIMATION_COLOUR, 0, "Anim:");	
				g_VisualDebug->Printf2D(fX+56.0f,fY, ulINFO_COLOUR, 0, "%d/%d %s",	m_iCurrentAnim,
																					m_pobCurrentEntity->g_pAnimationMap->m_totalAnims.size(),
																					ntStr::GetString(m_pobCurrentTemplateAnim->GetShortNameHash()));
			}
			else
			{
				g_VisualDebug->Printf2D(fX,fY, ulANIMATION_COLOUR, 0, "No anims found");
			}
			
			fY+=fV_SPACING;

			// Animator list
			//if (m_pobCurrentTemplateAnim)
			if (m_pobCurrentEntity->GetAnimator())
				m_pobCurrentEntity->GetAnimator()->DebugRenderPrint(fX,fY, ulANIMATION_COLOUR);

			// Render the world space AABB for this entity
			#ifdef _SHOW_ENTITY_AABB
			if (m_pobCurrentEntity->GetRenderableComponent())
				m_pobCurrentEntity->GetRenderableComponent()->GetWorldSpaceAABB().DebugRender(CVecMath::GetIdentity(),0xffffffff, DPF_WIREFRAME);
			#endif // _SHOW_ENTITY_AABB
		}
		else
		{
			g_VisualDebug->Printf2D(fX,fY, ulENTITY_COLOUR, 0, "No entities");
			fY+=fV_SPACING;
		}
	}

#endif // _RELEASE
}





/***************************************************************************************************
*
*	FUNCTION		CEntityBrowser::UpdateAxis
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntityBrowser::UpdateAxis ()
{
#ifndef _RELEASE

	if(!MouseInput::Exists())
		return;

	CInputKeyboard &obKeyboard=CInputHardware::Get().GetKeyboard();

	if (obKeyboard.IsKeyPressed(KEYC_A, KEYM_SHIFT))
		m_bDebugAxis=!m_bDebugAxis;

	if (!m_bDebugAxis)
		return;

	const CCamera* pobCamera = CamMan_Public::GetP();

	CMatrix obMatrix = pobCamera->GetViewTransform()->GetWorldMatrix();
	obMatrix.SetTranslation(CVecMath::GetZeroPoint());

	const float fVIEWPORT_WIDTH = g_VisualDebug->GetDebugDisplayWidth();
	const float fVIEWPORT_HEIGHT = g_VisualDebug->GetDebugDisplayHeight();

	float fHALF_WIDTH = 1.0f;
	float fSCREEN_POSITION_X = 675.0f / fVIEWPORT_WIDTH;
	float fSCREEN_POSITION_Y = (fVIEWPORT_HEIGHT-40.0f) / fVIEWPORT_HEIGHT;
	float fWORLD_POSITION_Z = 25.0f;
	
	CDirection obRay=MouseInput::Get().GetWorldRayFromScreenPos( fSCREEN_POSITION_X, fSCREEN_POSITION_Y );
	obRay *= fWORLD_POSITION_Z;

	CPoint obCentre=pobCamera->GetViewTransform()->GetWorldMatrix().GetTranslation() + CPoint(obRay);

	CPoint obX1=CPoint(fHALF_WIDTH,0.0f,0.0f)+obCentre;
	CPoint obX2=CPoint(-fHALF_WIDTH,0.0f,0.0f)+obCentre;

	CPoint obY1=CPoint(0.0f,fHALF_WIDTH,0.0f)+obCentre;
	CPoint obY2=CPoint(0.0f,-fHALF_WIDTH,0.0f)+obCentre;

	CPoint obZ1=CPoint(0.0f,0.0f,fHALF_WIDTH)+obCentre;
	CPoint obZ2=CPoint(0.0f,0.0f,-fHALF_WIDTH)+obCentre;

	g_VisualDebug->Printf3D( obX1, -5.0f, -5.0f, NTCOLOUR_ARGB(0xff, 0xff, 0x77, 0x77), 0, "X");
	g_VisualDebug->Printf3D( obY1, -5.0f, -5.0f, NTCOLOUR_ARGB(0xff, 0x00, 0xff, 0x00), 0, "Y");
	g_VisualDebug->Printf3D( obZ1, -5.0f, -5.0f, NTCOLOUR_ARGB(0xff, 0x77, 0x77, 0xff), 0, "Z");

	g_VisualDebug->RenderLine(obX1,obX2, 0xffff7777, DPF_NOZCOMPARE); // X axis
	g_VisualDebug->RenderLine(obY1,obY2, 0xff00ff00, DPF_NOZCOMPARE); // Y axis
	g_VisualDebug->RenderLine(obZ1,obZ2, 0xff0000ff, DPF_NOZCOMPARE); // Y axis

#endif // _RELEASE
}


/***************************************************************************************************
*
*	FUNCTION		CEntityBrowser::UpdateLuaStateDisplay
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntityBrowser::UpdateLuaStateDisplay ()
{
#ifndef _RELEASE

	CInputKeyboard &obKeyboard=CInputHardware::Get().GetKeyboard();
	
	if (obKeyboard.IsKeyPressed( KEYC_S, KEYM_CTRL ))
	{
		++m_iLuaStateDebugMode;

		if (m_iLuaStateDebugMode==5)
			m_iLuaStateDebugMode=0;
	}

	if (m_iLuaStateDebugMode==0)
		return;

	for(QuickPtrList<CEntity>::iterator obIt=CEntityManager::Get().m_entities.begin(); obIt!=CEntityManager::Get().m_entities.end(); ++obIt)
	{
		CEntity* pobEntity=(*obIt);
		
		if (pobEntity->HasAttributeTable() && pobEntity->GetMessageHandler())
		{
			bool bIsPlayer=(pobEntity->GetInputComponent() ? true : false);
			bool bIsEnemy=(pobEntity->IsAI() ? true : false);

			if ((m_iLuaStateDebugMode==1) ||
				(m_iLuaStateDebugMode==2 && bIsPlayer) ||
				(m_iLuaStateDebugMode==3 && bIsEnemy) ||
				(m_iLuaStateDebugMode==4 && !bIsPlayer && !bIsEnemy) )
			{
				CPoint obPosition(pobEntity->GetPosition());

				if (pobEntity->IsPlayer() || pobEntity->IsEnemy()) // Get the text to display above the character's head
					obPosition.Y()+=1.6f;
				
				CVector obTextColour;
				if (bIsPlayer) // This is a player
				{
					obTextColour.X()=0.25f;
					obTextColour.Y()=1.0f;
					obTextColour.Z()=1.0f;
					obTextColour.W()=1.0f;
				}
				else if (bIsEnemy) // This is an enemy
				{
					obTextColour.X()=1.0f;
					obTextColour.Y()=1.0f;
					obTextColour.Z()=0.25f;
					obTextColour.W()=1.0f;
				}
				else // This is an object
				{
					obTextColour.X()=0.25f;
					obTextColour.Y()=1.0f;
					obTextColour.Z()=0.25f;
					obTextColour.W()=1.0f;
				}

				if (pobEntity->GetFSM() ) // If the entity has no states, don't display anything
				{
					char acString[64];
					//sprintf(acString,"%s - %s",pobEntity->GetName().c_str(),pobEntity->GetMessageHandler()->GetCurrentState());
					sprintf(acString,"%s",pobEntity->GetFSM()->GetStateName() );
					g_VisualDebug->Printf3D( obPosition, obTextColour.GetNTColor(), 0, acString );
				}

				if( pobEntity->IsAI() )
				{
					obTextColour.Y() = 0.5f;

					char acString[64];
						sprintf(acString,"%s",((AI*)pobEntity)->GetAIComponent()->GetCurrentState());
					g_VisualDebug->Printf3D( obPosition, 0.f, 12.f, obTextColour.GetNTColor(), 0, acString);
				}
			}
		}
	}

#endif // _RELEASE
}


/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::RenderLockon
*
*	DESCRIPTION		
*
***************************************************************************************************/
void CEntityBrowser::RenderLockon ()
{
#ifndef _RELEASE

	if (!m_pobPlayer) // Make sure we have a valid player
		return;

	if (!m_pobPlayer->GetAttackComponent()) // Make sure player has an attack component
		return;
	
	CInputKeyboard &obKeyboard=CInputHardware::Get().GetKeyboard();

	bool bInContext=CInputHardware::Get().GetContext()==INPUT_CONTEXT_COMBAT ? true : false;

	if (bInContext && obKeyboard.IsKeyPressed(KEYC_L)) // Cycle lockon mode
	{
		m_iLockonMode=(m_iLockonMode+1)%3;
		//ntPrintf("lockon mode=%d\n",m_iLockonMode);
	}

	if (m_iLockonMode==0) // Lockon assist is disabled
		return;

	const float fRADIUS_X = 0.3f;
	const float fRADIUS_Y = 1.8f;
	const float fRADIUS_Z = 0.3f;
	const u_long ulCUBE_COLOUR = 0x7fff3333;
	const float fLOCKON_TIME = 0.25f;

	// Decide if the lockon cube should be rendered or not

	bool bRenderCube=false;

	m_pobLockonEntity=m_pobPlayer->GetAwarenessComponent()->GetCurrentAutoLockonP();

	if (m_iLockonMode==1) // Lockon assist is always on
	{
		m_fLockonTimer=0.0f;

		if (m_pobLockonEntity)
		{
			bRenderCube=true;
		}
	}
	else if (m_iLockonMode==2) // Lockon assist if player starts an attack
	{
		if (m_fLockonTimer==0.0f)
		{
			if (m_pobPlayer->GetAttackComponent()->AI_Access_GetState()==CS_ATTACKING && m_pobLockonEntity)
			{
				m_fLockonTimer=fLOCKON_TIME;
				bRenderCube=true;
			}
		}
		else
		{
			m_fLockonTimer-=CTimer::Get().GetGameTimeChange();

			if (m_fLockonTimer<0.0f)
				m_fLockonTimer=0.0f;

			bRenderCube=true;
		}
	}

	if (bRenderCube && m_pobLockonEntity)
	{
		CMatrix obTemp;
		obTemp.SetIdentity();
		obTemp[0][0]=fRADIUS_X;
		obTemp[1][1]=fRADIUS_Y;
		obTemp[2][2]=fRADIUS_Z;
		obTemp.SetTranslation(m_pobLockonEntity->GetPosition());
		g_VisualDebug->RenderCube(obTemp,ulCUBE_COLOUR);
	}

#endif // _RELEASE
}

/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::RenderHelp
*
*	DESCRIPTION		
*
****************************************************************************************************/
void CEntityBrowser::RenderHelp ()
{
#ifndef _RELEASE

	if (!m_bHelp)
		return;

	float fX=300.0f;
	float fY=290.0f;
	const u_long ulHELP_COLOUR		= 0xff00ffff;
 
	//g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "CTRL+E     Toggle entity debugger"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "L/R ARROW  Cycle entity (Quick cycle L/R+SHIFT)"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "U/D ARROW  Cycle anim (Quick cycle U/D+SHIFT)"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "A          Play selected anim"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "E          Toggle anim event debugger"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "T          Toggle transform render"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "SHIFT+C    Toggle collision redner"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "SHIFT+A    Toggle axis render"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "CTRL+S     Toggle entity state render"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "L          Toggle lockon render"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "R          Rebuild anim event lists"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "Z          Toggle movement component"); fY+=fV_SPACING;
	g_VisualDebug->Printf2D(fX,fY, ulHELP_COLOUR, 0, "S          Toggle anim display only"); fY+=fV_SPACING;
	
#endif // _RELEASE
}

/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::DisplayNames
*
*	DESCRIPTION		
*
****************************************************************************************************/

void CEntityBrowser::DisplayNames()
{
#ifndef _RELEASE

	if( !m_bShowNames || (CEntityManager::Get().m_entities.empty()) )
		return;

	QuickPtrList<CEntity>::iterator obIt = CEntityManager::Get().m_entities.begin();
	QuickPtrList<CEntity>::iterator obEndIt = CEntityManager::Get().m_entities.end();

	// 
	while( obIt != obEndIt )
	{
		if( !(*obIt)->GetParentEntity() )
		{
			g_VisualDebug->Printf3D( (*obIt)->GetPosition(), 0xffff0000, 0, "%s", (*obIt)->GetName().c_str() );
		}

		// Forward to the next entity
		++obIt;
	}

#endif
}

/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::DisplayNames
*
*	DESCRIPTION		
*
****************************************************************************************************/

void CEntityBrowser::DisplaySpeeds( void )
{
#ifndef _RELEASE
	static const CPoint PRINT_SPEED_OFFSET = CPoint(0.0f,1.8f,0.0f);

	QuickPtrList<CEntity>::iterator obIt = CEntityManager::Get().m_entities.begin();
	QuickPtrList<CEntity>::iterator obEndIt = CEntityManager::Get().m_entities.end();

	while( obIt != obEndIt )
	{
		CEntity* pEnt = (*obIt);
		
		if (pEnt->IsAI())
		{
			CAIMovement* pMov = ((AI*)pEnt)->GetAIComponent()->GetCAIMovement();
			if (!pMov)
				continue;

			float fCurrentSpeed = pMov->GetSpeedMagnitude();
			float fMaxSpeed = pMov->GetMaxSpeed();
			//float fBreakSpeed = pMov->GetBreakSpeed();
			float fBreakTime = pMov->GetRemainingSlowDownTime();
			bool bIsBreaking = pMov->IsSlowingDown();

			unsigned int uiColor =	fCurrentSpeed < 0.4f ? DC_BLUE :
			fCurrentSpeed < 0.75f ? DC_CYAN :
			fCurrentSpeed < 0.83f ? DC_RED : 
			fCurrentSpeed < 0.98f ? DC_WHITE : DC_YELLOW;

			g_VisualDebug->Printf3D( pEnt->GetPosition() + PRINT_SPEED_OFFSET, uiColor, 0, "%.3f/%.3f (T:%.2f) %s", fCurrentSpeed, fMaxSpeed, fBreakTime, bIsBreaking ? "BREAKING" : "" );
		}

		// Forward to the next entity
		++obIt;
	}

#endif
}

/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::DumpSuperLog
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::DumpSuperLog()
{
#if (!defined _RELEASE) && (!defined _MASTER)
	if ( m_pobCurrentEntity && m_pobCurrentEntity->GetMessageHandler() )
	{
		m_pobCurrentEntity->GetMessageHandler()->DumpSuperLog();
	}
#endif
	return CR_SUCCESS;
}

/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::AdvanceLuaDebugState
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::AdvanceLuaDebugState()
{
		return CR_SUCCESS;

}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::AdvanceLockonMode
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::AdvanceLockonMode()
{
		return CR_SUCCESS;

}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::ToggleDebugAnimEvents
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::ToggleDebugAnimEvents()
{
	if (m_pobCurrentEntity)
	{
		m_bDebugAnimevents=!m_bDebugAnimevents;

		if (m_pobCurrentEntity->GetAnimator())
		{
			m_pobCurrentEntity->GetAnimator()->GetAnimEventHandler().SetDebugMode(m_bDebugAnimevents);	
		}
		return CR_SUCCESS;
	}
	return CR_FAILED;
}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::RebuildAnimEventLists
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::RebuildAnimEventLists()
{
	if (m_pobCurrentEntity)
	{
		m_pobCurrentEntity->RebuildAnimEventLists();
		return CR_SUCCESS;
	}
	return CR_FAILED;
}

/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::ToggleSkinningDebug
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::ToggleSkinningDebug()
{
	if (m_pobCurrentEntity)
	{
		// [scee_st] now typedefed in the class itself for chunking
		// [scee_st] used to take a copy of the list?!
		CRenderableComponent::MeshInstanceList& list = m_pobCurrentEntity->GetRenderableComponent()->GetMeshInstances();
		
		if(list.size()>0)
		{
			bool bIsAllreadyOnSkinMode = false;
			MaterialInstanceBase* pMaterial = (*(list.begin()))->GetMaterialInstance();
			if(pMaterial!=0 && pMaterial->isFX())
			{
				FXMaterialInstance* pEffectMaterial = static_cast<FXMaterialInstance*>(pMaterial);
				if(pEffectMaterial->GetEffectMaterial()->GetMask().AllOfThem(FXMaterial::F_ISDEBUGSKINNING))
				{
					bIsAllreadyOnSkinMode = true;
				}
			}
			
			if(bIsAllreadyOnSkinMode)
			{
				for(	CRenderableComponent::MeshInstanceList::iterator it = list.begin();
					it != list.end();
					it++)
				{
					CMeshInstance* pMesh = *it;
					pMesh->MaterialRollBack();
				}
			}
			else
			{
				FXMaterial* pLitMaterial = FXMaterialManager::Get().FindMaterial("skinning_debug");
				if(pLitMaterial)
				{
					for(	CRenderableComponent::MeshInstanceList::iterator it = list.begin();
						it != list.end();
						it++)
					{
						CMeshInstance* pMesh = *it;
						pMesh->ForceFXMaterial(pLitMaterial);
					}
				}
			}
		}
		return CR_SUCCESS;
	}
	return CR_FAILED;
}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::SelectPreviousEntity
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::SelectPreviousEntity()
{
	if (m_pobCurrentEntity)
	{
		CEntity* pobFirstEntity=CEntityManager::Get().m_entities.front();
		CEntity* pobLastEntity=CEntityManager::Get().m_entities.back();

		if (m_pobCurrentEntity->GetAnimator())
		{
			m_pobCurrentEntity->GetAnimator()->GetAnimEventHandler().SetDebugMode(false);	
		}

		if (m_pobCurrentEntity==pobFirstEntity)
		{
			m_pobCurrentEntity=pobLastEntity;

			m_iCurrentEntity=CEntityManager::Get().m_entities.size();
		}
		else
		{
			--m_iCurrentEntity;

			CEntity* pobPreviousEntity=NULL;

			for(QuickPtrList<CEntity>::iterator obIt=CEntityManager::Get().m_entities.begin();
				obIt!=CEntityManager::Get().m_entities.end();
				++obIt)
			{
				if ((*obIt)==m_pobCurrentEntity)
				{
					m_pobCurrentEntity=pobPreviousEntity;
					break;
				}

				pobPreviousEntity=*obIt;
			}
		}

		m_pobCurrentTemplateAnim=NULL;
		m_iCurrentAnim=1;

		if (m_pobCurrentEntity->GetAnimator())
		{
			m_pobCurrentEntity->GetAnimator()->GetAnimEventHandler().SetDebugMode(m_bDebugAnimevents);	
		}

		if (!m_pobCurrentTemplateAnim && m_pobCurrentEntity->g_pAnimationMap &&
			!m_pobCurrentEntity->g_pAnimationMap->m_totalAnims.empty())
		{
			m_pobCurrentTemplateAnim = m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[0];
		}
		return CR_SUCCESS;
	}
	return CR_FAILED;
}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::SelectNextEntity
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::SelectNextEntity()
{
	if (m_pobCurrentEntity)
	{
		CEntity* pobFirstEntity=CEntityManager::Get().m_entities.front();
		CEntity* pobLastEntity=CEntityManager::Get().m_entities.back();

		if (m_pobCurrentEntity->GetAnimator())
		{
			m_pobCurrentEntity->GetAnimator()->GetAnimEventHandler().SetDebugMode(false);	
		}

		if (m_pobCurrentEntity==pobLastEntity)
		{
			m_pobCurrentEntity=pobFirstEntity;

			m_iCurrentEntity=1;
		}
		else
		{
			++m_iCurrentEntity;

			for(QuickPtrList<CEntity>::iterator obIt=CEntityManager::Get().m_entities.begin();
				obIt!=CEntityManager::Get().m_entities.end();
				++obIt)
			{
				if ((*obIt)==m_pobCurrentEntity)
				{
					++obIt;

					m_pobCurrentEntity=(*obIt);
					break;
				}
			}
		}

		m_pobCurrentTemplateAnim=NULL;
		m_iCurrentAnim=1;

		if (m_pobCurrentEntity->GetAnimator())
		{
			m_pobCurrentEntity->GetAnimator()->GetAnimEventHandler().SetDebugMode(m_bDebugAnimevents);	
		}

		if (!m_pobCurrentTemplateAnim && m_pobCurrentEntity->g_pAnimationMap &&
			!m_pobCurrentEntity->g_pAnimationMap->m_totalAnims.empty())
		{
			m_pobCurrentTemplateAnim=m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[0];
		}
		return CR_SUCCESS;
	}
	return CR_FAILED;
}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::SelectPreviousAnim
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::SelectPreviousAnim()
{
	if (m_pobCurrentEntity && m_pobCurrentEntity->g_pAnimationMap)
	{
		if (m_pobCurrentTemplateAnim)
		{
			const int iAnimCount = m_pobCurrentEntity->g_pAnimationMap->m_totalAnims.size();

			// retrieve first animation in this entity's container
			AnimShortCut* pobFirstTemplate = m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[0];
			
			// retrieve last animation in this entity's container
			AnimShortCut* pobLastTemplate = m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[iAnimCount-1];

			if (m_pobCurrentTemplateAnim==pobFirstTemplate)
			{
				m_pobCurrentTemplateAnim=pobLastTemplate;
				m_iCurrentAnim=iAnimCount;
			}
			else
			{
				AnimShortCut* pobPreviousTemplate=NULL;

				for (int i = 0; i < iAnimCount; i++)
				{
					if ( m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[i] == m_pobCurrentTemplateAnim )
					{
						m_pobCurrentTemplateAnim=pobPreviousTemplate;
						break;
					}

					pobPreviousTemplate = m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[i];
				}
				--m_iCurrentAnim;
			}
		}
		return CR_SUCCESS;
	}
	return CR_FAILED;
}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::SelectNextAnim
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::SelectNextAnim()
{
	if (m_pobCurrentEntity && m_pobCurrentEntity->g_pAnimationMap)
	{
		if (m_pobCurrentTemplateAnim)
		{
			const int iAnimCount = m_pobCurrentEntity->g_pAnimationMap->m_totalAnims.size();

			// retrieve first animation in this entity's container
			AnimShortCut* pobFirstTemplate = m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[0];
			
			// retrieve last animation in this entity's container
			AnimShortCut* pobLastTemplate = m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[iAnimCount-1];

			if (m_pobCurrentTemplateAnim==pobLastTemplate)
			{
				m_pobCurrentTemplateAnim=pobFirstTemplate;
				m_iCurrentAnim=1;
			}
			else
			{
				for (int i = 0; i < iAnimCount; i++)
				{
					if ( m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[i] == m_pobCurrentTemplateAnim )
					{
						m_pobCurrentTemplateAnim = m_pobCurrentEntity->g_pAnimationMap->m_totalAnims[i+1];
						break;
					}
				}
				++m_iCurrentAnim;
			}
		}
		return CR_SUCCESS;
	}
	return CR_FAILED;
}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::ToggleMovementController
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::ToggleMovementController()
{
	if (m_pobCurrentEntity)
	{
		if (m_pobCurrentEntity->GetMovement())
		{
			if (m_pobCurrentEntity->GetMovement()->IsEnabled())
				m_pobCurrentEntity->GetMovement()->SetEnabled( false );
			else
                m_pobCurrentEntity->GetMovement()->SetEnabled( true );
		}
		return CR_SUCCESS;
	}
	return CR_FAILED;
}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::StartSelectedAnim
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::StartSelectedAnim()
{
	if (m_pobCurrentEntity && m_pobCurrentTemplateAnim)
	{
		//m_pobCurrentEntity->GetAnimator()->RemoveAllAnimations();
		m_pobCurrentEntity->GetAnimator()->ClearAnimWeights();
		CAnimationPtr pobAnimation=m_pobCurrentEntity->GetAnimator()->CreateAnimation(m_pobCurrentTemplateAnim->GetShortNameHash());

		if ( pobAnimation->GetPriority() == 0 )
		pobAnimation->SetFlagBits( ANIMF_LOCOMOTING );

		m_pobCurrentEntity->GetAnimator()->AddAnimation(pobAnimation);
		return CR_SUCCESS;
	}
	return CR_FAILED;
}
/****************************************************************************************************
*
*	FUNCTION		CEntityBrowser::StartSelectedAnimWithAttackDuration
*
*	DESCRIPTION		
*
****************************************************************************************************/
COMMAND_RESULT CEntityBrowser::StartSelectedAnimWithAttackDuration()
{
	if (m_pobCurrentEntity && m_pobCurrentTemplateAnim)
	{
		//m_pobCurrentEntity->GetAnimator()->RemoveAllAnimations();
		m_pobCurrentEntity->GetAnimator()->ClearAnimWeights();
		CAnimationPtr pobAnimation=m_pobCurrentEntity->GetAnimator()->CreateAnimation(m_pobCurrentTemplateAnim->GetShortNameHash());
		pobAnimation->SetFlagBits( ANIMF_LOCOMOTING );
		m_pobCurrentEntity->GetAnimator()->AddAnimation(pobAnimation);

		// See if we can find an attack where it's anim name matches the selected anim
		ObjectContainer* pobContainer = ObjectDatabase::Get().GetPointerFromName<ObjectContainer*>("entities\\new\\characters\\hero\\hero_combat.xml");

		CAttackData* pobRes = FindAttackHelper(pobContainer, m_pobCurrentTemplateAnim->GetShortNameHash());
		if (pobRes)
		{		
			// Set the speed of the animation so that it matches the attacks duration
			pobAnimation->SetSpeed(pobAnimation->GetAnimationHeader()->GetDuration() / pobRes->GetAttackTime(1.0f));
		}
		else
		{
			// Set standard speed
			pobAnimation->SetSpeed(1.0f);
		}

		// Indicate that we are playing a movie
		return CR_SUCCESS;
	}
	return CR_FAILED;
}
