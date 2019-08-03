/***************************************************************************************************
*
*	$Header:: /game/statocentity.h 8     18/08/03 13:51 Dean                                             $
*
*
***************************************************************************************************/

#ifndef	_STATIC_ENTITY_H
#define	_STATIC_ENTITY_H

#include "game/entity.h"

class FileDate;

class Static : public CEntity
{
	public:
		HAS_INTERFACE( Static );
	    
		void	OnPostConstruct			();

		bool	IsVaultable				() const { return m_IsVaultable; }
		bool	ShouldVaultThroughCentre() const { return m_ShouldVaultThroughCentre; }
		bool	CanCrouchNextTo			() const { return m_CanCrouchNextTo; }

		Static();
		~Static();

	private:
		bool	m_bCollideOnlyWithCC;
		bool	m_IsVaultable;
		bool	m_ShouldVaultThroughCentre;
		bool	m_CanCrouchNextTo;
};




#endif
