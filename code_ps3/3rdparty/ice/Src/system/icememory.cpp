/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <new>
#include <stdlib.h>

using namespace std;

// We want to align to 16, but there is a strange bug with memalign which causes it to only
// align to 8 if requesting alignment to 16. Requesting 32-byte alignment does, however
// align to 32.

void*operator new(size_t size)
{
	return memalign(32, size);
}

void* operator new[](size_t size)
{
	return memalign(32, size);
}

void* operator new(size_t size, const std::nothrow_t&)
{
	return memalign(32, size);
}

void* operator new[](size_t size, const std::nothrow_t& )
{
	return memalign(32, size);
}

void operator delete(void* ptr)
{
	free(ptr);
}

void operator delete[](void* ptr)
{
	free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&)
{
	free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t& )
{
	free(ptr);
}
