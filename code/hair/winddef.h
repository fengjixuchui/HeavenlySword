#ifndef _WINDDEF_H_
#define _WINDDEF_H_

//--------------------------------------------------
//!
//!	\file winddef.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

class E3WindDef
{
public:
	CVector m_impulse1;
	CVector m_impulse2;
	CVector m_impulse3;
	CVector m_dummy1;
	CVector m_dummy2;
	CVector m_dummy3;
	CDirection m_direction;
	float m_fPower;
	float m_fConstantPower;
	E3WindDef();
	void PostConstruct();
}; // end of class E3Wind

#endif // end of _WINDDEF_H_
