//------------------------------------------------------
//!
//!	\file core\gameguid_pc.cpp
//!
//------------------------------------------------------
#include "objectdatabase/gameguid.h"
#include <rpc.h>
#include <sstream>

GameGUID::GameGUID()
{ 
	memset( &m_data,0, sizeof(GUID) ); 
}

GameGUID::GameGUID( const GameGUID& other )
{
	NT_MEMCPY( &m_data, &other.m_data, sizeof(GUID) );
}

GameGUID::GameGUID( unsigned long d1, unsigned short d2, unsigned short d3, unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f, unsigned char g, unsigned char h )
{
	m_data.Data1 = d1;
	m_data.Data2 = d2;
	m_data.Data3 = d3;
	m_data.Data4[0] = a;
	m_data.Data4[1] = b;
	m_data.Data4[2] = c;
	m_data.Data4[3] = d;
	m_data.Data4[4] = e;
	m_data.Data4[5] = f;
	m_data.Data4[6] = g;
	m_data.Data4[7] = h;
}

bool GameGUID::IsNull()  const
{
	if( m_data.Data1 == 0 && 
		m_data.Data2 == 0 && 
		m_data.Data3 == 0 &&
		((unsigned int*)m_data.Data4)[0] == 0 &&
		((unsigned int*)m_data.Data4)[1] == 0 )
		return true;
	else
		return false;
}

bool GameGUID::operator==(const GameGUID& rhs ) const
{
	RPC_STATUS dummy;
	return !UuidCompare((UUID*)&m_data, (UUID*)&rhs.m_data,&dummy);
}

bool GameGUID::operator<(const GameGUID& rhs ) const 
{
	RPC_STATUS dummy;
	return (UuidCompare((UUID*)&m_data, (UUID*)&rhs.m_data,&dummy)<0);
}

const ntstd::String GameGUID::GetAsString() const
{
	char *temp;
	ntstd::String realString;

	UuidToStringA( (UUID*) &m_data, (unsigned char **) &temp );
	realString = ntstd::String( temp );
	RpcStringFreeA( (unsigned char **) &temp );

	return realString;
}

void GameGUID::SetFromString( const ntstd::String& str)
{
	UuidFromStringA( (unsigned char*) str.c_str(), (UUID*) &m_data );
}

ntstd::Istream& operator >>(ntstd::Istream &is, GameGUID &obj)
{
	ntstd::String temp;
	is >> temp;
	obj.SetFromString( temp );
	return is;
}

ntstd::Ostream& operator <<(ntstd::Ostream &os, const GameGUID &obj)
{
	ntstd::String temp( obj.GetAsString() );
	os << temp;
	return os;
}
