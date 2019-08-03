/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkscenedata/hkSceneData.h>
#include <hkscenedata/graph/hkxNode.h>
#include <hkscenedata/attributes/hkxAttributeGroup.h>

#include <hkbase/class/hkClass.h>


const hkxAttributeGroup* hkxNode::findAttributeGroupByName (const char* name) const
{
	if( !name )
	{
		return HK_NULL;
	}

	for (int j=0; j < m_numAttributeGroups; ++j)
	{
		const hkxAttributeGroup& group = m_attributeGroups[j];
		if ( group.m_name && hkString::strCasecmp(group.m_name, name)==0 )
		{
			return &group;
		}
	}
	return HK_NULL;
}


const hkVariant* hkxNode::findAttributeVariantByName( const char* name ) const
{
	if( !name )
	{
		return HK_NULL;
	}

	for (int j=0; j < m_numAttributeGroups; ++j)
	{
		hkxAttributeGroup& g = m_attributeGroups[j];
		const hkVariant* var = g.findAttributeVariantByName( name );
		if (var) 
			return var;
	}
	return HK_NULL;
}

void* hkxNode::findAttributeObjectByName( const char* name, const hkClass* type ) const
{
	if( !name )
	{
		return HK_NULL;
	}

	const hkVariant* var =  findAttributeVariantByName( name );
	
	// compare class by name so that it deals with serialized classes etc better (for instance in the filters)
	if (var && (!type || (hkString::strCasecmp(type->getName(), var->m_class->getName()) == 0)) )
	{
		return var->m_object;
	}
	
	return HK_NULL;
}

hkxNode* hkxNode::findChildByName (const char* childName) const
{
	if( !childName )
	{
		return HK_NULL;
	}

	for (int i=0; i<m_numChildren; i++)
	{
		hkxNode* child = m_children[i];
		if( child->m_name && hkString::strCasecmp(child->m_name, childName)==0 )
		{
			return child;
		}
	}

	return HK_NULL;
}

hkxNode* hkxNode::findDescendantByName (const char* name) const
{
	if( !name )
	{
		return HK_NULL;
	}

	for (int i=0; i<m_numChildren; i++)
	{
		hkxNode* child = m_children[i];

		if( child->m_name && hkString::strCasecmp(child->m_name, name)==0 )
		{
			return child;
		}
		else
		{
			hkxNode* descendant = child->findDescendantByName(name);
			if (descendant != HK_NULL)
			{
				return descendant;
			}
		}
	}

	return HK_NULL;
}

hkResult hkxNode::getPathToNode (const hkxNode* theNode, hkArray<const hkxNode*>& pathOut) const
{
	pathOut.pushBack(this);

	if (this == theNode)
	{
		return HK_SUCCESS;
	}

	hkResult result = HK_FAILURE;

	for (int i=0; i<m_numChildren; i++)
	{
		result = m_children[i]->getPathToNode(theNode, pathOut);
		if (result==HK_SUCCESS) break;
	}

	if (result != HK_SUCCESS)
	{
		pathOut.popBack();
	}

	return result;
}


hkVariant* hkxNode::findVariantByObject( void* object )
{
	if( m_object.m_object == object )
	{
		return &m_object;
	}

	for( int i=0; i < m_numChildren; i++ )
	{
		hkVariant* var = m_children[i]->findVariantByObject( object );
		if( var )
		{
			return var;
		}
	}

	return HK_NULL;
}

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
