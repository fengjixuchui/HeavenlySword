//--------------------------------------------------------------------------------
//!
//! \file ntstr.h
//! String-related shims
//!	should only be included from keystring.h
//! all the operations over our string types should be expressed here
//! using these shims allows us to abstract from the actual string type used
//! 
//--------------------------------------------------------------------------------

#ifdef KEYSTRING_H_INTERNAL

#ifndef NTSTR_H
#define NTSTR_H

// Only made a struct in order to help 'friendship'
// Conceptually a namespace
struct ntStr
{
	// Character string representation accessors
	static inline const char* GetString(const CKeyString& str)
	{
		return str.GetString();
	}

	static inline const char* GetString(const CHashedString& str)
	{
		return str.GetDebugString();
	}

	static inline const char* GetString(const ntstd::String& str)
	{
		return str.c_str();
	}

	static inline const char* GetString(const char* str)
	{
		return str;
	}


	// Hash key accessors
	static inline uint32_t GetHashKey(const CHashedString& str)
	{
		return str.GetHash();
	}

	static inline uint32_t GetHashKey(const CKeyString& str)
	{
		return str.GetHash();
	}

	static inline uint32_t GetHashKey(const FwHashedString& str)
	{
		return str.Get();
	}


	// Length functions
	static inline uint32_t GetLength(const char* str)
	{
		return strlen(str);
	}

	static inline uint32_t GetLength(const ntstd::String& str)
	{
		return str.length();
	}

	static inline uint32_t GetLength(const CKeyString& str)
	{
		return strlen(ntStr::GetString(str));
	}

	// IsNull functions
	static inline bool IsNull(const CHashedString& str)
	{
		return str.IsNull();
	}

	static inline bool IsNull(const CKeyString& str)
	{
		return str.IsNull();
	}


	static inline bool IsNull(const char* str)
	{
		return NULL == str || !str[0];
	}

	static inline bool IsNull(const ntstd::String& str)
	{
		return str.empty() || str == "NULL";
	}

	//template < class String, template <class> class Container >
	//static inline void Parse(String strToParse, Container<String>& obParseList, const char* pcDelims)  // - VC++ did not like this
    template < class String, class Container >
	static inline void Parse(String strToParse, Container& obParseList, const char* pcDelims)
	{
		char cBuffer[256];
		ntAssert(GetLength(strToParse) < 256);
		strcpy(cBuffer, GetString(strToParse));

		char* pcNext = strtok(cBuffer, pcDelims);	
		// 
		while (pcNext != 0)
		{
			obParseList.push_back(String(pcNext));
			pcNext = strtok(0, pcDelims);
		}
	}
};

#endif
#endif
