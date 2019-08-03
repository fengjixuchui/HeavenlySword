//--------------------------------------------------------------------------------------------------
/**
	@file		GpFaceExprSet.inl

	@brief		contains the inline function definitions for the GpFaceExprSet class

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_FACE_EXPR_SET_INL_H
#define GP_FACE_EXPR_SET_INL_H

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceExprSet constructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceExprSet:: GpFaceExprSet( u32 numExprs,   		GpFaceExprFunc*  pExprFuncs,
  									  u32 numArgs,    		GpFaceExprArg*   pExprArgs,
									  u32 numJointAngles,  	GpFaceJointArg*  pJointAngles,
									  u32 numJointTrans,  	GpFaceJointArg*  pJointTrans,
									  u32 numShaders, 		GpFaceShaderArg* pShaderArgs,
									  u32 numOthers,  		FwHashedString*  pOtherArgs,
									  u32 numOrients, 		FwVector4*	     pJointOrients,
									  u32 numCustoms, 		FwHashedString*  pCustomArgs  )
{
	m_numExprs		= numExprs;
	m_pExprFuncs	= pExprFuncs;

	m_numArgs		= numArgs;
	m_pExprArgs		= pExprArgs;

	m_numJointAngles	= numJointAngles;
	m_pJointAngles		= pJointAngles;

	m_numJointTrans	= numJointTrans;
	m_pJointTrans	= pJointTrans;

	m_numShaderArgs	= numShaders;
	m_pShaderArgs	= pShaderArgs;

	m_numOtherArgs	= numOthers;
	m_pOtherArgs	= pOtherArgs;

	m_numOrients	= numOrients;
	m_pJointOrients	= pJointOrients;

	m_numCustomArgs	= numCustoms;
	m_pCustomArgs	= pCustomArgs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceExprSet destructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceExprSet::~GpFaceExprSet()
{
	// do nothing
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			call each of the expression functions in dependency order
**/
//--------------------------------------------------------------------------------------------------

inline bool GpFaceExprSet::Update(GpFaceInstance* pFace)
{
	for (u32 i=0; i<m_numExprs; i++)
		m_pExprFuncs[i](pFace);
	return true;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the total number of expressions in the set
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceExprSet::GetNumExprs() const
{
	return m_numExprs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the total number of arguments in the set
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceExprSet::GetNumArgs() const
{
	return m_numArgs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the number of arguments that are joint angles
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceExprSet::GetNumJointAngleArgs() const
{
	return m_numJointAngles;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the number of arguments that are joint translations
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceExprSet::GetNumJointTransArgs() const
{
	return m_numJointTrans;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the number of arguments that are shaders
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceExprSet::GetNumShaderArgs() const
{
	return m_numShaderArgs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the number of non joint or shader arguments
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceExprSet::GetNumOtherArgs() const
{
	return m_numOtherArgs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the number of custom attribute arguments
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceExprSet::GetNumCustomArgs() const
{
	return m_numCustomArgs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the number of joint orientations stored in the set
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceExprSet::GetNumJointOrients() const
{
	return m_numOrients;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get a joint orientation

	@param			i - index of joint orientation required
**/
//--------------------------------------------------------------------------------------------------

inline FwVector4 GpFaceExprSet::GetOrientation(u32 i) const
{
	FW_ASSERT(i<m_numOrients);
	return m_pJointOrients[i];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the index to the name of a specific argument

	@param			i - index of argument
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceExprSet::GetArgNameIndex(u32 i) const
{
	FW_ASSERT(i<m_numArgs);
	return m_pExprArgs[i].GetNameIndex();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the name hash of a specific argument

	@param			i - index of argument
**/
//--------------------------------------------------------------------------------------------------

inline FwHashedString GpFaceExprSet::GetArgName(u32 i) const
{
	FW_ASSERT(i<m_numArgs);
	switch ( GetArgType(i) )
	{
		case GpFaceExprArg::kJointAngle:
			return m_pJointAngles[ GetArgNameIndex(i) ].GetName();

		case GpFaceExprArg::kJointTrans:
			return m_pJointTrans[ GetArgNameIndex(i) ].GetName();

		case GpFaceExprArg::kShader:
			return m_pShaderArgs[ GetArgNameIndex(i) ].GetMeshName();

		case GpFaceExprArg::kRigHandle:
		case GpFaceExprArg::kBlendShape:
			return m_pOtherArgs[ GetArgNameIndex(i) ];

		case GpFaceExprArg::kCustomAttr:
			return m_pCustomArgs[ GetArgNameIndex( i ) ];

		default:
			return FwHashedString( u32(0) );
	}
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get argument object's type

	@param			i - index of argument
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceExprArg::ExprArgType GpFaceExprSet::GetArgType(u32 i) const
{
	FW_ASSERT(i<m_numArgs);
	return m_pExprArgs[i].GetType();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get the attribute index of the specified argument

	@param			i - index of argument

	@return			index of attribute
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceExprSet::GetArgAttrIndex(u32 i) const
{
	FW_ASSERT(i<m_numArgs);
	return m_pExprArgs[i].GetAttrIndex();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return whether a joint angle argument has an orientation specified

	@param			i - index of joint angle argument
**/
//--------------------------------------------------------------------------------------------------

inline bool GpFaceExprSet::JointAngleHasOrient(u32 i) const
{
	FW_ASSERT(i<m_numJointAngles);
	return (m_pJointAngles[i].GetJointOrientIndex() != -1);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return whether a joint angle argument has a rotate axis specified

	@param			i - index of joint angle argument
**/
//--------------------------------------------------------------------------------------------------

inline bool GpFaceExprSet::JointAngleHasRotAxis(u32 i) const
{
	FW_ASSERT(i<m_numJointAngles);
	return (m_pJointAngles[i].GetRotateAxisIndex() != -1);
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			get index of a joint angle argument's orientation entry

	@param			i - index of joint angle argument
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceExprSet::GetJointAngleOrientIndex(u32 i) const
{
	FW_ASSERT(i<m_numJointAngles);
	return m_pJointAngles[i].GetJointOrientIndex();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get index of a joint angle argument's rotate axis entry

	@param			i - index of joint angle argument
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceExprSet::GetJointAngleRotAxisIndex(u32 i) const
{
	FW_ASSERT(i<m_numJointAngles);
	return m_pJointAngles[i].GetRotateAxisIndex();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the name hash of the specified shader argument

	@param			i - index of shader argument
**/
//--------------------------------------------------------------------------------------------------

inline FwHashedString GpFaceExprSet::GetShaderArgName(u32 i) const
{
	FW_ASSERT(i<m_numShaderArgs);
	return m_pShaderArgs[i].GetParamName();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the size in floats of the shader argument's parameter

	@param			i - index of shader argument
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceExprSet::GetShaderArgSize(u32 i) const
{
	FW_ASSERT(i<m_numShaderArgs);
	return m_pShaderArgs[i].GetSize();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return a reference to the shader argument's descriptor

	@param			i - index of shader argument
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceShaderArg& GpFaceExprSet::GetShaderArgDesc(u32 i) const
{
	FW_ASSERT(i<m_numShaderArgs);
	return m_pShaderArgs[i];
}

//--------------------------------------------------------------------------------------------------

#endif // GP_FACE_EXPR_SET_INL_H

