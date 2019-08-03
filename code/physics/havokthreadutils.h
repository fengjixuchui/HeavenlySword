//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/havokthreadutils.h
//!	
//---------------------------------------------------------------------------------------------------------

#ifndef	HAVOKTHREADUTILS_H_
#define HAVOKTHREADUTILS_H_

#include "core/syncprims.h"

class hkWorldObject;

namespace Physics
{
	class WriteAccess
	{
		public:
			WriteAccess();
			WriteAccess( hkWorldObject *world_obj );

			~WriteAccess();

		private:			
			hkWorldObject *	m_WorldObj;
	};

	class ReadAccess
	{
		public:
			ReadAccess();
			ReadAccess( hkWorldObject *world_obj );

			~ReadAccess();

		private:
			hkWorldObject *	m_WorldObj;
	};

	// This is a separate define because castRay should really require RO access
	// but because Havok has fucked up their const-correctness it actually requires
	// RW access to the broadphase. :(
	// This should be changed to ReadAccess as soon as Havok fix this. [ARV].
#	define CastRayAccess ReadAccess
}

#endif // !HAVOKTHREADUTILS_H_
