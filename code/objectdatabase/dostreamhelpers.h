
inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const CHashedString &obj)
{
#ifdef KEYSTRING_INCLUDE_DEBUG_INFO
	os << ntStr::GetString(obj);
#else
	os << ntStr::GetHashKey(obj);
#endif
	return os;
}
inline ntstd::Istream& operator >>(ntstd::Istream &is, CHashedString &obj)
{
	ntstd::String str;
	is >> str;									   

	const char* cstr = ntStr::GetString(str);

	if (ntStr::IsNull(cstr))
	{
		obj = CHashedString(cstr);
		return is;
	}

	for (unsigned int i = 0; i < ntStr::GetLength(str); ++ i)
	{
		// Not a number
		if (cstr[i] > '9' || cstr[i] < '0')
		{
			obj = CHashedString(cstr);
			return is;
		}
	}

	unsigned int hashCode = atoi(cstr);

	if (0 != hashCode)
	{
		obj = CHashedString( hashCode );
	}
	else
	{
		obj = CHashedString( cstr );
	}

	return is;
}

inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const CKeyString &obj)
{
	os << ntStr::GetString(obj);
	return os;
}
inline ntstd::Istream& operator >>(ntstd::Istream &is, CKeyString &obj)
{
	ntstd::String str;
	is >> str;

	//if (atoi(str.c_str())!= 0)
	//{
	//	snPause();
	//}

	obj = CKeyString( str.c_str() );
	return is;
}


inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const CPoint &obj)
{
	os << obj.X() << "," << obj.Y() << "," << obj.Z();
	return os;
}
inline ntstd::Istream& operator >>(ntstd::Istream &is, CPoint &obj)
{
	char comma1, comma2; // dummies
//	is >> obj.X() >> comma1 >> obj.Y() >> comma2 >> obj.Z();  // ATTN! DEANO
	is >> obj.X() >> comma1 >> obj.Y() >> comma2 >> obj.Z(); obj.W() = 0.0f;

	return is;
}


inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const CVector &obj)
{
	os << obj.X() << "," << obj.Y() << "," << obj.Z() << "," << obj.W();
	return os;
}

inline ntstd::Istream& operator >>(ntstd::Istream &is, CVector &obj)
{
	char comma1, comma2, comma3; // dummies
	is >> obj.X() >> comma1 >> obj.Y() >> comma2 >> obj.Z() >> comma3 >> obj.W();
#ifdef PLATFORM_PS3
	// This is a nasty temporary thing to aid overcoming gcc rubishness
	for (unsigned int elem = 0; elem < 4; elem ++)
	{
		if (IsNan(obj[elem]))
		{
			obj[elem] = 0;
		}
	}
#endif

	return is;
}


inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const CQuat &obj)
{
	os << obj.X() << "," << obj.Y() << "," << obj.Z() << "," << obj.W();
	return os;
}
inline ntstd::Istream& operator >>(ntstd::Istream &is, CQuat &obj)
{
	char comma1, comma2, comma3; // dummies
	is >> obj.X() >> comma1 >> obj.Y() >> comma2 >> obj.Z() >> comma3 >> obj.W();

	return is;
}

inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const CDirection &obj)
{
	os << obj.X() << "," << obj.Y() << "," << obj.Z();
	return os;
}
inline ntstd::Istream& operator >>(ntstd::Istream &is, CDirection &obj)
{
	char comma1, comma2; // dummies
//	is >> obj.X() >> comma1 >> obj.Y() >> comma2 >> obj.Z();
	is >> obj.X() >> comma1 >> obj.Y() >> comma2 >> obj.Z(); obj.W() = 0.0f;

	return is;
}

#include "editable/flipflop.h"

REGISTER_TYPEOF_AS( 22, CFlipFlop, "flipflop" )
inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const CFlipFlop &obj)
{
	ntstd::String flipStr = obj.ToString();
	os << flipStr;
	return os;
}
inline ntstd::Istream& operator >>(ntstd::Istream &is, CFlipFlop &obj)
{
	ntstd::String flipStr;

	is >> flipStr;
	obj.Build( flipStr.c_str() );

	return is;
}

REGISTER_TYPEOF_AS( 23, DeepList, "DeepList" )
inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const DeepList &)
{
	ntError_p(false, ("Shouldn't get here") );
	return os;
}
inline ntstd::Istream& operator >>(ntstd::Istream &is, DeepList &)
{
	ntError_p(false, ("Shouldn't get here") );
	return is;
}


template<typename T,int C>
inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const ntstd::Vector<T, C> &listRef)
{
	os << listRef.size();
	typename ntstd::Vector<T, C>::const_iterator tIt = listRef.begin();
	while( tIt != listRef.end() )
	{
		os << " " << *tIt;
		++tIt;
	}
	return os;
}

template<typename T, int C>
inline ntstd::Istream& operator >>(ntstd::Istream &is, ntstd::Vector<T, C> &listRef)
{
	unsigned int listSize;
	is >> listSize;

	listRef.resize( listSize );
	typename ntstd::Vector<T,C>::iterator tIt = listRef.begin();
	while( tIt != listRef.end() )
	{
		T data = T();
		is >> data;
		*tIt = data;
		++tIt;
	}
	return is;
}

template<typename T, int C>
inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const ntstd::List<T, C> &listRef)
{
	os << listRef.size();
	typename ntstd::List<T, C>::const_iterator tIt = listRef.begin();
	while( tIt != listRef.end() )
	{
		os << " " << *tIt;
		++tIt;
	}
	return os;
}

template<typename T, int C>
inline ntstd::Istream& operator >>(ntstd::Istream &is, ntstd::List<T, C> &listRef)
{
	unsigned int listSize;
	is >> listSize;

	listRef.resize( listSize );
	typename ntstd::List<T, C>::iterator tIt = listRef.begin();
	while( tIt != listRef.end() )
	{
		T data = T();
		is >> data;
		*tIt = data;
		++tIt;
	}
	return is;
}
inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const DataObject* &)
{
	ntError_p(false, ("Shouldn't get here") );
	return os;
}

inline ntstd::Istream& operator >>(ntstd::Istream &is, DataObject* &)
{
	ntError_p(false, ("Shouldn't get here") );
	return is;
}

inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const ntstd::List<void*> &)
{
	ntError_p(false, ("Shouldn't get here") );
	return os;
}

inline ntstd::Istream& operator >>(ntstd::Istream &is, ntstd::List<void*> &)
{
	ntError_p(false, ("Shouldn't get here") );
	return is;
}
inline ntstd::Ostream& operator <<(ntstd::Ostream &os, const ntstd::List<DataObject*> &)
{
	ntError_p(false, ("Shouldn't get here") );
	return os;
}

inline ntstd::Istream& operator >>(ntstd::Istream &is, ntstd::List<DataObject*> &)
{
	ntError_p(false, ("Shouldn't get here") );
	return is;
}
