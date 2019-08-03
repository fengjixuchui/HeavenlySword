#include "winddef.h"

#include "objectdatabase/dataobject.h"
#include "hair/forcefield.h"






//------------------------------------------------------------------------------------------
//!                                                                                         
//!	ForceLinkFunction26
//!                                                                                         
//------------------------------------------------------------------------------------------
void ForceLinkFunction26()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction26() !ATTN!\n");
}








// Welder interface
START_CHUNKED_INTERFACE( E3WindDef, Mem::MC_PROCEDURAL )
	PUBLISH_VAR_WITH_DEFAULT(m_direction,CDirection(1.0f,0.0f,0.0f))
	PUBLISH_VAR_WITH_DEFAULT(m_impulse1,CVector(CONSTRUCT_CLEAR))
	PUBLISH_VAR_WITH_DEFAULT(m_impulse2,CVector(CONSTRUCT_CLEAR))
	PUBLISH_VAR_WITH_DEFAULT(m_impulse3,CVector(CONSTRUCT_CLEAR))
	PUBLISH_VAR_WITH_DEFAULT(m_dummy1,CVector(CONSTRUCT_CLEAR))
	PUBLISH_VAR_WITH_DEFAULT(m_dummy2,CVector(CONSTRUCT_CLEAR))
	PUBLISH_VAR_WITH_DEFAULT(m_dummy3,CVector(CONSTRUCT_CLEAR))
	PUBLISH_VAR_WITH_DEFAULT(m_fPower,1.0f)
	PUBLISH_VAR_WITH_DEFAULT(m_fConstantPower,1.0f)

	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
END_STD_INTERFACE





E3WindDef::E3WindDef()
{
	// nothing
}
//! post construct (real constructor)
void E3WindDef::PostConstruct()
{
	if(ForceFieldManager::Exists())
	{
		ForceFieldManager::Get().AddE3Wind(this);
	}
	else
	{
		ntPrintf("ForceFieldManager does not esist");
	}
}

