/***************************************************************************************************
*
*   $Header:: /game/debuganimation.h  $
*
*	Header for a debug function showing the animation hierarchy in the visual debugger
*
*	CREATED
*
*	06.01.2005	Mus		Created
*
***************************************************************************************************/


#ifndef _DEBUGANIMATION_H
#define _DEBUGANIMATION_H

class CAnimator;
class Transform;

template <typename T>
class hkArray;
class hkRigidBody;
class hkConstraint;
class hkVector4;
class hkQuaternion;
class hkBone;
class CEntity;
class CHierarchy;
class CPoint;
struct hkWorldRayCastInput;
class hkClosestRayHitCollector;
class CameraInterface;
class CAABB;
class CFrustum;
class SceneElementComponent;

//#define _DRAW_RAYCAST_IN_HKVDB
//#define _DRAW_OBB_IN_HKVDB
//#define _DRAW_SPHERE_IN_HKVDB
//#define _DRAW_VISIBILITY_IN_HKVDB

//#define _ENABLE_HAVOK_DEBUG_RENDERING

// Define this macros to see the CAnimator hierarchy in the visual debugger
//#define _DEBUG_DRAW_HIERARCHY_IN_HAVOK_VISUALISER

namespace Physics
{
	class HavokDebugDraw
	{
	public:

		// Enable this camera to be used in the visual debugger
		static void ViewCameraInHKVDB(const CameraInterface& obCamera);
		static void DrawCAABBInHKVDB(const CAABB& obAABB, const CMatrix& obMatrix, int color = 0xffffff00);
		static void DrawRaycastInVisualiser(const hkWorldRayCastInput& obInput, const hkClosestRayHitCollector& obOutput);
		static void DrawRayInVisualiser(const CPoint& obStart, const CPoint& obEnd, int color = 0xffff00ff);
		static void DrawFrustumInHKVDB(const CFrustum& obFrustum, const CMatrix& obMatrix);
		static void DrawSceneElementInHKVDB(const SceneElementComponent& obElement);
		static void DrawSphereQueryInHKVDB(const CPoint& obPosition, float fRadius, int iNumber, int color = 0xff00ff00);

		static void DrawMatrixInVisualiser(const CMatrix& obMatrix);

		static void DrawHierarchyInVisualiser(const CAnimator& obAnimator);
		static void DrawBoneInVisualiser(const Transform* obPrevious, const Transform* obCurrent, int obColor); 
		static void DrawBoneRagdollOnlyInVisualiser(const Transform* obPrevious, const Transform* obCurrent, int obColor);

	private:

		HavokDebugDraw();
	};

}

#endif	//_DEBUGANIMATION_H
