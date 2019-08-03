//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/clump_tools.h
//!	
//!	DYNAMICS COMPONENT:
//!		Tools to extract shapes from clumps
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.06
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_CLUMP_TOOLS_INC
#define _DYNAMICS_CLUMP_TOOLS_INC

#include "config.h"

class hkShape;
class hkConvexShape;
class hkRigidBodyCinfo;
class CColprimDesc;
class CEntity;
class hkRigidBodyCinfo;
class hkTransform;
class CHierarchy;

#include "collisionbitfield.h"

namespace Physics
{
	class StaticLG;
	class AnimatedLG;
	class SingleRigidLG;
	class CompoundLG;
	class SpearLG;

	// ---------------------------------------------------------------
	//	Tools class. Do not try to instanciate.
	// ---------------------------------------------------------------
	class ClumpTools
	{
	public:
		static ntstd::String	AlterFilename( const char * const filename );
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		static void				ComputeCentreOfMassAndInertiaTensor( hkShape* pobShape, hkRigidBodyCinfo& obInfo );
		static void				ComputeCentreOfMassAndInertiaTensorTransformed (hkShape* pobShape, hkRigidBodyCinfo& obInfo, hkTransform& obToto);
#endif
		//static hkConvexShape*	ConstructConvexShapeFromColprim( const CColprimDesc& obVolume );
		//static hkShape*			ConstructArbitraryShapeFromColprim( const CColprimDesc& obVolume );

		static AnimatedLG*		ConstructAnimatedLGFromClump( CEntity* p_entity );
		static StaticLG*		ConstructStaticLGFromClump( CEntity* p_entity );
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		static SingleRigidLG *	ConstructRigidLGFromClump( CEntity *p_entity, CHierarchy *pobHierarchy, const ntstd::String &name, hkRigidBodyCinfo* pobInfo );
		static SingleRigidLG*	ConstructRigidLGFromClump( CEntity* p_entity, hkRigidBodyCinfo* pobInfo  );
		static CompoundLG*		ConstructCompoundLGFromClump( CEntity* p_entity, hkRigidBodyCinfo* pobInfo );
		static SpearLG*			ConstructSpearLGFromClump( CEntity* p_entity, hkRigidBodyCinfo* pobInfo );
#endif

	private:
		struct ListShape
		{
			ntstd::List<hkShape*> obShapeList;
		};

		ClumpTools();
		~ClumpTools();
	};

}
#endif // _DYNAMICS_CLUMP_TOOLS_INC
