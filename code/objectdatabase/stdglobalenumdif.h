#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif


class GlobalEnum
{
public:
	typedef unsigned int value_type;
	typedef ntstd::Map<ntstd::String, value_type, ntstd::less<ntstd::String>, Mem::MC_ODB >	enum_valcontainer;
	typedef ntstd::Map<value_type, ntstd::String, ntstd::less<value_type>, Mem::MC_ODB >	enum_stringcontainer;
	typedef enum_stringcontainer::const_iterator const_iterator;

	const value_type GetValue( const ntstd::String& name ) const
	{
		enum_valcontainer::const_iterator vcIt = m_EnumValues.find( name );
		ntAssert_p( vcIt != m_EnumValues.end(), ("Enum String doesn't exist (%s)", name.c_str() ) );
		return (*vcIt).second;
	}

	const ntstd::String& GetName( const value_type val ) const
	{
		enum_stringcontainer::const_iterator scIt = m_EnumStrings.find( val );
		ntAssert_p( scIt != m_EnumStrings.end(), ("Enum value doesn't exist") );
		return (*scIt).second;
	}

	void AddEnumPair( const ntstd::String& name, value_type val )
	{
		ntAssert_p( m_EnumValues.find( name ) == m_EnumValues.end(), ("Enum String already exists") );
		ntAssert_p( m_EnumStrings.find( val ) == m_EnumStrings.end(), ("Enum value already exists") );
		m_EnumValues[ name ] = val;
		m_EnumStrings[ val ] = name;
	}
	const_iterator begin() const
	{
		return m_EnumStrings.begin();
	}
	const_iterator end() const
	{
		return m_EnumStrings.end();
	}

private:
	enum_valcontainer		m_EnumValues;
	enum_stringcontainer	m_EnumStrings;
};
