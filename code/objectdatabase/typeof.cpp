#include "objectdatabase/typeof.h"
#include "editable/flipflop.h"
#include "objectdatabase/dataobject.h"
#include "objectdatabase/repeat.h"

// this function needs updating everytime you register a new typeof number
// this is a not fast at all convertsion from the string to the typeof number
int ConvertTypeOfStringToNum( const char* pName )
{
#define localCTOSTN(x) if( strcmp( typeof_class<x>::typeof_nameFUNC(), pName) == 0 ){ return x; }

	// will cause a code sequence of CTOSTN(NUM_TYPEOF_MACROS) to CTOSTN(1)
	REPEAT( NUM_TYPEOF_MACROS, localCTOSTN );
	return 0;
#undef localCTOSTN
}



void* ConvertIStreamToType( ntstd::Istream &is, int typeof_number, void* data )
{
#define localTYPE(N) \
	case N:		\
	{			\
		typeof_class<N>::V* type;	\
		type = (typeof_class<N>::V*) data;	\
		is >> *type;							\
		return data;							\
	}
	switch( typeof_number )
	{
		REPEAT( NUM_TYPEOF_MACROS, localTYPE )
	default:
		return 0;
	}

#undef localTYPE
}

void* CreateTypeOfType( int typeof_number )
{
#define localCREATE(N)					\
	case N:								\
	{									\
		return NT_NEW_CHUNK(Mem::MC_ODB) typeof_class<N>::V;	\
	}
	switch( typeof_number )
	{
		REPEAT( NUM_TYPEOF_MACROS, localCREATE )
	default:
		return 0;
	}
#undef localCREATE
}

unsigned int ReturnTypeOfSize( int typeof_number )
{
#define localSIZEOF(N)					\
	case N:								\
	{									\
		return sizeof(typeof_class<N>::V);	\
	}
	switch( typeof_number )
	{
		REPEAT( NUM_TYPEOF_MACROS, localSIZEOF )
	default:
		return 0;
	}
#undef localSIZEOF
}
