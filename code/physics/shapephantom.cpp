#include "Physics/config.h"
#include "shapephantom.h"

#include "gfx/renderer.h"

#include "Physics/maths_tools.h"
#include "Physics/havokincludes.h"
#include "Physics/world.h"
#include "game/entitymanager.h"
#include "anim/hierarchy.h"
#include "game/messagehandler.h"
#include "camera/camutils.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#include <hkmath/hkmath.h>

#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkmath/basetypes/hkMotionState.h>
#include <hkcollide/agent/hkCollidable.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkcollide/shape/hkShapeRayCastOutput.h>

#endif

#include "game/luaattrtable.h" // For lua callbacks on events

#include "camera/camman_public.h" // Debugging
#include "core/osddisplay.h" // Debugging

#include "objectdatabase/dataobject.h"



START_STD_INTERFACE							( CShapePhantomOBB )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obParentEntity,			"WORLD",						ParentEntity )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obParentTransform,		"ROOT",							ParentTransform )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obPosition,				CPoint(0.0f, 0.0f, 0.0f),		Position )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obOrientation,			CPoint(0.0f, 0.0f, 0.0f),		Orientation )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obDimensions,			CDirection(1.0f, 1.0f, 1.0f),	Dimensions )

	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK	( DebugRender )
END_STD_INTERFACE

START_STD_INTERFACE							( CShapePhantomOBBMinMax )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obParentEntity,			"WORLD",						ParentEntity )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obParentTransform,		"ROOT",							ParentTransform )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obPosition,				CPoint(0.0f, 0.0f, 0.0f),		Position )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obOrientation,			CQuat(0.0f, 0.0f, 0.0f, 1.0f),	Orientation )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obMinPoint,				CPoint(0.0f, 0.0f, 0.0f),		MinPoint )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obMaxPoint,				CPoint(1.0f, 1.0f, 1.0f),		MaxPoint )

	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK	( DebugRender )
END_STD_INTERFACE

START_STD_INTERFACE							( CShapePhantomSphere )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obParentEntity,			"WORLD",						ParentEntity )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obParentTransform,		"ROOT",							ParentTransform )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obPosition,				CPoint(0.0f, 0.0f, 0.0f),		Position )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fRadius,				1.0f,							Radius )

	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK	( DebugRender )
END_STD_INTERFACE

START_STD_INTERFACE							( CShapePhantomCapsule )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obParentEntity,			"WORLD",						ParentEntity )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obParentTransform,		"ROOT",							ParentTransform )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obPosition,				CPoint(0.0f, 0.0f, 0.0f),		Position )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obOrientation,			CPoint(0.0f, 0.0f, 0.0f),		Orientation )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fLength,				1.0f,							Length )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fRadius,				0.5f,							Radius )

	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK	( DebugRender )
END_STD_INTERFACE

START_STD_INTERFACE							( CShapePhantomPortal )
	PUBLISH_VAR_AS							( m_obParentEntity,			ParentEntity )
	PUBLISH_VAR_AS							( m_obParentTransform,		ParentTransform )
	PUBLISH_VAR_AS							( m_obPosition,				Position )
	PUBLISH_VAR_AS							( m_obOrientation,			Orientation )
	PUBLISH_VAR_AS							( m_fWidth,					Width )
	PUBLISH_VAR_AS							( m_fHeight,				Height )

	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK	( DebugRender )
END_STD_INTERFACE

void ForceLinkFunction15()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction15() !ATTN!\n");
}



CShapePhantom::CShapePhantom ()
:	m_obParentEntity("WORLD"),
	m_obParentTransform("ROOT"),
	m_pobTransform(0),
	m_obPosition(CONSTRUCT_CLEAR),
	m_obOrientation(CONSTRUCT_CLEAR),
	m_obLocalMatrix(CONSTRUCT_IDENTITY),
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	m_pobShape(0)
#endif
{
	m_obEntityCFlag.base = 0;
}

CShapePhantom::CShapePhantom (const CPoint& obPosition, const CPoint& obOrientation)
:	m_obParentEntity("WORLD"),
	m_obParentTransform("ROOT"),
	m_pobTransform(0),
	m_obPosition(obPosition),
	m_obOrientation(obOrientation),
	m_obLocalMatrix(CONSTRUCT_IDENTITY),
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	m_pobShape(0)
#endif
{
	m_obEntityCFlag.base = 0;
}

void CShapePhantom::PostConstruct ()
{
	if (m_obParentEntity=="WORLD")
	{
		m_pobTransform=0;
	}
	else
	{
		CEntity* pobEntity=CEntityManager::Get().FindEntity(m_obParentEntity);

		if (pobEntity)
		{
			m_pobTransform=pobEntity->GetHierarchy()->GetTransform(m_obParentTransform); // NOTE: GetTransform should really handle CHashedString's as well
		}
		else
		{
			#ifndef _RELEASE
			DataObject* pobDataObject=ObjectDatabase::Get().GetDataObjectFromPointer(this);
			if (pobDataObject)
				ntPrintf("Failed to find entity %s for shape phantom %s\n",ntStr::GetString(m_obParentEntity), ntStr::GetString(pobDataObject->GetName()));
			#endif // _RELEASE
			m_pobTransform=0;
		}
	}
}

bool CShapePhantom::EditorChangeValue(CallBackParameter/*pcItem*/, CallBackParameter /*pcValue*/)
{
	PostConstruct();

	return true;
}

void CShapePhantom::GetCentre (CPoint& obCentre)
{
	if (m_pobTransform)
	{
		CMatrix obWorldMatrix=m_obLocalMatrix * m_pobTransform->GetWorldMatrix();

		obCentre=obWorldMatrix.GetTranslation();
	}
	else
	{
		obCentre=m_obLocalMatrix.GetTranslation();
	}
}

CPoint CShapePhantom::GetCentre()
{
	if (m_pobTransform)
	{
		CMatrix obWorldMatrix=m_obLocalMatrix * m_pobTransform->GetWorldMatrix();

		return obWorldMatrix.GetTranslation();
	}
	else
	{
		return m_obLocalMatrix.GetTranslation();
	}
}

bool CShapePhantom::IsInside (const CPoint& obPoint)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		// Do this by shooting 2 rays through the shape, one from one direction and one from the opposite direction, if both hit then the point is inside
	
		// Get the AABB
		hkTransform obTransform( Physics::MathsTools::CQuatTohkQuaternion(CQuat(m_obLocalMatrix)), Physics::MathsTools::CPointTohkVector(m_obLocalMatrix.GetTranslation()));
		hkAabb obShapeAabb;
		m_pobShape->getAabb(obTransform,0.001f,obShapeAabb);
		
		// Find largest extent in Y
		float fExtent = 0.0f;
		if (abs(obShapeAabb.m_min(1)) > abs(obShapeAabb.m_max(1)))
			fExtent = abs(obShapeAabb.m_min(1));
		else
			fExtent = abs(obShapeAabb.m_max(1));
		fExtent *= 1.1f; // Make it a bit longer just incase
			
		// Make an up vector from it
		CDirection obRayVector(0.0,1.0,0.0);
		obRayVector *= fExtent; // Lengthen it so its long enough to be outside the shape
		
		// Project our point along it
		CPoint obFrom(obPoint + obRayVector);
		
		// Cast a ray from this projected point to the point
		hkShapeRayCastInput obInput;
		hkShapeRayCastOutput obOutput;
		obInput.m_from.set(obFrom.X(),obFrom.Y(),obFrom.Z());
		obInput.m_to.set(obPoint.X(),obPoint.Y(),obPoint.Z());
		m_pobShape->castRay(obInput,obOutput);
		
		// If it's hit then we need to do it again from the opposite direction and check for a hit
		if (obOutput.hasHit())
		{
			// Reset ray stuff
			obOutput.reset();
			
			// Reflect vector
			obRayVector *= -1;
			
			// Project point along it
			obFrom = obPoint + obRayVector;
			
			// Cast a ray from this projected point to the point
			obInput.m_from.set(obFrom.X(),obFrom.Y(),obFrom.Z());
			obInput.m_to.set(obPoint.X(),obPoint.Y(),obPoint.Z());
			m_pobShape->castRay(obInput,obOutput);
			
			// If it's hit then the point must be within the volume
			if (obOutput.hasHit())
				return true;
		}
	}
#else
	UNUSED( obPoint );
#endif

	return false;
	//ntPrintf("IsInside: P: %f,%f,%f, C: %f,%f,%f, I: %f\n", obInput.m_from(0),obInput.m_from(1),obInput.m_from(2), GetCentre().X(), GetCentre().Y(), GetCentre().Z(), obOutput.m_hitFraction );
	//hkAabb obPointAabb(Physics::MathsTools::CPointTohkVector(obPoint),Physics::MathsTools::CPointTohkVector(obPoint));
}

bool CShapePhantom::IsInside (const CPoint& obStart,const CPoint& obEnd)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		hkShapeRayCastInput obInput;
		hkShapeRayCastOutput obOutput;

		obInput.m_from.set(obStart.X(),obStart.Y(),obStart.Z());
		obInput.m_to.set(obEnd.X(),obEnd.Y(),obEnd.Z());

		m_pobShape->castRay(obInput,obOutput);

		return obOutput.hasHit();
	}
#else
	UNUSED( obStart );
	UNUSED( obEnd );
#endif

	return false;
}

bool CShapePhantom::IsInside (const CPoint& obStart,const CPoint& obEnd, float& fIntersection)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		hkShapeRayCastInput obInput;
		hkShapeRayCastOutput obOutput;

		hkVector4 obFrom(obStart.X(),obStart.Y(),obStart.Z(),0.0f);
		hkVector4 obTo(obEnd.X(),obEnd.Y(),obEnd.Z(),0.0f);

		obInput.m_from = obFrom;
		obInput.m_to = obTo;

		m_pobShape->castRay(obInput,obOutput);
		
		fIntersection = obOutput.m_hitFraction;

		return obOutput.hasHit();
	}
#else
	UNUSED( obStart );
	UNUSED( obEnd );
#endif

	return false;
}

void CShapePhantom::GetIntersecting (ntstd::List<CEntity*>& obIntersectingList)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		CMatrix obWorldMatrix(m_obLocalMatrix);

		if (m_pobTransform)
			obWorldMatrix*=m_pobTransform->GetWorldMatrix();

		const CPoint& obPosition=obWorldMatrix.GetTranslation();
		CQuat obRotation(obWorldMatrix);
		obRotation.Normalise();
		
		hkVector4 obHKPosition(obPosition.X(),obPosition.Y(),obPosition.Z());
		hkQuaternion obHKRotation = Physics::MathsTools::CQuatTohkQuaternion(obRotation);

		hkMotionState obMotionState;
		hkTransform& obTransform  = obMotionState.getTransform();
		obTransform.setTranslation(obHKPosition);
		obTransform.setRotation(obHKRotation);

		hkCollidable obCollidable(m_pobShape,&obMotionState);
		obCollidable.setCollisionFilterInfo(m_obEntityCFlag.base);

		Physics::CPhysicsWorld::Get().GetIntersecting(&obCollidable,obIntersectingList);
	}
#else
	UNUSED( obIntersectingList );
#endif
}

void CShapePhantom::SetPosition(CPoint& obNewPosition)
{
	// This should only be used when we're not parented to something
	ntError( !m_pobTransform );

	m_obLocalMatrix.SetTranslation(obNewPosition);
}




CShapePhantomOBB::CShapePhantomOBB () 
	: CShapePhantom(),
	m_obDimensions(1.0f,1.0f,1.0f)
{
}

CShapePhantomOBB::CShapePhantomOBB (const CPoint& obPosition,const CPoint& obOrientation,const CDirection& obDimensions) 
	: CShapePhantom(obPosition, obOrientation),
	m_obDimensions(obDimensions)
{
	PostConstruct();
}

CShapePhantomOBB::~CShapePhantomOBB ()
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}
#endif
}

void CShapePhantomOBB::PostConstruct ()
{
	CShapePhantom::PostConstruct();

	// Set our local matrix

	CCamUtil::MatrixFromEuler_XYZ(m_obLocalMatrix,m_obOrientation.X()*DEG_TO_RAD_VALUE,m_obOrientation.Y()*DEG_TO_RAD_VALUE,m_obOrientation.Z()*DEG_TO_RAD_VALUE);
	m_obLocalMatrix.SetTranslation(m_obPosition);

	// Create shape
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}

	hkVector4 obExtents(m_obDimensions.X()*0.5f, m_obDimensions.Y()*0.5f, m_obDimensions.Z()*0.5f);
		
	m_pobShape=HK_NEW hkBoxShape( obExtents , 0 );

	m_obEntityCFlag.base = 0;
	m_obEntityCFlag.flags.i_am = Physics::TRIGGER_VOLUME_BIT;
	m_obEntityCFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
									Physics::CHARACTER_CONTROLLER_ENEMY_BIT		|
									Physics::RAGDOLL_BIT						|
									Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);

	// Shape is specified in world space, so convert to local space relative to parent (if specified)
	if (m_pobTransform)
	{
		m_obLocalMatrix *= m_pobTransform->GetWorldMatrix().GetAffineInverse();
	}
#endif
}

void CShapePhantomOBB::DebugRender ()
{
#ifndef _RELEASE

	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();

	CDirection obHalfExtents(m_obDimensions);
	obHalfExtents*=0.5f;

	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,0xffffffff,DPF_WIREFRAME);
	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,0x40ffffff);

	/*
	CPoint aobCorners[] = 
	{
		 CPoint(-obHalfExtents.X(), -obHalfExtents.Y(),  obHalfExtents.Z()),
		 CPoint(-obHalfExtents.X(),  obHalfExtents.Y(),  obHalfExtents.Z()),
		 CPoint(obHalfExtents.X(),  obHalfExtents.Y(),  obHalfExtents.Z()),
		 CPoint(obHalfExtents.X(), -obHalfExtents.Y(),  obHalfExtents.Z()),
		 CPoint(-obHalfExtents.X(), -obHalfExtents.Y(), -obHalfExtents.Z()),
		 CPoint(-obHalfExtents.X(),  obHalfExtents.Y(), -obHalfExtents.Z()),
		 CPoint(obHalfExtents.X(),  obHalfExtents.Y(), -obHalfExtents.Z()),
		 CPoint(obHalfExtents.X(), -obHalfExtents.Y(), -obHalfExtents.Z())
	};

	for(int i = 0; i < 8; i++)
	{
		aobCorners[i] = aobCorners[i] * obWorldMatrix;
	}

	for(int i = 0; i < 4; i++)
{
		int iWrapped = i + 1; if(iWrapped == 4) iWrapped = 0;

		CCamUtil::Render_Line(aobCorners[i], aobCorners[iWrapped], 1.0f,1.0f,1.0f,1.0f);
		CCamUtil::Render_Line(aobCorners[i+4], aobCorners[iWrapped+4], 1.0f,1.0f,1.0f,1.0f);
		CCamUtil::Render_Line(aobCorners[i], aobCorners[i+4], 1.0f,1.0f,1.0f,1.0f);
	}
	*/
#endif // _RELEASE
}

void CShapePhantomOBB::Render (u_long ulFrameColour,u_long ulBodyColour)
{
#ifdef _RELEASE
	UNUSED(ulFrameColour);
	UNUSED(ulBodyColour);
#else
	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();

	CDirection obHalfExtents(m_obDimensions);
	obHalfExtents*=0.5f;

	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,ulFrameColour,DPF_WIREFRAME);
	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,ulBodyColour);
#endif // _RELEASE
}














CShapePhantomSphere::CShapePhantomSphere () 
	: CShapePhantom(),
	m_fRadius(1.0f)
{
}

CShapePhantomSphere::CShapePhantomSphere (const CPoint& obPosition, float fRadius) 
	: CShapePhantom(obPosition, CPoint(CONSTRUCT_CLEAR)),
	m_fRadius(fRadius)
{
	PostConstruct();
}

CShapePhantomSphere::~CShapePhantomSphere ()
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}
#endif
}

void CShapePhantomSphere::PostConstruct ()
{
	CShapePhantom::PostConstruct();

	m_obLocalMatrix.SetIdentity();
	m_obLocalMatrix.SetTranslation(m_obPosition);
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}

	m_pobShape=HK_NEW hkSphereShape( m_fRadius );

	m_obEntityCFlag.base = 0;
	m_obEntityCFlag.flags.i_am = Physics::TRIGGER_VOLUME_BIT;
	m_obEntityCFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
									Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
									Physics::RAGDOLL_BIT						|
									Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);


	// Shape is specified in world space, so convert to local space relative to parent (if specified)
	if (m_pobTransform)
	{
		m_obLocalMatrix *= m_pobTransform->GetWorldMatrix().GetAffineInverse();
	}
#endif
}

void CShapePhantomSphere::DebugRender ()
{
#ifndef _RELEASE
	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();
	
	g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),obWorldMatrix.GetTranslation(),m_fRadius,0xffffffff,DPF_WIREFRAME);
	g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),obWorldMatrix.GetTranslation(),m_fRadius,0x40ffffff);
#endif //  _RELEASE
}

void CShapePhantomSphere::Render (u_long ulFrameColour,u_long ulBodyColour)
{
#ifdef _RELEASE
	UNUSED(ulFrameColour);
	UNUSED(ulBodyColour);
#else
	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();

	g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),obWorldMatrix.GetTranslation(),m_fRadius,ulFrameColour,DPF_WIREFRAME);
	g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),obWorldMatrix.GetTranslation(),m_fRadius,ulBodyColour);
#endif // _RELEASE
}

// For MrEd style implementation
CShapePhantomOBBMinMax::CShapePhantomOBBMinMax () 
	: CShapePhantom(),
	m_obMinPoint(0.0f,0.0f,0.0f),
	m_obMaxPoint(0.0f,0.0f,0.0f)
{
}

CShapePhantomOBBMinMax::CShapePhantomOBBMinMax (const CPoint& obPosition,const CPoint& obOrientation,const CDirection& obDimensions) 
	: CShapePhantom(obPosition, obOrientation),
	m_obMinPoint(0.0f,0.0f,0.0f),
	m_obMaxPoint(0.0f,0.0f,0.0f)
{
	PostConstruct();
}

CShapePhantomOBBMinMax::~CShapePhantomOBBMinMax ()
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}
#endif
}

void CShapePhantomOBBMinMax::PostConstruct ()
{
	CShapePhantom::PostConstruct();

	// Set our local matrix

	CCamUtil::MatrixFromEuler_XYZ(m_obLocalMatrix,m_obOrientation.X()*DEG_TO_RAD_VALUE,m_obOrientation.Y()*DEG_TO_RAD_VALUE,m_obOrientation.Z()*DEG_TO_RAD_VALUE);
	m_obLocalMatrix.SetTranslation(m_obPosition);

	// Create shape
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}

	// Half extents are the absolute value of the world min point minus the world translation
	hkVector4 obHalfExtents = Physics::MathsTools::CPointTohkVector(m_obMinPoint - m_obPosition);
	obHalfExtents(0) = abs(obHalfExtents(0));
	obHalfExtents(1) = abs(obHalfExtents(1));
	obHalfExtents(2) = abs(obHalfExtents(2));

	// Box will be centred on m_obPosition when used, shape is done in local space
	m_pobShape = HK_NEW hkBoxShape( obHalfExtents , 0 );

	m_obEntityCFlag.base = 0;
	m_obEntityCFlag.flags.i_am = Physics::TRIGGER_VOLUME_BIT;
	m_obEntityCFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
									Physics::CHARACTER_CONTROLLER_ENEMY_BIT		|
									Physics::RAGDOLL_BIT						|
									Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);

	// Shape is specified in world space, so convert to local space relative to parent (if specified)
	if (m_pobTransform)
	{
		m_obLocalMatrix *= m_pobTransform->GetWorldMatrix().GetAffineInverse();
	}
#endif
}

void CShapePhantomOBBMinMax::DebugRender ()
{
#ifndef _RELEASE

	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();

	// Half extents are the absolute value of the world min point minus the world translation
	CDirection obHalfExtents = CDirection( m_obMinPoint - m_obPosition );
	obHalfExtents.X() = abs(obHalfExtents.X());
	obHalfExtents.Y() = abs(obHalfExtents.Y());
	obHalfExtents.Z() = abs(obHalfExtents.Z());

	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,0xffffffff,DPF_WIREFRAME);
	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,0x40ffffff);
#endif // _RELEASE
}

void CShapePhantomOBBMinMax::Render (u_long ulFrameColour,u_long ulBodyColour)
{
#ifdef _RELEASE
	UNUSED(ulFrameColour);
	UNUSED(ulBodyColour);
#else
	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();

	// Half extents are the absolute value of the world min point minus the world translation
	CDirection obHalfExtents = CDirection( m_obMinPoint - m_obPosition );
	obHalfExtents.X() = abs(obHalfExtents.X());
	obHalfExtents.Y() = abs(obHalfExtents.Y());
	obHalfExtents.Z() = abs(obHalfExtents.Z());

	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,ulFrameColour,DPF_WIREFRAME);
	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,ulBodyColour);
#endif // _RELEASE
}












CShapePhantomCapsule::CShapePhantomCapsule () 
	: CShapePhantom(),
	m_fLength(1.0f),
	m_fRadius(0.5f)
{
}

CShapePhantomCapsule::CShapePhantomCapsule (const CPoint& obPosition, const CPoint& obOrientation, float fLength, float fRadius) 
	: CShapePhantom(obPosition, obOrientation),
	m_fLength(fLength),
	m_fRadius(fRadius)
{
	PostConstruct();
}

CShapePhantomCapsule::~CShapePhantomCapsule ()
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}
#endif
}

void CShapePhantomCapsule::PostConstruct ()
{
	CShapePhantom::PostConstruct();

	CCamUtil::MatrixFromEuler_XYZ(m_obLocalMatrix,m_obOrientation.X()*DEG_TO_RAD_VALUE,m_obOrientation.Y()*DEG_TO_RAD_VALUE,m_obOrientation.Z()*DEG_TO_RAD_VALUE);
	m_obLocalMatrix.SetTranslation(m_obPosition);
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}

	hkVector4 obPoint1(0.0f,0.0f,-m_fLength*0.5f);
	hkVector4 obPoint2(0.0f,0.0f,m_fLength*0.5f);

	m_pobShape=HK_NEW hkCapsuleShape( obPoint1,obPoint2,m_fRadius );

	m_obEntityCFlag.base = 0;
	m_obEntityCFlag.flags.i_am = Physics::TRIGGER_VOLUME_BIT;
	m_obEntityCFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
									Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
									Physics::RAGDOLL_BIT						|
									Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);


	// Shape is specified in world space, so convert to local space relative to parent (if specified)
	if (m_pobTransform)
	{
		m_obLocalMatrix *= m_pobTransform->GetWorldMatrix().GetAffineInverse();
	}
#endif
}

void CShapePhantomCapsule::DebugRender ()
{
#ifndef _RELEASE
	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();
	
	CQuat obOrientation(obWorldMatrix);

	g_VisualDebug->RenderCapsule(obOrientation,obWorldMatrix.GetTranslation(),m_fRadius,m_fLength,0xffffffff,DPF_WIREFRAME);

	g_VisualDebug->RenderCapsule(obOrientation,obWorldMatrix.GetTranslation(),m_fRadius,m_fLength,0x40ffffff);
#endif //  _RELEASE
}

void CShapePhantomCapsule::Render (u_long ulFrameColour,u_long ulBodyColour)
{
#ifdef _RELEASE
	UNUSED(ulFrameColour);
	UNUSED(ulBodyColour);
#else
	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();

	CQuat obOrientation(obWorldMatrix);

	g_VisualDebug->RenderCapsule(obOrientation,obWorldMatrix.GetTranslation(),m_fRadius,m_fLength,ulFrameColour,DPF_WIREFRAME);
	g_VisualDebug->RenderCapsule(obOrientation,obWorldMatrix.GetTranslation(),m_fRadius,m_fLength,ulBodyColour);
#endif // _RELEASE
}









CShapePhantomPortal::CShapePhantomPortal () 
	: CShapePhantom(),
	m_fWidth(1.0f),
	m_fHeight(1.0f)
{
}

CShapePhantomPortal::CShapePhantomPortal (const CPoint& obPosition, const CPoint& obOrientation, float fWidth, float fHeight) 
	: CShapePhantom(obPosition, obOrientation),
	m_fWidth(fWidth),
	m_fHeight(fHeight)
{
	PostConstruct();
}

CShapePhantomPortal::~CShapePhantomPortal ()
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}
#endif
}

void CShapePhantomPortal::PostConstruct ()
{
	CShapePhantom::PostConstruct();

	// Set our local matrix

	CCamUtil::MatrixFromEuler_XYZ(m_obLocalMatrix,m_obOrientation.X()*DEG_TO_RAD_VALUE,m_obOrientation.Y()*DEG_TO_RAD_VALUE,m_obOrientation.Z()*DEG_TO_RAD_VALUE);
	m_obLocalMatrix.SetTranslation(m_obPosition);

	// Create shape
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	if (m_pobShape)
	{
		HK_DELETE( m_pobShape );
	}

	hkVector4 obExtents(m_fWidth*0.5f, m_fHeight*0.5f, 0.1f*0.5f);
		
	m_pobShape=HK_NEW hkBoxShape( obExtents , 0 );

	m_obEntityCFlag.base = 0;
	m_obEntityCFlag.flags.i_am = Physics::TRIGGER_VOLUME_BIT;
	m_obEntityCFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
									Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
									Physics::RAGDOLL_BIT						|
									Physics::SMALL_INTERACTABLE_BIT				|
									Physics::LARGE_INTERACTABLE_BIT				);
#endif
}

void CShapePhantomPortal::DebugRender ()
{
#ifndef _RELEASE

	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();

	CDirection obHalfExtents(m_fWidth*0.5f,m_fHeight*0.5f,0.1f*0.5f);

	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,0xffffffff,DPF_WIREFRAME);
	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,0x40ffffff);

	CPoint obEndPoint=CPoint(0.0f,0.0f,0.5f) * obWorldMatrix;
	
	g_VisualDebug->RenderLine(obWorldMatrix.GetTranslation(),obEndPoint,0xffffffff,DPF_WIREFRAME);
#endif // _RELEASE
}

void CShapePhantomPortal::Render (u_long ulFrameColour,u_long ulBodyColour)
{
#ifdef _RELEASE
	UNUSED(ulFrameColour);
	UNUSED(ulBodyColour);
#else
	CMatrix obWorldMatrix(m_obLocalMatrix);

	if (m_pobTransform)
		obWorldMatrix *= m_pobTransform->GetWorldMatrix();

	CDirection obHalfExtents(m_fWidth*0.5f,m_fHeight*0.5f,0.1f*0.5f);

	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,ulFrameColour,DPF_WIREFRAME);
	g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,ulBodyColour);
	
	CPoint obEndPoint=CPoint(0.0f,0.0f,0.5f) * obWorldMatrix;
	
	g_VisualDebug->RenderLine(obWorldMatrix.GetTranslation(),obEndPoint,ulFrameColour,DPF_WIREFRAME);
#endif // _RELEASE
}

