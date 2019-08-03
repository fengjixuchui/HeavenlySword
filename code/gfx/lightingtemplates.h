//--------------------------------------------------
//!
//!	\file lightingtemplates.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _LIGHTING_TEMPLATES_H
#define _LIGHTING_TEMPLATES_H

//--------------------------------------------------
//!
//!	SortableList
//!	Template class that allows us to do useful things
//! based on the template class having a member function
//! float T::GetSortingValue()
//!
//--------------------------------------------------
template<typename T, int C = Mem::MC_MISC > class SortableList
{
public:
	SortableList() : m_eSortType(UNSORTED) {};

	enum SORT_TYPE
	{
		ASCENDING,
		DESCENDING,
		UNSORTED,
	};

	typedef ntstd::List<T*, C> listType;
	typedef typename listType::const_iterator listIt;

	listType	m_list;
	SORT_TYPE	GetSortType() const { return m_eSortType; }

	inline bool IsUnsorted() const;
	inline void SortListAscending();
	inline void SortListDescending();
	inline float GetBoundingNodes( float fKey, const T** ppPrev, const T** ppNext, float fWrapValue = -1.0f ) const;

private:
	SORT_TYPE m_eSortType;

	class comparatorLessThan
	{
	public:
		bool operator()( const T* pFirst, const T* pSecond ) const
		{
			return ( pFirst->GetSortingValue() < pSecond->GetSortingValue() );
		}
	};

	class comparatorGreaterThan
	{
	public:
		bool operator()( const T* pFirst, const T* pSecond ) const
		{
			return ( pFirst->GetSortingValue() > pSecond->GetSortingValue() );
		}
	};
};

//--------------------------------------------------
//!
//!	SortableList::IsUnsorted 
//! see if we require sorting
//!
//--------------------------------------------------
template<class T, int C> inline bool SortableList<T,C>::IsUnsorted() const
{
	if (m_eSortType == UNSORTED)
	{
		return true;
	}
	else if (m_eSortType == ASCENDING)
	{
		float fMin = -MAX_POS_FLOAT;
		for ( listIt it = m_list.begin(); it != m_list.end(); ++it )
		{
			if ((*it)->GetSortingValue() < fMin)
				return true;
			fMin = (*it)->GetSortingValue();
		}
	}
	else if (m_eSortType == DESCENDING)
	{
		float fMax = MAX_POS_FLOAT;
		for ( listIt it = m_list.begin(); it != m_list.end(); ++it )
		{
			if ((*it)->GetSortingValue() > fMax)
				return true;
			fMax = (*it)->GetSortingValue();
		}
	}
	return false;
}

//--------------------------------------------------
//!
//!	SortableList::SortListAscending 
//! sort the list based on ascending values
//!
//--------------------------------------------------
template<class T, int C> inline void SortableList<T,C>::SortListAscending()
{
	if ( !m_list.empty() )
	{
		int iElements = m_list.size();
		ntstd::Vector<T*, Mem::MC_GFX> temp( iElements );

		while ( iElements )
		{
			temp[--iElements] = m_list.back();
			m_list.pop_back();
		}

		ntstd::sort( &temp[0], &temp[0] + temp.size(), comparatorLessThan() );

		for (unsigned int i = 0; i < temp.size(); i++ )
		{
			m_list.push_back( temp[i] );
		}
		m_eSortType = ASCENDING;
	}
}

//--------------------------------------------------
//!
//!	SortableList::SortListAscending 
//! sort the list based on ascending values
//!
//--------------------------------------------------
template<class T, int C> inline void SortableList<T,C>::SortListDescending()
{
	if ( !m_list.empty() )
	{
		int iElements = m_list.size();
		ntstd::Vector<T*, Mem::MC_GFX> temp( iElements );

		while ( iElements )
		{
			temp[--iElements] = m_list.back();
			m_list.pop_back();
		}

		ntstd::sort( &temp[0], &temp[0] + temp.size(), comparatorGreaterThan() );

		for ( unsigned int i = 0; i < temp.size(); i++ )
		{
			m_list.push_back( temp[i] );
		}
		m_eSortType = DESCENDING;
	}
}

//--------------------------------------------------
//!
//!	SortableList::GetBoundingNodes 
//! retrieve the two nodes that bound this key value
//! returns the lerp value we are between prev and next
//! if fWrapValue is a non-negative number, its used to generate
//! a wrapped lerp value (say for a 24 hour clock)
//!
//--------------------------------------------------
template<class T, int C> float SortableList<T,C>::GetBoundingNodes( float fKey, const T** ppPrev, const T** ppNext, float fWrapValue ) const
{
	// we shouldnt have been called if we're empty
	ntAssert( !m_list.empty() );

	// make sure our list is still ordered
	ntAssert( !IsUnsorted() );

	*ppPrev = 0;
	*ppNext = 0;

	for ( listIt it = m_list.begin(); ; ++it )
	{
		listIt next( it ); ++next;

		// over the end of the list
		if (next == m_list.end())
		{
			*ppPrev = *it;

			if( fWrapValue < 0.0f )
			{
				*ppNext = *it;
				return 0.0f;
			}
			else // we are supposed to wrap
			{
				*ppNext = m_list.front();
				break;
			}
		}

		// over the start of the list
		if	(
			((m_eSortType == ASCENDING) && (fKey < (*it)->GetSortingValue())) ||
			((m_eSortType == DESCENDING) && (fKey > (*it)->GetSortingValue()))
			)
		{
			*ppNext = (*it);

			if( fWrapValue < 0.0f )
			{
				*ppPrev = *it;
				return 0.0f;
			}
			else
			{
				listIt prev( m_list.end() ); --prev;
				*ppPrev = (*prev);
				break;
			}
		}

		// found our nodes
		if	(
			((m_eSortType == ASCENDING) && (fKey <= (*next)->GetSortingValue())) ||
			((m_eSortType == DESCENDING) && (fKey >= (*next)->GetSortingValue()))
			)
		{
			*ppPrev = (*it);
			*ppNext = (*next);
			break;
		}
	}

	// calculate fraction between nodes we need to lerp.
	ntAssert(*ppPrev && *ppNext);

	if (*ppPrev != *ppNext)
	{
		float fMin = (*ppPrev)->GetSortingValue();
		float fMax = (*ppNext)->GetSortingValue();

		if ((m_eSortType == ASCENDING) && (fMin > fMax))
			fMax += fWrapValue;

		if ((m_eSortType == DESCENDING) && (fMin < fMax))
			fMin += fWrapValue;

		float fDiff = fMax - fMin;
		return (fabsf(fDiff) > EPSILON ) ? ((fKey - fMin) / fDiff) : 0.0f;
	}

	return 0.0f;
}

#endif // end _LIGHTING_TEMPLATES_H
