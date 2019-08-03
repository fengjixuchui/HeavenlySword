//--------------------------------------------------
//!
//!	\file effect_resetable.h
//!	simple class for editing interactions
//!
//--------------------------------------------------

#ifndef _RESETABLE_H
#define _RESETABLE_H

//--------------------------------------------------
//!
//!	Resetable
//! Provide an interface for classes plonked in a
//! ResetSet
//!
//--------------------------------------------------
class Resetable
{
public: 
	virtual ~Resetable(){};
	virtual void Reset( bool bInDestructor ) = 0;
};

//--------------------------------------------------
//!
//!	ResetSet
//! Debug class that allows us to do some housekeeping
//!
//--------------------------------------------------
template<typename T>
class ResetSet 
{
public:
	typedef ntstd::Set<T*>				typeSet;
	typedef typename typeSet::iterator	typeSetIt;

	typedef ntstd::List<T*>				typeList;
	typedef typename typeList::iterator	typeListIt;

	void RegisterThingToReset( T* pThingy ) const
	{
		ntAssert( pThingy );
		ntAssert( m_thingsToReset.find(pThingy) == m_thingsToReset.end() );
		m_thingsToReset.insert( pThingy );
	}

	void UnRegisterThingToReset( T* pThingy ) const
	{
		ntAssert( pThingy );
		typeSetIt it = m_thingsToReset.find(pThingy);
		ntAssert( it != m_thingsToReset.end() );
		m_thingsToReset.erase( it );
	}

	// use a proxy list incase what we're resting changes the contents
	// of our reset set.
	void ResetThings()
	{
		// copy to a proxy list
		typeList toReset;
		
		for (	typeSetIt it = m_thingsToReset.begin();
				it != m_thingsToReset.end(); ++it	)
		{
			toReset.push_back( *it );
		}

		// reset proxy list
		for (	typeListIt it = toReset.begin();
				it != toReset.end(); ++it	)
		{
			(*it)->Reset( false );
		}
	}

private:
	mutable typeSet m_thingsToReset;
};

#endif
