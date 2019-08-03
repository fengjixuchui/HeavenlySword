//--------------------------------------------------------------------------------------------------
/**
	@file		GpFaceInstance.h

	@brief		contains class definition of a facial animation instance

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_FACE_INSTANCE_H
#define GP_FACE_INSTANCE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fw/FwStd/FwStdIntrusivePtr.h>
#include <Fw/FwStd/FwHashedString.h>
#include <Fw/FwMaths/FwVector4.h>
#include <Fw/FwMaths/FwQuat.h>
#include <Fw/FwResource.h>

#include <Gp/GpFacialAnim/GpFaceExprSet.h>
#include <Gp/GpFacialAnim/GpFaceExprArg.h>
#include <Gp/GpFacialAnim/GpFaceDef.h>

//--------------------------------------------------------------------------------------------------
//  FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GpFaceInstance;

//--------------------------------------------------------------------------------------------------
//  TYPE DEFINITIONS
//--------------------------------------------------------------------------------------------------

typedef FwStd::IntrusivePtr<GpFaceInstance> GpFaceInstanceHandle;

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			GpFaceInstance

	@brief			A facial animation instance object

		This object encapsulates all instance data for facial animation. This includes the current & 
		default values of the rig handles, pointers to the joint, shader and blendshape arguments.
		The intention is for this object to be closely associated with a character entity class.
**/
//--------------------------------------------------------------------------------------------------

class GpFaceInstance : public FwNonCopyable
{
public:
	// methods
	static GpFaceInstanceHandle		Create( const u8* pFaceFile,
											size_t fileSize,
											GpFaceExprSet* pExprSet,
											void* pMem = NULL );

	inline static GpFaceInstanceHandle	Create( const FwResourceHandle& hResource,
												GpFaceExprSet* pExprSet,
												void* pMem = NULL );

	static int						QuerySizeInBytes( const u8* pFaceFile,
													  size_t fileSize,
													  GpFaceExprSet* pExprSet );

	inline static int				QuerySizeInBytes( const FwResourceHandle& hResource, 
													  GpFaceExprSet* pExprSet );

	bool							Update();
	int								FindRigHandle( FwHashedString ctrlName ) const;
	float							FindCustomAttrDefault( FwHashedString attrName ) const;
	inline float					Clamp(float min, float max, float val) const;		

	// Access
	inline u16						GetNumRigHandles() const;
	inline const FwHashedString		GetRigHandleName( u32 handleIndex ) const;
	inline FwVector4&				GetHandle( u32 handleIndex ) const;
	inline void						SetHandle( u32 handleIndex, FwVector4_arg value) const;
	inline void						SetHandlesToDefaults() const;
	inline const FwVector4&			GetHandleDefault( u32 handleIndex ) const;
	inline const GpFaceExprSet*		GetExprSet() const;

	inline s32						GetJointAngleIndex( u32 jointIndex ) const;
	inline s32						GetJointTransIndex( u32 jointIndex ) const;

	inline FwVector4				GetEulerCacheEntry( u32 entryIndex ) const;
	inline FwVector4				GetTransCacheEntry( u32 entryIndex ) const;

	inline bool						JointAngleHasRotAxis( u32 jointIndex ) const;
	inline bool						JointAngleHasOrient( u32 jointIndex ) const;
	inline FwQuat					GetJointAngleRotAxis( u32 jointIndex ) const;
	inline FwQuat					GetJointAngleOrient( u32 jointIndex ) const;

	// used to bind to argument objects
	int								GetArgIndex( u32 argIndex ) const;
	void							SetArgIndex( u32 argIndex, int objIndex ) const;

	// get and set an expression argument's value
	float							GetArgValue(u32 argIndex ) const;
	void							SetArgValue(u32 argIndex, float value );

private:
	GpFaceInstance();
	~GpFaceInstance();

	FwResourceHandle			m_hResource;			///< resource this instance references

	u16							m_numRigHandles;		///< number of rig handles
	FwVector4*					m_pRigHandleValues;		///< array of current rig handle values
	const FwVector4*			m_pRigHandleDefaults;	///< array of default rig handle values
	const FwHashedString*		m_pRigHandleNames;		///< array of hashed rig handle names

	u16							m_numCustomAttrs;		///< number of custom attributes
	const GpFaceCustomAttr*		m_pCustomAttrDefaults;	///< custom attribute name to default value mapping

	GpFaceExprSet*				m_pRigMapping;			///< represents mapping from rig to model parameters
		
	int*						m_pRigArgObjs;				///< array of indices to argument objects
	int*						m_pRigJointAngleIndices;	///< array of argument joint angle indices
	int*						m_pRigJointTransIndices;	///< array of argument joint trans indices
	int*						m_pRigShaderObjs;			///< array of pointers to shader objects
			
	float*						m_pRigArgValues;		///< array of current argument values
	FwVector4*					m_pRigEulerCache;		///< array of joint orientations as Euler angles
	FwVector4*					m_pRigTransCache;		///< array of joint translations
	FwQuat*						m_pRigJointOrients;		///< array of orientations for rotateAxis & jointOrient attribs
		
	int							m_refCount;             ///< reference count for the intrusive pointer pattern
	bool						m_ownsMemory;			///< true iff memory was allocated by us

	friend void IntrusivePtrAddRef( GpFaceInstance* p )
	{
		++p->m_refCount;
	}

	friend void	IntrusivePtrRelease( GpFaceInstance* p )
	{
		if ( --p->m_refCount == 0 )
		{
			bool ownsMemory = p->m_ownsMemory;

			p->~GpFaceInstance();	// explicitly call the destructor

			if ( ownsMemory )
				FW_DELETE_ARRAY( reinterpret_cast< u8* >( p ) );
		}
	}

	friend u32 IntrusivePtrGetRefCount( GpFaceInstance* p )
	{
		return (u32)( p->m_refCount );
	}
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gp/GpFacialAnim/GpFaceInstance.inl>

//--------------------------------------------------------------------------------------------------

#endif // GP_FACE_INSTANCE_H

