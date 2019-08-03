#ifndef _KEY_STRING_H
#define _KEY_STRING_H

#if defined(_DEBUG) || defined(_DEVELOPMENT)
// Enable this in debug to check for hash collsions
#define CHECK_HASH_COLLISIONS
// Enable this in debug so the string classes will get a direct pointer to a character string
// This also keeps the original strings for CHashedString class
#define KEYSTRING_INCLUDE_DEBUG_INFO
#endif

//#if defined( PLATFORM_PC )
//#include "jamhash.h"
//typedef CJamHashedString HashedStringBaseClass;
//#else
#include <Fw/FwStd/FwHashedString.h>
//! Define the underlying hash-generator type
typedef FwHashedString HashedStringBaseClass;
//#endif

#include "hashcodes.h"

// this is just a useful debug macro to check if a character string was missed during C++ - LUA - C++ transition
// it usually indicates that LUA only received a hash code from C++ but C++ tries to accept char* back from LUA
// best solution is to make C++ lua-exposed functions to operate on hashed strings only
//#define CHECK_STR(str) ntAssert(strstr((str), "LUA") == 0);
#define CHECK_STR(str) 

class CHashedString;

//--------------------------------------------------------------------------------
//!
//! A callback parameter 
//! ok I know I did not have to do this... but so everyone now knows that it's not literally a char*
//! and suitable for passing nice little CHashedString's just as well
//! besides this union will make it a bit more type-safe
//! it can be extended to hold floating point members if needed.
//!
//--------------------------------------------------------------------------------
union CallBackParameter
{
public:
	CallBackParameter(const CHashedString& str);

	CallBackParameter(unsigned int data)	: m_intData(data) {}

	template <typename T>
	CallBackParameter(T* data) : m_ptrData((void*)data)	{}

	unsigned int	GetInt()
	{
		return m_intData;
	}

	void*			GetPtr()
	{
		return m_ptrData;
	}

private:
	void*			m_ptrData;
	unsigned int	m_intData;
};

extern const char* defaultHashedString;
extern const char* nullString;


//--------------------------------------------------------------------------------
//!
//! Templated base for CHashedString and CKeyString
//! its only purpose is to contain the code shared by the two classes
//! note that since the base is templated it does not introduce any formal relationship 
//! between CHashedString and CKeyString
//! 
//--------------------------------------------------------------------------------
template <class Derived>
class CHashedStringBase	
{
	friend struct ntStr;

public:

	typedef uint32_t HashKeyType;

protected:

	CHashedStringBase() : base_(m_null)
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		, str_(defaultHashedString)
#endif
	{}

	CHashedStringBase(HashKeyType hash) : base_(hash)
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		, str_(defaultHashedString)
#endif
	{}

#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
	CHashedStringBase(CHashCode hashCode) : base_(hashCode.code_)
		, str_(hashCode.str_)
	{}
#endif

	CHashedStringBase(const char* str) : base_((str && str[0])? str : nullString)
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		, str_(str ? defaultHashedString : nullString)
#endif
	{
	}


public:
	// -------- All these must actually be made private so the hash value could only be accessed via the appropriate shim
	HashKeyType GetValue() const
	{
		return GetHash();
	}


	HashKeyType GetHash() const
	{
		return base_.Get();
	}


	HashKeyType Get() const
	{
		return GetHash();
	}

	bool IsNull() const
	{
		return (GetHash() == m_null);
	}
	// --------



	// Comparisons  we need them publicly
	bool operator< (Derived other) const
	{
		return (GetHash() < other.GetHash());
	}

	bool operator == (Derived other) const
	{
		return (GetHash() == other.GetHash()); 
	}

	bool operator != (Derived other) const
	{
		return !(*this == other);
	}

	bool operator == (HashedStringBaseClass other) const
	{
		return GetHash() == other.Get();
	}

	bool operator != (HashedStringBaseClass other) const
	{
		return !(*this == other);
	}


protected:
	void CreateFromString(const char* str)
	{
		extern const char* AddString(HashKeyType, const char*);

		#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		str_ = 
		#endif
		AddString(GetHash(), str);
	}

	const char* GetCharString()	const
	{
		extern const char*	GetString(HashKeyType);

		return GetString(GetHash());
	}


	// Disallow direct creation of the base type
	~CHashedStringBase() {}

private:
	HashedStringBaseClass	base_;

	#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
	const char*	str_;
	#endif

	static const HashKeyType	m_null = HASH_STRING_NULL;
};


class CKeyString;

//--------------------------------------------------------------------------------
//!
//! A hashed string class. IMO the only string class you need in a game
//! If KEYSTRING_INCLUDE_DEBUG_INFO is not defined then it only contains a hash code (4 bytes) 
//! and the actual string representation is not available
//! it is an ideal candidate for being used as a key 
//! 
//--------------------------------------------------------------------------------
class CHashedString : public CHashedStringBase<CHashedString>
{
	typedef CHashedStringBase<CHashedString> MyBase;

public :
	CHashedString() {}
	CHashedString(FwHashedString hash) : MyBase(hash.Get()) {}
	explicit CHashedString(HashKeyType hash) : MyBase(hash) {}
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
	explicit CHashedString(CHashCode hashCode) : MyBase(hashCode) {}
#endif
	explicit CHashedString(const CKeyString&);

	// TODO : better if made explicit
	CHashedString(const char* str) : MyBase(str)
	{
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		if (str)
		{
			CreateFromString(str);
		}
#endif
	}
	explicit CHashedString(const ntstd::String& str) : MyBase(str.empty() ? NULL : str.c_str())
	{
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
		if (!str.empty())
		{
			CreateFromString(str.c_str());
		}
#endif
	}
	explicit CHashedString(CallBackParameter param) : MyBase(param.GetInt()) {}
	~CHashedString() {}


	const char* GetDebugString() const
	{
		return GetCharString();
	}

public:
	static const CHashedString	nullString; 

};

//--------------------------------------------------------------------------------
//!
//! A key string string class. 
//! This is different from CHashedString in that the actual character string is always available
//! So each time a new string of this kind is created it will have to alloc memory, for the string
//! and add it to the global hash table, It will also have to go through the whole string and generate the hash code.
//! Please keep its usage to the minimum. 
//! It is only useful when you need the actual characters and want to search for the string later on.
//! 
//--------------------------------------------------------------------------------
class CKeyString : public CHashedStringBase<CKeyString>
{
	typedef CHashedStringBase<CKeyString> MyBase;
public:
	CKeyString() {}

	// TODO : better if made explicit
	CKeyString(const char* str) : MyBase(str)
	{
		if (str)
		{
			CreateFromString(str);
		}
	}

	explicit CKeyString(const ntstd::String& str) : MyBase(str.empty() ? NULL : str.c_str())
	{
		if (!str.empty())
		{
			CreateFromString(str.c_str());
		}
	}

	explicit CKeyString(const CHashedString& hash) : MyBase(hash.GetHash()) {}
	~CKeyString() {}

	const char* GetString() const
	{
		return GetCharString();
	}

	const char* GetDebugString() const
	{
		return "KeyString debug string";
	}

};

#define KEYSTRING_H_INTERNAL
#include "ntstr.h"
#undef KEYSTRING_H_INTERNAL

inline CHashedString::CHashedString(const CKeyString& keyStr) : MyBase(ntStr::GetHashKey(keyStr))
{
}


inline CallBackParameter::CallBackParameter(const CHashedString& str) : m_intData(ntStr::GetHashKey(str))	{}

// some useful comparison operators
//! A reversed == to compare a hash code (integer) directly to a HashedStrinbg
inline bool operator== (unsigned int hash, CHashedString str)
{
	return ntStr::GetHashKey(str) == hash;
}

#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
inline bool operator== (const CHashCode& hashCode, CHashedString str )
{
	return hashCode.code_ == str;
}
#endif

//! Similar to above but for the base class (FwHashedString)
inline bool operator== (HashedStringBaseClass baseStr, CHashedString str)
{
	return str == baseStr;
}

//! Makes it easier to compare KeyString's and HashedString's
inline bool operator== (CHashedString hashStr, CKeyString keyStr)
{
	return ntStr::GetHashKey(hashStr) == ntStr::GetHashKey(keyStr);
}

inline bool operator== (CKeyString keyStr, CHashedString hashStr)
{
	return ntStr::GetHashKey(hashStr) == ntStr::GetHashKey(keyStr);
}


#endif // _KEY_STRING_H
