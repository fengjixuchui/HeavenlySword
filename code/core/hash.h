//--------------------------------------------------
//!
//!	\file hash.h
//!	switch between jam and atg hashed string
//!
//--------------------------------------------------

#ifndef CORE_HASH_H
#define CORE_HASH_H

#include "debug.h"

//#if defined( PLATFORM_PC )
//	
//	#include "jamhash.h"
//	typedef CJamHashedString CHashedString;
//
//#elif defined( PLATFORM_PS3 )
	#include "keystring.h"
	
	//#include <Fw/FwStd/FwHashedString.h>
	
	/***************************************************************************************************
	*
	*	CLASS			CHashedString
	*
	*	DESCRIPTION		compatablility class for platform agnostic hash string code
	*
	***************************************************************************************************/
	//class CHashedString: public FwHashedString
	//{
	//public:
	//	// explicit conversion constructor from FwHashedString to CHashedString
	//	CHashedString(FwHashedString hash):FwHashedString(hash.Get()) {}
	//	CHashedString():FwHashedString() {}
	//	CHashedString(const char* pcString):FwHashedString(pcString) {}
	//	u32 GetValue() const {return Get();}
	//	static unsigned int GenerateHash( const char* pcString ) { return FwHashedString( pcString ).Get(); }

	//}; // end of class CHashedString

//#endif

/***************************************************************************************************
*
*	CLASS			QuickPtrList
*
*	DESCRIPTION		fast retrieval of pointers to type T
*
*	REQUIRES		T implements a method 'const char* T::GetHashable() const' that is the basis of our
*					generated key.
*
*	NOTE			only implements basic iterator access, add(T&), remove(T&), find(const char*)
*					and is not supposed to be STL like. This is intentional!
*
*	WHY?			Because we dont have fast allocating hash_map, and this can be optimised.
*
***************************************************************************************************/
template< typename T>
class QuickPtrList
{
private:
	// internal typedefs
	typedef typename ntstd::Map<uint32_t, T*>	map;
	typedef typename map::iterator				mapit;
	typedef typename map::const_iterator		cmapit;
	map m_map;

public:
	// iterators that hide map implementation
	class iterator
	{
	public:
		explicit iterator( mapit me ) { m_realIt = me; }
		T*		operator*() { return m_realIt->second; }
		void	operator++() { m_realIt++; }
		bool	operator==(const iterator& other) const { return (this->m_realIt == other.m_realIt); }
		bool	operator!=(const iterator& other) const { return (this->m_realIt != other.m_realIt); }
		mapit m_realIt;
	};

	class const_iterator
	{
	public:
		explicit const_iterator( cmapit me ) { m_realIt = me; }
		T*		operator*() { return m_realIt->second; }
		void	operator++() { m_realIt++; }
		bool	operator==(const const_iterator& other) const { return (this->m_realIt == other.m_realIt); }
		bool	operator!=(const const_iterator& other) const { return (this->m_realIt != other.m_realIt); }
		cmapit m_realIt;
	};

	// pass through to ntstd::Map functions, no validation, minimal implementation
	iterator		begin() { return iterator( m_map.begin() ); }
	iterator		end()	{ return iterator( m_map.end() ); }

	const_iterator	begin() const { return const_iterator( m_map.begin() ); }
	const_iterator	end()	const { return const_iterator( m_map.end() ); }

	iterator erase(iterator where) { return iterator( m_map.erase(where.m_realIt) ); }
	bool empty() const { return m_map.empty(); }
	uint32_t size() const { return m_map.size(); }

	// list like operators
	T* front() const
	{
		if (empty()) return 0;
		return m_map.begin()->second;
	}

	T* back() const
	{
		if (empty()) return 0;
		cmapit it = m_map.end(); --it;
		return it->second;
	}

	// add an object to the list. note, takes a reference rather than a
	// pointer, so we do not require validation.
	// check for duplicated adds on _DEBUG only
	void add(T& obj)
	{
		ntAssert_p( !find(obj), ("Object: %s already within list", ntStr::GetString(obj.GetName())) );
    	uint32_t key = obj.GetHashKey();
		m_map[key] = &obj;
	}

	// remove an object from the list. note, takes a reference rather than a
	// pointer, so we do not require validation.
	// check for spurious removals on _DEBUG only
	void remove(T& obj)
	{
		//uint32_t key = CHashedString::GenerateHash( obj.GetHashable() );
		uint32_t key = obj.GetHashKey();
		mapit where = m_map.find(key);
		ntAssert_p( where != m_map.end(), ("Object: %s not within list", ntStr::GetString(obj.GetName())) );
		m_map.erase(where);
	}

	// Find an object given its hash key
	T* find( uint32_t key )
	{
		mapit where = m_map.find(key);
		if (where != m_map.end())
			return where->second;
		return 0;
	}

	// find an object based on the same string as its T::GetHashable() method
	// returns. NOTE! this is intentionally case sensitive. Perfectly valid to
	// look for something that does not exist.
	T* find(const char* pHashable)
	{
		ntAssert_p( pHashable, ("Must provide hashable string for find") );
		uint32_t key = ntStr::GetHashKey(CHashedString( pHashable ));
		return find(key);
	}

	// see if this object is inside the map
	// NOTE! could implement a reverse map rather than generate a new hash,
	// as would be faster at the expense of doubling the size.
	T* find(T& obj) { return find( obj.GetHashKey() ); }
};


#endif // end of CORE_HASH_H
