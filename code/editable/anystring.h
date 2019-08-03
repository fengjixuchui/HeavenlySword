//------------------------------------------------------------------------------------------
//!
//!	\file anystring.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_ANYSTRING_H
#define	_ANYSTRING_H

// Necessary includes

// Foward declaration

//------------------------------------------------------------------------------------------
//!
//!	AnyString
//!	A class to overcome some problems with the welder interface.  Should be removed when
//!	this interface is improved.  Allows a decoupling between objects.
//!
//------------------------------------------------------------------------------------------
class AnyString
{
public:
	ntstd::String m_obString;
};


#endif //_ANYSTRING_H
