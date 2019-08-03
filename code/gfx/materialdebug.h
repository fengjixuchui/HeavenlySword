//--------------------------------------------------
//!
//!	\file materialdebug.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _MATERIALDEBUG_H
#define _MATERIALDEBUG_H

class NameCounter;

//--------------------------------------------------
//!
//!	Classes for recording material usage
//!
//--------------------------------------------------
class ExclusiveNameList
{
public:
	ExclusiveNameList() : m_pNameList( NULL ) {}
	~ExclusiveNameList() { ClearList(); }

	void InsertIntoList( const char* );
	void PrintList();
	void ClearList();

private:
	typedef ntstd::List<NameCounter*, Mem::MC_GFX> NameCounterList;
	NameCounterList* m_pNameList;
};

//--------------------------------------------------
//!
//!	material debugging shtuff
//!
//--------------------------------------------------
//#define RECORD_MATERIAL_USAGE
class MaterialDebug
{
public:
	static void ShortenName( const char* inStr, char* pOutStr );

	static void DumpMaterialUsage()
	{
#ifdef RECORD_MATERIAL_USAGE
		m_materialRecord.PrintList();
		m_materialRecord.ClearList();
#endif
	}

	static void RecordMaterialUsage( const char* pName )
	{
		UNUSED(pName);
#ifdef RECORD_MATERIAL_USAGE
		m_materialRecord.InsertIntoList( pName );
#endif
	}

private:
	ExclusiveNameList m_materialRecord;
};

//--------------------------------------------------
//!
//!	material debugging shtuff
//!
//--------------------------------------------------
class NameCounter
{
public:
	NameCounter( const char* pName ) : m_iCount(1)
	{
		ntAssert( pName );
		MaterialDebug::ShortenName( pName, m_pName );
	};

	void Increment() { m_iCount++; }
	const char* GetName() const { return m_pName; }
	int GetCount() const { return m_iCount; }
	int Compare( const NameCounter& test ) const { return strcmp( m_pName, test.GetName() ); }

private:
	char	m_pName[128];
	int		m_iCount;
};

#endif
