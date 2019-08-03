#ifndef _TCDEBUGUPDATE_H
#define _TCDEBUGUPDATE_H

#include "tcdebugupdatemode.h"

class Transform;

class TransformControllerDebugUpdate
{
	public:
		virtual ~TransformControllerDebugUpdate(){};

		//
		//	Which mode is this object representing?
		//
		virtual TC_DEBUG_UPDATE_MODE	GetMode		()	const	= 0;
		virtual const char * const		GetDesc		()	const	= 0;

	public:
		//
		//	Update
		//
		virtual void					Update		( PAD_NUMBER ePad, float fMoveFactor, float fRotateFactor, CPoint &ptPosition,
													  Transform *pTransform, float &fPhi, float &fTheta )	const	= 0;

	public:
		//
		//	Public Creation/Destruction
		//
		static TransformControllerDebugUpdate *	Create	( TC_DEBUG_UPDATE_MODE mode );
		static void								Destroy	( TransformControllerDebugUpdate *du );
};

#endif // _TCDEBUGUPDATE_H
