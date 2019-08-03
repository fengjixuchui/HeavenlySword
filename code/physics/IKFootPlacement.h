//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/IKFootPlacement.h
//!	
//!	DYNAMICS COMPONENT:
//!		Hacking the HKA IK system to be used by our animation system.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.06.03
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _IK_FOOT_HACK
#define _IK_FOOT_HACK

#include "config.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/hkmath.h>

#else
class hkVector4
{};
#endif

class hsFootPlacementIkSolver;
class CHierarchy;
class hkSkeleton;
class hsRaycastInterface;
class CEntity;
class hkPose;
class CMatrix;

namespace Physics
{
	class IKFootPlacement
	{
	public:

		//! Constructors / Destructors 		
		IKFootPlacement(CHierarchy* pobHierarchy);
		~IKFootPlacement();

		//! Public methods
		void Update( bool valid, float timestep);
		void EntityRootTransformHasChanged()
		{
			m_resetPelvisY = true;
		}

		bool m_resetPelvisY;
		float m_previousPlevisY;
		static bool m_gIKEnable;

	private:
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		//! IK Solver, one per foot
		hsFootPlacementIkSolver*	m_pobLeftIK;
		hsFootPlacementIkSolver*	m_pobRightIK;
#endif

		//! Data needed to setup/control the IK
		CHierarchy*					m_pobHierarchy;
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkSkeleton*					m_pobSkeleton;	
		hkPose*                     m_pobAnimPose; 
#endif
		void Reset();
		void Create();

		// Debug draw function.
		void DrawIKParameters( CMatrix& modelToWorld );
		void DrawCurrentSkeletonPose( hkPose& animPose, CMatrix& modelToWorld, int color );
	};

	class IKLookAt
	{
	public:

		//! Constructors / Destructors 
		IKLookAt(CHierarchy* pobHierarchy, CEntity* currentEntity);
		~IKLookAt();

		//! Public methods
		void Update();

		void SetLookAtEntity( const CEntity* entity );

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	public:
		void SetLookAtPosition( const hkVector4& pos );
	private:
		hkVector4 m_lookAtLastTargetWS;
#endif
	private:
		float m_lookAtWeight;

		//! Data needed to setup/control the IK
		CHierarchy*					m_pobHierarchy;
		hkSkeleton*					m_pobSkeleton;	
		const CEntity*				m_lookAtEntity;
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkVector4					m_lookAtPosition;
#endif
		CEntity*					m_currentEntity;
	};
}

#endif // _IK_FOOT_HACK
