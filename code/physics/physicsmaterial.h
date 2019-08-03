
//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/physicsmaterial.h
//!	
//!	Physics materals.
//! 
//!
//!	Author: Peter Feher
//! Created: 2006.07.07
//!
//---------------------------------------------------------------------------------------------------------


#ifndef	_PHYSICALSMATERIAL_H
#define	_PHYSICALSMATERIAL_H

#pragma once


#include <hkcollide/shape/mesh/hkMeshMaterial.h>
#include "objectdatabase/dataobject.h"


namespace Physics
{
	class psPhysicsMaterial
	{
	public:
		psPhysicsMaterial(void) :
			m_fFriction(0.0f),
			m_fRestitution(0.0f),
			m_fPenetrability(-1),
			m_iEffectMaterialBit(0),
			m_uiEffectMaterialBit(0)
		{
		}

		psPhysicsMaterial(const CHashedString& name, float fFriction, float fRestitution, float fPenetrability,int iGroupBit) :
			m_fFriction(fFriction),
			m_fRestitution(fRestitution),
			m_fPenetrability(fPenetrability),
			m_iEffectMaterialBit(iGroupBit),
			m_name(name),
			m_uiEffectMaterialBit(0)
		{
			OnPostConstruct();
		}

		void OnPostConstruct ()
		{
			// Convert out bit number to an actual 64-bit unsigned integer
			if (m_iEffectMaterialBit>0 && m_iEffectMaterialBit<64)
			{
				m_uiEffectMaterialBit = ((uint64_t)1) << (m_iEffectMaterialBit - 1);
			}
		}

		float			GetFriction( void ) const					{ return m_fFriction; }
		float			GetRestitution( void ) const				{ return m_fRestitution; }
		float			GetPenetrability( void ) const				{ return m_fPenetrability; }
		bool			QHasValidFrictionAndRestitution() const		{ return m_fFriction >= 0 && m_fRestitution >= 0; }

		uint32_t		GetMaterialId( void ) const					{ return m_name.GetHash(); }; //< Id is hash generated from name
		const char* 	GetMaterialName( void ) const				{ return m_name.GetDebugString(); }
		void			SetMaterialName( const CKeyString &name )	{ m_name = CHashedString(name); }

		uint64_t		GetEffectMaterialBit(void) const			{ return m_uiEffectMaterialBit; }

		// ----- Serialised members -----

		float				m_fFriction; // if m_fFriction or m_fRestitution value is negative... material will no modify the ones from entity
		float				m_fRestitution;
		float				m_fPenetrability; // 0 -- nothing will penetrate, 1 -- easy to penetrate... 
		int					m_iEffectMaterialBit; // This is a value between 1 and 63

	protected:

		CHashedString		m_name;
		
		uint64_t			m_uiEffectMaterialBit;
	};

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

	class PhysicsMaterialTable : public Singleton<PhysicsMaterialTable>
	{
	public:

		PhysicsMaterialTable(const char * xmlFile);

		// index is 1 base (0 is wrong index)
		uint32_t			GetIndexFromId(uint32_t id) const;
		psPhysicsMaterial*	GetMaterialFromIndex(uint32_t indx) const {if (indx > 0) return m_table[indx - 1]; return 0;};
		psPhysicsMaterial*	GetMaterialFromId(uint32_t id) const;

		// Effect material bitfields
		uint64_t			GetEffectMaterialBit (const CHashedString& obMaterialName); // Get the collision effect material bitfield from a particular material
		uint64_t			GetEffectMaterialBitfield (const char* pcString); // Calculate the collision effect material bitfield from a string
		void				Debug_GetMaterialsFromBitfield (uint64_t uiEffectMaterialBitfield,char* pcOutput);

		template<class TYPE> 
			TYPE GetValueFromIndex(uint32_t indx, TYPE (psPhysicsMaterial::*getMember)() const ) const
		{ return (*GetMaterialFromIndex(indx).*getMember)();};

		template<class TYPE> 
			TYPE GetValueFromId(uint32_t indx, TYPE (psPhysicsMaterial::*getMember)() const ) const
		{ return (*GetMaterialFromId(indx).*getMember)();};

	protected:

		ntstd::Vector<psPhysicsMaterial *> m_table;
		typedef ntstd::Vector<psPhysicsMaterial *>::const_iterator TableIterator; 
	};

	// -----------------------------------------------------------------------------------
	//	hsMeshMaterial
	//	
	//		The material for mesh see hkMesh::Subpart for details
	//
	// -----------------------------------------------------------------------------------
	class hsMeshMaterial : public hkMeshMaterial
	{
	public:
		hsMeshMaterial() : m_pobMaterial(0) {};

		psPhysicsMaterial * GetMaterial() const {return m_pobMaterial;};
		void SetMaterial(psPhysicsMaterial * mat) {m_pobMaterial = mat;};

	protected:
		psPhysicsMaterial *	m_pobMaterial;
	};

	// -----------------------------------------------------------------------------------
	//	Generic physics materials
	// -----------------------------------------------------------------------------------
	extern psPhysicsMaterial obPelvisPhysicsMaterial; 
	extern psPhysicsMaterial obSpinePhysicsMaterial;
	extern psPhysicsMaterial obHeadPhysicsMaterial;
	extern psPhysicsMaterial obLLegPhysicsMaterial;
	extern psPhysicsMaterial obRLegPhysicsMaterial;
	extern psPhysicsMaterial obLArmPhysicsMaterial;
	extern psPhysicsMaterial obRArmPhysicsMaterial;
};


#endif //_PHYSICALSMATERIAL_H
