/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEDATA_GRAPH_HKXNODE_HKCLASS_H
#define HKSCENEDATA_GRAPH_HKXNODE_HKCLASS_H

/// hkxNode meta information
extern const class hkClass hkxNodeClass;

/// A node in a scene graph
class hkxNode
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxNode );
		HK_DECLARE_REFLECTION();

			/// This structure describes a note track annotation ( text and time ).
			/// AnnotationData is DEPRICATED. Use hkxAttribute data, and look for animated strings types
		struct AnnotationData
		{
			HK_DECLARE_REFLECTION();

				/// annotation time stamp.
			hkReal m_time;
			
				/// A string description of the event
			char* m_description;
		};
				
		//
		// Members
		//
	public:
		
			/// Human readable name this object.
		char* m_name;
		
			/// The object at this node, if one. (mesh, skin, light, camera, etc.) Check class
			/// (m_class) of reflected to find out if not null.
		hkVariant m_object;
		
			/// Raw keyframe data
		hkMatrix4* m_keyFrames;
		hkInt32 m_numKeyFrames;
		
			/// The children of this node. This link forms the scene graph
		hkxNode** m_children;
		hkInt32 m_numChildren;
		
			/// Annotation Data for this node
		struct AnnotationData* m_annotations;
		hkInt32 m_numAnnotations;
		
			/// User data
		char* m_userProperties;

			/// Selection flag
		hkBool m_selected;

			/// Attributes associated with this node.
			/// You can use hkxAttribute::findObjectByName and related methods
			/// to search the array for a given attribute
			/// They are organized by groups, each group with a string type identifier
		struct hkxAttributeGroup* m_attributeGroups;
		int m_numAttributeGroups;

			/// Looks for an attribute group by name 
		const hkxAttributeGroup* findAttributeGroupByName (const char* name) const;

			/// Looks for the first child that matches the given name (case insensitive)
		hkxNode* findChildByName (const char* childName) const;

			/// Recursively looks for the first descendant with each name (case insensitive). This is done depth-first.
		hkxNode* findDescendantByName (const char* name) const;

			/// Constructs a path (parent-first list of nodes from this one to the node we search, both included)
			/// Returns HK_FAILURE if the node is not a descendant of this
		hkResult getPathToNode (const hkxNode* theNode, hkArray<const hkxNode*>& pathOut) const;

			/// Search an array of attributes for one by name, and optionally
			/// check the type of that attribute to make sure it matches the 
			/// given class. If no attribute matches by name (and by class if given)
			/// it will return HK_NULL.
			/// Will search all groups;
		void* findAttributeObjectByName( const char* name, const hkClass* type = HK_NULL ) const;

			/// Search an array of attributes for one by name and return the variant  
			/// of that attribute. Will return HK_NULL if not found.
			/// Will search all groups;
		const hkVariant* findAttributeVariantByName( const char* name ) const;

			/// Search the node and its children for a hkVariant containing the specified object.
			/// This is done depth-first. Will return HK_NULL if not found.
		hkVariant* findVariantByObject( void* object );
};

#endif // HKSCENEDATA_GRAPH_HKXNODE_HKCLASS_H

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
