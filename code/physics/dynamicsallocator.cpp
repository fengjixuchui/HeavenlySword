//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/maths_tools.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.10
//!
//---------------------------------------------------------------------------------------------------------


#include "dynamicsallocator.h"


namespace Physics
{

	/***************************************************************************************************
	*
	*	FUNCTION		CDynamicsAllocator::FreeAllocations
	*
	*	DESCRIPTION		Cleanup recorded allocations
	*
	***************************************************************************************************/
	void	CDynamicsAllocator::FreeAllocations( void )
	{
		while(!m_obIndexList.empty())
		{
			NT_DELETE_ARRAY( m_obIndexList.back() );
			m_obIndexList.pop_back();
		}

		while(!m_obVolumeList.empty())
		{
			NT_DELETE( m_obVolumeList.back() );
			m_obVolumeList.pop_back();
		}
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CDynamicsAllocator::AllocateNewIndexBuffer
	*
	*	DESCRIPTION		Cache this new allocation for later free
	*
	***************************************************************************************************/
	unsigned short*	CDynamicsAllocator::AllocateNewIndexBuffer( u_int uiSize )
	{
		unsigned short* pausReturn = NT_NEW unsigned short[uiSize];
		m_obIndexList.push_back( pausReturn );
		return pausReturn;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CDynamicsAllocator::AllocateNewVolumeData
	*
	*	DESCRIPTION		Cache this new allocation for later free
	*
	***************************************************************************************************/
	CVolumeData*	CDynamicsAllocator::AllocateNewVolumeData( void )
	{
		CVolumeData* pobReturn = NT_NEW CVolumeData();
		m_obVolumeList.push_back( pobReturn );
		return pobReturn;
	}
}
