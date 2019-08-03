//------------------------------------------------------
//!
//!	\file keywords.h
//!
//------------------------------------------------------

#include "keywords.h"


//--------------------------------------------------
//!
//! CKeywords::CKeywords
//!
//--------------------------------------------------
CKeywords::CKeywords ()
{
	Set(0);
}

//--------------------------------------------------
//!
//! CKeywords::CKeywords
//!
//--------------------------------------------------
CKeywords::CKeywords (const char* pcString)
{
	Set(pcString);
}

//--------------------------------------------------
//!
//! CKeywords::Set
//!
//--------------------------------------------------
void CKeywords::Set (const char* pcString)
{
	if (!pcString || *pcString == 0)
	{
		for(uint iCount=0; iCount<MAX_KEYWORDS; ++iCount)
		{
			m_uiKeywordList[iCount]=0;
		}

		m_uiKeywordCount=0;
			
		return;
	}

	int iStringLength=strlen(pcString);

	if (iStringLength==0)
		return;

	if (iStringLength>=(int)MAX_KEYWORD_LENGTH)
	{
		ntPrintf("Keyword %s exceeds the 64 character limit\n",pcString);
		return;
	}

#ifdef _USE_STRINGS
	strcpy(m_acKeywords,pcString);
#endif // _USE_STRINGS

	int iListIndex=0;

	int iStart=0;
	int iEnd=0;

	while(iEnd<=iStringLength)
	{
		if ((pcString[iEnd]==',' || pcString[iEnd]=='\0') && iStart!=iEnd)
		{
			char acBuffer [MAX_KEYWORD_LENGTH];

			ntAssert( iEnd-iStart < (int)(sizeof(acBuffer) - 1) );
			strncpy(acBuffer,pcString+iStart,iEnd-iStart);

			acBuffer[iEnd-iStart]='\0';

			iStart=iEnd+1;

			if (iListIndex<(int)MAX_KEYWORDS)
			{
				m_uiKeywordList[iListIndex]=CHashedString(acBuffer).GetValue();

				++iListIndex;
			}			
		}

		++iEnd;
	}

	m_uiKeywordCount=iListIndex;

	while(iListIndex<(int)MAX_KEYWORDS)
	{
		m_uiKeywordList[iListIndex]=0;
		
		++iListIndex;
	}
}

//--------------------------------------------------
//!
//! CKeywords::AddKeyword
//!
//--------------------------------------------------
void CKeywords::AddKeyword (const char* pcString)
{
	if (m_uiKeywordCount==MAX_KEYWORDS)
	{
		ntPrintf("CKeywords: Can't add keyword, max limit has been reached\n");
		return;
	}

	m_uiKeywordList[m_uiKeywordCount]= ntStr::GetHashKey(CHashedString(pcString));

	++m_uiKeywordCount;
}

//--------------------------------------------------
//!
//! CKeywords::RemoveKeyword
//!
//--------------------------------------------------
void CKeywords::RemoveKeyword (const char* pcString)
{
	if (m_uiKeywordCount==0)
		return;

	u_int uiKeyword = ntStr::GetHashKey(CHashedString(pcString));

	for(u_int uiIndex=0; uiIndex<m_uiKeywordCount; ++uiIndex)
	{
		if (uiKeyword==m_uiKeywordList[uiIndex])
		{
			while(uiIndex<m_uiKeywordCount-1)
			{
				m_uiKeywordList[uiIndex]=m_uiKeywordList[uiIndex+1];
				++uiIndex;
			}

			--m_uiKeywordCount;	

			m_uiKeywordList[m_uiKeywordCount]=0;
		}
	}
}

//--------------------------------------------------
//!
//! CKeywords::operator==
//!
//--------------------------------------------------
bool CKeywords::operator== (const CKeywords& obOther) const
{
	if (m_uiKeywordCount!=obOther.m_uiKeywordCount)
		return false;

	return (*this & obOther);
}

//--------------------------------------------------
//!
//! CKeywords::operator&
//!
//--------------------------------------------------
bool CKeywords::operator& (const CKeywords& obOther) const
{
	return !ContainsAny( obOther ) ? false : true;
}

//--------------------------------------------------
//!
//! CKeywords::Dump
//!
//--------------------------------------------------
void CKeywords::Dump ()
{
#ifdef _USE_STRINGS

	for(u_int iCount=0; iCount<MAX_KEYWORDS; ++iCount)
	{
		ntPrintf("#%d: %d\n",iCount,m_uiKeywordList[iCount]);
	}

#endif // _USE_STRINGS
}

//--------------------------------------------------
//!
//! CKeywords::Contains
//!
//--------------------------------------------------
bool CKeywords::Contains( const CHashedString& robHash ) const
{
	// Run through all the current keywords looking for a match
	for(u_int uiIndex=0; uiIndex<m_uiKeywordCount; ++uiIndex)
	{
		// Is there a match?
		if( m_uiKeywordList[uiIndex] == robHash.GetValue() )
			return true;
	}

	// No match found - return false. 
	return false;
}

//--------------------------------------------------
//!
//! CKeywords::ContainsAny
//!
//--------------------------------------------------
u_int CKeywords::ContainsAny( const CKeywords& obOther ) const
{
	if (m_uiKeywordCount==0 || obOther.m_uiKeywordCount==0)
		return 0;

	u_int uiFound=0;

	for(u_int uiThisIndex=0; uiThisIndex<m_uiKeywordCount; ++uiThisIndex)
	{
		for(u_int uiOtherIndex=0; uiOtherIndex<obOther.m_uiKeywordCount; ++uiOtherIndex)
		{
			if (m_uiKeywordList[uiThisIndex]==obOther.m_uiKeywordList[uiOtherIndex])
			{
				++uiFound;
				break;
			}
		}
	}

	return uiFound;
}
