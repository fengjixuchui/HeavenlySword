/*
 *  sceaatomic_deletable_node.h
 *  libsceaatomic
 *
 *  Created by Alex Rosenberg on 10/11/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 */

#ifndef __SCEAATOMIC_DELETABLE_NODE_H__
#define __SCEAATOMIC_DELETABLE_NODE_H__ 1

#include "sceaatomic_node.h"
#include <cstdio>

namespace SCEA { namespace Atomic {

template <class T> struct deletable_node;

template <class T>
struct deletable_node : public atomic_node<T>
{
	typedef atomic_node<T>	base_type;
	
	// copy semantics empty the next_on_delete_list pointer to avoid having dangling pointers
	inline deletable_node()											: next_on_delete_list()					{}
	explicit inline deletable_node(const deletable_node<T>& rhs)	: base_type(rhs), next_on_delete_list()	{}
	inline deletable_node& operator=(const deletable_node<T> & rhs)
	{
		base_type::operator=(rhs);
		next_on_delete_list = rhs.next_on_delete_list;
		return *this;
	}
	
	main_ptr<deletable_node<T> >	next_on_delete_list;
};

}} // namespace SCEA::Atomic

#endif // __SCEAATOMIC_DELETABLE_NODE_H__
