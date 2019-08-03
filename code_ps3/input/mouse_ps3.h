//--------------------------------------------------
//!
//!	\file mouse_ps3.h
//!	mouse input singleton.
//!
//--------------------------------------------------

#ifndef INPUT_MOUSE_PS3_H
#define INPUT_MOUSE_PS3_H

// forward decl
class MouseInput;

//--------------------------------------------------
//!
//!	MouseInputPlatform
//! Platform specific mouse input
//! The ATG library don't appear to have a working
//! mouse library yet... so this is only partially
//! complete.
//!
//--------------------------------------------------
class	MouseInputPlatform
{
public:
	MouseInputPlatform( MouseInput& parent );

	//! platform specific update
	void Update();

private:
	MouseInput& m_Parent;
};

#endif // end MOUSE_PS3_H
