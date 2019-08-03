#ifndef _MESSAGEDATA_H_
#define _MESSAGEDATA_H_

// Necessary includes

// Forward Declarations

class BaseMessageDataDef
{
	HAS_INTERFACE( BaseMessageDataDef );

public:
	BaseMessageDataDef() {};
	~BaseMessageDataDef() {};

protected:
	CHashedString m_obKey;
};

class MessageDataInt : public BaseMessageDataDef
{
	HAS_INTERFACE( MessageDataInt );

public:
	MessageDataInt() {};
	~MessageDataInt() {};

	void PostConstruct( void );

private:
	int	m_iValue;
};

class MessageDataString : public BaseMessageDataDef
{
	HAS_INTERFACE( MessageDataString );

public:
	MessageDataString() {};
	~MessageDataString() {};

	void PostConstruct( void );

private:
	ntstd::String	m_obValue;
};

class MessageDataManager
{
public:
	MessageDataManager() {};
	~MessageDataManager();

	void Reset( void );

	bool GetValue ( CHashedString obKey, int& iValue );
	bool SetValue ( CHashedString obKey, int iValue );
	void CreateValue ( CHashedString obKey, int iValue );

	bool GetValue ( CHashedString obKey, ntstd::String& obValue );
	bool SetValue ( CHashedString obKey, ntstd::String obValue );
	void CreateValue ( CHashedString obKey, ntstd::String obValue );

	bool ToStringW ( CHashedString obKey, WCHAR_T* pwcBuffer, int iBufferLength );

protected:  
	ntstd::Map< CHashedString, int > m_aobIntList;

	ntstd::Map< CHashedString, ntstd::String > m_aobStringList;
};

typedef ntstd::Map< CHashedString, int >::iterator iMapIter;
typedef ntstd::Map< CHashedString, ntstd::String >::iterator strMapIter;
#endif // _MESSAGEDATA_H_
