//--------------------------------------------------
//!
//!	\file LookAtInfo.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _LOOKATINFO_H_
#define _LOOKATINFO_H_

#include "tbd/xmlfileinterface.h"

 //--------------------------------------------------
//!
//!	Look-at information for a given entity/thing
//!	This structure  holds the necessary information
//! for an entity to be "lookable"
//! For character entities, it is possible to look at 
//! a specific bone transform
//!
//--------------------------------------------------
struct LookAtInfo
{
	HAS_INTERFACE(LookAtInfo)

	ntstd::String				m_obTransformName;			//!< the specific transform to look at (if any)
	CPoint						m_obOffset;					//!< some local offset UNUSED
	u_int						m_iPriority;				//!< priority. Zero is the highest
};


#endif // end of _LOOKATINFO_H_
