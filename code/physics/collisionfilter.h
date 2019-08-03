//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/collisionfilter.h
//!	
//!	DYNAMICS COMPONENT:
//!		This is the definition of the collision filter we want to use in our game
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.02.21
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_COLLISIONFILTER_INC
#define _DYNAMICS_COLLISIONFILTER_INC

#include "config.h"
#include <hkbase/config/hkConfigVersion.h>
#include "physics/havokincludes.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

// Forward Declaractions
class hkCollisionFilter;
class hkBool;
class hkCollidable;
class hkPhantom;
struct hkCollisionInput;
class hkCdBody;
class hkShapeCollection;
//class hkShapeKey;
struct hkShapeRayCastInput;
struct hkWorldRayCastInput;
class hkRigidBody;

//#include <hkbase/baseobject/hkbaseobject.h>
#include <hkcollide/filter/hkCollisionFilter.h>

// Main dynamics component namespace
namespace Physics {

//--------------------------------------------------------------------------
//!
//!	Collision Filter
//!	Unique collision filter defining the behavior for the whole game
//!
//--------------------------------------------------------------------------
class HSCollisionFilter : public hkCollisionFilter
{
public:

	// Construction
	HSCollisionFilter();

	// Checks two collidables 
	virtual hkBool isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const;

	// hkRayCollidableFilter interface forwarding
	virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& aInput, const hkCollidable& collidableB ) const;

#if HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1
	// hkShapeCollectionFilter interface forwarding
	virtual hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& collectionB, hkShapeKey keyB  ) const;

	// hkRayShapeCollectionFilter interface forwarding
	virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape&, const hkShapeContainer& bCollection, hkShapeKey bKey ) const;
#else
	// hkShapeCollectionFilter interface forwarding
	virtual hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeCollection& collectionB, hkShapeKey keyB  ) const;

	// hkRayShapeCollectionFilter interface forwarding
	virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShapeCollection& bCollection, hkShapeKey bKey ) const;

#endif //HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1 

	

protected:

	hkBool isCollisionEnabledPhantomVSPhantom( const hkPhantom*, const hkPhantom* ) const;
	hkBool isCollisionEnabledBodyVSPhantom( const hkRigidBody*, const hkPhantom* ) const;
	hkBool isCollisionEnabledBodyVSBody( const hkRigidBody*,const hkRigidBody* ) const;
	hkBool isCollisionEnabledTriggerVSBody( const hkCollidable*,const hkRigidBody* ) const;
	hkBool isCollisionEnabledTriggerVSPhantom( const hkCollidable*,const hkPhantom* ) const;

};

} // Physics namespace

#endif
#endif // _DYNAMICS_COLLISIONFILTER_INC
