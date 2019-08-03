/***************************************************************************************************
*
*   $Header:: /game/debuganimation.cpp $
*
*	
*	Implementation for the CAnimator rendering function in the visual debugger.
*
*	CHANGES
*
*	06.01.2005	Mus		Created
*
***************************************************************************************************/

#include "config.h"
#include "physics/havokincludes.h"
#include "maths_tools.h"
#include "debugdraw.h"

#include "camera/camman.h" // Havok user camera
#include "camera/camview.h"
#include "camera/sceneelementcomponent.h"

#include "core/boundingvolumes.h"

#include "core/frustum.h"
#include "camera/elementmanager.h"

//#include "memorydebug.h"

#include "physics/world.h"
#include "game/entity.h"
#include "game/entity.inl"


#include "anim/animator.h"
#include "anim/hierarchy.h"
#include "anim/transform.h"

#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
//#include <hkbase/memory/hkmemory.h>
#include <hkmath/hkmath.h>
#include <hkvisualize/shape/hkDisplayAABB.h>
#include <hkvisualize/shape/hkDisplaySphere.h>
#include <hkvisualize/hkDebugDisplay.h>
#include <hkbase/stream/hkIstream.h>
#include <hkSerialize/hkSerialize.h>
#include <hkcollide\castutil\hkWorldRayCastInput.h>
#include <hkcollide\collector\raycollector\hkClosestRayHitCollector.h>
#include <hkvisualize/type/hkColor.h>
#endif

namespace Physics
{

	void HavokDebugDraw::DrawSceneElementInHKVDB(const SceneElementComponent& obElement)
	{
	#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
	#ifdef _ENABLE_HAVOK_DEBUG_RENDERING

		hkSphere obSphere(Physics::MathsTools::CPointTohkVector(obElement.GetPosition()), obElement.GetRadius());
		hkDisplaySphere obDSphere(obSphere, 5, 5);

		hkArray<hkDisplayGeometry*> hkGeometries;
		hkGeometries.pushBack(&obDSphere);

		HK_DISPLAY_GEOMETRY(hkGeometries, hkColor::CYAN);

	#endif // _ENABLE_HAVOK_DEBUG_RENDERING
	#else
		UNUSED(obElement);
	#endif
	}

	void HavokDebugDraw::DrawRayInVisualiser(const CPoint& obStart, const CPoint& obEnd, int color)
	{
	#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
	#ifdef _DRAW_RAYCAST_IN_HKVDB

		hkVector4 obFrom, obTo;
		Physics::MathsTools::CPointTohkVector(obStart,obFrom);
		Physics::MathsTools::CPointTohkVector(obEnd,obTo);

		HK_DISPLAY_LINE(obFrom, obTo, color);

	#endif // _ENABLE_HAVOK_DEBUG_RENDERING
#else
		UNUSED(obStart);
		UNUSED(obEnd);
		UNUSED(color);
	#endif
	}

	void HavokDebugDraw::DrawSphereQueryInHKVDB(const CPoint& obPosition, float fRadius, int iNumber, int color)
	{
	#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
	#ifdef _ENABLE_HAVOK_DEBUG_RENDERING

		/*hkSphere obSphere(CPointTohkVector(obPosition), fRadius);
		hkDisplaySphere obDSphere(obSphere, 5, 5);

		hkArray<hkDisplayGeometry*> hkGeometries;
		hkGeometries.pushBack(&obDSphere);

		HK_DISPLAY_GEOMETRY(hkGeometries, color);
		char buffer[50];
		itoa( iNumber, buffer, 10 );

		HK_DISPLAY_TEXT(buffer, color);*/

		CPoint previous = obPosition + fRadius * CPoint(1.0,0.f,0.f);
		for(int i = 1; i <= 13; i++)
		{
			CPoint next = obPosition + fRadius * cos(2.f*PI*i/13) * CPoint(1.0,0.f,0.f) + fRadius * sin(2.f*PI*i/13) * CPoint(0.0,0.f,1.f);
			HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(previous), Physics::MathsTools::CPointTohkVector(next), color);
			previous = next;
		};

	#endif // _ENABLE_HAVOK_DEBUG_RENDERING
#else
		UNUSED(obPosition);
		UNUSED(fRadius);
		UNUSED(color);
		UNUSED(iNumber);
	#endif
	}

#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
	void HavokDebugDraw::DrawRaycastInVisualiser(const hkWorldRayCastInput& obInput, const hkClosestRayHitCollector& obOutput)
	{
	#ifdef _DRAW_RAYCAST_IN_HKVDB

		if(!obOutput.hasHit())
		{
			// White Line for failed Raycast
			HK_DISPLAY_LINE(obInput.m_from, obInput.m_to, hkColor::WHITE);

		} else {

			CPoint obFrom, obTo;
			Physics::MathsTools::hkVectorToCPoint(obInput.m_from,obFrom);
			Physics::MathsTools::hkVectorToCPoint(obInput.m_to,obTo);

			CPoint obHit = CPoint::Lerp(	obFrom, obTo, 
											obOutput.getHit().m_hitFraction );
			hkVector4 vHit;
			Physics::MathsTools::CPointTohkVector(obHit,vHit);

			// Red line for successfull ones
			HK_DISPLAY_LINE(obInput.m_from, vHit, hkColor::RED);

			// [MUS] BUGTRACK_003
			// For some reason, Havok return a garbage normal in the case when hitfraction is 1.f
			// STATUS:
			//		--> RESOLVED: Was a bug in the raycast filter.
			// REFERENCE:
			//		--> BUGTRACK_004
			// if(obOutput.getHit().m_hitFraction != 1.f)
			{
				CPoint obNormal;
				Physics::MathsTools::hkVectorToCPoint(obOutput.getHit().m_normal,obNormal);
				obNormal *= 0.5f;

				CPoint obHitNormal =  obHit + obNormal;

				hkVector4 vHitNormal;
				Physics::MathsTools::CPointTohkVector(obHitNormal,vHitNormal);
			
				// Green represent the normal at contact point
				HK_DISPLAY_LINE(vHit, vHitNormal, hkColor::GREEN);
			}
		}

	#endif // _DRAW_RAYCAST_IN_HKVDB
	};
#endif

	void HavokDebugDraw::DrawFrustumInHKVDB(const CFrustum& obFrustum, const CMatrix& obMatrix)
	{
	#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
	#ifdef _ENABLE_HAVOK_DEBUG_RENDERING

		CPoint obPositions[8];
		obFrustum.GetExtremePoints((CPoint*)&obPositions);

		for(int iVertex = 0; iVertex < 8; ++iVertex)
			obPositions[iVertex] = obPositions[iVertex]*obMatrix;

		/*
		pobPositions[0] = fZNear*aobQuad[0];
		pobPositions[1] = fZNear*aobQuad[1];
		pobPositions[2] = fZNear*aobQuad[2];
		pobPositions[3] = fZNear*aobQuad[3];
		pobPositions[4] = fZFar*aobQuad[0];
		pobPositions[5] = fZFar*aobQuad[1];
		pobPositions[6] = fZFar*aobQuad[2];
		pobPositions[7] = fZFar*aobQuad[3];
		*/

		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[0]), Physics::MathsTools::CPointTohkVector(obPositions[1]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[1]), Physics::MathsTools::CPointTohkVector(obPositions[2]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[2]), Physics::MathsTools::CPointTohkVector(obPositions[3]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[3]), Physics::MathsTools::CPointTohkVector(obPositions[0]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[4]), Physics::MathsTools::CPointTohkVector(obPositions[5]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[5]), Physics::MathsTools::CPointTohkVector(obPositions[6]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[6]), Physics::MathsTools::CPointTohkVector(obPositions[7]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[7]), Physics::MathsTools::CPointTohkVector(obPositions[0]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[0]), Physics::MathsTools::CPointTohkVector(obPositions[4]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[1]), Physics::MathsTools::CPointTohkVector(obPositions[5]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[2]), Physics::MathsTools::CPointTohkVector(obPositions[6]), hkColor::BLUE);
		HK_DISPLAY_LINE(Physics::MathsTools::CPointTohkVector(obPositions[3]), Physics::MathsTools::CPointTohkVector(obPositions[7]), hkColor::BLUE);

	#endif // _ENABLE_HAVOK_DEBUG_RENDERING
#else
		UNUSED(obFrustum);
		UNUSED(obMatrix);
	#endif
	}

	void HavokDebugDraw::DrawCAABBInHKVDB(const CAABB& obAABB, const CMatrix& obMatrix, int color)
	{
	#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
	#ifdef _ENABLE_HAVOK_DEBUG_RENDERING

		hkVector4 vMin;
		hkVector4 vMax;
		Physics::MathsTools::CPointTohkVector(obAABB.Min(),vMin);
		Physics::MathsTools::CPointTohkVector(obAABB.Max(),vMax);

		hkDisplayAABB hkAABB(vMin,vMax);
		hkArray<hkDisplayGeometry*> hkGeometries;
		hkGeometries.pushBack(&hkAABB);

		hkVector4		vTrans;
		hkQuaternion	rRot;

		Physics::MathsTools::CPointTohkVector(CPoint(obMatrix.GetTranslation()),vTrans);
		Physics::MathsTools::CMatrixTohkQuaternion(obMatrix,rRot);

		HK_DISPLAY_GEOMETRY_WITH_TRANSFORM(hkGeometries, hkTransform(rRot,vTrans), color);

	#endif // _ENABLE_HAVOK_DEBUG_RENDERING
#else
		UNUSED(obAABB);
		UNUSED(obMatrix);
		UNUSED(color);
	#endif
	}

	void HavokDebugDraw::DrawMatrixInVisualiser(const CMatrix& obMatrix)
	{
	#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
	#ifdef _ENABLE_HAVOK_DEBUG_RENDERING

		hkVector4 obXStart(obMatrix.GetTranslation().X(),
						obMatrix.GetTranslation().Y(),
						obMatrix.GetTranslation().Z());
		hkVector4 obXEnd(	obMatrix.GetTranslation().X()+(obMatrix.GetXAxis().X()*0.1f),
							obMatrix.GetTranslation().Y()+(obMatrix.GetXAxis().Y()*0.1f),
							obMatrix.GetTranslation().Z()+(obMatrix.GetXAxis().Z()*0.1f));

		hkVector4 obYStart(obMatrix.GetTranslation().X(),
						obMatrix.GetTranslation().Y(),
						obMatrix.GetTranslation().Z());
		hkVector4 obYEnd(obMatrix.GetTranslation().X()+(obMatrix.GetYAxis().X()*0.1f),
						obMatrix.GetTranslation().Y()+(obMatrix.GetYAxis().Y()*0.1f),
						obMatrix.GetTranslation().Z()+(obMatrix.GetYAxis().Z()*0.1f));

		hkVector4 obZStart(obMatrix.GetTranslation().X(),
						obMatrix.GetTranslation().Y(),
						obMatrix.GetTranslation().Z());
		hkVector4 obZEnd(obMatrix.GetTranslation().X()+(obMatrix.GetZAxis().X()*0.1f),
						obMatrix.GetTranslation().Y()+(obMatrix.GetZAxis().Y()*0.1f),
						obMatrix.GetTranslation().Z()+(obMatrix.GetZAxis().Z()*0.1f));

		HK_DISPLAY_LINE(obXStart, obXEnd, hkColor::RED);
		HK_DISPLAY_LINE(obYStart, obYEnd, hkColor::GREEN);
		HK_DISPLAY_LINE(obZStart, obZEnd, hkColor::BLUE);

	#endif // _ENABLE_HAVOK_DEBUG_RENDERING
#else
		UNUSED(obMatrix);
	#endif
	};

	void HavokDebugDraw::ViewCameraInHKVDB (const CameraInterface& obCamera)
	{
#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
		CMatrix obCameraMatrix(obCamera.GetTransform());

		CPoint obTo(obCameraMatrix.GetTranslation() + obCameraMatrix.GetZAxis());

		HK_UPDATE_CAMERA(	Physics::MathsTools::CPointTohkVector(	obCameraMatrix.GetTranslation() ), 
							Physics::MathsTools::CPointTohkVector(	obTo ), 
							hkVector4(	obCameraMatrix.GetYAxis().X(), obCameraMatrix.GetYAxis().Y(), obCameraMatrix.GetYAxis().Z() ), 
							0.5f,	// [MUS] Arr... Harcoded values again :P 0.5f stands for the near Z plane...
							5000.f,	// ... whereas 5000.f is the far Z plan.
							CamMan::Get().GetPrimaryView()->GetFOVAngle() * RAD_TO_DEG_VALUE,	
							obCamera.GetCameraName().GetDebugString());
#endif
	}

	void HavokDebugDraw::DrawBoneRagdollOnlyInVisualiser(const Transform* obPrevious, const Transform* obCurrent, int obColor)
	{
	#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
	#ifdef _ENABLE_HAVOK_DEBUG_RENDERING
		if(obCurrent==0)
			return;

		// Draw the bone
		if(	(obPrevious!=0)						&&
			(obPrevious!=obCurrent)				) 
		{

			const CMatrix mPrevious = obPrevious->GetWorldMatrix();
			const CMatrix mCurrent  = obCurrent->GetWorldMatrix();

			hkVector4 vStart, vEnd;
			Physics::MathsTools::CPointTohkVector(mPrevious.GetTranslation(),vStart);
			Physics::MathsTools::CPointTohkVector(mCurrent.GetTranslation(),vEnd);

			if(	(vStart.lengthSquared3()>1e-3f)	&&
				(vEnd.lengthSquared3()>1e-3f)	)
			{
				HK_DISPLAY_LINE(vStart, vEnd, hkColor::GREY25);
			}
		}

		switch(obColor) 
		{
			case hkColor::RED: 
				{
					obColor=hkColor::GREEN;
					break;
				}
			case hkColor::GREEN: 
				{
					obColor=hkColor::BLUE;
					break;
				}
			case hkColor::BLUE: 
				{
					obColor=hkColor::RED;
					break;
				}
		}

		// Recurse child branch
		if(obCurrent->GetFirstChild()) {
			DrawBoneInVisualiser(obCurrent,obCurrent->GetFirstChild(),obColor);
		};

		// Recurse sibling branch
		if(obCurrent->GetNextSibling()) {
			DrawBoneInVisualiser(obPrevious,obCurrent->GetNextSibling(),obColor);
		};
	#else
		UNUSED(obPrevious);
		UNUSED(obCurrent);
		UNUSED(obColor);
	#endif
#else
		UNUSED(obPrevious);
		UNUSED(obCurrent);
		UNUSED(obColor);
#endif
	}

	void HavokDebugDraw::DrawBoneInVisualiser(const Transform* obPrevious, const Transform* obCurrent, int obColor) 
	{
	#ifndef  _PS3_RUN_WITHOUT_HAVOK_BUILD
	#ifdef _ENABLE_HAVOK_DEBUG_RENDERING
		if(obCurrent==0)
			return;

		// Draw the bone
		if(	(obPrevious!=0)						&&
			(obPrevious!=obCurrent)				) 
		{

			const CMatrix mPrevious = obPrevious->GetWorldMatrix();
			const CMatrix mCurrent  = obCurrent->GetWorldMatrix();

			hkVector4 vStart, vEnd;
			Physics::MathsTools::CPointTohkVector(mPrevious.GetTranslation(),vStart);
			Physics::MathsTools::CPointTohkVector(mCurrent.GetTranslation(),vEnd);

			if(	(vStart.lengthSquared3()>1e-3f)	&&
				(vEnd.lengthSquared3()>1e-3f)	)
			{
				HK_DISPLAY_LINE(vStart, vEnd, obColor);
			}
		}

		switch(obColor) 
		{
			case hkColor::RED: 
				{
					obColor=hkColor::GREEN;
					break;
				}
			case hkColor::GREEN: 
				{
					obColor=hkColor::BLUE;
					break;
				}
			case hkColor::BLUE: 
				{
					obColor=hkColor::RED;
					break;
				}
		}

		// Recurse child branch
		if(obCurrent->GetFirstChild()) {
			DrawBoneInVisualiser(obCurrent,obCurrent->GetFirstChild(),obColor);
		};

		// Recurse sibling branch
		if(obCurrent->GetNextSibling()) {
			DrawBoneInVisualiser(obPrevious,obCurrent->GetNextSibling(),obColor);
		};
	#else
		UNUSED(obPrevious);
		UNUSED(obCurrent);
		UNUSED(obColor);
	#endif
	#else
		UNUSED(obPrevious);
		UNUSED(obCurrent);
		UNUSED(obColor);
#endif
	}

	void HavokDebugDraw::DrawHierarchyInVisualiser(const CAnimator& obAnimator)
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		DrawBoneInVisualiser(0,obAnimator.GetHierarchy()->GetRootTransform()->GetFirstChild(),hkColor::RED);
#else
		UNUSED(obAnimator);
#endif
	}
}
