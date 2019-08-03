/*
 *  sceaatomic_node.h
 *  libsceaatomic
 *
 *  Created by Alex Rosenberg on 10/4/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 * TODO:
 */

#ifndef __SCEAATOMIC_NODE_H__
#define __SCEAATOMIC_NODE_H__ 1

#include "sceaatomic_dma.h"

namespace SCEA { namespace Atomic {

template <class T>
struct atomic_node
{
	// copy semantics empty the next pointer to avoid having dangling pointers
	inline atomic_node()										: next() {}
	explicit inline atomic_node(const atomic_node<T>& rhs)		: next() {}
	inline atomic_node& operator=(const atomic_node<T> & rhs)	{ next = rhs.next; return *this; }

	SCEA_ALIGN_BEG(16)
		main_ptr<T>		next // force alignment for SPU DMA use
	SCEA_ALIGN_END(16);
};

}} // namespace SCEA::Atomic

#endif // __SCEAATOMIC_NODE_H__
