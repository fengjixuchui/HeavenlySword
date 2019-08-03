/***************************************************************************************************
*
*       DESCRIPTION    CKeyString header
*
*       NOTES          See class description 
*
***************************************************************************************************/

//#include "core/hash.h"

#include <Fw/FwCrc.h>
#include "keystring.h"
#include "core/gatso.h"


/// DEFINES
// This symbol is emitted by the linker (in case if the default linker script is used) and marks the end of the elf in memory
#define ELF_END_MARKER		_end

#ifdef PLATFORM_PS3
// Check if we are dealing with a globally allocated string 
#define IS_STATIC(str) ((str) < g_ElfEnd)
extern char ELF_END_MARKER;
const char*	g_ElfEnd = &ELF_END_MARKER;

#else
// I don't bother doing this on pc
#define IS_STATIC(str) (false)

#endif


/// TYPES
// yes I want my own list type because I dont need doubly-linked std::list where single-linked would suffice
// besides I want to manage memory myself
template <class T>
class CListElement
{
protected:
	CListElement() : next_(NULL)
	{}

public:
	T*	Next() const
	{
		return next_;
	}

	T* Link(T* newElem)
	{
		ntAssert_p(!next_, ("Trying to replace a non empty node"));
		next_ = newElem;
		return next_;
	}
private:
	T* next_;
};

class CStringElement : public CListElement<CStringElement>
{
public:
	CStringElement() : str_(NULL) {}
	CStringElement(CHashedString::HashKeyType hash, const char* str)
	{
		Set(hash, str);
	}

	CHashedString::HashKeyType GetHash() const
	{
		return hash_;
	}

	const char*	GetString() const
	{
		return str_;
	}

	bool IsAvailable() const
	{
		return NULL == str_;
	}

	const char* Set(CHashedString::HashKeyType hash, const char* str)
	{
		hash_ = hash;

		// If the string is inside the elf image then it is most likely a literal
		// so i just store a pointer to it
		// Therefore global non-const character arrays must be banned! Hopefully noone tries to do anything like it
		if (IS_STATIC(str))
		{
			// I will manage the constness myself!
			str_ = const_cast<char*>(str);
		}
		else
		{
			unsigned int len = strlen(str);

			// replace with a call to string allocator
			str_ = NT_NEW char[len + 1];
			NT_MEMCPY(str_, str, len + 1);
		}

		return str_;
	}


	CHashedString::HashKeyType	hash_;
	char*					str_;
};


/// GLOBALS

const unsigned int c_hashTableBits = 13;
const unsigned int c_hashTableSize = 1 << c_hashTableBits;
const unsigned int c_hashMask = c_hashTableSize - 1;

const CHashedString CHashedString::nullString; 
const char* defaultHashedString = "String not available";
const char* nullString	= "NULL";

CStringElement* GetHashTable()
{
	static CStringElement hashTable[c_hashTableSize];

	return hashTable;
}

const CStringElement*	GetNotFoundString()
{
	static CStringElement	notFound(0, "Not Found");

	return &notFound;
}

const char*	GetString(CHashedString::HashKeyType hash)
{
	CGatso::Start( "GetString" );
	unsigned int index			= hash & c_hashMask;
	const CStringElement* elem	= GetHashTable() + index;
	while (elem -> GetHash() != hash)
	{
		elem = elem -> Next();

		// at the moment some strings may not be in the table (hashed strings from ATG)
		// in the future once I start loading the hash database I may get away with just an assert here
		if (!elem)
		{
			//ntAssert_p( 0, ("String not found in the hash table"));
			elem = GetNotFoundString();
			break;
		}
	}

	CGatso::Stop( "GetString" );
	return elem -> GetString();
}

//------------------------------------------------------
//!
//! Adds a string to the hash table if it's not in there yet
//! Returns a direct pointer to string - to be used for quick access in debugger (DEBUG mode only!)
//! Use HASHEDSTRING_CHECK_HASH_CONSISTENCY to check for hash clashes
//!
//------------------------------------------------------
const char* AddString(CHashedString::HashKeyType hash, const char* str)
{
	CGatso::Start( "AddString" );
	unsigned int index			= hash & c_hashMask;
	CStringElement* elem	= GetHashTable() + index;
	CStringElement* newElem	= elem;

	if (!str || *str == 0)
	{
		str = nullString;
	}

	// this is the first element in the subhash list then
	if (elem -> IsAvailable())
	{
		return elem -> Set(hash, str);
	}
	else do 
	{
		elem = newElem;

		// The string is already in the database... unless it is a hash clash!
		if (elem -> GetHash() == hash)
		{
#ifdef CHECK_HASH_COLLISIONS
			const char* existingString = elem -> GetString();
			if (strcmp(existingString, str) != 0)
			{
				ntPrintf( Debug::DCU_RESOURCES, "**** Hash clash found between \"%s\" and \"%s\" ****\n" , existingString, str);
				//ntBreakpoint();
			}
#endif
			return elem -> GetString();
		}
		else
		{
			newElem = elem -> Next();
		}
	} while(newElem);

	//snPause();

	// if we get here then there is no such record in the database yet. Need to create one!
	newElem = elem -> Link(NT_NEW CStringElement());

	//return newElem -> Set(hash, str);
	const char* ret = newElem -> Set(hash, str);
	CGatso::Stop( "AddString" );

	return ret;

}

//------------------------------------------------------
//!
//! Deallocates all heap memory occupied by the hash table
//! Note: this all should be changed to work with a custom memory allocator
//!
//------------------------------------------------------
void DestroyGlobalStringTable()
{
	CStringElement*	table = GetHashTable();

	for ( unsigned int elem = 0; elem < c_hashTableSize; ++ elem )
	{
		CStringElement*		stringItem = &table[elem];
		if (!IS_STATIC(stringItem -> str_))
		{
			NT_DELETE(stringItem -> str_);
		}

		CStringElement*		nextStringItem = stringItem -> Next();
		while(nextStringItem)
		{	
			stringItem = nextStringItem;

#ifdef PLATFORM_PS3
			if (!IS_STATIC(stringItem -> str_))
#else
			if (stringItem -> str_)
#endif
			{
				NT_DELETE(stringItem -> str_);
			}
			nextStringItem = stringItem -> Next();
			NT_DELETE(stringItem);
		}  
	}
}
