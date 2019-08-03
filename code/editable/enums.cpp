/***************************************************************************************************
*
*	Enum delaration support functions
*
*	CHANGES		
*
*	03/10/2003	Mike	Created
*
*	NOTES:		
*
***************************************************************************************************/

#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

// Build some editable enums

#define ENUM_STARTEX(n,t)														\
	{																			\
		if(t&ENUM_XML)															\
		{																		\
			GlobalEnum& newEnum = ObjectDatabase::Get().AddGlobalEnum( #n );	\
			unsigned int iIndex = 0;											\

#define ENUM_SET(n,v)															\
			newEnum.AddEnumPair( #n, (v) );										\
			iIndex = (v)+1;

#define ENUM_SET_AS(n,v,as)														\
			newEnum.AddEnumPair( #n, (v) );										\
			iIndex = (v)+1;

#define ENUM_AUTO(n)															\
			newEnum.AddEnumPair( #n, iIndex );									\
			iIndex++;

#define ENUM_AUTO_AS(n, as)														\
			newEnum.AddEnumPair( #n, iIndex );									\
			iIndex++;

#define ENUM_END()																\
		}																		\
	}

#define ENUM_STARTEX_PUBLISH_AS(n,t,a) ENUM_STARTEX(n,t)

// Define a global function that builds the CInterfaces needed 

void BuildEnums()
{
#include "editable/enumlist.h"
#include "editable/enums_formations.h"
#include "editable/enums_ai.h"
}
