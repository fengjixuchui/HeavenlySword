//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/dynamicsallocator.h
//!	
//!	DYNAMICS COMPONENT:
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.10
//!
//---------------------------------------------------------------------------------------------------------

#ifndef	_DYNAMICS_ALLOCATOR_H
#define	_DYNAMICS_ALLOCATOR_H

class CEntity;

namespace Physics
{
	class CVolumeData
	{
	public:
		CEntity*	m_pobEntity;
		void*		m_pobVolumeMaterial;

		// [Mus] - 2005.02.14 - Unitialised data are evil, especially when it comes to pointers...
		CVolumeData() :
			m_pobEntity(0),
			m_pobVolumeMaterial(0)
		{};
	};

	/***************************************************************************************************
	*	
	*	CLASS			CDynamicsAllocator
	*
	*	DESCRIPTION		This singleton is used to tag allocations that are handed off to havok, so we
	*					can clean them up afterwards...
	*
	*	NOTES			Assumes havok will not be freeing these allocations.
	*
	***************************************************************************************************/
	class CDynamicsAllocator : public Singleton<CDynamicsAllocator>
	{
	public:
		~CDynamicsAllocator( void ) { FreeAllocations(); }

		void			FreeAllocations( void );
		unsigned short*	AllocateNewIndexBuffer( u_int uiSize );
		CVolumeData*	AllocateNewVolumeData( void );

	private:
		ntstd::List<unsigned short*>	m_obIndexList;
		ntstd::List<CVolumeData*>		m_obVolumeList;
	};

}

#endif //_DYNAMICS_H
