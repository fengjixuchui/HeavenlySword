//--------------------------------------------------------------------------------------------------
/**
	@file		GpFaceExprArg.h

	@brief		Class definition for an argument descriptor used for facial animation

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_FACE_EXPR_ARG_H
#define GP_FACE_EXPR_ARG_H

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			GpFaceExprArg

	@brief			Descriptor of an argument to a facial anim expression function

		This class is used to describe the arguments to the functions found in the 
		GpFaceExprSet. An argument is a single float attribute of a rig handle, joint orientation,
		blend weight or shader parameter, some examples are:-

		-	joint.rotateX
		-	shader.bumpDepth
		-	handle.translateY

		The descriptor is used to bind to the correct instance of the argument object and also
		provides a way to specify the arguments used in the expression set which maps from
		rig handle positions to blendweights, joint orientations and shader parameters.
**/
//--------------------------------------------------------------------------------------------------

class GpFaceExprArg
{
public:
	enum ExprArgType			///< enumeration of the type of argument
	{
		kUnknown = -1,
		kRigHandle,
		kJointAngle,
		kJointTrans,
		kBlendShape,
		kShader,
		kCustomAttr
	};

	// Construction & Destruction
	inline GpFaceExprArg(s32 nameIndex, ExprArgType argType, s32 attrIndex);
	inline ~GpFaceExprArg();

	// Access
	inline s32			GetNameIndex() const;
	inline ExprArgType	GetType() const;
	inline s32			GetAttrIndex() const;

private:
	s32				m_nameIndex;			///< index into name array
	ExprArgType		m_type;					///< type of argument object
	s32				m_attrIndex;			///< index to access attribute value of arg
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceExprArg constructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceExprArg::GpFaceExprArg(s32 nameIndex, ExprArgType argType, s32 attrIndex)
{
	m_nameIndex	= nameIndex;
	m_type		= argType;
	m_attrIndex	= attrIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceExprArg destructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceExprArg::~GpFaceExprArg()
{
	// do nothing
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the index in the name array
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceExprArg::GetNameIndex() const
{
	return m_nameIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the type of this argument
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceExprArg::ExprArgType GpFaceExprArg::GetType() const
{
	return m_type;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the argument object's attribute index
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceExprArg::GetAttrIndex() const
{
	return m_attrIndex;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_FACE_EXPR_ARG_H
