#ifndef _USERDATABASE_H_
#define _USERDATABASE_H_

#include <hkbase/types/hkBaseTypes.h>
#include <algorithm>


extern const hkClass UserDataBaseClass;
struct UserDataBase
{
	HK_DECLARE_REFLECTION();

	UserDataBase(const char* pVersionTag)
	{
		unsigned int tagSize = sizeof(m_uiVersionTag);
		memset(m_uiVersionTag, 0, tagSize);
		if (pVersionTag)
		{
			unsigned int numChars = strlen(pVersionTag);
			assert(numChars <= tagSize);
			memcpy(m_uiVersionTag, pVersionTag, std::min(numChars, tagSize));
		}
	}

	hkChar m_uiVersionTag[4];
};

//Havok  HK_OFFSET_OF macro produces warning during PS3 build so use this one instead
#undef HK_OFFSET_OF
#define HK_OFFSET_OF(CLASS,MEMBER) ((reinterpret_cast<long>(&reinterpret_cast<CLASS*>(16)->MEMBER)-16))


#endif // _USERDATABASE_H_
