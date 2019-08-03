#ifndef CORE_UTIL_H_
#define CORE_UTIL_H_

#if defined( _BIG_ENDIAN ) // big endian version pack macros

//	#define	MAKE_ID(a, b, c, d)		( ( (a) << 24 ) | ( (b) << 16 ) | ( (c) << 8 ) | (d) )
	//! ATTN! for the moment we DONT swap file ID endian-ness
	#define	MAKE_ID(a, b, c, d)		( ( (d) << 24 ) | ( (c) << 16 ) | ( (b) << 8 ) | (a) )
	
	//! NOTE! these are not the reverse of a little endian NTCOLOUR
	//! but rather the packing required to map a 4*UByte type to
	//! (x,y,z,w == r,g,b,a) in vertex stream expansion. 

	#define NTCOLOUR_R_SHIFT	(24)
	#define NTCOLOUR_G_SHIFT	(16)
	#define NTCOLOUR_B_SHIFT	(8)
	#define NTCOLOUR_A_SHIFT	(0)	

#else // big endian pack macros

	#define	MAKE_ID(a, b, c, d)		( ( (d) << 24 ) | ( (c) << 16 ) | ( (b) << 8 ) | (a) )
	
	#define NTCOLOUR_R_SHIFT	(16)
	#define NTCOLOUR_G_SHIFT	(8)
	#define NTCOLOUR_B_SHIFT	(0)
	#define NTCOLOUR_A_SHIFT	(24)

#endif // end endian specific macros

// maps unsigned 8 bits/channel to NT_COLOR
#define NTCOLOUR_ARGB(a,r,g,b)  ((uint32_t)(	\
	(((r)&0xff)<<NTCOLOUR_R_SHIFT) |			\
	(((g)&0xff)<<NTCOLOUR_G_SHIFT) |			\
	(((b)&0xff)<<NTCOLOUR_B_SHIFT) |			\
	(((a)&0xff)<<NTCOLOUR_A_SHIFT)))

// specify NT_COLOR in more natural rgba order/
#define NTCOLOUR_RGBA(r,g,b,a) NTCOLOUR_ARGB(a,r,g,b)

// maps floating point channels (0.f to 1.f range) to NT_COLOR
#define NTCOLOUR_FROM_FLOATS(r,g,b,a) NTCOLOUR_RGBA( \
	(uint32_t)((r)*255.f),							\
	(uint32_t)((g)*255.f),							\
	(uint32_t)((b)*255.f),							\
	(uint32_t)((a)*255.f) )

// extract float values from an NT_COLOR
#define NTCOLOUR_EXTRACT_FLOATS(col,r,g,b,a) \
	r = ((float)((col>>NTCOLOUR_R_SHIFT)&0xff)) * (1/255.0f);	\
	g = ((float)((col>>NTCOLOUR_G_SHIFT)&0xff)) * (1/255.0f);	\
	b = ((float)((col>>NTCOLOUR_B_SHIFT)&0xff)) * (1/255.0f);	\
	a = ((float)((col>>NTCOLOUR_A_SHIFT)&0xff)) * (1/255.0f);

#define ROUND_POW2( num, pow2 )	( ( ( num ) + ( ( pow2 ) - 1 ) ) & ~( ( pow2 ) - 1 ) )

#define _R(X)					static_cast<float>(X)

#if !defined(UNUSED)
# define UNUSED(X)				(void) X
#endif

#define HAS_INTERFACE(name) friend class DataVisitor; friend class name##Interface;

// Lua interface
struct LuaInterfaceId { int m_Id; LuaInterfaceId() { static int iCounter = 0; m_Id = ++iCounter;}  };
#define HAS_LUA_INTERFACE() const NinjaLua::LuaObject* pMetaTable;
#define ATTACH_LUA_INTERFACE(name) pMetaTable = CLuaGlobal::Get().State().GetMetaTableP( "ninjaLua."#name );
#define COPY_LUA_INTERFACE(other) pMetaTable = other.pMetaTable;

#define NOT_IMPLEMENTED

template<typename T> inline void ResolveOffset(T*& pobMember, const void* pvHead) 
{ 
	if(pobMember) 
		pobMember = reinterpret_cast<T*>(reinterpret_cast<intptr_t>(pvHead) + reinterpret_cast<intptr_t>(pobMember)); 
}

template<typename T> inline void RecoverOffset(T*& pobMember, const void* pvHead) 
{ 
	if(pobMember) 
		pobMember = reinterpret_cast<T*>(reinterpret_cast<intptr_t>(pobMember) - reinterpret_cast<intptr_t>(pvHead)); 
}

// Resolve offset for inline use - mainly for PPU/SPU shared code.
template < typename RetType, typename BaseType >
inline RetType *ResolveOffset( int32_t member_offset, const BaseType * const base )
{
	return member_offset == 0 ? NULL : reinterpret_cast< RetType * >( reinterpret_cast< intptr_t >( base ) + member_offset );
}

// Alters 'offset' to be relative to 'new_base' rather than 'current_base'.
inline void ChangeOffsetBase( int32_t &offset, const void * const current_base, const void * const new_base )
{
	const void * const addr = static_cast< const int8_t * const >( current_base ) + offset;
	offset = int32_t( reinterpret_cast< intptr_t >( addr ) - reinterpret_cast< intptr_t >( new_base ) );
}

// Use to define an offset "pointer" from the base of a class. Really just a way to keep track of the actual type.
#define DEF_OFFSET( type ) int32_t

// Use to resolve an offset "pointer" to a pointer of known type.
#define OFFSET_PTR( type, member ) ResolveOffset< type >( member, this )

#if defined( PLATFORM_PC )
#	include "core/util_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "core/util_ps3.h"
#endif

// fix for broken NT_MEMCPY in current PS3 crt
//#ifdef PLATFORM_PS3
//#define BROKEN_MEMCPY
//#endif

#define NT_MEMCPY(ptr,src,length)	FwMemcpy(ptr,src,length)

#endif //_IO_H_
