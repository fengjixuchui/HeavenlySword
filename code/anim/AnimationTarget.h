//------------------------------------------------------------------------------------------
//!
//!	\file AnimationTarget.h
//!
//------------------------------------------------------------------------------------------

#ifndef	ANIMATIONTARGET_H_
#define	ANIMATIONTARGET_H_

//------------------------------------------------------------------------------------------
//!
//!	AnimationTarget
//!	This class defines the target storage area for computed animation data.
//!
//------------------------------------------------------------------------------------------
ALIGNTO_PREFIX( 16 ) struct AnimationTarget
{
	CQuat		m_obRotation;
	CPoint		m_obTranslation;
	CVector		m_obTracking;
}
ALIGNTO_POSTFIX( 16 );

#endif	// !ANIMATIONTARGET_H_


