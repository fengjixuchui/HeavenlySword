//--------------------------------------------------------------------------------------------------
/**
	@file		GpFaceJointArg.h

	@brief		Class definition for the joint descriptor used for facial animation

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_FACE_JOINT_ARG_H
#define GP_FACE_JOINT_ARG_H

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			GpFaceJointArg

	@brief			Descriptor of a joint used in a facial anim expression function

		This class encapsulates name and orientation data for a joint that is driven by the
		facial animation component. These would typically be eye, eyelid and jaw joints.
		The rotate axis and joint orientation indices reference an array of orientations stored
		as Euler angles in the GpFaceExprSet this joint is referenced by. Ideally, when building
		the facial animation model, all joints would not have these extra orientations
		and therefore these indices would be negative (indicating the lack of extra rotations 
		required).
**/
//--------------------------------------------------------------------------------------------------

class GpFaceJointArg 
{
public:
	// Construction & Destruction
	inline GpFaceJointArg(FwHashedString name, s32 rotateAxis, s32 jointOrient);
	inline ~GpFaceJointArg();

	// Access
	inline FwHashedString	GetName() const;
	inline s32				GetRotateAxisIndex() const;
	inline s32				GetJointOrientIndex() const;

private:
	// Attributes
	FwHashedString	m_name;					///< hash name of this joint
	s32				m_rotateAxisIndex;      ///< index to the rotate axis orientation if any
	s32				m_jointOrientIndex;		///< index to the joint orientation if any
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceJointArg constructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceJointArg::GpFaceJointArg(FwHashedString name, s32 rotateAxis, s32 jointOrient)
{
	m_name				= name;
	m_rotateAxisIndex	= rotateAxis;
	m_jointOrientIndex	= jointOrient;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceJointArg destructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceJointArg::~GpFaceJointArg()
{
	// do nothing
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the name has of this joint
**/
//--------------------------------------------------------------------------------------------------

inline FwHashedString GpFaceJointArg::GetName() const
{
	return m_name;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the index into the orientation table for this joint's rotate axis

	@note			-1 indicates no rotate axis is specified for this joint
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceJointArg::GetRotateAxisIndex() const
{
	return m_rotateAxisIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the index into the orientation table for this joint's orientation

	@note			-1 indicates no rotate orientation is specified for this joint
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceJointArg::GetJointOrientIndex() const
{
	return m_jointOrientIndex;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_FACE_JOINT_ARG_H
