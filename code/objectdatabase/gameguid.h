//------------------------------------------------------
//!
//!	\file core\gameguid.h
//!
//------------------------------------------------------
#if !defined(OBJECTDATABASE_GAMEGUID_H)
#define OBJECTDATABASE_GAMEGUID_H

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    uint32_t  Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
} GUID;
#endif /* GUID_DEFINED */

struct GameGUID
{ 
	GameGUID();

	GameGUID( const GameGUID& other );

	// compatible with the CreateGuid tool in visual studio (use the static const version delete the { and add GameGuid( ... ) around it
	GameGUID( unsigned long d1, unsigned short d2, unsigned short d3, unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f, unsigned char g, unsigned char h );

	bool IsNull()  const;

	bool operator==(const GameGUID& rhs ) const;

	bool operator<(const GameGUID& rhs ) const;

	const ntstd::String GetAsString() const;

	void SetFromString( const ntstd::String& str);

	friend ntstd::Istream& operator >>(ntstd::Istream &is, GameGUID &obj);
	
	friend ntstd::Ostream& operator <<(ntstd::Ostream &os, const GameGUID &obj );

	GUID m_data;
};


#endif
