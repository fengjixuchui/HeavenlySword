//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/xmlinterfaces.cpp
//!	
//!	DYNAMICS COMPONENT:	Intermediary structure used at load time.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.07.26
//!
//---------------------------------------------------------------------------------------------------------
#include "config.h"

#include "objectdatabase/dataobject.h"

#include "xmlinterfaces.h"

START_CHUNKED_INTERFACE(psShape, MC_PHYSICS)
	PUBLISH_ACCESSOR_WITH_DEFAULT( uint32_t, m_obMaterialID, GetMaterialId, SetMaterialId,  INVALID_MATERIAL)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psBoxShape, MC_PHYSICS)
	DEFINE_INTERFACE_INHERITANCE(psShape)
	COPY_INTERFACE_FROM(psShape)

	PUBLISH_ACCESSOR( CPoint, m_fTranslation, GetPosition, SetPosition )
	PUBLISH_ACCESSOR( CQuat, m_fRotation, GetRotation, SetRotation )
	PUBLISH_ACCESSOR( CVector, m_obHalfExtent, GetHalfExtent, SetHalfExtent )
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psCapsuleShape, MC_PHYSICS)
	DEFINE_INTERFACE_INHERITANCE(psShape)
	COPY_INTERFACE_FROM(psShape)

	PUBLISH_ACCESSOR( CPoint, m_fTranslation, GetPosition, SetPosition )
	PUBLISH_ACCESSOR( CQuat, m_fRotation, GetRotation, SetRotation )
	PUBLISH_ACCESSOR( CVector, m_obVertexA, GetVertexA, SetVertexA )
	PUBLISH_ACCESSOR( CVector, m_obVertexB, GetVertexB, SetVertexB )
	PUBLISH_VAR_AS( m_fRadius, m_fRadius )
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psCylinderShape, MC_PHYSICS)
	DEFINE_INTERFACE_INHERITANCE(psShape)
	COPY_INTERFACE_FROM(psShape)

	PUBLISH_ACCESSOR( CPoint, m_fTranslation, GetPosition, SetPosition )
	PUBLISH_ACCESSOR( CQuat, m_fRotation, GetRotation, SetRotation )
	PUBLISH_ACCESSOR( CVector, m_obVertexA, GetVertexA, SetVertexA )
	PUBLISH_ACCESSOR( CVector, m_obVertexB, GetVertexB, SetVertexB )
	PUBLISH_VAR_AS( m_fRadius, m_fRadius )
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psSphereShape, MC_PHYSICS)
	DEFINE_INTERFACE_INHERITANCE(psShape)
	COPY_INTERFACE_FROM(psShape)

	PUBLISH_ACCESSOR( CPoint, m_fTranslation, GetPosition, SetPosition )
	PUBLISH_ACCESSOR( CQuat, m_fRotation, GetRotation, SetRotation )
	PUBLISH_VAR_AS( m_fRadius, m_fRadius )
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psListShape, MC_PHYSICS)
	DEFINE_INTERFACE_INHERITANCE(psShape)
	COPY_INTERFACE_FROM(psShape)

	PUBLISH_PTR_CONTAINER_AS( m_shapesGuids, m_shapesGuids )
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psConvexVerticesShape, MC_PHYSICS)
	DEFINE_INTERFACE_INHERITANCE(psShape)
	COPY_INTERFACE_FROM(psShape)

	PUBLISH_ACCESSOR( CPoint, m_fTranslation, GetPosition, SetPosition )
	PUBLISH_ACCESSOR( CQuat, m_fRotation, GetRotation, SetRotation )
	PUBLISH_VAR_AS( m_eStride, m_eStride )
	PUBLISH_VAR_AS( m_iNumVertices, m_iNumVertices )
	PUBLISH_CONTAINER_AS( m_obVertexBuffer, m_obVertexBuffer )      // was PUBLISH_VAR_AS
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psMeshShape, MC_PHYSICS)
	DEFINE_INTERFACE_INHERITANCE(psShape)
	COPY_INTERFACE_FROM(psShape)

	PUBLISH_ACCESSOR( CPoint, m_fTranslation, GetPosition, SetPosition )
	PUBLISH_ACCESSOR( CQuat, m_fRotation, GetRotation, SetRotation )
	PUBLISH_VAR_AS( m_eStride, m_eStride )
	PUBLISH_VAR_AS( m_iNumVertices, m_iNumVertices )
	PUBLISH_VAR_AS( m_iIndices, m_iIndices )
	PUBLISH_CONTAINER_AS( m_obIndexBuffer, m_obIndexBuffer )		// was PUBLISH_VAR_AS
	PUBLISH_CONTAINER_AS( m_obVertexBuffer, m_obVertexBuffer )      // was PUBLISH_VAR_AS
	PUBLISH_CONTAINER_AS( m_obMaterialBuffer, m_obMaterialBuffer )  // was PUBLISH_VAR_AS
END_STD_INTERFACE


START_CHUNKED_INTERFACE(psRigidBody, MC_PHYSICS)
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	PUBLISH_VAR_AS( m_info.m_rigidBodyCinfo.m_mass, m_fMass )
	PUBLISH_VAR_AS( m_info.m_rigidBodyCinfo.m_friction, m_fFriction )
	PUBLISH_VAR_AS( m_info.m_rigidBodyCinfo.m_restitution, m_fRestitution )
	PUBLISH_VAR_AS( m_info.m_rigidBodyCinfo.m_linearDamping, m_fLinearDamping )
	PUBLISH_VAR_AS( m_info.m_rigidBodyCinfo.m_angularDamping, m_fAngularDamping )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_info.m_rigidBodyCinfo.m_maxLinearVelocity, 200, m_fMaxLinearVelocity  )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_info.m_rigidBodyCinfo.m_maxAngularVelocity, 200, m_fMaxAngularVelocity  )
	PUBLISH_ACCESSOR_WITH_DEFAULT( CPoint, m_fMassCenterOffset, GetMassCenterOffset, SetMassCenterOffset, CPoint(0,0,0) )
#endif
	PUBLISH_VAR_AS( m_eMotionType, m_eMotionType )
	PUBLISH_VAR_AS( m_eQualityType, m_eQualityType )
	PUBLISH_ACCESSOR( CPoint, m_fTranslation, GetPosition, SetPosition )
	PUBLISH_ACCESSOR( CQuat, m_fRotation, GetRotation, SetRotation )

	PUBLISH_PTR_AS	( m_shape,		m_shapesGuid )
	PUBLISH_VAR_AS( m_uiTransformHash, m_uiTransformHash )

	/*PUBLISH_VAR_AS( m_obInertia, m_obInertia )
	PUBLISH_VAR_AS( m_bInactive, m_bInactive )
	PUBLISH_VAR_AS( m_bDisabled, m_bDisabled )	
	PUBLISH_VAR_AS( m_bBoundingVolume, m_bBoundingVolume )
	PUBLISH_VAR_AS( m_iNumShapes, m_iNumShapes )
	PUBLISH_VAR_AS( m_pcMaterialName, m_pcMaterialName )*/
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psConstraint_Hinge, MC_PHYSICS)
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	PUBLISH_PTR_AS	( m_BodyA,		m_BodyAGuid )
	PUBLISH_PTR_AS	( m_BodyB,		m_BodyBGuid )
	PUBLISH_ACCESSOR( CPoint, m_fPivotA, GetPivotA, SetPivotA )
	PUBLISH_ACCESSOR( CPoint, m_fPivotB, GetPivotB, SetPivotB )
	PUBLISH_ACCESSOR( CVector, m_vAxisA, GetAxisA, SetAxisA )
	PUBLISH_ACCESSOR( CVector, m_vAxisB, GetAxisB, SetAxisB )
	PUBLISH_ACCESSOR( CVector, m_vPerpAxisA, GetPerpAxisA, SetPerpAxisA )
	PUBLISH_ACCESSOR( CVector, m_vPerpAxisB, GetPerpAxisB, SetPerpAxisB )
	PUBLISH_VAR_AS( m_bIsLimited, m_bIsLimited )
	PUBLISH_VAR_AS( m_bBreakable, m_bBreakable )
	PUBLISH_VAR_AS( m_fLinearBreakingStrength, m_fLinearBreakingStrength )
	PUBLISH_VAR_AS( m_FRange, m_FRange )
	PUBLISH_VAR_AS( m_fMaxLimitAngleA, m_fMaxLimitAngleA )
#endif
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psConstraint_P2P, MC_PHYSICS)
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	PUBLISH_PTR_AS	( m_BodyA,		m_BodyAGuid )
	PUBLISH_PTR_AS	( m_BodyB,		m_BodyBGuid )
	PUBLISH_ACCESSOR( CPoint, m_fPivotA, GetPivotA, SetPivotA )
	PUBLISH_ACCESSOR( CPoint, m_fPivotB, GetPivotB, SetPivotB )
	PUBLISH_VAR_AS( m_bBreakable, m_bBreakable )
	PUBLISH_VAR_AS( m_fLinearBreakingStrength, m_fLinearBreakingStrength )
	PUBLISH_VAR_AS( m_iType, m_iType )
#endif
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psConstraint_Spring, MC_PHYSICS)
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	PUBLISH_PTR_AS	( m_BodyA,		m_BodyAGuid )
	PUBLISH_PTR_AS	( m_BodyB,		m_BodyBGuid )
	PUBLISH_ACCESSOR( CPoint, m_fPivotA, GetPivotA, SetPivotA )
	PUBLISH_ACCESSOR( CPoint, m_fPivotB, GetPivotB, SetPivotB )
	PUBLISH_VAR_AS( m_fRestLength, m_fRestLength )
	PUBLISH_VAR_AS( m_fStiffness, m_fStiffness )
	PUBLISH_VAR_AS( m_fDamping, m_fDamping )
	PUBLISH_VAR_AS( m_bActOnCompression, m_bActOnCompression )
	PUBLISH_VAR_AS( m_bActOnExtension, m_bActOnExtension )
#endif
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psConstraint_StiffSpring, MC_PHYSICS)
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	PUBLISH_PTR_AS	( m_BodyA,		m_BodyAGuid )
	PUBLISH_PTR_AS	( m_BodyB,		m_BodyBGuid )
	PUBLISH_ACCESSOR( CPoint, m_fPivotA, GetPivotA, SetPivotA )
	PUBLISH_ACCESSOR( CPoint, m_fPivotB, GetPivotB, SetPivotB )
	PUBLISH_VAR_AS( m_bBreakable, m_bBreakable )
	PUBLISH_VAR_AS( m_fLinearBreakingStrength, m_fLinearBreakingStrength )
	PUBLISH_VAR_AS( m_fRestLength, m_fRestLength )
#endif
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psConstraint_Ragdoll, MC_PHYSICS)
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	PUBLISH_PTR_AS	( m_BodyA,		m_BodyAGuid )
	PUBLISH_PTR_AS	( m_BodyB,		m_BodyBGuid )
	PUBLISH_ACCESSOR( CPoint, m_fPivotA, GetPivotA, SetPivotA )
	PUBLISH_ACCESSOR( CPoint, m_fPivotB, GetPivotB, SetPivotB )
	PUBLISH_ACCESSOR( CVector, m_vTwistAxisA, GetTwistA, SetTwistA )
	PUBLISH_ACCESSOR( CVector, m_vTwistAxisB, GetTwistB, SetTwistB )
	PUBLISH_ACCESSOR( CVector, m_vPlaneAxisA, GetPlaneAxisA, SetPlaneAxisA )
	PUBLISH_ACCESSOR( CVector, m_vPlaneAxisB, GetPlaneAxisB, SetPlaneAxisB )
	PUBLISH_VAR_AS( m_bBreakable, m_bBreakable )
	PUBLISH_VAR_AS( m_fLinearBreakingStrength, m_fLinearBreakingStrength )
	PUBLISH_VAR_AS( m_fTwistMin, m_fTwistMin )
	PUBLISH_VAR_AS( m_fTwistMax, m_fTwistMax )
	PUBLISH_VAR_AS( m_fConeMin, m_fConeMin )
	PUBLISH_VAR_AS( m_fConeMax, m_fConeMax )
	PUBLISH_VAR_AS( m_fPlaneMin, m_fPlaneMin )
	PUBLISH_VAR_AS( m_fPlaneMax, m_fPlaneMax )
	PUBLISH_VAR_AS( m_fFriction, m_fFriction )
#endif
END_STD_INTERFACE

START_CHUNKED_INTERFACE(psShapeTrfLink, MC_PHYSICS)
	PUBLISH_VAR_AS( m_trfIndex, m_trfIndex )
	PUBLISH_PTR_AS	( m_shape,	m_shapeGuid )
END_STD_INTERFACE

void ForceLinkFunction10()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction10() !ATTN!\n");
}

//Creates hkConvexListShape if it is possible, hkListShape otherwise. 
//BEWARE: Function also removeReference() from input shapes. 
hkShape * CreateListShape(hkShape ** list, int n)
{	
	// Ticket#: 619-2531546 
	// hkConvexListShape produces instability, let's use hkListShape till they solve the problem
	hkShape * retShape;
	retShape = HK_NEW hkListShape(list,n);

	//if (!AreReadyForConvexListShape(list,n))
	//{
	//	retShape = HK_NEW hkListShape(list,n);
	//}
	//else
	//{
	//	CScopedArray<hkConvexShape *> convexList(NT_NEW hkConvexShape*[ n ]);
	//	for(int i = 0; i < n; i++)
	//	{
	//	  convexList[i] = static_cast<hkConvexShape *>(list[i]);
	//	}
	//   retShape = HK_NEW hkConvexListShape(*convexList,n);	
	//);

	for(int i = 0; i < n; i++)
	{
		list[i]->removeReference();
	}

	retShape->setUserData(INVALID_MATERIAL);
	return retShape;
};

//Creates hkConvexTransformShape if shape is convex, hkTransformShape otherwise.
//BEWARE: Function also removeReference() from input shape. 
hkShape * CreateTransformShape(hkShape *shape, const hkTransform& trf)
{
	hkShape* trfShape;
	if (IsConvex(shape))
		trfShape = HK_NEW hkConvexTransformShape( static_cast<hkConvexShape *>(shape), trf );
	else
		trfShape = HK_NEW hkTransformShape( shape, trf );

	shape->removeReference();
	trfShape->setUserData(shape->getUserData());
	return trfShape;
};

hkConstraintInstance* psConstraint_Ragdoll::BuildConstraint( CEntity* pentity )
{
#ifdef CONSTRAINT_CLUMP_SPACE
	// If we're using a ragdoll clump, make sure we have the hierarchy
	if(m_bUseRagdollHierarchy)
	{
		ntError_p(pentity->IsCharacter(), ("psConstraint_Ragdoll::BuildConstraint - Entity is not a Character\n"));
		ntError_p(((Character*)pentity)->GetRagdollHierarchy(),("Tried to use ragdoll hierarchy but there is no ragdoll hierarchy on %s", ntStr::GetString( ObjectDatabase::Get().GetNameFromPointer(pentity) )));
	}

	hkConstraintData* data = 0;
	hkRagdollConstraintData* data2 = HK_NEW hkRagdollConstraintData;

	CMatrix m2;
	if (m_bUseRagdollHierarchy)
	{
		Character* pCharacter = (Character*)pentity;
		m2.SetFromQuat(		pCharacter->GetRagdollHierarchy()->GetBindPoseJointRotation( 0 ) );
		m2.SetTranslation(	pCharacter->GetRagdollHierarchy()->GetBindPoseJointTranslation( 0 ) );
		m2 = m2.GetFullInverse();
	}
	else
	{
		m2.SetFromQuat(		pentity->GetHierarchy()->GetBindPoseJointRotation( 0 ) );
		m2.SetTranslation(	pentity->GetHierarchy()->GetBindPoseJointTranslation( 0 ) );
		m2 = m2.GetFullInverse();
	}

	m_fPivotA = m_fPivotA * m2;

	CMatrix m;
	if (m_bUseRagdollHierarchy)
		m = ((Character*)pentity)->GetRagdollHierarchy()->GetRootTransform()->GetWorldMatrix();
	else
		m = pentity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();

	m_fPivotA = m_fPivotA * m;
	m.SetTranslation( CPoint(0.0f, 0.0f, 0.0f) );
	m_vPlaneAxisA = m_vPlaneAxisA * m;
	m_vTwistAxisA = m_vTwistAxisA * m;

	hkTransform transformOutA;
	m_BodyA->m_associatedRigid->approxTransformAt( Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutA ); 
	hkTransform transformOutB;
	m_BodyB->m_associatedRigid->approxTransformAt( Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutB ); 

	data2->setInWorldSpace(	transformOutA, transformOutB,
							Physics::MathsTools::CPointTohkVector(m_fPivotA), 
							Physics::MathsTools::CVectorTohkVector(m_vPlaneAxisA),
							Physics::MathsTools::CVectorTohkVector(m_vTwistAxisA) );  
	data2->setAsymmetricConeAngle( m_fConeMin, m_fConeMax);
	data2->setMaxFrictionTorque( m_fFriction );
	data2->setPlaneMaxAngularLimit( m_fPlaneMax );
	data2->setPlaneMinAngularLimit( m_fPlaneMin );
	data2->setTwistMaxAngularLimit( m_fTwistMax );
	data2->setTwistMinAngularLimit( m_fTwistMin );

	data = data2;


#else
	hkConstraintData* data = 0;
	hkRagdollConstraintData* data2 = HK_NEW hkRagdollConstraintData;	
	
	data2->setInBodySpace(	Physics::MathsTools::CPointTohkVector(m_fPivotA),
							Physics::MathsTools::CPointTohkVector(m_fPivotB),
							Physics::MathsTools::CVectorTohkVector(m_vPlaneAxisA),
							Physics::MathsTools::CVectorTohkVector(m_vPlaneAxisB),
							Physics::MathsTools::CVectorTohkVector(m_vTwistAxisA),
							Physics::MathsTools::CVectorTohkVector(m_vTwistAxisB));  

	data2->setAsymmetricConeAngle( m_fConeMin, m_fConeMax);
	data2->setMaxFrictionTorque( m_fFriction );
	data2->setPlaneMaxAngularLimit( m_fPlaneMax );
	data2->setPlaneMinAngularLimit( m_fPlaneMin );
	data2->setTwistMaxAngularLimit( m_fTwistMax );
	data2->setTwistMinAngularLimit( m_fTwistMin );

	data = data2;
#endif

	if( m_bBreakable && !m_bUseRagdollHierarchy)
	{
		hkBreakableConstraintData* breaker = HK_NEW hkBreakableConstraintData( data, Physics::CPhysicsWorld::Get().GetHavokWorldP() );
		breaker->setThreshold( m_fLinearBreakingStrength );
		breaker->setRemoveWhenBroken(true);
		data = breaker;

	}
	
	m_pobHavokConstraint = HK_NEW hkConstraintInstance( m_BodyA->m_associatedRigid,  m_BodyB->m_associatedRigid, data );
	return m_pobHavokConstraint;

}

hkConstraintInstance* psConstraint_Hinge::BuildConstraint( CEntity* pentity )
{
#ifdef CONSTRAINT_CLUMP_SPACE
	// If we're using a ragdoll clump, make sure we have the hierarchy
	if (m_bUseRagdollHierarchy)
	{
		ntError_p(pentity->IsCharacter(), ("psConstraint_Ragdoll::BuildConstraint - Entity is not a Character\n"));
		ntError_p(((Character*)pentity)->GetRagdollHierarchy(),("Tried to use ragdoll hierarchy but there is no ragdoll hierarchy on %s", ntStr::GetString( ObjectDatabase::Get().GetNameFromPointer(pentity) )));
	}

	hkConstraintData* data;

	// Need more parameters!
	if( m_bIsLimited )
	{
		hkLimitedHingeConstraintData* data2 = HK_NEW hkLimitedHingeConstraintData;

		CPoint pivot = m_fPivotA + m_fPivotB;
		pivot *= 0.5f;
		
		
		CMatrix m2;
		if (m_bUseRagdollHierarchy)
		{
			m2.SetFromQuat(		((Character*)pentity)->GetRagdollHierarchy()->GetBindPoseJointRotation( 0 ) );
			m2.SetTranslation(	((Character*)pentity)->GetRagdollHierarchy()->GetBindPoseJointTranslation( 0 ) );
			m2 = m2.GetFullInverse();
		}
		else
		{
			m2.SetFromQuat(		pentity->GetHierarchy()->GetBindPoseJointRotation( 0 ) );
			m2.SetTranslation(	pentity->GetHierarchy()->GetBindPoseJointTranslation( 0 ) );
			m2 = m2.GetFullInverse();
		}
		pivot = pivot * m2;
		
		
		CMatrix m;
		if (m_bUseRagdollHierarchy)
		{
			m = ((Character*)pentity)->GetRagdollHierarchy()->GetRootTransform()->GetWorldMatrix();
		}
		else
		{
			m = pentity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
		}
		pivot = pivot * m;

		
		float fMaxLimit = m_fMaxLimitAngleA;
		float fMinLimit = m_fMaxLimitAngleA - m_FRange;
		if ( fMinLimit < -PI )
		{
			fMaxLimit = fMinLimit;
			fMinLimit = m_fMaxLimitAngleA;
			m_fMaxLimitAngleA = fMaxLimit;
		}
		data2->setMaxAngularLimit( fMaxLimit ); 
		data2->setMinAngularLimit( fMinLimit );
		
		
		CVector axis = m_vAxisA;
		m.SetTranslation( CPoint(0.0f, 0.0f, 0.0f) );
		axis = axis * m;
		
		hkRotation r; r.setIdentity();
		hkTransform transformOutA;
		m_BodyA->m_associatedRigid->approxTransformAt( Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutA); 
		//transformOutA.setRotation( r );
		hkTransform transformOutB;
		m_BodyB->m_associatedRigid->approxTransformAt(  Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutB); 
		//transformOutB.setRotation( r );
		data2->setInWorldSpace( transformOutA, 
								transformOutB, 
								Physics::MathsTools::CPointTohkVector(pivot),
								Physics::MathsTools::CVectorTohkVector(axis));
		
		data = data2;
	} 
	else
	{
		hkHingeConstraintData* data2 = HK_NEW hkHingeConstraintData;

		CPoint pivot = m_fPivotA + m_fPivotB;
		pivot *= 0.5f;

		CMatrix m2;
		if (m_bUseRagdollHierarchy)
		{
			m2.SetFromQuat(		((Character*)pentity)->GetRagdollHierarchy()->GetBindPoseJointRotation( 0 ) );
			m2.SetTranslation(	((Character*)pentity)->GetRagdollHierarchy()->GetBindPoseJointTranslation( 0 ) );
			m2 = m2.GetFullInverse();
		}
		else
		{
			m2.SetFromQuat(		pentity->GetHierarchy()->GetBindPoseJointRotation( 0 ) );
			m2.SetTranslation(	pentity->GetHierarchy()->GetBindPoseJointTranslation( 0 ) );
			m2 = m2.GetFullInverse();
		}

		pivot = pivot * m2;

		CMatrix m;
		if (m_bUseRagdollHierarchy)
			m = ((Character*)pentity)->GetRagdollHierarchy()->GetRootTransform()->GetWorldMatrix();
		else
			m = pentity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();

		pivot = pivot * m;

		CVector axis = m_vAxisA;
		m.SetTranslation( CPoint(0.0f, 0.0f, 0.0f) );
		axis = axis * m;

		hkTransform transformOutA;
		m_BodyA->m_associatedRigid->approxTransformAt( Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutA); 
		hkTransform transformOutB;
		m_BodyB->m_associatedRigid->approxTransformAt(  Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutB); 
		data2->setInWorldSpace( transformOutA, 
								transformOutB, 
								Physics::MathsTools::CPointTohkVector(pivot),
								Physics::MathsTools::CVectorTohkVector(axis));

		data = data2;
	}
#else	
	hkConstraintData* data;

	// Need more parameters!
	if( m_bIsLimited )
	{
		hkLimitedHingeConstraintData* data2 = HK_NEW hkLimitedHingeConstraintData;		
				
		float fMaxLimit = m_fMaxLimitAngleA;
		float fMinLimit = m_fMaxLimitAngleA - m_FRange;
		if ( fMinLimit < -PI )
		{
			fMaxLimit = fMinLimit;
			fMinLimit = m_fMaxLimitAngleA;
			m_fMaxLimitAngleA = fMaxLimit;
		}
		data2->setMaxAngularLimit( fMaxLimit ); 
		data2->setMinAngularLimit( fMinLimit );
			
		data2->setInBodySpace( Physics::MathsTools::CPointTohkVector(m_fPivotA), 
							   Physics::MathsTools::CPointTohkVector(m_fPivotB), 
							   Physics::MathsTools::CVectorTohkVector(m_vAxisA),
							   Physics::MathsTools::CVectorTohkVector(m_vAxisB),
							   Physics::MathsTools::CVectorTohkVector(m_vPerpAxisA),
							   Physics::MathsTools::CVectorTohkVector(m_vPerpAxisB));
		
		//data2->setMaxAngularLimit( m_FRange ); 
		//data2->setMinAngularLimit( m_fMaxLimitAngleA );
		data = data2;
	} 
	else
	{	
		hkLimitedHingeConstraintData* data2 = HK_NEW hkLimitedHingeConstraintData;	
		data2->setInBodySpace( Physics::MathsTools::CPointTohkVector(m_fPivotA), 
							   Physics::MathsTools::CPointTohkVector(m_fPivotB), 
							   Physics::MathsTools::CVectorTohkVector(m_vAxisA),
							   Physics::MathsTools::CVectorTohkVector(m_vAxisB),
							   Physics::MathsTools::CVectorTohkVector(m_vPerpAxisA),
							   Physics::MathsTools::CVectorTohkVector(m_vPerpAxisB));
		data = data2;
	}
#endif

	if( m_bBreakable && !m_bUseRagdollHierarchy)
	{
		hkBreakableConstraintData* breaker = HK_NEW hkBreakableConstraintData( data, Physics::CPhysicsWorld::Get().GetHavokWorldP() );
		breaker->setThreshold( m_fLinearBreakingStrength );
		breaker->setRemoveWhenBroken(true);
		data = breaker;

	}

	m_pobHavokConstraint =  HK_NEW hkConstraintInstance( m_BodyA->m_associatedRigid,  m_BodyB->m_associatedRigid, data );
	return m_pobHavokConstraint;
}

#define THIN_BOX_INERTIA_MAX_FRAC 0.3f
void ComputeCentreOfMassAndInertiaTensor (hkShape* pobShape, hkRigidBodyCinfo& obInfo)
{
	// This function sets the centre of mass to the centre of the object based on its AABB.
	// It then sets the inertia tensor to ensure the mass is properly distributed.

	// Calculate the centre of mass
	hkAabb obAABB;
	pobShape->getAabb(hkTransform::getIdentity(),0.0f,obAABB);
	obInfo.m_centerOfMass(0) += (obAABB.m_min(0) + obAABB.m_max(0)) * 0.5f;
	obInfo.m_centerOfMass(1) += (obAABB.m_min(1) + obAABB.m_max(1)) * 0.5f;
	obInfo.m_centerOfMass(2) += (obAABB.m_min(2) + obAABB.m_max(2)) * 0.5f;

	// Calculate inertia tensor (weight distribution)
	hkVector4 obHalfExtents;
	obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
	obHalfExtents.mul4(0.5f);

	hkMassProperties obMassProperties;
	obMassProperties.m_centerOfMass = obInfo.m_centerOfMass;
	obMassProperties.m_mass = obInfo.m_mass;

	const hkShape* pobChildShape = 0;
	if (pobShape->getType() == HK_SHAPE_TRANSFORM)
	{
		hkTransformShape* pobConvex = (hkTransformShape*)pobShape;
		pobChildShape = pobConvex->getChildShape();
	} 
	else if (pobShape->getType() == HK_SHAPE_CONVEX_TRANSFORM)
	{
		hkConvexTransformShape* pobConvex = (hkConvexTransformShape*)pobShape;
		pobChildShape = pobConvex->getChildShape();
	}

	if (pobShape->getType() == HK_SHAPE_CAPSULE)
	{
		hkCapsuleShape* pobCapsule = (hkCapsuleShape*)pobShape;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobCapsule->getVertex(0), pobCapsule->getVertex(1), pobCapsule->getRadius(), obInfo.m_mass, obMassProperties);
	}
	else if (pobChildShape && pobChildShape->getType() == HK_SHAPE_CAPSULE)
	{
		hkCapsuleShape* pobCapsule = (hkCapsuleShape*)pobChildShape;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobCapsule->getVertex(0), pobCapsule->getVertex(1), pobCapsule->getRadius(), obInfo.m_mass, obMassProperties);
	}
	else
	{
		// fight instability for thin_boxes --> fake box shape sizes
		float maxSize = max(obHalfExtents(0), max(obHalfExtents(1), obHalfExtents(2)));
		float invMaxSize = 1.0f / maxSize;

		for(int i = 0; i < 3; i++)
		{
			if (obHalfExtents(i) * invMaxSize < THIN_BOX_INERTIA_MAX_FRAC)
				obHalfExtents(i) = maxSize * THIN_BOX_INERTIA_MAX_FRAC;
		}

		hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,obInfo.m_mass,obMassProperties);
	}

	obInfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;

}

void ComputeCentreOfMassAndInertiaTensorTransformed (hkShape* pobShape, hkRigidBodyCinfo& obInfo, hkTransform& obToto)
{
	// This function sets the centre of mass to the centre of the object based on its AABB.
	// It then sets the inertia tensor to ensure the mass is properly distributed.

	// Calculate the centre of mass
	hkAabb obAABB;
	pobShape->getAabb(obToto,0.0f,obAABB);
	obInfo.m_centerOfMass(0) +=(obAABB.m_min(0) + obAABB.m_max(0)) * 0.5f;
	obInfo.m_centerOfMass(1) +=(obAABB.m_min(1) + obAABB.m_max(1)) * 0.5f;
	obInfo.m_centerOfMass(2) +=(obAABB.m_min(2) + obAABB.m_max(2)) * 0.5f;

	// Calculate inertia tensor (weight distribution)
	hkVector4 obHalfExtents;
	obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
	obHalfExtents.mul4(0.5f);

	hkMassProperties obMassProperties;
	obMassProperties.m_centerOfMass = obInfo.m_centerOfMass;
	obMassProperties.m_mass = obInfo.m_mass;
	hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,obInfo.m_mass,obMassProperties);
	obInfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;

}



Physics::BodyCInfo psRigidBody::GetBodyInfo(const char* name, CHierarchy* hierarchy, hkRigidBodyCinfo * pSrcInfo)
{
	m_info.m_name = name;
	if (hierarchy)
	{
	m_info.m_transform = hierarchy->GetRootTransform();

	if( hierarchy->GetTransformFromHash(m_uiTransformHash) != 0 )
	{
		m_info.m_transform = hierarchy->GetTransformFromHash(m_uiTransformHash);
	}
	}
	else
	{
		m_info.m_transform = 0; 
	}

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD	
	
	if (pSrcInfo)
	{
		// override some body values with values suggested by source info
		m_info.m_rigidBodyCinfo.m_motionType = pSrcInfo->m_motionType;		
		m_info.m_rigidBodyCinfo.m_qualityType = pSrcInfo->m_qualityType;	
	}
	else
	{
		if( KEYFRAMED == m_eMotionType )
		{
			m_info.m_rigidBodyCinfo.m_motionType = hkMotion::MOTION_KEYFRAMED;
		} else if( DYNAMIC == m_eMotionType ) {
			m_info.m_rigidBodyCinfo.m_motionType = hkMotion::MOTION_DYNAMIC;
		}

		switch( m_eQualityType )
		{

		case HS_COLLIDABLE_QUALITY_FIXED:
			m_info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;
			break;
		case HS_COLLIDABLE_QUALITY_KEYFRAMED:
			m_info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_KEYFRAMED;
			break;
		case HS_COLLIDABLE_QUALITY_DEBRIS:
			m_info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_DEBRIS;
			break;
		case HS_COLLIDABLE_QUALITY_MOVING:
			m_info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
			break;
		case HS_COLLIDABLE_QUALITY_CRITICAL:
			m_info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
			break;
		case HS_COLLIDABLE_QUALITY_BULLET:
			m_info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_BULLET;
			break;
		default:
			m_info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
			break;
		}
	}

	if( m_info.m_rigidBodyCinfo.m_mass == 0.0f )
	{
		m_info.m_rigidBodyCinfo.m_motionType = hkMotion::MOTION_FIXED;
		m_info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;
	}
	else
		m_info.m_rigidBodyCinfo.m_centerOfMass = Physics::MathsTools::CPointTohkVector(m_fMassCenterOffset);
		
	hkShape* build = m_shape->BuildShape();
	
	// Use the size of the shape to determine small or largeness
	hkAabb obAabb;
	build->getAabb( hkTransform::getIdentity(), 0.01f, obAabb );
	hkVector4 obCentre(obAabb.m_min);
	obCentre.add4(obAabb.m_max);
	obCentre.mul4(0.5f);

	float fHalfExtentX = obCentre(0) - obAabb.m_min(0);
	float fHalfExtentY = obCentre(1) - obAabb.m_min(1);
	float fHalfExtentZ = obCentre(2) - obAabb.m_min(2);

	Physics::EntityCollisionFlag obCollisionFlag;
	obCollisionFlag.base = 0;

	if ((fHalfExtentX * fHalfExtentY * fHalfExtentZ) * 2.0f > LARGE_INTERACTABLE_VOLUME_THRESHOLD)
		obCollisionFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
	else
		obCollisionFlag.flags.i_am = Physics::SMALL_INTERACTABLE_BIT;

	obCollisionFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
												Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
												Physics::RAGDOLL_BIT						|
												Physics::SMALL_INTERACTABLE_BIT				|
												Physics::LARGE_INTERACTABLE_BIT				|
												Physics::AI_WALL_BIT);

	m_info.m_rigidBodyCinfo.m_collisionFilterInfo = obCollisionFlag.base;

	// Need to see if this is a box, and if it is, if it is thin, and if it is whether we need to restrict the angular velocity so it doesn't go mental
	// NOTE:	Only do this if the mass > 0 as it won't be moving if the mass == 0... and Havok complains if the mass is zero and the motion type
	//			isn't fixed. [ARV].
	if ( m_info.m_rigidBodyCinfo.m_mass > 0.0f )
	{
		hkShape* pobTestShape = build;
		if ( pobTestShape->getType() == HK_SHAPE_BOX ||
			pobTestShape->getType() == HK_SHAPE_TRANSFORM )
		{
			psBoxShape* pobHSBoxShape = 0;
			if (pobTestShape->getType() == HK_SHAPE_BOX)
			{
				pobHSBoxShape = (psBoxShape*)m_shape;
			}
			else
			{
				hkTransformShape* pobTestTransformShape = (hkTransformShape*)pobTestShape;
				if (pobTestTransformShape->getChildShape()->getType() == HK_SHAPE_BOX)
					pobHSBoxShape = (psBoxShape*)m_shape;
			}

			if (pobHSBoxShape)
			{
				// Check extents to see if it's thin, and restrict angular velocity accordingly if so
				float x = pobHSBoxShape->m_obHalfExtent.X();
				float y = pobHSBoxShape->m_obHalfExtent.Y();
				float z = pobHSBoxShape->m_obHalfExtent.Z();

				// If the relative size of extents is over a certain value, we need to treat this box specially
				float fThinThreshold = 3.0f;
				if ((x/y > fThinThreshold || y/x > fThinThreshold) ||
					(x/z > fThinThreshold || z/x > fThinThreshold) ||
					(y/z > fThinThreshold || z/y > fThinThreshold) )
				{
					m_info.m_rigidBodyCinfo.m_motionType = hkMotion::MOTION_THIN_BOX_INERTIA;
				}
			}
		}
	}

	if ( !TransformWouldBeIdentity( m_fRotation, m_fTranslation ) )
	{
		hkTransform trf	= hkTransform( Physics::MathsTools::CQuatTohkQuaternion( m_fRotation ) , 
				Physics::MathsTools::CPointTohkVector( m_fTranslation ) );

		build = CreateTransformShape(build, trf );		
	}	

	if( m_info.m_rigidBodyCinfo.m_mass != 0.0f )
		ComputeCentreOfMassAndInertiaTensor( build, m_info.m_rigidBodyCinfo);

	m_info.m_rigidBodyCinfo.m_shape = build;
#endif
	return m_info;
}

Physics::BodyCInfo psRigidBody::GetStaticBodyInfo(const char* name, CHierarchy* hierarchy)
{
	m_info.m_name = name;
	m_info.m_transform = hierarchy->GetRootTransform();

	if( hierarchy->GetTransformFromHash(m_uiTransformHash) != 0 )
	{
		m_info.m_transform = hierarchy->GetTransformFromHash(m_uiTransformHash);
	}

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD	
	
	m_info.m_rigidBodyCinfo.m_motionType = hkMotion::MOTION_FIXED;
	m_info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;

	hkShape* build = m_shape->BuildStaticShape();
	
	// Use the size of the shape to determine small or largeness
	hkAabb obAabb;
	build->getAabb( hkTransform::getIdentity(), 0.01f, obAabb );
	hkVector4 obCentre(obAabb.m_min);
	obCentre.add4(obAabb.m_max);
	obCentre.mul4(0.5f);

	float fHalfExtentX = obCentre(0) - obAabb.m_min(0);
	float fHalfExtentY = obCentre(1) - obAabb.m_min(1);
	float fHalfExtentZ = obCentre(2) - obAabb.m_min(2);

	Physics::EntityCollisionFlag obCollisionFlag;
	obCollisionFlag.base = 0;

	if ((fHalfExtentX * fHalfExtentY * fHalfExtentZ) * 2.0f > LARGE_INTERACTABLE_VOLUME_THRESHOLD)
		obCollisionFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
	else
		obCollisionFlag.flags.i_am = Physics::SMALL_INTERACTABLE_BIT;

	obCollisionFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
												Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
												Physics::RAGDOLL_BIT						|
												Physics::SMALL_INTERACTABLE_BIT				|
												Physics::LARGE_INTERACTABLE_BIT				|
												Physics::AI_WALL_BIT);

	m_info.m_rigidBodyCinfo.m_collisionFilterInfo = obCollisionFlag.base;

	if ( !TransformWouldBeIdentity( m_fRotation, m_fTranslation ) )
	{
		hkTransform trf			= hkTransform(		Physics::MathsTools::CQuatTohkQuaternion( m_fRotation ) , 
													Physics::MathsTools::CPointTohkVector( m_fTranslation ) );

		build = CreateTransformShape(build, trf );		
	}	

	m_info.m_rigidBodyCinfo.m_shape = build;
#endif
	return m_info;
}

//eof
