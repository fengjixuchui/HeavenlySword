//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		WwsJob audits / still experimental code

	@internal

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_ANIMATOR_UPDATE_AUDIT_H
#define GP_ANIMATOR_UPDATE_AUDIT_H

//--------------------------------------------------------------------------------------------------
//	INCLUDE
//--------------------------------------------------------------------------------------------------

#ifdef __SPU__
#	include<../Gc/GcColour.h>
#else //__SPU__
#	include<Gc/GcColour.h>
#endif//__SPU__

//--------------------------------------------------------------------------------------------------
//	FORWARD DELCARATIONS
//--------------------------------------------------------------------------------------------------

class AuditManager;

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Container for audits enums / string definitions

	@internal
**/
//--------------------------------------------------------------------------------------------------

class GpAnimatorUpdateAudit
{
public:
	enum AuditId
	{
		kEnumStart = 0xFFF,

#define AUDIT_DATA( auditId, auditText, auditColour ) auditId,
#	ifdef __SPU__
#		include <../Gp/GpAnimator/GpAnimatorUpdateAudit.inc>
#	else//__SPU__
#		include <Gp/GpAnimator/GpAnimatorUpdateAudit.inc>
#	endif//__SPU__
#undef AUDIT_DATA

		kEnumEnd
	};

	// Get enum information
	static	int				GetNumAudits( void );
	static	const char*		GetAuditText( AuditId idAudit );
	static	GcColour		GetAuditDefaultColour( AuditId idAudit);

private:
	// Static data
	static	const char*		ms_auditStrings[];
	static	GcColour		ms_auditColours[];
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns number of audits defined
**/	
//--------------------------------------------------------------------------------------------------

inline int GpAnimatorUpdateAudit::GetNumAudits( void )
{
	return kEnumEnd - kEnumStart - 1;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns audit string
**/
//--------------------------------------------------------------------------------------------------

inline const char* GpAnimatorUpdateAudit::GetAuditText( AuditId idAudit )
{
	const char* pText = NULL;

	if ( ( idAudit > kEnumStart ) &&  ( idAudit < kEnumEnd ) )
	{
		pText = ms_auditStrings[ idAudit - kEnumStart];
	}

	return pText;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Returns audit colour
**/
//--------------------------------------------------------------------------------------------------

inline GcColour GpAnimatorUpdateAudit::GetAuditDefaultColour( AuditId idAudit )
{
	GcColour colour( Gc::kColourBlack );

	if ( ( idAudit > kEnumStart ) &&  ( idAudit < kEnumEnd ) )
	{
		colour = ms_auditColours[ idAudit - kEnumStart ];
	}	
	return colour;
}

//--------------------------------------------------------------------------------------------------

#endif//GP_ANIMATOR_UPDATE_AUDIT_H

