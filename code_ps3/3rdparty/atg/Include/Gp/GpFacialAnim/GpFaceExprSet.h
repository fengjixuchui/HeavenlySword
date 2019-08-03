//--------------------------------------------------------------------------------------------------
/**
	@file		GpFaceExprSet.h

	@brief		contains class definition of the expression set wrapper class

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_FACE_EXPR_SET_H
#define GP_FACE_EXPR_SET_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gp/GpFacialAnim/GpFaceExprArg.h>
#include <Gp/GpFacialAnim/GpFaceJointArg.h>
#include <Gp/GpFacialAnim/GpFaceShaderArg.h>

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class FwHashedString;
class GpFaceInstance;

//--------------------------------------------------------------------------------------------------
//  TYPE DEFINITIONS
//--------------------------------------------------------------------------------------------------

typedef void (*GpFaceExprFunc)(GpFaceInstance*);	///< type definition for a single expression function

//--------------------------------------------------------------------------------------------------
/**
	@class			GpFaceExprSet

	@brief			Set of expression functions and their argument descriptors for facial animation

		This class simply gathers everything need to map from rig handles to blend weight,
		joint orientation and shader parameters together. This includes the array of expression
		functions ordered according to their dependencies. The array of all argument descriptors
		and individual descriptors for specific argument object types (used to bind to those
		objects on initialisation).
**/
//--------------------------------------------------------------------------------------------------

class GpFaceExprSet
{
public:
	// Construction & Destruction
	inline GpFaceExprSet( u32 numExprs,   		GpFaceExprFunc*	 pExprFuncs,
						  u32 numArgs,    		GpFaceExprArg*   pExprArgs,
						  u32 numJointAngles,  	GpFaceJointArg*  pJointAngles,
						  u32 numJointTrans,  	GpFaceJointArg*  pJointTrans,
						  u32 numShaders, 		GpFaceShaderArg* pShaderArgs,
						  u32 numOthers,  		FwHashedString*  pOtherArgs,
						  u32 numOrients, 		FwVector4*		 pJointOrients,
						  u32 numCustoms, 		FwHashedString*  pCustomArgs );
	inline ~GpFaceExprSet();

	// Operations
	inline bool							Update(GpFaceInstance* pFace);

	// Access
	inline u32							GetNumExprs() const;
	inline u32							GetNumArgs() const;
	inline u32							GetNumJointAngleArgs() const;
	inline u32							GetNumJointTransArgs() const;
	inline u32							GetNumShaderArgs() const;
	inline u32							GetNumOtherArgs() const;
	inline u32							GetNumCustomArgs() const;

	inline GpFaceExprArg::ExprArgType	GetArgType(u32 i) const;
	inline FwHashedString				GetArgName(u32 i) const;
	inline s32							GetArgNameIndex(u32 i) const;
	inline s32							GetArgAttrIndex(u32 i) const;

	inline u32							GetNumJointOrients() const;
	inline FwVector4					GetOrientation(u32 i) const;

	inline FwHashedString				GetShaderArgName(u32 i) const;
	inline u32							GetShaderArgSize(u32 i) const;
	inline GpFaceShaderArg&				GetShaderArgDesc(u32 i) const;

	inline bool							JointAngleHasRotAxis(u32 i) const;
	inline bool							JointAngleHasOrient(u32 i) const;

	inline s32							GetJointAngleRotAxisIndex(u32 i) const;
	inline s32							GetJointAngleOrientIndex(u32 i) const;

private:
	u32					m_numExprs;				///< number of expressions in the set
	GpFaceExprFunc*		m_pExprFuncs;			///< pointer to an array of expression functions
	
	u32					m_numArgs;				///< total number of arguments
	GpFaceExprArg*		m_pExprArgs;			///< pointer to an array of argument descriptors

	u32					m_numJointAngles;		///< number of arguments that are joint angles
	GpFaceJointArg*		m_pJointAngles;			///< pointer to an array of joint argument descriptors

	u32					m_numJointTrans;		///< number of arguments that are joint translations
	GpFaceJointArg*		m_pJointTrans;			///< pointer to an array of joint argument descriptors

	u32					m_numShaderArgs;		///< number of arguments that are shader params
	GpFaceShaderArg*	m_pShaderArgs;			///< pointer to an array of shader argument descriptors

	u32					m_numOtherArgs;			///< number of arguments that are not joints
	FwHashedString*		m_pOtherArgs;			///< pointer to an array of argument names

	u32					m_numOrients;			///< total number of joint orientations
	FwVector4*			m_pJointOrients;		///< pointer to an array of joint orientations

	u32					m_numCustomArgs;		///< total number of custom attributes
	FwHashedString*		m_pCustomArgs;			///< pointer to an array of custom attribute names
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gp/GpFacialAnim/GpFaceExprSet.inl>

//--------------------------------------------------------------------------------------------------

#endif // GP_FACE_EXPR_SET_H

