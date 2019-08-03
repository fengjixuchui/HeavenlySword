#ifndef _SHAPEUSERDATA_H_
#define _SHAPEUSERDATA_H_

#include "UserDataBase.h"


extern const hkClass ShapeUserDataClass;
struct ShapeUserData : public UserDataBase
{
	HK_DECLARE_REFLECTION();

	ShapeUserData(hkUint32 transformHash, hkUint32 materialID)
		: UserDataBase("SH01")
		, m_uiTransformHash(transformHash)
		, m_uiMaterialID(materialID)
	{
	}

	hkUint32 m_uiTransformHash;
	hkUint32 m_uiMaterialID;
};


#endif // _SHAPEUSERDATA_H_
