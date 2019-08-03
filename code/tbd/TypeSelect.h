/***************************************************************************************************
*
*	DESCRIPTION		Generic programming routine to select a type based on a template bool arg.
*					N.B. Relies on partial template specialisation support.
*
*	NOTES
*
***************************************************************************************************/

#ifndef TYPESELECT_H_
#define TYPESELECT_H_

//**************************************************************************************
//
//**************************************************************************************
template
<
	bool		UseFirstArg,
	typename	FirstArg,
	typename	SecondArg
>
struct TypeSelect;

template
<
	typename	FirstArg,
	typename	SecondArg
>
struct TypeSelect< true, FirstArg, SecondArg >
{
	typedef FirstArg	ResultType;
};

template
<
	typename	FirstArg,
	typename	SecondArg
>
struct TypeSelect< false, FirstArg, SecondArg >
{
	typedef SecondArg	ResultType;
};



#endif	// !TYPESELECT_H_

