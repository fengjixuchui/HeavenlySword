//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Driven Key Definition

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_DRIVEN_KEYS_H
#define GP_DRIVEN_KEYS_H

#include	<Fw/FwStd/FwHashedString.h>
#include	<Fw/FwMaths/FwQuat.h>

class	GpSkeleton;
class	GpDrivenKeysRotationCache;

//--------------------------------------------------------------------------------------------------
/**
	@class			GpDrivenKeyData
	
	@brief			
**/
//--------------------------------------------------------------------------------------------------

class	GpDrivenKeyData
{
public:
	float					m_data[ 4 ];
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpDrivenKeySourceControl
	
	@brief			
**/
//--------------------------------------------------------------------------------------------------

class	GpDrivenKeySourceControl
{
public:
	u32							GetIdentifier( void ) const			{ return m_identifier; };
	s32							GetControlType( void ) const		{ return ( s32 )m_controlType; };
	s32							GetComponentIndex( void ) const		{ return ( s32 )m_componentIndex; };

	float						GetMinCurveInput( void ) const		{ return m_minCurveInput; };
	float						GetMaxCurveInput( void ) const		{ return m_maxCurveInput; };

	s32							GetKeyframeCount( void ) const		{ return m_keyframeCount; };
	const	GpDrivenKeyData*	GetKeyframeDataArray( void ) const	{ return m_keyframeDataArray.Get(); };
	const	float*				GetKeyframeTimeArray( void ) const	{ return m_keyframeTimeArray.Get(); };

private:
	u32									m_identifier;			// Index into rotation cache, or source scalar cache, joint index into skeleton
	s16									m_controlType;			// ROTATE, TRANSLATE, SCALE, SCALAR
	s16									m_componentIndex;		// 0, 1, 2 (ignored for SCALAR component type)

	float								m_minCurveInput;		// Start & end values for curve input (clamp to these)
	float								m_maxCurveInput;		

	s32									m_keyframeCount;		// Standard curve stuff, just like in FpAnimClip
	FwOffset<const GpDrivenKeyData>		m_keyframeDataArray;	
	FwOffset<const float>				m_keyframeTimeArray; 
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpDrivenKeyTargetControl
	
	@brief			
**/
//--------------------------------------------------------------------------------------------------

class	GpDrivenKeyTargetControl
{
public:
	u32				GetIdentifier( void ) const			{ return m_identifier; };
	s32				GetControlType( void ) const		{ return ( s32 )m_controlType; };
	s32				GetComponentIndex( void ) const		{ return ( s32 )m_componentIndex; };

private:
	u32				m_identifier;			// Index into rotation cache, or target scalar cache, joint index into skeleton
	s16				m_controlType;			// ROTATE, TRANSLATE, SCALE, SCALAR
	s16				m_componentIndex;		// 0, 1, 2 (ignored for SCALAR component type)
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpDrivenKeyItem
	
	@brief			Defines a single driven key association
**/
//--------------------------------------------------------------------------------------------------

class	GpDrivenKeyItem
{
public:
	enum	ControlType
	{
		kRotation,
		kTranslation,
		kScale,
		kScalar,
	};

	s32									GetSourceControlCount( void ) const		{ return m_sourceCount; };
	const	GpDrivenKeySourceControl*	GetSourceControlArray( void ) const		{ return m_sourceArray.Get(); };

	s32									GetTargetControlCount( void ) const		{ return m_targetCount; };
	const	GpDrivenKeyTargetControl*	GetTargetControlArray( void ) const		{ return m_targetArray.Get(); };

	

private:
	s32											m_sourceCount;	// A number of sources contributing to this driven key
	FwOffset<const GpDrivenKeySourceControl>	m_sourceArray;

	s32											m_targetCount;	// A number of sources contributing to this driven key
	FwOffset<const GpDrivenKeyTargetControl>	m_targetArray;

};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpDrivenKeyRotationCacheInfo
	
	@brief			Defines rotation order & index for rotation cache usage
**/
//--------------------------------------------------------------------------------------------------

class	GpDrivenKeyRotationCacheInfo
{
public:
	FwQuat::RotationOrder	GetRotationOrder( void ) const	{ return ( FwQuat::RotationOrder )m_rotationOrder; }
	s16						GetRotationIndex( void ) const	{ return m_rotationIndex; }

private:
	s16		m_rotationOrder;
	s16		m_rotationIndex;
};

//--------------------------------------------------------------------------------------------------
/**
	@class			GpDrivenKeysDef
	
	@brief			Defines the driven key collection
**/
//--------------------------------------------------------------------------------------------------

class	GpDrivenKeysDef
{
public:
	s32										GetItemCount( void ) const					{ return m_itemCount; };
	const	GpDrivenKeyItem*				GetItemArray( void ) const					{ return m_itemArray.Get(); };
	
	s32										GetSourceScalarNameCount( void ) const		{ return m_sourceScalarNameCount; };
	const	FwHashedString*					GetSourceScalarNameArray( void ) const		{ return m_sourceScalarNameArray.Get(); };

	s32										GetTargetScalarNameCount( void ) const		{ return m_targetScalarNameCount; };
	const	FwHashedString*					GetTargetScalarNameArray( void ) const		{ return m_targetScalarNameArray.Get(); };

	s32										GetRotationCacheCount( void ) const			{ return m_rotationCacheCount; };
	const	GpDrivenKeyRotationCacheInfo*	GetRotationCacheInfo( void ) const			{ return m_rotationCacheInfo.Get(); };

private:
	s32												m_itemCount;
	FwOffset<const GpDrivenKeyItem>					m_itemArray;

	s32												m_sourceScalarNameCount;		// Number of scalar source components
	FwOffset<const FwHashedString>					m_sourceScalarNameArray;		// Contains an array of hashed string values (sorted)

	s32												m_targetScalarNameCount;		// Number of scalar target components
	FwOffset<const FwHashedString>					m_targetScalarNameArray;		// Contains an array of hashed string values (sorted)

	s32												m_rotationCacheCount;			// Number of cached source/target rotations
	FwOffset<const GpDrivenKeyRotationCacheInfo>	m_rotationCacheInfo;			// This contains a rotation-cache-to-joint-index mapping, and rotation order.
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpDrivenKeys
	
	@brief			An instantiation of the driven keys data
**/
//--------------------------------------------------------------------------------------------------

class	GpDrivenKeysRotationCache
{
public:
	enum	RotationCache
	{
		kWriteback = ( 1 << 0 ),
	};

	s32		m_flags;
	float	m_rotation[ 3 ];
};

//--------------------------------------------------------------------------------------------------
/**
	@class			GpDrivenKeys
	
	@brief			An instantiation of the driven keys data
**/
//--------------------------------------------------------------------------------------------------

class	GpDrivenKeys
{
	friend	class GpSkeleton;

public:
	void	Update( void );
	bool	Get( FwHashedString itemName, float& result );
	bool	Set( FwHashedString scalarName, float value );

protected:
	static	int	ComputeSize( const GpDrivenKeysDef* pDrivenKeysDef );
	void		Construct( const GpDrivenKeysDef* pDrivenKeysDef, GpSkeleton* pSkeleton );

private:
	const GpDrivenKeysDef*		m_pDrivenKeysDef;
	GpSkeleton*					m_pSkeleton;
	float*						m_pSourceScalarData;
	float*						m_pTargetScalarData;
	GpDrivenKeysRotationCache*	m_pRotationCache;	
};

#endif	// GP_DRIVEN_KEYS_H
