//--------------------------------------------------------------------------------------------------
/**
	@file		GpFaceDef.h

	@brief		contains class definition of facial animation control rig descriptor

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_FACE_DEF_H
#define GP_FACE_DEF_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------
									
#include <Fw/FwStd/FwHashedString.h>
							 
//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			GpFaceDef

	@brief			Facial animation definition 

		This is the header of the .face definition files. It describes the facial animation
		control rig. The number of rig handles and their types is the main information conveyed.
		
**/
//--------------------------------------------------------------------------------------------------

class GpFaceDef : public FwNonCopyable 
{
public:
	enum RigType			///< specifies the type of rig, currently only custom are supported
	{
		kFaceUnknown = -1,
		kFaceSymmetric,
		kFaceAsymmetric,
		kFaceHybrid,
		kFaceCustom,
		kFaceNumTypes
	};

	enum HandleType			///< specifies the type of a rig handle (indicates it's DOFs)
	{
		kFaceRigHandleX = 0,
		kFaceRigHandleY,
		kFaceRigHandleZ,
		kFaceRigHandleXY,
		kFaceRigHandleXZ,
		kFaceRigHandleYZ,
		kFaceRigHandleXYZ
	};

	// Construction & Destruction
	inline GpFaceDef();
	inline ~GpFaceDef();

	// Access
	inline u8			GetFlags()				const;
	inline RigType		GetRigType()			const;
	inline u16			GetNumHandles()			const;
	inline u16			GetNumCustomAttrs()		const;
	inline HandleType	GetHandleType(u32 i)	const;

private:
	u8			m_flags;				///< reserved for future use
	u8			m_rigType;				///< type of rig
	u16			m_numControls;			///< number of control nodes (<= rig params)
	u16			m_numCustomAttrs;		///< number of custom attributes
	u16			m_handleType[1];		///< array of types for each rig handle
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpFaceCustomAttr

	@brief			Helper struct for custom attr name / default value array in face definiton file
**/
//--------------------------------------------------------------------------------------------------

class GpFaceCustomAttr : public FwNonCopyable
{
public:
	FwHashedString	GetName( void ) const		{ return m_nameHash;	}
	float			GetDefault( void ) const	{ return m_default;		}

private:
	FwHashedString	m_nameHash;
	float   		m_default;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceDef constructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceDef::GpFaceDef()
{
	// do nothing
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceDef destructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceDef::~GpFaceDef()
{
	// do nothing
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the type of this rig

	@note			the original intention for this was to be able to store optimised
					versions of standard rig setups. A custom rig is the only type currently
					exported by the exportFacialRig.mel script.
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceDef::RigType GpFaceDef::GetRigType() const
{
	return (RigType)m_rigType;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the number of handles in this rig
**/
//--------------------------------------------------------------------------------------------------

inline u16 GpFaceDef::GetNumHandles() const
{
	return m_numControls;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the number of custom attributes in this rig
**/
//--------------------------------------------------------------------------------------------------

inline u16 GpFaceDef::GetNumCustomAttrs() const
{
	return m_numCustomAttrs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the type of a particular handle

	@param			i - index of a handle
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceDef::HandleType GpFaceDef::GetHandleType(u32 i) const
{
	FW_ASSERT(i<m_numControls);
	return (HandleType)m_handleType[i];
}

//--------------------------------------------------------------------------------------------------

#endif // GP_FACE_DEF_H
