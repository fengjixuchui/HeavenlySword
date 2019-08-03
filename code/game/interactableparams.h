#ifndef _INTERACTABLEPARAMS_H
#define _INTERACTABLEPARAMS_H

class CUsePointAttr
{
public:

	HAS_INTERFACE(CUsePointAttr)

	ntstd::List<CHashedString>	m_obUsePointAnimList;
	void OnPostConstruct();
	CHashedString GetAnim(const char* pobAnimType) const 
	{	
		ntstd::Map< CHashedString, CHashedString >::const_iterator obIt = m_obCharUseAnimMap.find(CHashedString(pobAnimType));
		if (obIt!=m_obCharUseAnimMap.end())
		{
			return (*obIt).second;
		}
        return CHashedString(HASH_STRING_NULL);
	}

//	bool HeroCanUse() const		{	return m_bHeroCanUse;	}
//	bool ArcherCanUse() const	{	return m_bArcherCanUse;	}
protected:
	// the hashed string of anim type per player type (map)
    ntstd::Map< CHashedString, CHashedString > m_obCharUseAnimMap;

private:
//	bool m_bArcherCanUse;
//	bool m_bHeroCanUse;

};

class Attr_Interactable
{
public:
	// scee.sbashow instead of a reference to a sperate 
	// CUsePointAttr structure, this could be part of this class, I guess.
	// However, if different interfaces use different chunks, it could be problematic.
	CUsePointAttr	*m_pobUsePointAttrs;

};

#endif



