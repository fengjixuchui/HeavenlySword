/***************************************************************************************************
*
*	DESCRIPTION		Generic functor implementation.
*					Copied from here: http://www.newty.de/fpt/functor.html#functors
*
*	NOTES
*
***************************************************************************************************/

#ifndef FUNCTOR_H_
#define FUNCTOR_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "tbd/TypeSelect.h"

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	Base functor class
//**************************************************************************************
template
<
	typename RetType
>
class Functor
{
	public:
		//
		//	Two possible functions to call member function. Virtual because derived
		//	classes will use a pointer to an object and a pointer to a member function
		//	to make the function call.
		//
		virtual RetType 	operator ()	()		= 0;			// call using operator
		virtual RetType 	Call		()		= 0;			// call using function

		virtual ~Functor() {}
};

//**************************************************************************************
//	Templated implementation functor class.
//**************************************************************************************
template
<
	class		TClass,
	typename	RetType,
	bool		IsConstMember
>
class SpecificFunctor : public Functor< RetType >
{
	private:
		typedef RetType ( TClass::*NonConstFuncType )();
		typedef RetType ( TClass::*ConstFuncType )	 ()	const;

		typedef typename TypeSelect< IsConstMember, ConstFuncType, NonConstFuncType >::ResultType FuncType;

		FuncType fpt;			// pointer to member function
		TClass *pt2Object;		// pointer to object

	public:
		// constructor - takes pointer to an object and pointer to a member and stores
		// them in two private variables
		SpecificFunctor( TClass *_pt2Object, FuncType _fpt )
		{
			pt2Object = _pt2Object;
			fpt = _fpt;
		};

		virtual ~SpecificFunctor() {}

		// override operator "()"
		virtual RetType operator()()
		{
			return ( *pt2Object.*fpt )();					// execute member function
		}

		// override function "Call"
		virtual RetType Call()
		{
			return ( *pt2Object.*fpt )();						// execute member function
		}
};


#endif	// !FUNCTOR_H_

