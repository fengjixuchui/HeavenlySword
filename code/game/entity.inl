
#ifndef _ENTITY_INL
#define _ENTITY_INL

#include "anim/hierarchy.h"

#include "audio/gameaudiocomponents.h"

#include "camera/sceneelementcomponent.h"

#include "game/anonymouscomponent.h"
#include "game/entityai.h"
#include "game/entitycharacter.h"
#include "game/entityinteractable.h"
#include "game/luaattrtable.h"
#include "game/messagehandler.h"
#include "game/renderablecomponent.h"




//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::IsPlayer
//!	Returns true if this entity is a player entity
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::IsPlayer(void) const
{
	return GetEntType() == EntType_Player;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::IsBoss
//!	Returns true if this entity is a boss entity
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::IsBoss(void) const
{
	return GetEntType() == EntType_Boss;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::IsStatic
//!	Returns true if this entity is a static entity
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::IsStatic() const
{
	return m_eType == EntType_Static;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::IsCharacter
//!	Returns true if this entity is a character entity
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::IsCharacter() const
{
	return 0 != (m_eType & EntType_Character);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::IsAI
//!	Returns true if this entity is a AI entity
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::IsAI() const
{
	return 0 != (m_eType & EntType_AI);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::IsFriendly
//!	Returns true if this entity is a friendly entity
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::IsFriendly() const
{
	return IsAI() && !IsEnemy();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::IsInteractable
//!	Returns true if this entity is an interactable entity
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::IsInteractable() const
{
	return 0 != (m_eType & EntType_Interactable);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::IsProjectile
//!	Returns true if this entity is a projectile entity
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::IsProjectile() const
{
	return 0 != (m_eType & EntType_Projectile);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToCharacter
//!	Downcast CEntity to Character
//!                                                                                         
//------------------------------------------------------------------------------------------
inline Character* CEntity::ToCharacter() 
{
	ntError_p(IsCharacter(), ("ToCharacter() Downcast applied to non Character type"));
	return (Character*)this;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToCharacter
//!	Downcast CEntity to Character
//!                                                                                         
//------------------------------------------------------------------------------------------
inline const Character* CEntity::ToCharacter() const 
{
	ntError_p(IsCharacter(), ("ToCharacter() Downcast applied to non Character type"));
	return (const Character*)this;
} 

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToPlayer
//!	Downcast CEntity to Player
//!                                                                                         
//------------------------------------------------------------------------------------------
inline Player* CEntity::ToPlayer() 
{
	ntError_p(IsPlayer(), ("ToPlayer() Downcast applied to non Player type"));
	return (Player*)this;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToPlayer
//!	Downcast CEntity to Player
//!                                                                                         
//------------------------------------------------------------------------------------------
inline const Player* CEntity::ToPlayer() const 
{
	ntError_p(IsPlayer(), ("ToPlayer() Downcast applied to non Player type"));
	return (const Player*)this;
} 

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToAI
//!	Downcast CEntity to AI
//!                                                                                         
//------------------------------------------------------------------------------------------
inline AI* CEntity::ToAI() 
{
	ntError_p(IsAI(), ("ToAI() Downcast applied to non AI type"));
	return (AI*)this;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToAI
//!	Downcast CEntity to AI
//!                                                                                         
//------------------------------------------------------------------------------------------
inline const AI* CEntity::ToAI() const 
{
	ntError_p(IsAI(), ("ToAI() Downcast applied to non AI type"));
	return (const AI*)this;
} 


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToInteractable
//!	Downcast CEntity to Interactable
//!                                                                                         
//------------------------------------------------------------------------------------------
inline Interactable* CEntity::ToInteractable() 
{
	ntError_p(IsInteractable(), ("ToInteractable() Downcast applied to non Interactable type"));
	return (Interactable*)this;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToInteractable
//!	Downcast CEntity to Interactable
//!                                                                                         
//------------------------------------------------------------------------------------------
inline const Interactable* CEntity::ToInteractable() const 
{
	ntError_p(IsInteractable(), ("ToInteractable() Downcast applied to non Interactable type"));
	return (const Interactable*)this;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToProjectile
//!	Downcast CEntity to Interactable
//!                                                                                         
//------------------------------------------------------------------------------------------
inline Object_Projectile* CEntity::ToProjectile()
{
	ntError_p(IsProjectile(), ("ToInteractable() Downcast applied to non Projectile type"));
	return (Object_Projectile*)this;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::ToProjectile
//!	Downcast CEntity to Interactable
//!                                                                                         
//------------------------------------------------------------------------------------------
inline const Object_Projectile* CEntity::ToProjectile() const
{
	ntError_p(IsProjectile(), ("ToProjectile() Downcast applied to non Projectile type"));
	return (const Object_Projectile*)this;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::IsEnemy
//!	Returns true if this entity is an enemy
//! This has NOT been implemented as a virtual function so as to avoid vtable overhead
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::IsEnemy() const
{
	bool bRet = false;
	if (GetEntType() == EntType_AI)
	{
		const AI* pobAI;
		pobAI = (const AI*)this;
		bRet = pobAI->IsEnemy();
	}
	// Bosses are also our enemies
	bRet |= (GetEntType() == EntType_Boss);
	return bRet;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::SetAttributeTable
//!	Sets an entities attribute table
//!                                                                                         
//------------------------------------------------------------------------------------------
inline void CEntity::SetAttributeTable( LuaAttributeTable* attrTable )
{
	m_obAttributeTable = attrTable;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CEntity::HasLuaTable
//!	Returns true if this entity has a LUA table
//!                                                                                         
//------------------------------------------------------------------------------------------
inline bool CEntity::HasLuaTable() const
{ 
	return ( HasAttributeTable() && m_obAttributeTable->HasLuaTable() ); 
}




//--------------------------------------------------
//!
//! Returns the entity name
//!
//-------------------------------------------------
inline ntstd::String	CEntity::GetName( void ) const 
{ 
	if( m_obAttributeTable )
	{
		NinjaLua::LuaObject name = m_obAttributeTable->GetAttribute( "Name" );
		if( name.IsString() )
		{
			return name.GetString();
		}
	}

	return "TheEntityWithNoName"; 
}

//--------------------------------------------------
//!
//! Returns the entity type
//!
//-------------------------------------------------
inline ntstd::String	CEntity::GetType( void ) const 
{ 
	if( m_obAttributeTable )
	{
		NinjaLua::LuaObject type = m_obAttributeTable->GetAttribute( "Type" );
		if( type.IsString() )
		{
			return type.GetString();
		}
	}

	return "TheEntityWithNoType"; 
}

//-----------------------------------------------------------------------------------------------
//!
//! Returns the entity world position - this gets the root position of the item.  Where the
//!	object is technically placed in the world.
//!
//-----------------------------------------------------------------------------------------------
inline CPoint CEntity::GetPosition( void ) const
{
	// If we have a heirarchy, get the root
	if ( GetHierarchy() )
		return GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation();
	
	// If there is no heirarchy we get the initial position
	else
		return m_InitialPosition;
}

//--------------------------------------------------
//!
//! Returns the worlds space rotation
//!
//-------------------------------------------------
inline const CMatrix&	CEntity::GetMatrix( void ) const
{
	// for the moment the only orientation we have are via a heirachy. THIS WILL CHANGE!
	ntAssert_p( GetHierarchy(), ("Cannot yet query the matrix of an entity with no Heirachy"));
	return GetHierarchy()->GetRootTransform()->GetWorldMatrix();
}


//--------------------------------------------------
//!
//! Returns the root transform object
//!
//-------------------------------------------------
inline const Transform* CEntity::GetRootTransformP( void ) const
{
	ntAssert_p( GetHierarchy(), ("Cannot yet query the matrix of an entity with no Heirachy"));
	return GetHierarchy()->GetRootTransform();
}

//--------------------------------------------------
//!
//! Get a particular transform in the hierarchy for the entity
//!
//-------------------------------------------------
inline const Transform* CEntity::GetTransformP( CHashedString obTransformName ) const
{
	ntAssert_p( GetHierarchy(), ("Cannot yet query the matrix of an entity with no Heirachy"));
	return GetHierarchy()->GetTransform( obTransformName );
}


//--------------------------------------------------
//!
//! Checks if a particular transform exists in the hierarchy for the entity
//!
//-------------------------------------------------
inline bool CEntity::DoesTransformExist( const char* pcTransformName ) const
{
	return GetHierarchy()->DoesTransformExist( pcTransformName );
}


//--------------------------------------------------
//!
//! Get a particular transform in the hierarchy for the entity
//!
//-------------------------------------------------
inline Transform* CEntity::GetCharacterTransformP( CHARACTER_BONE_ID eBone )
{
	ntAssert_p( GetHierarchy(), ("Cannot yet query the matrix of an entity with no Heirachy"));
	return GetHierarchy()->GetCharacterBoneTransform( eBone );
}

inline const Transform* CEntity::GetCharacterTransformP( CHARACTER_BONE_ID eBone ) const
{
	ntAssert_p( GetHierarchy(), ("Cannot yet query the matrix of an entity with no Heirachy"));
	return GetHierarchy()->GetCharacterBoneTransform( eBone );
}

//--------------------------------------------------
//!
//! Get the camera correct position of the entity.  
//! (e.g. Look at the heroines arse not her feet...)
//!
//-------------------------------------------------
inline CPoint CEntity::GetCamPosition() const
{
	if(m_pSceneElementComponent)
		return m_pSceneElementComponent->GetPosition();
	else
		return GetPosition();
}

//--------------------------------------------------
//!
//! Get the object's key, for use in QuickPtrList
//!
//-------------------------------------------------
inline uint32_t CEntity::GetHashKey() const
{
	return m_Name.GetHash();
}

//--------------------------------------------------
//!
//! Update the state of the anim container (used to assist wielder)
//!
//-------------------------------------------------
inline NinjaLua::LuaObject CEntity::GetAttrib(void) const
{
	return GetAttributeTable()->GetLuaObjectWrapper();
}


//-----------------------------------------------------------------------------------------------
//!
//! CEntity::InstallAudioChannel
//!	
//-----------------------------------------------------------------------------------------------
inline void CEntity::InstallAudioChannel()
{
	ntAssert( m_pobEntityAudioChannel == 0 );	// already got one?
	m_pobEntityAudioChannel = NT_NEW_CHUNK(Mem::MC_ENTITY) EntityAudioChannel( this );
}

//-----------------------------------------------------------------------------------------------
//!
//! CEntity::InstallMessageHandler
//!	
//-----------------------------------------------------------------------------------------------
inline void CEntity::InstallMessageHandler()
{
	ntAssert( m_pobMessageHandler == 0 );	// already got one?
	m_pobMessageHandler = NT_NEW_CHUNK(Mem::MC_ENTITY) CMessageHandler( this, m_pobGameEventsList );
}




#endif // _ENTITY_INL


